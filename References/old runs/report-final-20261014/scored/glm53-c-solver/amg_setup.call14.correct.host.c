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
static tlscr *g_tls = NULL;
static int    g_tls_thr = 0;   /* threads g_tls was allocated for */
static i64    g_tls_np = -1;   /* row count g_tls arrays are sized for */

static int g_nthr = 1;

static void tls_init(int nthr, i64 np)
{
    g_np = np;
    /* allocate once per process; realloc only if the shape changed */
    if (g_tls != NULL && g_tls_thr == nthr && g_tls_np >= np) return;
    if (g_tls != NULL) {
        for (int t = 0; t < g_tls_thr; t++) {
            free(g_tls[t].acc);
            free(g_tls[t].mark);
            free(g_tls[t].touched);
            free(g_tls[t].cnt32);
            free(g_tls[t].cur32);
        }
        free(g_tls);
    }
    g_tls = (tlscr *)calloc((size_t)nthr, sizeof(tlscr));
    if (!g_tls) abort();
    g_tls_thr = nthr;
    g_tls_np = np;
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

/* ---------- parallel exclusive prefix sum of i32 counts ----------
 * Deterministic: block partial sums, serial scan of block totals, block-local
 * serial scan.  Result is exactly the left-to-right exclusive prefix. */
static void prefix_excl_i32(i32 *a, i64 n)
{
    if (n <= 0) return;
    const i64 BLK = 1 << 16;
    const i64 nblk = (n + BLK - 1) / BLK;
    i64 part[257];
    i64 total = 0;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int nth = omp_get_num_threads();
        i64 b0 = (nblk * tid) / nth;
        i64 b1 = (nblk * (tid + 1)) / nth;
        i64 acc = 0;
        for (i64 b = b0; b < b1; b++) {
            i64 lo = b * BLK;
            i64 hi = (lo + BLK < n) ? lo + BLK : n;
            i64 loc = 0;
            for (i64 i = lo; i < hi; i++) loc += a[i];
            part[b] = loc;
            acc += loc;
        }
        (void)acc;
        #pragma omp barrier
        #pragma omp single
        {
            i64 run = 0;
            for (i64 b = 0; b < nblk; b++) {
                i64 x = part[b];
                part[b] = run;
                run += x;
            }
            total = run;
        }
        #pragma omp barrier
        for (i64 b = b0; b < b1; b++) {
            i64 lo = b * BLK;
            i64 hi = (lo + BLK < n) ? lo + BLK : n;
            i64 run = part[b];
            for (i64 i = lo; i < hi; i++) {
                i32 x = a[i];
                a[i] = (i32)run;
                run += x;
            }
        }
    }
    a[n] = (i32)total;
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
 * Per-row accumulation order matches the reference; the norm is a parallel
 * reduction (reassociation covered by the tolerance).  One parallel region per
 * iteration. */
static void spectral_radius(const csr *a, const double *diag, i64 n,
                             double *v, double *w, double *rho_out)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    double rho = 0.0;
    double nrm2 = 1.0;
    double psum[257];
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        #pragma omp for schedule(static)
        for (i64 i = 0; i < n; i++) v[i] = 1.0;
        for (int it = 0; it < RHO_ITERS; it++) {
            double partial = 0.0;
            #pragma omp for schedule(static) nowait
            for (i64 i = 0; i < n; i++) {
                double accv = 0.0;
                for (i32 k = ip[i], e = ip[i + 1]; k < e; k++)
                    accv += ax[k] * v[ci[k]];
                w[i] = accv / diag[i];
                partial += w[i] * w[i];
            }
            psum[tid] = partial;
            #pragma omp barrier
            /* every thread combines identically (deterministic order) */
            {
                double s = 0.0;
                for (int u = 0; u < g_nthr; u++) s += psum[u];
                nrm2 = sqrt(s);
                if (tid == 0) rho = nrm2;
            }
            #pragma omp for schedule(static)
            for (i64 i = 0; i < n; i++) v[i] = w[i] / nrm2;
        }
    }
    *rho_out = rho;
}

/* ---------- smoothed prolongation P = (I - omega D^-1 A) T ----------
 * Two passes: distinct-aggregate count, then first-touch fill.  Per-row order of
 * first touches and of the accumulation matches the reference exactly. */
