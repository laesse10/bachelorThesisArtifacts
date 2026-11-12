/* AMG smoothed-aggregation setup -- optimized C port of the NumPy reference.
 *
 * Exactness rules honoured:
 *  - every per-row accumulation (mark/scatter) keeps the reference's inner order,
 *  - reductions that feed values (the spectral-radius norm) stay sequential in row order,
 *  - the greedy aggregation is sequential, in identical node order,
 *  - the transpose is a stable counting transpose (ascending row order within a column),
 *    exactly what the reference's sequential scatter produces.
 * Internal index arrays are int32 (values < 2^31); row offsets and all arithmetic stay int64.
 * Prefix sums are integer, so they are parallelised (any partition gives the same result).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LMAX 16
#define MAX_COARSE 100
#define RHO_ITERS 15
#define NT_CAP (64)

/* ---------------- per-thread scatter scratch ---------------- */

typedef struct {
    int32_t  *mark;   /* column -> tag of the row that last touched it (this thread) */
    double   *acc;    /* column accumulator */
    int32_t  *touched; /* row columns, first-touch order; scratch elsewhere: counts */
    int64_t   tag;    /* running tag; grows, never reused */
} ThrScr;

static ThrScr *g_thr = NULL;
static int      g_nthr = 0;
static int64_t g_thr_dim = 0;
static int64_t g_tbase[NT_CAP + 1]; /* per-thread block sums for the parallel prefix */
static int64_t g_part[NT_CAP + 2];
static int32_t g_dummy_slot[NT_CAP];  /* out-of-range garbage sink, one per thread */
static double g_npart[NT_CAP];      /* per-thread partial norm sums */

static uint8_t *g_buf = NULL;
static int64_t   g_buf_cap = 0;

static uint8_t *get_buf(int64_t need)
{
    need = (need + 1023) & ~(int64_t)1023;
    if (g_buf_cap >= need) return g_buf;
    free(g_buf);
    g_buf = NULL;
    g_buf_cap = 0;
    g_buf = (uint8_t *)malloc((size_t)need);
    if (g_buf != NULL) g_buf_cap = need;
    return g_buf;
}

static ThrScr *get_thr(int64_t dim)
{
    int nt = omp_get_max_threads();
    if (nt < 1) nt = 1;
    if (nt > NT_CAP) nt = NT_CAP;
    if (g_thr != NULL && g_nthr >= nt && g_thr_dim >= dim) return g_thr;
    if (g_thr != NULL) {
        for (int t = 0; t < g_nthr; t++) {
            free(g_thr[t].mark);
            free(g_thr[t].acc);
            free(g_thr[t].touched);
        }
        free(g_thr);
        g_thr = NULL;
    }
    g_thr = (ThrScr *)calloc((size_t)nt, sizeof(ThrScr));
    if (g_thr == NULL) { g_nthr = 0; g_thr_dim = 0; return NULL; }
    g_nthr = nt;
    g_thr_dim = dim;
    for (int t = 0; t < nt; t++) {
        g_thr[t].mark    = (int32_t *)malloc((size_t)dim * sizeof(int32_t));
        g_thr[t].acc     = (double *)malloc((size_t)dim * sizeof(double));
        g_thr[t].touched = (int32_t *)malloc((size_t)dim * sizeof(int32_t));
        if (g_thr[t].mark != NULL)
            memset(g_thr[t].mark, 0xFF, (size_t)dim * sizeof(int32_t));
        g_thr[t].tag = 0;
    }
    return g_thr;
}

/* C = A * B, all CSR, exact mark/scatter accumulation in reference (ka,kb) order.
 * When ndiag != NULL it the emit also extracts the next level's diagonal |c_ii|. */
