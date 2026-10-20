/* AMG smoothed-aggregation setup (CSR, fp64) - C port of the NumPy reference.
 * Outputs graded: agg0, level_n, level_nnz, nlevels (hierarchy shape only).
 * Aggregation and transpose scatter order are deterministic (they decide the shape);
 * every row-independent phase is threaded with per-row accumulation order identical
 * to the reference. */
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

typedef struct {
    int64_t *indptr, *indices;
    double  *data;
} csr;

/* per-thread mark/scatter scratch; stamped, so it never needs clearing */
typedef struct {
    double  *acc;      /* size g_np */
    int64_t *mark;     /* size g_np, 0 = untouched, else last stamp */
    int64_t *touched;  /* size g_np */
    int64_t  stamp;    /* ever-increasing row stamp */
    int      live;
} tlscr;

static int64_t g_np = 1;
static tlscr  *g_tls = NULL;

static void tls_init(int nthr, int64_t np)
{
    g_np = np;
    g_tls = (tlscr *)calloc((size_t)nthr, sizeof(tlscr));
    if (!g_tls) abort();
}

static tlscr *tls_get(void)
{
    tlscr *s = &g_tls[omp_get_thread_num()];
    if (!s->live) {
        size_t b = (size_t)g_np;
        s->acc     = (double  *)malloc(b * sizeof(double));
        s->mark    = (int64_t *)malloc(b * sizeof(int64_t));
        s->touched = (int64_t *)malloc(b * sizeof(int64_t));
        if (!s->acc || !s->mark || !s->touched) abort();
        memset(s->mark, 0, b * sizeof(int64_t));
        s->stamp = 0;
        s->live = 1;
    }
    return s;
}
/* ---------------- row diagonal ---------------- */
static void row_diagonal(const csr *a, double *diag, int64_t n)
{
    const int64_t *ip = a->indptr, *ci = a->indices;
    const double  *ax = a->data;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        double d = 0.0;
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++)
            if (ci[k] == i) d = fabs(ax[k]);
        diag[i] = d;
    }
}

/* ---------------- strength graph ---------------- */
static void strength_graph(const csr *a, const double *diag, int64_t n, double theta,
                           csr *s)
{
    const int64_t *ip = a->indptr, *ci = a->indices;
    const double  *ax = a->data;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        const double di = diag[i];
        int64_t c = 0;
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) {
            int64_t j = ci[k];
            if (j != i && fabs(ax[k]) > theta * sqrt(di * diag[j])) c++;
        }
        s->indptr[i] = c;
    }
    int64_t pos = 0;
    for (int64_t i = 0; i < n; i++) {
        int64_t c = s->indptr[i];
        s->indptr[i] = pos;
        pos += c;
    }
    s->indptr[n] = pos;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        const double di = diag[i];
        int64_t q = s->indptr[i];
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) {
            int64_t j = ci[k];
            if (j != i && fabs(ax[k]) > theta * sqrt(di * diag[j])) s->indices[q++] = j;
        }
    }
}

/* ---- greedy aggregation: sequential in node order, it defines the shape ---- */
static void aggregate(const csr *s, int64_t n, int64_t *agg, int64_t *na_out)
{
    const int64_t *ip = s->indptr, *ci = s->indices;
    for (int64_t i = 0; i < n; i++) agg[i] = -1;
    int64_t na = 0;
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] >= 0) continue;
        bool isfree = true;
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++)
            if (agg[ci[k]] >= 0) { isfree = false; break; }
        if (isfree) {
            agg[i] = na;
            for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) agg[ci[k]] = na;
            na++;
        }
    }
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] >= 0) continue;
        int64_t picked = -1;
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) {
            int64_t c = agg[ci[k]];
            if (c >= 0) { picked = c; break; }
        }
        if (picked < 0) { agg[i] = na; na++; }
        else agg[i] = picked;
    }
    *na_out = na;
}

/* ---------------- tentative scaling ---------------- */
static void tentative_scale(const int64_t *agg, int64_t n, int64_t na,
                            double *cnt, double *tscale)
{
    #pragma omp parallel for schedule(static)
    for (int64_t c = 0; c < na; c++) cnt[c] = 0.0;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        #pragma omp atomic
        cnt[agg[i]] += 1.0;   /* exact integers, order-irrelevant */
    }
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
}

/* ---------- spectral radius of D^-1 A by power iteration ---------- */
static void spectral_radius(const csr *a, const double *diag, int64_t n,
                             double *v, double *w, double *rho_out)
{
    const int64_t *ip = a->indptr, *ci = a->indices;
    const double  *ax = a->data;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; it++) {
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            double acc = 0.0;
            for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++)
                acc += ax[k] * v[ci[k]];
            w[i] = acc / diag[i];
        }
        double nrm = 0.0;                    /* serial left-to-right, as reference */
        for (int64_t i = 0; i < n; i++) nrm += w[i] * w[i];
        nrm = sqrt(nrm);
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) v[i] = w[i] / nrm;
        rho = nrm;
    }
    *rho_out = rho;
}