static void smoothed_prolongation(const csr *a, const double *diag, const i32 *agg,
                                 const double *tscale, i64 n, double omega,
                                 csr *p)
{
    const i32    *ip = a->indptr, *ci = a->indices;
    const double *ax = a->data;
    i32 *pip = p->indptr;
    #pragma omp parallel
    {
        tlscr *t = tls_get();
        #pragma omp for schedule(static)
        for (i64 i = 0; i < n; i++) {
            i32 stamp = ++t->stamp;
            i32 *mk = t->mark, *tc = t->touched;
            i32 ntouch = 0;
            i32 c0 = agg[i];
            if (mk[c0] != stamp) { mk[c0] = stamp; tc[ntouch++] = c0; }
            for (i32 k = ip[i], e = ip[i + 1]; k < e; k++) {
                i32 cj = agg[ci[k]];
                if (mk[cj] != stamp) { mk[cj] = stamp; tc[ntouch++] = cj; }
            }
            pip[i] = ntouch;
        }
    }
    prefix_excl_i32(pip, n);
    i32 *pci = p->indices;
    double *px = p->data;
    #pragma omp parallel
    {
        tlscr *t = tls_get();
        #pragma omp for schedule(static)
        for (i64 i = 0; i < n; i++) {
            i32 stamp = ++t->stamp;
            double *acc = t->acc;
            i32 *mk = t->mark, *tc = t->touched;
            i32 ntouch = 0;
            i32 c0 = agg[i];
            if (mk[c0] != stamp) { mk[c0] = stamp; acc[c0] = 0.0; tc[ntouch++] = c0; }
            acc[c0] += tscale[i];
            for (i32 k = ip[i], e = ip[i + 1]; k < e; k++) {
                i32 cj = agg[ci[k]];
                if (mk[cj] != stamp) { mk[cj] = stamp; acc[cj] = 0.0; tc[ntouch++] = cj; }
                acc[cj] -= omega * (ax[k] / diag[i]) * tscale[ci[k]];
            }
            i32 q = pip[i];
            for (i32 u = 0; u < ntouch; u++) {
                i32 c = tc[u];
                pci[q] = c;
                px[q] = acc[c];
                q++;
            }
        }
    }
}

/* ---------- padded-fill scratch: grown once, reused across calls ---------- */
static i32    *g_pc  = NULL;   /* column pad */
static double *g_pv  = NULL;   /* value pad */
static i64     g_pcap = 0;     /* entry capacity */
static i32    *g_uoff = NULL;  /* per-row pad offsets */
static i64     g_uoff_n = 0;
static i64     g_wtot[257];   /* per-thread segment totals */
static i64     g_wbase[257];  /* per-thread segment bases */

#define PAD_CAP ((i64)1 << 28)   /* 2686 entries = 3.2 GB cap */

static int pad_reserve(i64 need)
{
    if (need <= g_pcap) return 1;
    if (need > PAD_CAP) return 0;
    i64 nc = need + (need >> 3);
    i32    *pc = (i32    *)malloc((size_t)nc * sizeof(i32));
    double *pv = (double  *)malloc((size_t)nc * sizeof(double));
    if (!pc || !pv) { free(g_pc); free(g_pv); g_pc = pc; g_pv = pv; g_pcap = nc; return 1; }
    free(pc); free(pv);
    return 0;
}

static int uoff_reserve(i64 n)
{
    if (n <= g_uoff_n) return 1;
    i32 *p = (i32 *)malloc((size_t)(n + 1) * sizeof(i32));
    if (!p) return 0;
    free(g_uoff);
    g_uoff = p;
    g_uoff_n = n;
    return 1;
}

/* ---------- CSR matmul C = A B: dense mark/acc two-pass (fallback) ---------- */
static void csr_matmul_twopass(const csr *a, const csr *b, i64 nrow, csr *c)
{
    const i32    *aip = a->indptr, *aci = a->indices;
    const double *ax = a->data;
    const i32    *bip = b->indptr, *bci = b->indices;
    const double *bx = b->data;
    i32 *cip = c->indptr;
    #pragma omp parallel
    {
        tlscr *t = tls_get();
        #pragma omp for schedule(static)
        for (i64 i = 0; i < nrow; i++) {
            i32 stamp = ++t->stamp;
            i32 *mk = t->mark, *tc = t->touched;
            i32 ntouch = 0;
            for (i32 ka = aip[i], ae = aip[i + 1]; ka < ae; ka++) {
                i32 j = aci[ka];
                for (i32 kb = bip[j], be = bip[j + 1]; kb < be; kb++) {
                    i32 col = bci[kb];
                    if (mk[col] != stamp) { mk[col] = stamp; tc[ntouch++] = col; }
                }
            }
            cip[i] = ntouch;
        }
    }
    prefix_excl_i32(cip, nrow);
    i32 *cci = c->indices;
    double *cx = c->data;
    #pragma omp parallel
    {
        tlscr *t = tls_get();
        #pragma omp for schedule(static)
        for (i64 i = 0; i < nrow; i++) {
            i32 stamp = ++t->stamp;
            double *acc = t->acc;
            i32 *mk = t->mark, *tc = t->touched;
            i32 ntouch = 0;
            for (i32 ka = aip[i], ae = aip[i + 1]; ka < ae; ka++) {
                i32 j = aci[ka];
                double av = ax[ka];
                for (i32 kb = bip[j], be = bip[j + 1]; kb < be; kb++) {
                    i32 col = bci[kb];
                    if (mk[col] != stamp) { mk[col] = stamp; acc[col] = 0.0; tc[ntouch++] = col; }
                    acc[col] += av * bx[kb];
                }
            }
            i32 q = cip[i];
            for (i32 u = 0; u < ntouch; u++) {
                i32 col = tc[u];
                cci[q] = col;
                cx[q] = acc[col];
                q++;
            }
        }
    }
}

