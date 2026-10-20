/* AMG smoothed-aggregation setup (CSR, fp64) - optimized C port of the NumPy reference.
 * Graded outputs: agg0, level_n, level_nnz, nlevels (hierarchy shape only).
 * Determinism kept wherever it decides the shape: greedy aggregation node order,
 * transpose scatter stability, first-touch column order in every mark/scatter
 * accumulation.  Internal index arrays are int32 (input converted once); values keep
 * the reference's per-row accumulation order, so coarse operators match it. */
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

typedef int32_t i32;
typedef int64_t i64;

typedef struct {
    i32    *indptr, *indices;
    double *data;
} csr;

/* per-thread mark/scatter scratch; stamped so it never needs clearing */
typedef struct {
    double *acc;      /* size g_np */
    i32    *mark;     /* size g_np, 0 = untouched, else last stamp */
    i32    *touched;  /* size g_np */
    i32    *cnt32;    /* size g_np, per-thread aggregate histogram */
    i32    *cur32;    /* size g_np, per-thread scatter cursor */
    i32     stamp;    /* ever-increasing row stamp */
    int     live;
} tlscr;

static i64    g_np = 1;
static int    g_nthr = 1;
static tlscr *g_tls = NULL;

static void tls_init(int nthr, i64 np)
{
    g_np = np;
    g_tls = (tlscr *)calloc((size_t)nthr, sizeof(tlscr));
    if (!g_tls) abort();
    #pragma omp parallel
    {
        #pragma omp single
        g_nthr = omp_get_num_threads();
    }
}

static tlscr *tls_get(void)
{
    tlscr *s = &g_tls[omp_get_thread_num()];
    if (!s->live) {
        size_t b = (size_t)g_np;
        s->acc     = (double *)malloc(b * sizeof(double));
        s->mark    = (i32    *)malloc(b * sizeof(i32));
        s->touched = (i32    *)malloc(b * sizeof(i32));
        s->cnt32   = (i32    *)malloc(b * sizeof(i32));
        s->cur32   = (i32    *)malloc(b * sizeof(i32));
        if (!s->acc || !s->mark || !s->touched || !s->cnt32 || !s->cur32) abort();
        memset(s->mark, 0, b * sizeof(i32));
        s->stamp = 0;
        s->live = 1;
    }
    return s;
}

/* ---------- parallel exclusive prefix sum (int32 counts) ----------
 * Deterministic: same result as the serial scan. arr[n] receives the total. */
static void prefix_excl_i32(i32 *arr, i64 n)
{
    const int nthr = g_nthr;
    if (n < 8192 || nthr <= 1) {
        i64 s = 0;
        for (i64 i = 0; i < n; i++) { i32 c = arr[i]; arr[i] = (i32)s; s += c; }
        arr[n] = (i32)s;
        return;
    }
    i64 part[257];
    const i64 chunk = (n + nthr - 1) / nthr;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        i64 lo = (i64)tid * chunk;
        i64 hi = lo + chunk;
        if (hi > n) hi = n;
        i64 s = 0;
        for (i64 i = lo; i < hi; i++) { i32 c = arr[i]; arr[i] = (i32)s; s += c; }
        part[tid] = s;
    }
    i64 acc = 0;
    for (int t = 0; t < nthr; t++) { i64 p = part[t]; part[t] = acc; acc += p; }
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        i64 lo = (i64)tid * chunk;
        i64 hi = lo + chunk;
        if (hi > n) hi = n;
        const i32 off = (i32)part[tid];
        for (i64 i = lo; i < hi; i++) arr[i] += off;
    }
    arr[n] = (i32)acc;
}

/* per-wave chaining of block positions between waves (phases sequential) */
static i64 g_wbase[257];
static i64 g_wtot[257];

/* ---------- per-thread block scratch ---------- */
typedef struct {
    i32    *col;
    double *val;
    i64     cap;      /* capacity in entries */
    i32    *ubstart;  /* per-block ub prefix, RMAX+1 entries */
} bscr;

#define RMAX 4096

