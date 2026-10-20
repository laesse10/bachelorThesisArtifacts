/* AMG smoothed-aggregation setup -- optimized C port of the NumPy reference.
 *
 * Exactness rules honoured:
 *  - every per-row accumulation (mark/scatter) keeps the reference's inner order,
 *  - reductions that feed values (the spectral-radius norm) stay sequential in row order,
 *  - the greedy aggregation is sequential, in identical node order,
 *  - the transpose is a stable counting transpose (ascending row order within a column),
 *    exactly what the reference's sequential scatter produces.
 * Everything else is parallelised over rows.
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
    int32_t *mark;     /* column -> tag of the row that last touched it (this thread) */
    double  *acc;      /* column accumulator */
    int32_t *touched;  /* row columns, first-touch order; doubles as transpose block counts */
    int64_t  tag;      /* running tag; grows forever, never reused */
} ThrScr;

static ThrScr *g_thr = NULL;
static int      g_nthr = 0;
static int64_t g_thr_dim = 0;

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
        g_thr[t].acc    = (double *)malloc((size_t)dim * sizeof(double));
        g_thr[t].touched = (int32_t *)malloc((size_t)dim * sizeof(int32_t));
        if (g_thr[t].mark != NULL)
            memset(g_thr[t].mark, 0xFF, (size_t)dim * sizeof(int32_t));
        g_thr[t].tag = 0;
    }
    return g_thr;
}

/* C = A * B, all CSR, exact mark/scatter accumulation in reference (ka,kb) order. */
static void csr_mul(
    const int64_t *restrict a_ip, const int64_t *restrict a_ic, const double *restrict a_d,
    const int64_t *restrict b_ip, const int64_t *restrict b_ic, const double *restrict b_d,
    const int64_t nrow,
    int64_t *restrict c_ip, int64_t *restrict c_ic, double *restrict c_d,
    ThrScr *restrict thr, int64_t *restrict filltmp)
{
    /* pass 1: exact distinct-column count per row */
    #pragma omp parallel
    {
        ThrScr *S = &thr[omp_get_thread_num()];
        int32_t *mark = S->mark;
        if (S->tag > (int64_t)1 << 30) {
            memset(mark, 0xFF, (size_t)g_thr_dim * sizeof(int32_t));
            S->tag = 0;
        }
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++) {
            const int64_t tag = ++S->tag;
            int64_t c = 0;
            const int64_t ea = a_ip[i + 1];
            for (int64_t ka = a_ip[i]; ka < ea; ka++) {
                const int64_t j = a_ic[ka];
                const int64_t eb = b_ip[j + 1];
                for (int64_t kb = b_ip[j]; kb < eb; kb++) {
                    const int64_t col = b_ic[kb];
                    if ((int64_t)mark[col] != tag) {
                        mark[col] = (int32_t)tag;
                        c++;
                    }
                }
            }
            filltmp[i] = c;
        }
    }
    {   /* exclusive prefix (nrow <= n0, cheap) */
        int64_t pos = 0;
        for (int64_t i = 0; i < nrow; i++) { c_ip[i] = pos; pos += filltmp[i]; }
        c_ip[nrow] = pos;
    }
    /* pass 2: accumulate and emit in first-touch order */
    #pragma omp parallel
    {
        ThrScr *S = &thr[omp_get_thread_num()];
        int32_t *mark = S->mark;
        double *acc = S->acc;
        int32_t *touched = S->touched;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++) {
            const int64_t tag = ++S->tag;
            int64_t ntouch = 0;
            const int64_t ea = a_ip[i + 1];
            for (int64_t ka = a_ip[i]; ka < ea; ka++) {
                const int64_t j = a_ic[ka];
                const double av = a_d[ka];
                const int64_t eb = b_ip[j + 1];
                for (int64_t kb = b_ip[j]; kb < eb; kb++) {
                    const int64_t col = b_ic[kb];
                    if ((int64_t)mark[col] != tag) {
                        mark[col] = (int32_t)tag;
                        acc[col] = 0.0;
                        touched[ntouch++] = (int32_t)col;
                    }
                    acc[col] = acc[col] + av * b_d[kb];
                }
            }
            const int64_t pos = c_ip[i];
            for (int64_t t = 0; t < ntouch; t++) {
                const int64_t col = touched[t];
                c_ic[pos + t] = col;
                c_d[pos + t] = acc[col];
            }
        }
    }
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
        64 * zpad + 40 * (n0 + 1) + 56 * n0 + 16 * (int64_t)nt_req * n0 + 4096;
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
#define TAKE(cnt) (base + (off = (off + ((int64_t)(cnt)) * 8 + 63) & ~INT64_C(63)) - ((int64_t)(cnt)) * 8)
    int64_t *cur_indptr = (int64_t *)TAKE(n0 + 1);
    int64_t *s_indptr   = (int64_t *)TAKE(n0 + 1);
    int64_t *p_indptr   = (int64_t *)TAKE(n0 + 1);
    int64_t *pt_indptr  = (int64_t *)TAKE(n0 + 1);
    int64_t *ap_indptr  = (int64_t *)TAKE(n0 + 1);
    int64_t *cur_indices = (int64_t *)TAKE(zpad);
    double   *cur_data   = (double   *)TAKE(zpad);
    int64_t *s_indices   = (int64_t *)TAKE(zpad); /* reused as p_indices */
    double   *p_data     = (double   *)TAKE(zpad);
    int64_t *pt_indices  = (int64_t *)TAKE(zpad);
    double   *pt_data    = (double   *)TAKE(zpad);
    int64_t *ap_indices  = (int64_t *)TAKE(zpad);
    double   *ap_data    = (double   *)TAKE(zpad);
    int64_t *agg    = (int64_t *)TAKE(n0);
    int64_t *fill   = (int64_t *)TAKE(n0 + 1);
    double   *diag  = (double  *)TAKE(n0);
    double   *cnt_  = (double  *)TAKE(n0);
    double   *tscale = (double *)TAKE(n0);
    double   *vv    = (double   *)TAKE(n0);
    double   *ww    = (double   *)TAKE(n0);