static void csr_mul(
    const int64_t *restrict a_ip, const int32_t *restrict a_ic, const double *restrict a_d,
    const int64_t *restrict b_ip, const int32_t *restrict b_ic, const double *restrict b_d,
    const int64_t nrow,
    int64_t *restrict c_ip, int32_t *restrict c_ic, double *restrict c_d,
    ThrScr *restrict thr, int64_t *restrict fill,
    double *restrict ndiag,
    int32_t *restrict tscr, double *restrict vscr,
    const int64_t tcap, const int64_t nbcol)
{
    #pragma omp parallel
    {
        const int t = omp_get_thread_num();
        const int nth = omp_get_num_threads();
        ThrScr *S = &thr[t];
        int32_t *mark = S->mark;
        double *acc = S->acc;
        int32_t *tsmall = S->touched;
        if (S->tag > (int64_t)INT32_MAX / 2) {
            memset(mark, 0xFF, (size_t)g_thr_dim * sizeof(int32_t));
            S->tag = 0;
        }
        /* balanced row partition by exact pair counts */
        const int64_t s0 = nrow * t / nth, s1 = nrow * (t + 1) / nth;
        for (int64_t i = s0; i < s1; i++) {
            int64_t pc = 0;
            const int64_t ea = a_ip[i + 1];
            for (int64_t ka = a_ip[i]; ka < ea; ka++) {
                const int64_t j = a_ic[ka];
                pc += (b_ip[j + 1] - b_ip[j]);
            }
            fill[i] = pc;
        }
        {
            int64_t loc = 0;
            for (int64_t i = s0; i < s1; i++) loc += fill[i];
            g_tbase[t] = loc;
        }
        #pragma omp barrier
        #pragma omp single
        {
            int64_t pos = 0;
            for (int q = 0; q < nth; q++) {
                const int64_t v = g_tbase[q];
                g_tbase[q] = pos;
                pos += v;
            }
            g_part[NT_CAP] = pos; /* total pairs */
        }
        {
            int64_t pos = g_tbase[t];
            for (int64_t i = s0; i < s1; i++) {
                const int64_t v = fill[i];
                fill[i] = pos;
                pos += v;
            }
        }
        #pragma omp barrier
        #pragma omp single
        {
            const int64_t total = g_part[NT_CAP];
            g_part[0] = 0;
            for (int q = 1; q < nth; q++) {
                const int64_t target = total * q / nth;
                int64_t lo = 0, hi = nrow;
                while (lo < hi) {
                    const int64_t mid = (lo + hi) >> 1;
                    if (fill[mid] <= target) lo = mid + 1; else hi = mid;
                }
                g_part[q] = lo;
            }
            g_part[nth] = nrow;
        }
        const int64_t r0 = g_part[t], r1 = g_part[t + 1];
        const int64_t cap = tcap / nth;
        int32_t *tbig = tscr + (int64_t)t * cap;
        double *tval = vscr + (int64_t)t * cap;
        int64_t u = 0;
        int64_t ovrow = r1;
        /* single pass: distinct-column count + accumulate + record order */
        for (int64_t i = r0; i < r1; i++) {
            const int64_t tag = ++S->tag;
            int64_t c = 0;
            if (ndiag != NULL) ndiag[i] = 0.0;
            const int rec = (u + nbcol <= cap);
            if (!rec && ovrow == r1) ovrow = i;
            const int64_t ea = a_ip[i + 1];
            for (int64_t ka = a_ip[i]; ka < ea; ka++) {
                const int64_t j = a_ic[ka];
                const double av = a_d[ka];
                const int64_t eb = b_ip[j + 1];
                for (int64_t kb = b_ip[j]; kb < eb; kb++) {
                    const int32_t col = b_ic[kb];
                    if (mark[col] != (int32_t)tag) {
                        mark[col] = (int32_t)tag;
                        acc[col] = 0.0;
                        if (rec) tbig[u] = col;
                        u++;
                        c++;
                    }
                    acc[col] = acc[col] + av * b_d[kb];
                }
            }
            /* finalize this row's values before acc is reused */
            if (rec) {
                const int64_t u0 = u - c;
                for (int64_t w = 0; w < c; w++) {
                    const int32_t col = tbig[u0 + w];
                    const double v = acc[col];
                    tval[u0 + w] = v;
                    if (ndiag != NULL && col == (int32_t)i) ndiag[i] = fabs(v);
                }
            }
            fill[i] = c;
        }
        /* integer prefix, parallel */
        {
            int64_t loc = 0;
            for (int64_t i = r0; i < r1; i++) loc += fill[i];
            g_tbase[t] = loc;
        }
        #pragma omp barrier
        #pragma omp single
        {
            int64_t pos = 0;
            for (int q = 0; q < nth; q++) {
                const int64_t v = g_tbase[q];
                g_tbase[q] = pos;
                pos += v;
            }
            c_ip[nrow] = pos;
        }
        const int64_t gbase = g_tbase[t];
        {
            int64_t pos = gbase;
            for (int64_t i = r0; i < r1; i++) { c_ip[i] = pos; pos += fill[i]; }
        }
        /* emit: linear copy from the recorded order (re-derive if overflowed) */
        const int64_t ov = ovrow;
        int64_t loc_ov = 0;
        for (int64_t i = r0; i < ov; i++) loc_ov += fill[i];
        if (loc_ov > 0) {
            memcpy(c_ic + gbase, tbig, (size_t)loc_ov * sizeof(int32_t));
            memcpy(c_d + gbase, tval, (size_t)loc_ov * sizeof(double));
        }
        for (int64_t i = ov; i < r1; i++) {
            /* overflow fallback: full per-row recompute, direct emit */
            const int64_t tag = ++S->tag;
            int64_t nt = 0;
            if (ndiag != NULL) ndiag[i] = 0.0;
            const int64_t ea = a_ip[i + 1];
            for (int64_t ka = a_ip[i]; ka < ea; ka++) {
                const int64_t j = a_ic[ka];
                const double av = a_d[ka];
                const int64_t eb = b_ip[j + 1];
                for (int64_t kb = b_ip[j]; kb < eb; kb++) {
                    const int32_t col = b_ic[kb];
                    if (mark[col] != (int32_t)tag) {
                        mark[col] = (int32_t)tag;
                        acc[col] = 0.0;
                        tsmall[nt++] = col;
                    }
                    acc[col] = acc[col] + av * b_d[kb];
                }
            }
            const int64_t pos = c_ip[i];
            for (int64_t w = 0; w < nt; w++) {
                const int32_t col = tsmall[w];
                c_ic[pos + w] = col;
                const double v = acc[col];
                c_d[pos + w] = v;
                if (ndiag != NULL && col == (int32_t)i) ndiag[i] = fabs(v);
            }
        }
    }
}