static void bscr_reserve(bscr *sc, i64 need)
{
    if (need > sc->cap) {
        i64 cap = sc->cap ? sc->cap : 4096;
        while (cap < need) cap *= 2;
        sc->col = (i32    *)realloc(sc->col, (size_t)cap * sizeof(i32));
        sc->val = (double *)realloc(sc->val, (size_t)cap * sizeof(double));
        if (!sc->col || !sc->val) abort();
        sc->cap = cap;
    }
}

/* ---------- strength graph: one masked pass, then a compaction pass ----------
 * sq[k] = 1 iff entry k is strong.  The comparison expression is identical to the
 * reference: |a_ij| > theta*sqrt(di_i*di_j), diagonal excluded. */
static void strength_graph(const csr *a, const double *diag, i64 n, double theta,
                           csr *s, int8_t *sq)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    const double *dg = diag;
    i32 *sip = s->indptr;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        const double di = dg[i];
        i32 c = 0;
        for (i32 k = ip[i], e = ip[i + 1]; k < e; k++) {
            const i64 j = ci[k];
            int8_t m = (j != i) && (fabs(ax[k]) > theta * sqrt(di * dg[j]));
            sq[k] = m;
            c += m;
        }
        sip[i] = c;
    }
    prefix_excl_i32(sip, n);
    i32 *sci = s->indices;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        i32 q = sip[i];
        for (i32 k = ip[i], e = ip[i + 1]; k < e; k++)
            if (sq[k]) sci[q++] = ci[k];
    }
}

/* ---- greedy aggregation: sequential in node order; it defines the shape ---- */
static void aggregate(const csr *s, i64 n, i32 *agg, i64 *na_out)
{
    const i32 *ip = s->indptr, *ci = s->indices;
    for (i64 i = 0; i < n; i++) agg[i] = -1;
    i64 na = 0;
    for (i64 i = 0; i < n; i++) {
        if (agg[i] >= 0) continue;
        bool isfree = true;
        for (i32 k = ip[i], e = ip[i + 1]; k < e; k++)
            if (agg[ci[k]] >= 0) { isfree = false; break; }
        if (isfree) {
            agg[i] = (i32)na;
            for (i32 k = ip[i], e = ip[i + 1]; k < e; k++) agg[ci[k]] = (i32)na;
            na++;
        }
    }
    for (i64 i = 0; i < n; i++) {
        if (agg[i] >= 0) continue;
        i32 picked = -1;
        for (i32 k = ip[i], e = ip[i + 1]; k < e; k++) {
            i32 c = agg[ci[k]];
            if (c >= 0) { picked = c; break; }
        }
        if (picked < 0) { agg[i] = (i32)na; na++; }
        else agg[i] = picked;
    }
    *na_out = na;
}

/* ---------- tentative scaling: per-thread integer histograms ---------- */
static void tentative_scale(const i32 *agg, i64 n, i64 na, double *cnt, double *tscale)
{
    #pragma omp parallel
    {
        tlscr *t = tls_get();
        i32 *h = t->cnt32;
        memset(h, 0, (size_t)na * sizeof(i32));
        #pragma omp for schedule(static) nowait
        for (i64 i = 0; i < n; i++) h[agg[i]]++;
    }
    #pragma omp parallel for schedule(static)
    for (i64 c = 0; c < na; c++) {
        i64 sv = 0;
        for (int t = 0; t < g_nthr; t++) sv += g_tls[t].cnt32[c];
        cnt[c] = (double)sv;
    }
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
}

/* ---------- spectral radius of D^-1 A by power iteration ----------
 * Per-row accumulation order matches the reference; the norm reduction is the
 * reference's serial left-to-right sum. */
static void spectral_radius(const csr *a, const double *diag, i64 n,
                             double *v, double *w, double *rho_out)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; it++) {
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) {
            double accv = 0.0;
            for (i32 k = ip[i], e = ip[i + 1]; k < e; k++)
                accv += ax[k] * v[ci[k]];
            w[i] = accv / diag[i];
        }
        double nrm = 0.0;
        for (i64 i = 0; i < n; i++) nrm += w[i] * w[i];
        nrm = sqrt(nrm);
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) v[i] = w[i] / nrm;
        rho = nrm;
    }
    *rho_out = rho;
}