/* ---------- CSR matmul C = A B, per-thread padded segments ----------
 * Each thread owns a contiguous row range and its pad pad segment sized to the
 * rows' upper upper bound, so its fill writes writes and sequentially into its segment.  A final
 * pass compacts per row into the exact offsets.  Gather order, first first-touch order and the
 * accumulation order are identical to the two two-pass form so the result is bit-identical. */
static void csr_matmul(const csr *a, const csr *b, i64 nrow, csr *c)
{
    const i32    *aip = a->indptr, *aci = a->indices;
    const double *ax = a->data;
    const i32    *bip = b->indptr, *bci = b->indices;
    const double *bx = b->data;
    i32 *cip = c->indptr;

    /* the padded path only pays off on large products; below this, count+fill
     * through the dense mark array is faster */
    if (nrow < 262144) { csr_matmul_twopass(a, b, nrow, c); return; }
    if (!uoff_reserve(nrow)) { csr_matmul_twopass(a, b, nrow, c); return; }
    int ub_ok = 1;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int nth = omp_get_num_threads();
        i64 r0 = (nrow * tid) / nth;
        i64 r1 = (nrow * (tid + 1)) / nth;
        i64 loc = 0;
        for (i64 i = r0; i < r1; i++) {
            i32 u = 0;
            for (i32 ka = aip[i], ae = aip[i + 1]; ka < ae; ka++)
                u += bip[aci[ka] + 1] - bip[aci[ka]];
            g_uoff[i] = (i32)loc;
            loc += u;
        }
        g_wtot[tid] = loc;
        #pragma omp barrier
        #pragma omp single
        {
            i64 base = 0;
            for (int tt = 0; tt < nth; tt++) {
                g_wbase[tt] = base;
                base += g_wtot[tt];
            }
            ub_ok = pad_reserve(base);
        }
        if (ub_ok) {
            tlscr *t = tls_get();
            const i64 seg = g_wbase[tid];
            for (i64 i = r0; i < r1; i++) g_uoff[i] += (i32)seg;
            for (i64 i = r0; i < r1; i++) {
                i32 stamp = ++t->stamp;
                double *acc = t->acc;
                i32 *mk = t->mark, *tc = t->touched;
                i32 ntouch = 0;
                i32 q = g_uoff[i];
                for (i32 ka = aip[i], ae = aip[i + 1]; ka < ae; ka++) {
                    i32 j = aci[ka];
                    double av = ax[ka];
                    for (i32 kb = bip[j], be = bip[j + 1]; kb < be; kb++) {
                        i32 col = bci[kb];
                        if (mk[col] != stamp) { mk[col] = stamp; acc[col] = 0.0; tc[ntouch++] = col; }
                        acc[col] += av * bx[kb];
                    }
                }
                for (i32 v = 0; v < ntouch; v++) {
                    i32 col = tc[v];
                    g_pc[q] = col;
                    g_pv[q] = acc[col];
                    q++;
                }
                cip[i] = ntouch;
            }
        }
    }
    if (!ub_ok) { csr_matmul_twopass(a, b, nrow, c); return; }
    prefix_excl_i32(cip, nrow);
    i32 *cci = c->indices;
    double *cx = c->data;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int nth = omp_get_num_threads();
        i64 r0 = (nrow * tid) / nth;
        i64 r1 = (nrow * (tid + 1)) / nth;
        for (i64 i = r0; i < r1; i++) {
            i32 q = cip[i];
            i32 u = g_uoff[i];
            i32 m = cip[i + 1] - q;
            memcpy(cci + q, g_pc + u, (size_t)m * sizeof(i32));
            memcpy(cx + q, g_pv + u, (size_t)m * sizeof(double));
        }
    }
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

    tls_init(omp_get_max_threads(), npad);

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
                smoothed_prolongation(&cur, diag, agg, tscale, n, omega, &p);
                csr_matmul(&cur, &p, n, &ap);         /* AP  = A  @ P  */
                csr_transpose(&p, n, na, &pt);        /* PT  = P^T     */
                csr_matmul(&pt, &ap, na, &nxt);       /* NXT = PT @ AP  */
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
}