/* ---- greedy aggregation, exactly the reference's node order (sequential) ---- */
typedef struct {
    const int64_t *s_indptr;
    const int32_t *s_indices;
    int64_t n;
    int32_t *agg;
    int64_t na;
} AggArg;

static void *aggregate_fn(void *p)
{
    AggArg *a = (AggArg *)p;
    const int64_t *s_indptr = a->s_indptr;
    const int32_t *s_indices = a->s_indices;
    const int64_t n = a->n;
    int32_t *agg = a->agg;
    memset(agg, 0xFF, (size_t)n * sizeof(int32_t));
    int64_t na = 0;
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] < 0) {
            const int64_t s0 = s_indptr[i], s1 = s_indptr[i + 1];
            int freef = 1;
            for (int64_t k = s0; k < s1; k++) {
                if (agg[s_indices[k]] >= 0) { freef = 0; break; }
            }
            if (freef) {
                agg[i] = (int32_t)na;
                for (int64_t k = s0; k < s1; k++)
                    agg[s_indices[k]] = (int32_t)na;
                na++;
            }
        }
    }
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int64_t picked = -1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++) {
                const int64_t c = agg[s_indices[k]];
                if (c >= 0) { picked = c; break; }
            }
            if (picked < 0) { agg[i] = (int32_t)na; na++; }
            else agg[i] = (int32_t)picked;
        }
    }
    a->na = na;
    return NULL;
}

void amg_setup_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    int64_t *restrict agg0,
    int64_t *restrict level_n,
    int64_t *restrict level_nnz,
    int64_t *restrict nlevels,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const double theta,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    const int64_t n0 = NX * NY * NZ;
    const int64_t nnz0 = (3 * NX - 2) * (3 * NY - 2) * (3 * NZ - 2);
    const int64_t zpad = 3 * (nnz0 + n0) / 2;

    memset(level_n, 0, sizeof(int64_t) * (size_t)LMAX);
    memset(level_nnz, 0, sizeof(int64_t) * (size_t)LMAX);

    /* ---- scratch arena ---- */
    int nt_req = omp_get_max_threads();
    if (nt_req < 1) nt_req = 1;
    if (nt_req > NT_CAP) nt_req = NT_CAP;
    const int64_t need =
        48 * zpad + 48 * (n0 + 1) + 44 * n0 + 16 * (int64_t)nt_req * n0 + 65536;
    uint8_t *base = NULL;
    if (workspace != NULL && workspace_size >= need) {
        base = workspace;
    } else {
        base = get_buf(need);
        if (base == NULL) { nlevels[0] = 0; return; }
    }
    ThrScr *thr = get_thr(n0);
    if (thr == NULL) { nlevels[0] = 0; return; }

    int64_t off = 0;