/* ---------- CSR matmul C = A B, one accumulation pass ----------
 * Rows are filled into a per-block upper-bound scratch and compacted wave by
 * wave; the pattern (first-touch order) and the values (accumulation order)
 * are identical to the two-pass mark/scatter form. */
static void csr_matmul(const csr *a, const csr *b, i64 nrow, csr *c,
                       i32 *ubtmp, bscr *scratch)
{
    const i32    *aip = a->indptr, *aci = a->indices;
    const double *ax = a->data;
    const i32    *bip = b->indptr, *bci = b->indices;
    const double *bx = b->data;
    i32 *cip = c->indptr;
    i32 *cci = c->indices;
    double *cx = c->data;

    /* per-row upper bound: sum of nnz(B_j) over j in A's row */
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < nrow; i++) {
        i64 u = 0;
        for (i32 ka = aip[i], ae = aip[i + 1]; ka < ae; ka++)
            u += (bip[aci[ka] + 1] - bip[aci[ka]]);
        ubtmp[i] = (i32)u;
    }

    i64 R = nrow / (4 * (i64)g_nthr);
    if (R < 64) R = 64;
    if (R > RMAX) R = RMAX;
    const i64 nblk = (nrow + R - 1) / R;

    i64 running = 0;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        tlscr *t = tls_get();
        bscr *sc = &scratch[tid];
        if (!sc->ubstart) {
            sc->ubstart = (i32 *)malloc((size_t)(RMAX + 1) * sizeof(i32));
            if (!sc->ubstart) abort();
        }
        for (i64 w0 = 0; w0 < nblk; w0 += (i64)g_nthr) {
            const i64 blk = w0 + tid;
            i64 r0 = 0, r1 = 0;
            if (blk < nblk) {
                r0 = blk * R;
                r1 = (r0 + R < nrow) ? r0 + R : nrow;
                i64 su = 0;
                for (i64 i = r0; i < r1; i++) {
                    sc->ubstart[i - r0] = (i32)su;
                    su += ubtmp[i];
                }
                bscr_reserve(sc, su);
                i32 *scol = sc->col;
                double *sval = sc->val;
                for (i64 i = r0; i < r1; i++) {
                    const i32 stamp = ++t->stamp;
                    double *acc = t->acc;
                    i32 *mk = t->mark, *tc = t->touched;
                    i32 ntouch = 0;
                    i32 q = sc->ubstart[i - r0];
                    for (i32 ka = aip[i], ae = aip[i + 1]; ka < ae; ka++) {
                        const i32 j = aci[ka];
                        const double av = ax[ka];
                        for (i32 kb = bip[j], be = bip[j + 1]; kb < be; kb++) {
                            const i32 col = bci[kb];
                            if (mk[col] != stamp) {
                                mk[col] = stamp;
                                acc[col] = 0.0;
                                tc[ntouch++] = col;
                            }
                            acc[col] += av * bx[kb];
                        }
                    }
                    for (i32 u = 0; u < ntouch; u++) {
                        const i32 col = tc[u];
                        scol[q] = col;
                        sval[q] = acc[col];
                        q++;
                    }
                    cip[i] = ntouch;
                }
                i64 bt = 0;
                for (i64 i = r0; i < r1; i++) bt += cip[i];
                g_wtot[tid] = bt;
            } else {
                g_wtot[tid] = 0;
            }
            #pragma omp barrier
            #pragma omp single
            {
                i64 base = running;
                for (int tt = 0; tt < g_nthr; tt++) {
                    g_wbase[tt] = base;
                    base += g_wtot[tt];
                }
                running = base;
            }
            if (blk < nblk) {
                i32 q = (i32)g_wbase[tid];
                for (i64 i = r0; i < r1; i++) {
                    const i32 c = cip[i];
                    cip[i] = q;
                    q += c;
                    memcpy(cci + cip[i], sc->col + sc->ubstart[i - r0],
                           (size_t)c * sizeof(i32));
                    memcpy(cx + cip[i], sc->val + sc->ubstart[i - r0],
                           (size_t)c * sizeof(double));
                }
            }
        }
    }
    cip[nrow] = (i32)running;
}