/* ---------- smoothed prolongation P = (I - omega D^-1 A) T ---------- */
static void smoothed_prolongation(const csr *a, const double *diag, const int64_t *agg,
                                  const double *tscale, int64_t n, double omega,
                                  csr *p)
{
    const int64_t *ip = a->indptr, *ci = a->indices;
    const double  *ax = a->data;
    int64_t *pip = p->indptr;
    /* pass 1: distinct-aggregate count per row (defines p's pattern exactly) */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        tlscr *t = tls_get();
        int64_t stamp = ++t->stamp;
        int64_t *mk = t->mark, *tc = t->touched;
        int64_t ntouch = 0;
        int64_t c0 = agg[i];
        if (mk[c0] != stamp) { mk[c0] = stamp; tc[ntouch++] = c0; }
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) {
            int64_t cj = agg[ci[k]];
            if (mk[cj] != stamp) { mk[cj] = stamp; tc[ntouch++] = cj; }
        }
        pip[i] = ntouch;
    }
    int64_t pos = 0;
    for (int64_t i = 0; i < n; i++) {
        int64_t c = pip[i];
        pip[i] = pos;
        pos += c;
    }
    pip[n] = pos;
    /* pass 2: values, same per-row order as the reference */
    int64_t *pci = p->indices;
    double  *px = p->data;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        tlscr *t = tls_get();
        int64_t stamp = ++t->stamp;
        double *acc = t->acc;
        int64_t *mk = t->mark, *tc = t->touched;
        int64_t ntouch = 0;
        int64_t c0 = agg[i];
        if (mk[c0] != stamp) { mk[c0] = stamp; acc[c0] = 0.0; tc[ntouch++] = c0; }
        acc[c0] += tscale[i];
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) {
            int64_t cj = agg[ci[k]];
            if (mk[cj] != stamp) { mk[cj] = stamp; acc[cj] = 0.0; tc[ntouch++] = cj; }
            acc[cj] -= omega * (ax[k] / diag[i]) * tscale[ci[k]];
        }
        int64_t q = pip[i];
        for (int64_t u = 0; u < ntouch; u++) {
            int64_t c = tc[u];
            pci[q] = c;
            px[q] = acc[c];
            q++;
        }
    }
}

/* ---------- CSR matmul C = A B, mark/scatter row accumulation ---------- */
static void csr_matmul(const csr *a, const csr *b, int64_t nrow, csr *c)
{
    const int64_t *aip = a->indptr, *aci = a->indices;
    const double  *ax = a->data;
    const int64_t *bip = b->indptr, *bci = b->indices;
    const double  *bx = b->data;
    int64_t *cip = c->indptr;
    /* pass 1: distinct output columns per row */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < nrow; i++) {
        tlscr *t = tls_get();
        int64_t stamp = ++t->stamp;
        int64_t *mk = t->mark, *tc = t->touched;
        int64_t ntouch = 0;
        for (int64_t ka = aip[i], ae = aip[i + 1]; ka < ae; ka++) {
            int64_t j = aci[ka];
            for (int64_t kb = bip[j], be = bip[j + 1]; kb < be; kb++) {
                int64_t col = bci[kb];
                if (mk[col] != stamp) { mk[col] = stamp; tc[ntouch++] = col; }
            }
        }
        cip[i] = ntouch;
    }
    int64_t pos = 0;
    for (int64_t i = 0; i < nrow; i++) {
        int64_t x = cip[i];
        cip[i] = pos;
        pos += x;
    }
    cip[nrow] = pos;
    /* pass 2: accumulate */
    int64_t *cci = c->indices;
    double  *cx = c->data;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < nrow; i++) {
        tlscr *t = tls_get();
        int64_t stamp = ++t->stamp;
        double *acc = t->acc;
        int64_t *mk = t->mark, *tc = t->touched;
        int64_t ntouch = 0;
        for (int64_t ka = aip[i], ae = aip[i + 1]; ka < ae; ka++) {
            int64_t j = aci[ka];
            double av = ax[ka];
            for (int64_t kb = bip[j], be = bip[j + 1]; kb < be; kb++) {
                int64_t col = bci[kb];
                if (mk[col] != stamp) { mk[col] = stamp; acc[col] = 0.0; tc[ntouch++] = col; }
                acc[col] += av * bx[kb];
            }
        }
        int64_t q = cip[i];
        for (int64_t u = 0; u < ntouch; u++) {
            int64_t col = tc[u];
            cci[q] = col;
            cx[q] = acc[col];
            q++;
        }
    }
}