#define TAKE8(cnt)  (base + (off = (off + ((int64_t)(cnt)) * 8 + 63) & ~INT64_C(63)) - ((int64_t)(cnt)) * 8)
#define TAKE4(cnt)  (base + (off = (off + ((int64_t)(cnt)) * 4 + 63) & ~INT64_C(63)) - ((int64_t)(cnt)) * 4)
    int64_t *cur_indptr = (int64_t *)TAKE8(n0 + 1);
    int64_t *s_indptr   = (int64_t *)TAKE8(n0 + 1);
    int64_t *p_indptr   = (int64_t *)TAKE8(n0 + 1);
    int64_t *pt_indptr  = (int64_t *)TAKE8(n0 + 1);
    int64_t *ap_indptr  = (int64_t *)TAKE8(n0 + 1);
    int64_t *fill   = (int64_t *)TAKE8(n0 + 1);
    int32_t *agg    = (int32_t *)TAKE4(n0);
    int32_t *cur_indices = (int32_t *)TAKE4(zpad);
    double   *cur_data   = (double   *)TAKE8(zpad);
    int32_t *s_indices   = (int32_t *)TAKE4(zpad); /* reused as p_indices */
    double   *p_data     = (double   *)TAKE8(zpad);
    int32_t *pt_indices  = (int32_t *)TAKE4(zpad);
    double   *pt_data    = (double   *)TAKE8(zpad);
    int32_t *ap_indices  = (int32_t *)TAKE4(zpad);
    double   *ap_data    = (double   *)TAKE8(zpad);
    double   *diag  = (double  *)TAKE8(n0);
    double   *cnt_  = (double  *)TAKE8(n0);
    double   *tscale = (double *)TAKE8(n0);
    double   *vv    = (double   *)TAKE8(n0);
    double   *ww    = (double   *)TAKE8(n0);