/* ---------- smoothed prolongation P = (I - omega D^-1 A) T, one fill pass,
 * wave by wave, same structure as csr_matmul.  First-touch order and
 * accumulation order match the reference exactly. */
static void smoothed_prolongation(const csr *a, const double *diag, const i32 *agg,
                                  const double *tscale, i64 n, double omega,
                                  csr *p, i32 *ubtmp, bscr *scratch)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    i32 *pip = p->indptr;
    i32 *pci = p->indices;
    double *px = p->data;

    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++)
        ubtmp[i] = ip[i + 1] - ip[i] + 1;

    i64 R = n / (4 * (i64)g_nthr);
    if (R < 64) R = 64;
    if (R > RMAX) R = RMAX;
    const i64 nblk = (n + R - 1) / R;

    i64 running = 0;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        tlscr *t = tls_get();
        bscr *sc = &scratch[tid];
        if (!sc->ubstart) {
            sc->ubstart = (i32 *)malloc((size_t)(RMAX + 1) * sizeof(i32));
            if (!sc->ubstart) abort();
        }
        for (i64 w0 = 0; w0 < nblk; w0 += (i64)g_nthr) {
            const i64 blk = w0 + tid;
            i64 r0 = 0, r1 = 0;
            if (blk < nblk) {
                r0 = blk * R;
                r1 = (r0 + R < n) ? r0 + R : n;
                i64 su = 0;
                for (i64 i = r0; i < r1; i++) {
                    sc->ubstart[i - r0] = (i32)su;
                    su += ubtmp[i];
                }
                bscr_reserve(sc, su);
                i32 *scol = sc->col;
                double *sval = sc->val;
                for (i64 i = r0; i < r1; i++) {
                    const i32 stamp = ++t->stamp;
                    double *acc = t->acc;
                    i32 *mk = t->mark, *tc = t->touched;
                    i32 ntouch = 0;
                    i32 q = sc->ubstart[i - r0];
                    const i32 c0 = agg[i];
                    if (mk[c0] != stamp) { mk[c0] = stamp; acc[c0] = 0.0; tc[ntouch++] = c0; }
                    acc[c0] += tscale[i];
                    for (i32 k = ip[i], e = ip[i + 1]; k < e; k++) {
                        const i32 cj = agg[ci[k]];
                        if (mk[cj] != stamp) { mk[cj] = stamp; acc[cj] = 0.0; tc[ntouch++] = cj; }
                        acc[cj] -= omega * (ax[k] / diag[i]) * tscale[ci[k]];
                    }
                    for (i32 u = 0; u < ntouch; u++) {
                        const i32 c = tc[u];
                        scol[q] = c;
                        sval[q] = acc[c];
                        q++;
                    }
                    pip[i] = ntouch;
                }
                i64 bt = 0;
                for (i64 i = r0; i < r1; i++) bt += pip[i];
                g_wtot[tid] = bt;
            } else {
                g_wtot[tid] = 0;
            }
            #pragma omp barrier
            #pragma omp single
            {
                i64 base = running;
                for (int tt = 0; tt < g_nthr; tt++) {
                    g_wbase[tt] = base;
                    base += g_wtot[tt];
                }
                running = base;
            }
            if (blk < nblk) {
                i32 q = (i32)g_wbase[tid];
                for (i64 i = r0; i < r1; i++) {
                    const i32 c = pip[i];
                    pip[i] = q;
                    q += c;
                    memcpy(pci + pip[i], sc->col + sc->ubstart[i - r0],
                           (size_t)c * sizeof(i32));
                    memcpy(px + pip[i], sc->val + sc->ubstart[i - r0],
                           (size_t)c * sizeof(double));
                }
            }
        }
    }
    pip[n] = (i32)running;
}

/* ---------- CSR transpose: parallel, stable counting sort ----------
 * Within a column, entries come out in ascending source-row order (block-stable
 * offsets), exactly as the reference's serial scatter produces. */