#undef TAKE

    int64_t *p_indices = s_indices; /* strength graph is dead once aggregation has run */

    /* level-0 operator: read the input in place, no copy */
    const int64_t *cip = A_indptr;
    const int64_t *cic = A_indices;
    const double   *cd = A_data;

    int64_t n = n0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int64_t lv = 0; lv < LMAX - 1; lv++) {
        if (n <= MAX_COARSE) break;

        /* ---- row diagonal ---- */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            double t = 0.0;
            const int64_t e = cip[i + 1];
            for (int64_t k = cip[i]; k < e; k++)
                if (cic[k] == i) t = fabs(cd[k]);
            diag[i] = t;
        }

        /* ---- strength graph: count, prefix, fill ---- */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            int64_t c = 0;
            const double di = diag[i];
            const int64_t e = cip[i + 1];
            for (int64_t k = cip[i]; k < e; k++) {
                const int64_t j = cic[k];
                if (j != i && fabs(cd[k]) > theta * sqrt(di * diag[j])) c++;
            }
            fill[i] = c;
        }
        {
            int64_t pos = 0;
            for (int64_t i = 0; i < n; i++) { s_indptr[i] = pos; pos += fill[i]; }
            s_indptr[n] = pos;
        }
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            int64_t pos = s_indptr[i];
            const double di = diag[i];
            const int64_t e = cip[i + 1];
            for (int64_t k = cip[i]; k < e; k++) {
                const int64_t j = cic[k];
                if (j != i && fabs(cd[k]) > theta * sqrt(di * diag[j]))
                    s_indices[pos++] = j;
            }
        }

        /* ---- greedy aggregation (sequential, exact node order) ---- */
        for (int64_t i = 0; i < n; i++) agg[i] = -1;
        int64_t na = 0;
        for (int64_t i = 0; i < n; i++) {
            if (agg[i] < 0) {
                int freef = 1;
                for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++)
                    if (agg[s_indices[k]] >= 0) { freef = 0; break; }
                if (freef) {
                    agg[i] = na;
                    for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++)
                        agg[s_indices[k]] = na;
                    na++;
                }
            }
        }
        for (int64_t i = 0; i < n; i++) {
            if (agg[i] < 0) {
                int64_t picked = -1;
                for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++) {
                    const int64_t c = agg[s_indices[k]];
                    if (c >= 0 && picked < 0) picked = c;
                }
                if (picked < 0) { agg[i] = na; na++; }
                else agg[i] = picked;
            }
        }

        if (na >= n) { n = 0; break; }

        if (depth == 1)
            memcpy(agg0, agg, sizeof(int64_t) * (size_t)n0);

        /* ---- tentative scaling ---- */
        #pragma omp parallel for schedule(static)
        for (int64_t c = 0; c < na; c++) cnt_[c] = 0.0;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            #pragma omp atomic
            cnt_[agg[i]] += 1.0;
        }
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++)
            tscale[i] = 1.0 / sqrt(cnt_[agg[i]]);

        /* ---- spectral radius by power iteration ---- */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) vv[i] = 1.0;
        double rho = 0.0;
        for (int it = 0; it < RHO_ITERS; it++) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < n; i++) {
                double a = 0.0;
                const int64_t e = cip[i + 1];
                for (int64_t k = cip[i]; k < e; k++)
                    a += cd[k] * vv[cic[k]];
                ww[i] = a / diag[i];
            }
            double nrm = 0.0;
            for (int64_t i = 0; i < n; i++) nrm += ww[i] * ww[i];
            nrm = sqrt(nrm);
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < n; i++) vv[i] = ww[i] / nrm;
            rho = nrm;
        }
        const double omega = 4.0 / (3.0 * rho);

        /* ---- smoothed prolongation: P = (I - omega D^-1 A) T ---- */
        /* count distinct columns per row, then prefix */
        #pragma omp parallel
        {
            ThrScr *S = &thr[omp_get_thread_num()];
            int32_t *mark = S->mark;
            if (S->tag > (int64_t)1 << 30) {
                memset(mark, 0xFF, (size_t)g_thr_dim * sizeof(int32_t));
                S->tag = 0;
            }
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) {
                const int64_t tag = ++S->tag;
                int64_t c = 1;
                mark[agg[i]] = (int32_t)tag;
                const int64_t e = cip[i + 1];
                for (int64_t k = cip[i]; k < e; k++) {
                    const int64_t cj = agg[cic[k]];
                    if ((int64_t)mark[cj] != tag) { mark[cj] = (int32_t)tag; c++; }
                }
                fill[i] = c;
            }
        }
        {
            int64_t pos = 0;
            for (int64_t i = 0; i < n; i++) { p_indptr[i] = pos; pos += fill[i]; }
            p_indptr[n] = pos;
        }
        #pragma omp parallel
        {
            ThrScr *S = &thr[omp_get_thread_num()];
            int32_t *mark = S->mark;
            double *acc = S->acc;
            int32_t *touched = S->touched;
            if (S->tag > (int64_t)1 << 30) {
                memset(mark, 0xFF, (size_t)g_thr_dim * sizeof(int32_t));
                S->tag = 0;
            }
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) {
                const int64_t tag = ++S->tag;
                int64_t ntouch = 0;
                const int64_t c0 = agg[i];
                if ((int64_t)mark[c0] != tag) {
                    mark[c0] = (int32_t)tag;
                    acc[c0] = 0.0;
                    touched[ntouch++] = (int32_t)c0;
                }
                acc[c0] = acc[c0] + tscale[i];
                const int64_t e = cip[i + 1];
                for (int64_t k = cip[i]; k < e; k++) {
                    const int64_t j = cic[k];
                    const int64_t cj = agg[j];
                    if ((int64_t)mark[cj] != tag) {
                        mark[cj] = (int32_t)tag;
                        acc[cj] = 0.0;
                        touched[ntouch++] = (int32_t)cj;
                    }
                    acc[cj] = acc[cj] - omega * (cd[k] / diag[i]) * tscale[j];
                }
                const int64_t pos = p_indptr[i];
                for (int64_t t = 0; t < ntouch; t++) {
                    const int64_t c = touched[t];
                    p_indices[pos + t] = c;
                    p_data[pos + t] = acc[c];
                }
            }
        }

        /* ---- Pt = P^T : stable counting transpose, ascending i within each row ---- */
        {
            const int64_t nP = p_indptr[n];
            #pragma omp parallel
            {
                const int t = omp_get_thread_num();
                const int nth = omp_get_num_threads();
                int32_t *bc = thr[t].touched;      /* block counts for my row range */
                const int64_t r0 = n * t / nth;
                const int64_t r1 = n * (t + 1) / nth;
                memset(bc, 0, (size_t)na * sizeof(int32_t));
                for (int64_t i = r0; i < r1; i++) {
                    const int64_t e = p_indptr[i + 1];
                    for (int64_t k = p_indptr[i]; k < e; k++)
                        bc[p_indices[k]]++;
                }
                #pragma omp barrier
                /* column totals into fill[0..na-1], chunked over columns */
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
                #pragma omp barrier
                /* block offsets: bc[t][c] = start position of this block within column c */
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
                #pragma omp barrier
                /* copy the row starts, then scatter my row range (ascending i) */
                #pragma omp for schedule(static)
                for (int64_t c = 0; c < na; c++) pt_indptr[c] = fill[c];
                for (int64_t i = r0; i < r1; i++) {
                    const int64_t e = p_indptr[i + 1];
                    for (int64_t k = p_indptr[i]; k < e; k++) {
                        const int64_t c = p_indices[k];
                        const int64_t pos = bc[c]++;
                        pt_indices[pos] = i;
                        pt_data[pos] = p_data[k];
                    }
                }
            }
        }

        /* ---- AP = A * P ---- */
        csr_mul(cip, cic, cd, p_indptr, p_indices, p_data, n,
                ap_indptr, ap_indices, ap_data, thr, fill);

        /* ---- nxt = Pt * AP  (over the cur buffers; cur is dead here) ---- */
        csr_mul(pt_indptr, pt_indices, pt_data, ap_indptr, ap_indices, ap_data, na,
                cur_indptr, cur_indices, cur_data, thr, fill);

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