#undef TAKE8
#undef TAKE4

    int32_t *p_indices = s_indices; /* strength graph is dead once aggregation has run */

    /* level-0 operator: indices narrowed to int32 (in place conversion), data read in place */
    const int64_t *cip = A_indptr;
    int32_t *cic = cur_indices;
    const double   *cd = A_data;

    int64_t n = n0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int64_t lv = 0; lv < LMAX - 1; lv++) {
        if (n <= MAX_COARSE) break;

        /* ---- level 0: convert indices to int32 and pick up the row diagonal;
         *      later levels: diag comes from the previous level's triple product ---- */
        if (lv == 0) {
            #pragma omp parallel
            {
                const int t = omp_get_thread_num();
                const int nth = omp_get_num_threads();
                const int64_t r0 = n * t / nth, r1 = n * (t + 1) / nth;
                for (int64_t i = r0; i < r1; i++) {
                    double d = 0.0;
                    const int64_t e = cip[i + 1];
                    for (int64_t k = cip[i]; k < e; k++) {
                        const int64_t j = A_indices[k];
                        cic[k] = (int32_t)j;
                        if (j == i) d = fabs(A_data[k]);
                    }
                    diag[i] = d;
                }
            }
        }

        /* ---- strength graph: single pass count+record, prefix, block copy ---- */
        #pragma omp parallel
        {
            const int t = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            const int64_t r0 = n * t / nth, r1 = n * (t + 1) / nth;
            const int64_t cap = zpad / nth;
            int32_t *tbig = pt_indices + (int64_t)t * cap;
            int64_t u = 0;
            int64_t ovrow = r1;
            for (int64_t i = r0; i < r1; i++) {
                const double di = diag[i];
                const int32_t i32 = (int32_t)i;
                int64_t c = 0;
                const int64_t ks = cip[i];
                const int64_t e = cip[i + 1];
                const int rec = (u + (e - ks) <= cap);
                if (!rec && ovrow == r1) ovrow = i;
                const int64_t uend = u + (e - ks);
                for (int64_t k = ks; k < e; k++) {
                    const int32_t j = cic[k];
                    const int64_t m = (j != i32) & (fabs(cd[k]) > theta * sqrt(di * diag[j]));
                    int32_t *const dst = (u < uend) ? &tbig[u] : &g_dummy_slot[t];
                    *dst = j;
                    u += m;
                    c += m;
                }
                fill[i] = c;
            }
            {
                int64_t loc = 0;
                for (int64_t i = r0; i < r1; i++) loc += fill[i];
                g_tbase[t] = loc;
            }
            #pragma omp barrier
            #pragma omp single
            {
                int64_t pos = 0;
                for (int q = 0; q < nth; q++) {
                    const int64_t v = g_tbase[q];
                    g_tbase[q] = pos;
                    pos += v;
                }
                s_indptr[n] = pos;
            }
            const int64_t gbase = g_tbase[t];
            {
                int64_t pos = gbase;
                for (int64_t i = r0; i < r1; i++) { s_indptr[i] = pos; pos += fill[i]; }
            }
            if (ovrow == r1) {
                memcpy(s_indices + gbase, tbig, (size_t)u * sizeof(int32_t));
            } else {
                const int64_t ov = ovrow;
                int64_t uloc = 0;
                for (int64_t i = r0; i < ov; i++) uloc += fill[i];
                memcpy(s_indices + gbase, tbig, (size_t)uloc * sizeof(int32_t));
                for (int64_t i = ov; i < r1; i++) {
                    int64_t pos = s_indptr[i];
                    const int64_t eend = cip[i + 1];
                    const double di = diag[i];
                    const int32_t i32 = (int32_t)i;
                    for (int64_t k = cip[i]; k < eend; k++) {
                        const int32_t j = cic[k];
                        const int64_t m = (j != i32) & (fabs(cd[k]) > theta * sqrt(di * diag[j]));
                        if (m) s_indices[pos++] = j;
                    }
                }
            }
        }

        /* ---- greedy aggregation (sequential, exact node order) ---- */
        AggArg ag;
        ag.s_indptr = s_indptr;
        ag.s_indices = s_indices;
        ag.n = n;
        ag.agg = agg;
        ag.na = 0;
        aggregate_fn(&ag);
        const int64_t na = ag.na;

        if (na >= n) { n = 0; break; }

        if (depth == 1) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < n0; i++) agg0[i] = agg[i];
        }

        /* ---- spectral radius (power iterations, sequential norm reduction) ---- */
        double rho = 0.0;
        {
            #pragma omp parallel
            {
                const int t = omp_get_thread_num();
                const int nth = omp_get_num_threads();
                const int64_t q0 = n * (int64_t)t / nth, q1 = n * (int64_t)(t + 1) / nth;
                for (int64_t i = q0; i < q1; i++) vv[i] = 1.0;
                #pragma omp barrier
                for (int it = 0; it < RHO_ITERS; it++) {
                    for (int64_t i = q0; i < q1; i++) {
                        double a = 0.0;
                        const int64_t e = cip[i + 1];
                        if (it == 0) {
                            /* vv == 1.0 exactly: cd[k]*1.0 == cd[k] bitwise */
                            for (int64_t k = cip[i]; k < e; k++) a += cd[k];
                        } else {
                            for (int64_t k = cip[i]; k < e; k++) a += cd[k] * vv[cic[k]];
                        }
                        ww[i] = a / diag[i];
                    }
                    #pragma omp barrier
                    {
                        double part = 0.0;
                        for (int64_t i = q0; i < q1; i++) part = part + ww[i] * ww[i];
                        g_npart[t] = part;
                    }
                    #pragma omp barrier
                    #pragma omp single
                    {
                        double nrm = 0.0;
                        for (int q = 0; q < nth; q++) nrm = nrm + g_npart[q];
                        nrm = sqrt(nrm);
                        rho = nrm;
                    }
                    if (it + 1 < RHO_ITERS) {
                        for (int64_t i = q0; i < q1; i++) vv[i] = ww[i] / rho;
                    }
                    #pragma omp barrier
                }
            }
        }

        /* ---- tentative scaling (integer counts, order-free) ---- */
        #pragma omp parallel
        {
            const int t = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            int32_t *tc = thr[t].touched;
            memset(tc, 0, (size_t)na * sizeof(int32_t));
            #pragma omp barrier
            const int64_t r0 = n * t / nth, r1 = n * (t + 1) / nth;
            for (int64_t i = r0; i < r1; i++) tc[agg[i]]++;
            #pragma omp barrier
            #pragma omp for schedule(static)
            for (int64_t c = 0; c < na; c++) {
                double s = 0.0;
                for (int q = 0; q < nth; q++) s += (double)thr[q].touched[c];
                cnt_[c] = s;
            }
            #pragma omp for schedule(static)
            for (int64_t c = 0; c < na; c++)
                vv[c] = 1.0 / sqrt(cnt_[c]);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++)
                tscale[i] = vv[agg[i]];
        }
        const double omega = 4.0 / (3.0 * rho);

        /* ---- smoothed prolongation: P = (I - omega D^-1 A) T ---- */
        #pragma omp parallel
        {
            const int t = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            ThrScr *S = &thr[t];
            int32_t *mark = S->mark;
            double *acc = S->acc;
            int32_t *touched = S->touched;
            const int64_t r0 = n * t / nth, r1 = n * (t + 1) / nth;
            const int64_t cap = zpad / nth;
            int32_t *tbig = pt_indices + (int64_t)t * cap;
            double *tval = pt_data + (int64_t)t * cap;
            int64_t u = 0;
            int64_t ovrow = r1;
            /* single pass: count + accumulate + record + finalize per row */
            for (int64_t i = r0; i < r1; i++) {
                const int64_t tag = ++S->tag;
                int64_t c = 0;
                const int rec = (u + na <= cap);
                if (!rec && ovrow == r1) ovrow = i;
                const int32_t c0 = agg[i];
                if (mark[c0] != (int32_t)tag) {
                    mark[c0] = (int32_t)tag;
                    acc[c0] = 0.0;
                    if (rec) tbig[u] = c0;
                    u++;
                    c++;
                }
                acc[c0] = acc[c0] + tscale[i];
                const int64_t e = cip[i + 1];
                for (int64_t k = cip[i]; k < e; k++) {
                    const int32_t j = cic[k];
                    const int32_t cj = agg[j];
                    if (mark[cj] != (int32_t)tag) {
                        mark[cj] = (int32_t)tag;
                        acc[cj] = 0.0;
                        if (rec) tbig[u] = cj;
                        u++;
                        c++;
                    }
                    acc[cj] = acc[cj] - omega * (cd[k] / diag[i]) * tscale[j];
                }
                if (rec) {
                    const int64_t u0 = u - c;
                    for (int64_t w = 0; w < c; w++) tval[u0 + w] = acc[tbig[u0 + w]];
                }
                fill[i] = c;
            }
            {
                int64_t loc = 0;
                for (int64_t i = r0; i < r1; i++) loc += fill[i];
                g_tbase[t] = loc;
            }
            #pragma omp barrier
            #pragma omp single
            {
                int64_t pos = 0;
                for (int q = 0; q < nth; q++) {
                    const int64_t v = g_tbase[q];
                    g_tbase[q] = pos;
                    pos += v;
                }
                p_indptr[n] = pos;
            }
            const int64_t gbase = g_tbase[t];
            {
                int64_t pos = gbase;
                for (int64_t i = r0; i < r1; i++) { p_indptr[i] = pos; pos += fill[i]; }
            }
            /* emit recorded rows */
            const int64_t ov = ovrow;
            int64_t loc_ov = 0;
            for (int64_t i = r0; i < ov; i++) loc_ov += fill[i];
            if (loc_ov > 0) {
                memcpy(p_indices + gbase, tbig, (size_t)loc_ov * sizeof(int32_t));
                memcpy(p_data + gbase, tval, (size_t)loc_ov * sizeof(double));
            }
            /* overflow fallback: full per-row recompute, direct emit */
            for (int64_t i = ov; i < r1; i++) {
                const int64_t tag = ++S->tag;
                int64_t nt = 0;
                const int32_t c0 = agg[i];
                mark[c0] = (int32_t)tag;
                acc[c0] = 0.0;
                touched[nt++] = c0;
                acc[c0] = acc[c0] + tscale[i];
                const int64_t e = cip[i + 1];
                for (int64_t k = cip[i]; k < e; k++) {
                    const int32_t j = cic[k];
                    const int32_t cj = agg[j];
                    if (mark[cj] != (int32_t)tag) {
                        mark[cj] = (int32_t)tag;
                        acc[cj] = 0.0;
                        touched[nt++] = cj;
                    }
                    acc[cj] = acc[cj] - omega * (cd[k] / diag[i]) * tscale[j];
                }
                const int64_t pos = p_indptr[i];
                for (int64_t w = 0; w < nt; w++) {
                    const int32_t c = touched[w];
                    p_indices[pos + w] = c;
                    p_data[pos + w] = acc[c];
                }
            }
        }

        /* ---- AP = A * P ---- */
        csr_mul(cip, cic, cd, p_indptr, p_indices, p_data, n,
                ap_indptr, ap_indices, ap_data, thr, fill, NULL,
                pt_indices, pt_data, zpad, na);
        /* ---- Pt = P^T : stable counting transpose, ascending i within each column ---- */
        {
            const int64_t nP = p_indptr[n];
            #pragma omp parallel
            {
                const int t = omp_get_thread_num();
                const int nth = omp_get_num_threads();
                int32_t *bc = thr[t].touched;
                const int64_t r0 = n * t / nth;
                const int64_t r1 = n * (t + 1) / nth;
                memset(bc, 0, (size_t)na * sizeof(int32_t));
                for (int64_t i = r0; i < r1; i++) {
                    const int64_t e = p_indptr[i + 1];
                    for (int64_t k = p_indptr[i]; k < e; k++)
                        bc[p_indices[k]]++;
                }
                #pragma omp barrier
                #pragma omp for schedule(static)
                for (int64_t c0 = 0; c0 < na; c0 += 4096) {
                    const int64_t c1 = (c0 + 4096 < na) ? c0 + 4096 : na;
                    for (int64_t c = c0; c < c1; c++) fill[c] = 0;
                    for (int t2 = 0; t2 < nth; t2++) {
                        const int32_t *b2 = thr[t2].touched;
                        for (int64_t c = c0; c < c1; c++) fill[c] += b2[c];
                    }
                }
                #pragma omp barrier
                #pragma omp single
                {
                    int64_t pos = 0;
                    for (int64_t c = 0; c < na; c++) {
                        const int64_t tc = fill[c];
                        fill[c] = pos;
                        pos += tc;
                    }
                    pt_indptr[na] = nP;
                }
                #pragma omp for schedule(static)
                for (int64_t c0 = 0; c0 < na; c0 += 4096) {
                    const int64_t c1 = (c0 + 4096 < na) ? c0 + 4096 : na;
                    int64_t offv[4096];
                    for (int64_t c = c0; c < c1; c++) offv[c - c0] = fill[c];
                    for (int t2 = 0; t2 < nth; t2++) {
                        int32_t *b2 = thr[t2].touched;
                        for (int64_t c = c0; c < c1; c++) {
                            const int32_t v = b2[c];
                            b2[c] = (int32_t)offv[c - c0];
                            offv[c - c0] += v;
                        }
                    }
                }
                #pragma omp for schedule(static)
                for (int64_t c = 0; c < na; c++) pt_indptr[c] = fill[c];
                for (int64_t i = r0; i < r1; i++) {
                    const int64_t e = p_indptr[i + 1];
                    for (int64_t k = p_indptr[i]; k < e; k++) {
                        const int64_t c = p_indices[k];
                        const int64_t pos = bc[c]++;
                        pt_indices[pos] = (int32_t)i;
                        pt_data[pos] = p_data[k];
                    }
                }
            }
        }



        /* ---- nxt = Pt * AP  (into the cur buffers; cur is dead here) ---- */
        csr_mul(pt_indptr, pt_indices, pt_data, ap_indptr, ap_indices, ap_data, na,
                cur_indptr, cur_indices, cur_data, thr, fill, diag,
                s_indices, p_data, zpad, na);

        level_n[depth] = na;
        level_nnz[depth] = cur_indptr[na];
        depth++;

        cip = cur_indptr;
        cic = cur_indices;
        cd = cur_data;
        n = na;
    }

    nlevels[0] = depth;
}