static void csr_transpose(const csr *a, i64 nrow, i64 ncol, csr *t)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    i32    *tip = t->indptr, *tci = t->indices;
    double *tx = t->data;
    const i64 nnz = ip[nrow];
    const int nthr = g_nthr;

    #pragma omp parallel
    {
        tlscr *t = tls_get();
        i32 *h = t->cnt32;                     /* local histogram */
        i32 *cur = t->cur32;                   /* my block's scatter cursor */
        memset(h, 0, (size_t)ncol * sizeof(i32));
        int tid = omp_get_thread_num();
        i64 b0 = (nnz * tid) / nthr;
        i64 b1 = (nnz * (tid + 1)) / nthr;
        for (i64 k = b0; k < b1; k++) h[ci[k]]++;
        #pragma omp barrier
        #pragma omp single
        {
            tip[0] = 0;
            for (i64 c = 0; c < ncol; c++) {
                i64 s = 0;
                for (int u = 0; u < nthr; u++) s += g_tls[u].cnt32[c];
                tip[c + 1] = (i32)s;
            }
            for (i64 c = 0; c < ncol; c++) tip[c + 1] += tip[c];
        }
        #pragma omp barrier
        /* block-stable cursor: tip[c] + counts of all earlier threads */
        for (i64 c = 0; c < ncol; c++) {
            i64 off = tip[c];
            for (int u = 0; u < tid; u++) off += g_tls[u].cnt32[c];
            cur[c] = (i32)off;
        }
        #pragma omp barrier
        /* scatter my k-range in ascending k (= ascending source row) */
        if (b1 > b0) {
            i64 i = 0;
            {   /* first row whose range meets b0 */
                i64 lo = 0, hi = nrow;
                while (lo < hi) {
                    i64 mid = (lo + hi) >> 1;
                    if (ip[mid + 1] <= b0) lo = mid + 1; else hi = mid;
                }
                i = lo;
            }
            for (i64 k = b0; k < b1; k++) {
                while (k >= ip[i + 1]) i++;
                i32 c = ci[k];
                i32 q = cur[c]++;
                tci[q] = (i32)i;
                tx[q] = ax[k];
            }
        }
    }
}

/* ---------- row diagonal (levels > 0; level 0 is fused into the input copy) --- */
static void row_diagonal(const csr *a, double *diag, i64 n)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        double d = 0.0;
        for (i32 k = ip[i], e = ip[i + 1]; k < e; k++)
            if (ci[k] == i) d = fabs(ax[k]);
        diag[i] = d;
    }
}

/* ---------------- main driver ---------------- */
typedef struct {
    uint8_t *base;
    size_t   off;
    int      owned;
} arena;

static void *arena_alloc(arena *ar, size_t bytes)
{
    size_t o = (ar->off + 63) & ~(size_t)63;
    void *ptr = ar->base + o;
    ar->off = o + bytes;
    return ptr;
}