/* ---------- CSR transpose by counting sort (stable, ascending source row) ------- */
static void csr_transpose(const csr *a, int64_t nrow, int64_t ncol, csr *t,
                          int64_t *fill)
{
    const int64_t *ip = a->indptr, *ci = a->indices;
    const double  *ax = a->data;
    int64_t *tip = t->indptr, *tci = t->indices;
    double  *tx = t->data;
    int64_t nnz = ip[nrow];
    for (int64_t c = 0; c <= ncol; c++) tip[c] = 0;
    for (int64_t k = 0; k < nnz; k++) tip[ci[k] + 1]++;
    for (int64_t c = 0; c < ncol; c++) tip[c + 1] += tip[c];
    for (int64_t c = 0; c < ncol; c++) fill[c] = tip[c];
    for (int64_t i = 0; i < nrow; i++)
        for (int64_t k = ip[i], e = ip[i + 1]; k < e; k++) {
            int64_t col = ci[k];
            int64_t q = fill[col]++;
            tci[q] = i;
            tx[q] = ax[k];
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
    size_t o = (ar->off + 63) & ~(size_t)63;   /* align */
    void *p = ar->base + o;
    ar->off = o + bytes;
    return p;
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
    const int64_t n0 = NX * NY * NZ;
    const int64_t nnz0 = A_indptr[n0];
    const int64_t zpad = (3 * (nnz0 + n0)) / 2;
    const int64_t npad = n0;

    /* scratch: 6 csr arrays + vectors; same total as the NumPy reference */
    size_t per_csr = (size_t)(npad + 1) * 8 + (size_t)zpad * 16;
    size_t total = 6 * per_csr + (size_t)npad * 8 * 6 + (size_t)npad * 8 * 3
                 + (size_t)(npad + 1) * 8 + 256;

    arena ar;
    ar.owned = 0;
    if (workspace != NULL && (int64_t)total <= workspace_size) {
        ar.base = workspace;
    } else {
        ar.base = (uint8_t *)malloc(total);
        ar.owned = 1;
        if (!ar.base) abort();
        memset(ar.base, 0, total);
    }
    ar.off = 0;

    csr cur, nxt, s, p, pt, ap;
    cur.indptr = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);
    cur.indices = (int64_t *)arena_alloc(&ar, (size_t)zpad * 8);
    cur.data = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    nxt.indptr = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);
    nxt.indices = (int64_t *)arena_alloc(&ar, (size_t)zpad * 8);
    nxt.data = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    s.indptr = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);
    s.indices = (int64_t *)arena_alloc(&ar, (size_t)zpad * 8);
    s.data = NULL;
    p.indptr = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);
    p.indices = (int64_t *)arena_alloc(&ar, (size_t)zpad * 8);
    p.data = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    pt.indptr = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);
    pt.indices = (int64_t *)arena_alloc(&ar, (size_t)zpad * 8);
    pt.data = (double *)arena_alloc(&ar, (size_t)zpad * 8);
    ap.indptr = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);
    ap.indices = (int64_t *)arena_alloc(&ar, (size_t)zpad * 8);
    ap.data = (double *)arena_alloc(&ar, (size_t)zpad * 8);

    double *diag    = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *cnt     = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *tscale  = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *vv      = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *ww      = (double *)arena_alloc(&ar, (size_t)npad * 8);
    double *acc     = (double *)arena_alloc(&ar, (size_t)npad * 8);
    int64_t *agg    = (int64_t *)arena_alloc(&ar, (size_t)npad * 8);
    int64_t *mark   = (int64_t *)arena_alloc(&ar, (size_t)npad * 8);
    int64_t *touched = (int64_t *)arena_alloc(&ar, (size_t)npad * 8);
    int64_t *fill   = (int64_t *)arena_alloc(&ar, (size_t)(npad + 1) * 8);

    tls_init(omp_get_max_threads(), npad);

    for (int lv = 0; lv < LMAX; lv++) { level_n[lv] = 0; level_nnz[lv] = 0; }

    memcpy(cur.indptr, A_indptr, (size_t)(n0 + 1) * 8);
    memcpy(cur.indices, A_indices, (size_t)nnz0 * 8);
    memcpy(cur.data, A_data, (size_t)nnz0 * 8);

    int64_t n = n0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int lv = 0; lv < LMAX - 1; lv++) {
        if (n > MAX_COARSE) {
            row_diagonal(&cur, diag, n);
            strength_graph(&cur, diag, n, theta, &s);
            for (int64_t c = 0; c < n; c++) mark[c] = -1;
            int64_t na = 0;
            aggregate(&s, n, agg, &na);
            if (na < n) {
                if (depth == 1) {
                    #pragma omp parallel for schedule(static)
                    for (int64_t i = 0; i < n0; i++) agg0[i] = agg[i];
                }
                tentative_scale(agg, n, na, cnt, tscale);
                double rho;
                spectral_radius(&cur, diag, n, vv, ww, &rho);
                double omega = 4.0 / (3.0 * rho);
                smoothed_prolongation(&cur, diag, agg, tscale, n, omega, &p);
                csr_matmul(&cur, &p, n, &ap);          /* AP  = A  @ P  */
                csr_transpose(&p, n, na, &pt, fill);    /* PT  = P^T     */
                csr_matmul(&pt, &ap, na, &nxt);        /* NXT = PT @ AP  */
                level_n[depth] = na;
                level_nnz[depth] = nxt.indptr[na];
                depth++;
                int64_t cnz = nxt.indptr[na];
                memcpy(cur.indptr, nxt.indptr, (size_t)(na + 1) * 8);
                memcpy(cur.indices, nxt.indices, (size_t)cnz * 8);
                memcpy(cur.data, nxt.data, (size_t)cnz * 8);
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;
    if (ar.owned) free(ar.base);
}