void amg_setup_csr_fp64(
    const double  *restrict A_data,
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
    const i64 n0 = NX * NY * NZ;
    const i64 nnz0 = A_indptr[n0];
    const i64 zpad = (3 * (nnz0 + n0)) / 2;
    const i64 npad = n0;

    /* scratch: strictly smaller than the NumPy reference's padded buffers */
    size_t per_csr = (size_t)(npad + 1) * 4 + (size_t)zpad * 12;
    size_t total = 5 * per_csr                                /* cur nxt p pt ap */
                 + (size_t)(npad + 1) * 4 + (size_t)zpad * 4  /* s (indices only) */
                 + (size_t)npad * 8 * 5                       /* diag tscale vv ww cnt */
                 + (size_t)npad * 4                           /* agg */
                 + (size_t)zpad                                /* sq */
                 + 256;

    arena ar;
    ar.owned = 0;
    if (workspace != NULL && (i64)total <= workspace_size) {
        ar.base = workspace;
    } else {
        ar.base = (uint8_t *)malloc(total);
        ar.owned = 1;
        if (!ar.base) abort();
        memset(ar.base, 0, total);
    }
    ar.off = 0;

    csr cur, nxt, s, p, pt, ap;
    cur.indptr  = (i32 *)arena_alloc(&ar, (size_t)(npad + 1) * 4);
    cur.indices = (i32 *)arena_alloc(&ar, (size_t)zpad * 4);
    cur.data    = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    nxt.indptr  = (i32 *)arena_alloc(&ar, (size_t)(npad + 1) * 4);
    nxt.indices = (i32 *)arena_alloc(&ar, (size_t)zpad * 4);
    nxt.data    = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    s.indptr    = (i32 *)arena_alloc(&ar, (size_t)(npad + 1) * 4);
    s.indices   = (i32 *)arena_alloc(&ar, (size_t)zpad * 4);
    s.data      = NULL;
    p.indptr    = (i32 *)arena_alloc(&ar, (size_t)(npad + 1) * 4);
    p.indices   = (i32 *)arena_alloc(&ar, (size_t)zpad * 4);
    p.data      = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    pt.indptr   = (i32 *)arena_alloc(&ar, (size_t)(npad + 1) * 4);
    pt.indices  = (i32 *)arena_alloc(&ar, (size_t)zpad * 4);
    pt.data     = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    ap.indptr   = (i32 *)arena_alloc(&ar, (size_t)(npad + 1) * 4);
    ap.indices  = (i32 *)arena_alloc(&ar, (size_t)zpad * 4);
    ap.data     = (double *)arena_alloc(&ar, (size_t)zpad * 8);

    double *diag   = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *tscale = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *vv     = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *ww     = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *cnt    = (double *)arena_alloc(&ar, (size_t)npad * 8);
    i32    *agg    = (i32    *)arena_alloc(&ar, (size_t)npad * 4);
    int8_t *sq     = (int8_t *)arena_alloc(&ar, (size_t)zpad);
    i32    *ubtmp  = (i32    *)arena_alloc(&ar, (size_t)(npad + 1) * 4);

    tls_init(omp_get_max_threads(), npad);
    bscr *scratch = (bscr *)calloc((size_t)g_nthr, sizeof(bscr));
    if (!scratch) abort();

    for (int lv = 0; lv < LMAX; lv++) { level_n[lv] = 0; level_nnz[lv] = 0; }

    /* input copy (int64 -> int32) with the level-0 diagonal fused in */
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n0; i++) {
        double d = 0.0;
        for (i64 k = A_indptr[i], e = A_indptr[i + 1]; k < e; k++) {
            cur.indices[k] = (i32)A_indices[k];
            cur.data[k] = A_data[k];
            if (A_indices[k] == i) d = fabs(A_data[k]);
        }
        cur.indptr[i] = (i32)A_indptr[i];
        diag[i] = d;
    }
    cur.indptr[n0] = (i32)A_indptr[n0];

    i64 n = n0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    i64 depth = 1;

    for (int lv = 0; lv < LMAX - 1; lv++) {
        if (n > MAX_COARSE) {
            if (lv > 0) row_diagonal(&cur, diag, n);
            strength_graph(&cur, diag, n, theta, &s, sq);
            i64 na = 0;
            aggregate(&s, n, agg, &na);
            if (na < n) {
                if (depth == 1) {
                    #pragma omp parallel for schedule(static)
                    for (i64 i = 0; i < n0; i++) agg0[i] = agg[i];
                }
                const int need_vals = !(na <= MAX_COARSE || lv == LMAX - 2);
                double omega = 1.0;
                if (need_vals) {
                    double rho;
                    spectral_radius(&cur, diag, n, vv, ww, &rho);
                    omega = 4.0 / (3.0 * rho);
                }
                tentative_scale(agg, n, na, cnt, tscale);
                smoothed_prolongation(&cur, diag, agg, tscale, n, omega, &p, ubtmp, scratch);
                csr_matmul(&cur, &p, n, &ap, ubtmp, scratch);   /* AP = A @ P */
                csr_transpose(&p, n, na, &pt);                  /* PT = P^T   */
                csr_matmul(&pt, &ap, na, &nxt, ubtmp, scratch); /* NXT = PT @ AP */
                level_n[depth] = na;
                level_nnz[depth] = nxt.indptr[na];
                depth++;
                { csr sw = cur; cur = nxt; nxt = sw; }   /* swap, no copy */
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;
    if (ar.owned) free(ar.base);
    for (int t = 0; t < g_nthr; t++) {
        free(scratch[t].col);
        free(scratch[t].val);
        free(scratch[t].ubstart);
    }
    free(scratch);
}
