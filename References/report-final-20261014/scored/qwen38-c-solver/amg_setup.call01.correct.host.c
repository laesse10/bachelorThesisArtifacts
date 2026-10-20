/*
 * amg_setup -- smoothed-aggregation AMG setup, parallelized with OpenMP.
 *
 * Semantics follow amg_setup_numpy.py exactly:
 *   - row diagonal extraction            (parallel over rows)
 *   - strength-of-connection selection   (parallel over rows, count + fill)
 *   - greedy aggregation                 (STRICTLY sequential: node order is
 *                                         part of the algorithm)
 *   - spectral radius power iteration    (parallel matvec / norm / rescale)
 *   - smoothed prolongation and RAP      (parallel mark/scatter over rows:
 *                                         walks the reference's exact pair
 *                                         sequence, so first-touch output
 *                                         order and per-column accumulate
 *                                         chains are reproduced)
 *   - CSR transpose (counting sort)      (parallel; within-column row order
 *                                         kept via per-thread start tables)
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LMAX 16
#define MAX_COARSE 100
#define SMOOTH_NUM 4.0
#define SMOOTH_DEN 3.0
#define RHO_ITERS 15

/* ---------------- row diagonal ---------------- */

static void row_diag(const int64_t *indptr, const int64_t *indices, const double *data,
                     int64_t n, double *diag) {
#pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        double d = 0.0;
        int64_t e0 = indptr[i], e1 = indptr[i + 1];
        for (int64_t k = e0; k < e1; k++) {
            if (indices[k] == i) d = fabs(data[k]);
        }
        diag[i] = d;
    }
}

/* ---------------- strength of connection ----------------
 * S_ij = 1 where |A_ij| > theta*sqrt(|A_ii||A_jj|), j != i; entries keep the
 * row's stored order (ascending column at level 0, first-touch later on).
 */

static void strength_count(const int64_t *indptr, const int64_t *indices, const double *data,
                           const double *diag, int64_t n, double theta, int64_t *scnt) {
#pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        int64_t c = 0;
        int64_t e0 = indptr[i], e1 = indptr[i + 1];
        for (int64_t k = e0; k < e1; k++) {
            int64_t j = indices[k];
            if (j != i && fabs(data[k]) > theta * sqrt(diag[i] * diag[j])) c++;
        }
        scnt[i] = c;
    }
}

static int64_t strength_fill(const int64_t *indptr, const int64_t *indices, const double *data,
                             const double *diag, const int64_t *scnt, double theta, int64_t n,
                             int64_t *s_indptr, int64_t *s_indices) {
    int64_t pos = 0;
    for (int64_t i = 0; i < n; i++) { s_indptr[i] = pos; pos += scnt[i]; }
    s_indptr[n] = pos;
    /* each row writes its own disjoint span of s_indices */
#pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) {
        int64_t p = s_indptr[i];
        int64_t e0 = indptr[i], e1 = indptr[i + 1];
        for (int64_t k = e0; k < e1; k++) {
            int64_t j = indices[k];
            if (j != i && fabs(data[k]) > theta * sqrt(diag[i] * diag[j])) s_indices[p++] = j;
        }
    }
    return pos;
}

/* ---------------- aggregation: strictly sequential ---------------- */

static int64_t aggregate(const int64_t *s_indptr, const int64_t *s_indices, int64_t n,
                         int64_t *agg) {
    int64_t i, k;
    for (i = 0; i < n; i++) agg[i] = -1;
    int64_t na = 0;
    for (i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int64_t free = 1;
            for (k = s_indptr[i]; k < s_indptr[i + 1]; k++) {
                if (agg[s_indices[k]] >= 0) { free = 0; break; }
            }
            if (free) {
                agg[i] = na;
                for (k = s_indptr[i]; k < s_indptr[i + 1]; k++) agg[s_indices[k]] = na;
                na++;
            }
        }
    }
    for (i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int64_t picked = -1;
            for (k = s_indptr[i]; k < s_indptr[i + 1]; k++) {
                int64_t c = agg[s_indices[k]];
                if (c >= 0 && picked < 0) picked = c;
            }
            if (picked < 0) { agg[i] = na; na++; } else agg[i] = picked;
        }
    }
    return na;
}

/* ---------------- spectral radius (power iteration) ---------------- */

static double spectral_radius(const int64_t *indptr, const int64_t *indices, const double *data,
                              const double *diag, int64_t n, double *v, double *w) {
    for (int64_t i = 0; i < n; i++) v[i] = 1.0;
    double rho = 0.0;
    for (int64_t it = 0; it < RHO_ITERS; it++) {
#pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            double acc = 0.0;
            int64_t e0 = indptr[i], e1 = indptr[i + 1];
            for (int64_t k = e0; k < e1; k++) acc += data[k] * v[indices[k]];
            w[i] = acc / diag[i];
        }
        /* serial: the reference accumulates in index order */
        double nrm = 0.0;
        for (int64_t i = 0; i < n; i++) nrm += w[i] * w[i];
        nrm = sqrt(nrm);
        const double inv = 1.0 / nrm;
#pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) v[i] = w[i] * inv;
        rho = nrm;
    }
    return rho;
}

/* ---------------- row-parallel sparse products (mark/scatter) ----------------
 *
 * The reference emits each output row in FIRST-TOUCH order of the
 * contributing (list, entry) pairs, and each column's value is the ordered
 * chain  acc = acc + scale * w  over the lists in list order.  Walking the
 * identical pair sequence into a per-thread stamp/accumulator table indexed
 * by output column reproduces both properties at once (works whether or not
 * the B rows are sorted):
 *   phase 1 (parallel over rows): walk the pairs, count distinct columns
 *   serial: prefix sum -> out_indptr
 *   phase 2 (parallel over rows): walk again, mark/scatter into acc[],
 *     record the first-touch order in touched[], emit at the row's prefix.
 * Stamps are tagged with a monotonically increasing base + row id, so the
 * table is reused across calls and levels with no clearing.
 */

typedef struct {
    int64_t *stamp; /* per-column tag                    */
    double *acc;    /* per-column accumulator            */
    int64_t *touched; /* first-touch order, current row  */
    int64_t  cap_a; /* stamp/acc slots initialised to 0  */
    int64_t  cap_t;
} TBuf;

#define TBUF_MAX_THREADS 128
static TBuf tbuf_pool[TBUF_MAX_THREADS];

static void tbuf_ensure(TBuf *tb, int64_t ncol, int64_t nt_max) {
    if (ncol > tb->cap_a) {
        int64_t nc = tb->cap_a ? tb->cap_a : 1024;
        while (nc < ncol) nc *= 2;
        tb->stamp = (int64_t *)realloc(tb->stamp, (size_t)nc * sizeof(int64_t));
        tb->acc = (double *)realloc(tb->acc, (size_t)nc * sizeof(double));
        memset(tb->stamp + tb->cap_a, 0, (size_t)(nc - tb->cap_a) * sizeof(int64_t));
        tb->cap_a = nc;
    }
    if (nt_max > tb->cap_t) {
        int64_t nc = tb->cap_t ? tb->cap_t : 256;
        while (nc < nt_max) nc *= 2;
        tb->touched = (int64_t *)realloc(tb->touched, (size_t)nc * sizeof(int64_t));
        tb->cap_t = nc;
    }
}

static TBuf *tbuf_cur(void) {
    int tid = omp_get_thread_num();
    if (tid < 0 || tid >= TBUF_MAX_THREADS) tid = 0;
    return &tbuf_pool[tid];
}

static int64_t tag_base = 0;
static int64_t dbg_level = 0;

/* C = A*B.  phase !emit: cnt[i] = distinct column count of row i.
 * phase emit:  c_indices/c_data at c_indptr positions, first-touch order. */
static void matmul_phase(const int64_t *a_indptr, const int64_t *a_indices, const double *a_data,
                         const int64_t *b_indptr, const int64_t *b_indices, const double *b_data,
                         int64_t nrow, int64_t ncol, int64_t stride, int64_t *cnt,
                         int64_t *c_indptr, int64_t *c_indices, double *c_data,
                         int64_t zpad_total) {
    const int emit = (c_indices != 0);
    tag_base += stride + 1;
    const int64_t base = tag_base;
#pragma omp parallel
    {
        TBuf *tb = tbuf_cur();
#pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++) {
            const int64_t tag = base + i;
            int64_t e0 = a_indptr[i], e1 = a_indptr[i + 1];
            int64_t L = 0;
            for (int64_t t = e0; t < e1; t++)
                L += b_indptr[a_indices[t] + 1] - b_indptr[a_indices[t]];
            tbuf_ensure(tb, ncol, L);
            int64_t *stamp = tb->stamp;
            int64_t nt = 0;
            if (!emit) {
                for (int64_t t = e0; t < e1; t++) {
                    int64_t f0 = b_indptr[a_indices[t]], f1 = b_indptr[a_indices[t] + 1];
                    for (int64_t kb = f0; kb < f1; kb++) {
                        int64_t c = b_indices[kb];
                        if (stamp[c] != tag) { stamp[c] = tag; nt++; }
                    }
                }
                cnt[i] = nt;
            } else {
                double *acc = tb->acc;
                int64_t *touched = tb->touched;
                for (int64_t t = e0; t < e1; t++) {
                    const double av = a_data[t];
                    int64_t f0 = b_indptr[a_indices[t]], f1 = b_indptr[a_indices[t] + 1];
                    for (int64_t kb = f0; kb < f1; kb++) {
                        int64_t c = b_indices[kb];
                        if (stamp[c] != tag) {
                            stamp[c] = tag;
                            acc[c] = 0.0;
                            touched[nt++] = c;
                        }
                        acc[c] += av * b_data[kb];
                    }
                }
                int64_t ppos = c_indptr[i];
                if (ppos + nt > zpad_total) {
                    fprintf(stderr,
                            "amg_setup: matmul row %lld overflow (nrow=%lld nt=%lld lv=%lld)\n",
                            (long long)i, (long long)nrow, (long long)nt, (long long)dbg_level);
                    abort();
                }
                for (int64_t t = 0; t < nt; t++) {
                    int64_t c = touched[t];
                    c_indices[ppos + t] = c;
                    c_data[ppos + t] = acc[c];
                }
            }
        }
    }
}

static void csr_matmul(const int64_t *a_indptr, const int64_t *a_indices, const double *a_data,
                       const int64_t *b_indptr, const int64_t *b_indices, const double *b_data,
                       int64_t nrow, int64_t ncol, int64_t stride, int64_t *c_indptr,
                       int64_t *c_indices, double *c_data, int64_t *cnt, int64_t zpad) {
    matmul_phase(a_indptr, a_indices, a_data, b_indptr, b_indices, b_data, nrow, ncol, stride, cnt,
                 0, 0, 0, zpad);
    int64_t pos = 0;
    for (int64_t i = 0; i < nrow; i++) { c_indptr[i] = pos; pos += cnt[i]; }
    c_indptr[nrow] = pos;
    matmul_phase(a_indptr, a_indices, a_data, b_indptr, b_indices, b_data, nrow, ncol, stride, cnt,
                 c_indptr, c_indices, c_data, zpad);
}

/* P = (I - omega D^-1 A) T: pair stream per row =
 *   (agg[i], +tscale[i])  then, per A entry k: (agg[j_k], -(omega*(a_k/di)) * tscale[j_k]) */
static void smooth_prolong(const int64_t *a_indptr, const int64_t *a_indices, const double *a_data,
                           const double *diag, const int64_t *agg, const double *tscale,
                           int64_t n, int64_t ncol, int64_t stride, double omega,
                           int64_t *p_indptr, int64_t *p_indices, double *p_data, int64_t *cnt,
                           int64_t zpad_total) {
    for (int phase = 0; phase < 2; phase++) {
        tag_base += stride + 1;
        const int64_t base = tag_base;
        if (phase == 1) {
            int64_t pos = 0;
            for (int64_t i = 0; i < n; i++) { p_indptr[i] = pos; pos += cnt[i]; }
            p_indptr[n] = pos;
        }
#pragma omp parallel
        {
            TBuf *tb = tbuf_cur();
#pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) {
                const int64_t tag = base + i;
                int64_t e0 = a_indptr[i], e1 = a_indptr[i + 1];
                int64_t m = (e1 - e0) + 1;
                tbuf_ensure(tb, ncol, m);
                int64_t *stamp = tb->stamp;
                int64_t nt = 0;
                const double di = diag[i];
                if (!phase) {
                    int64_t c = agg[i];
                    if (stamp[c] != tag) { stamp[c] = tag; nt++; }
                    for (int64_t k = e0; k < e1; k++) {
                        c = agg[a_indices[k]];
                        if (stamp[c] != tag) { stamp[c] = tag; nt++; }
                    }
                    cnt[i] = nt;
                } else {
                    double *acc = tb->acc;
                    int64_t *touched = tb->touched;
                    {
                        int64_t c = agg[i];
                        if (stamp[c] != tag) {
                            stamp[c] = tag;
                            acc[c] = 0.0;
                            touched[nt++] = c;
                        }
                        acc[c] += tscale[i];
                    }
                    for (int64_t k = e0; k < e1; k++) {
                        const double s = -(omega * (a_data[k] / di));
                        int64_t j = a_indices[k];
                        int64_t c = agg[j];
                        if (stamp[c] != tag) {
                            stamp[c] = tag;
                            acc[c] = 0.0;
                            touched[nt++] = c;
                        }
                        acc[c] += s * tscale[j];
                    }
                    int64_t ppos = p_indptr[i];
                    if (ppos + nt > zpad_total) {
                        fprintf(stderr, "amg_setup: prolong row %lld overflow (n=%lld nt=%lld lv=%lld)\n",
                                (long long)i, (long long)n, (long long)nt, (long long)dbg_level);
                        abort();
                    }
                    for (int64_t t = 0; t < nt; t++) {
                        int64_t c = touched[t];
                        p_indices[ppos + t] = c;
                        p_data[ppos + t] = acc[c];
                    }
                }
            }
        }
    }
}

/* ---------------- transpose (counting sort, order-preserving) ---------------- */

static void csr_transpose(const int64_t *indptr, const int64_t *indices, const double *data,
                          int64_t nrow, int64_t ncol, int64_t *t_indptr, int64_t *t_indices,
                          double *t_data) {
    int nt = omp_get_max_threads();
    if (nt < 1) nt = 1;
    /* per-thread column counts, then a per-thread start table so the scatter
       can run in parallel yet keep the reference's within-column row order */
    int64_t *cntt = (int64_t *)malloc((size_t)nt * (size_t)(ncol + 1) * sizeof(int64_t));
    memset(cntt, 0, (size_t)nt * (size_t)(ncol + 1) * sizeof(int64_t));
#pragma omp parallel num_threads(nt)
    {
        int64_t tid = omp_get_thread_num();
        int64_t *cnt = cntt + (size_t)tid * (size_t)(ncol + 1);
        int64_t chunk = (nrow + nt - 1) / nt;
        int64_t rs = (int64_t)tid * chunk, re = rs + chunk;
        if (re > nrow) re = nrow;
        for (int64_t i = rs; i < re; i++) {
            int64_t e0 = indptr[i], e1 = indptr[i + 1];
            for (int64_t k = e0; k < e1; k++) cnt[indices[k]]++;
        }
    }
    /* column totals + exclusive prefix -> t_indptr */
#pragma omp parallel for schedule(static)
    for (int64_t c = 0; c <= ncol; c++) {
        int64_t s = 0;
        for (int64_t t = 0; t < nt; t++) s += cntt[(size_t)t * (size_t)(ncol + 1) + c];
        t_indptr[c] = s;
    }
    {
        int64_t run = t_indptr[0];
        t_indptr[0] = 0;
        for (int64_t c = 1; c <= ncol; c++) {
            int64_t tc = t_indptr[c];
            t_indptr[c] = run;
            run += tc;
        }
    }
    /* start table: start[t][c] = t_indptr[c] + sum_{t'<t} cnt[t'][c];
       reuse cntt in place as cursors */
    for (int64_t c = 0; c <= ncol; c++) {
        int64_t base = t_indptr[c];
        for (int64_t t = 0; t < nt; t++) {
            int64_t *p = cntt + (size_t)t * (size_t)(ncol + 1) + c;
            int64_t keep = *p;
            *p = base;
            base += keep;
        }
    }
#pragma omp parallel num_threads(nt)
    {
        int64_t tid = omp_get_thread_num();
        int64_t *cur = cntt + (size_t)tid * (size_t)(ncol + 1);
        int64_t chunk = (nrow + nt - 1) / nt;
        int64_t rs = (int64_t)tid * chunk, re = rs + chunk;
        if (re > nrow) re = nrow;
        for (int64_t i = rs; i < re; i++) {
            int64_t e0 = indptr[i], e1 = indptr[i + 1];
            for (int64_t k = e0; k < e1; k++) {
                int64_t col = indices[k];
                int64_t p = cur[col]++;
                t_indices[p] = i;
                t_data[p] = data[k];
            }
        }
    }
    free(cntt);
}

/* ---------------- driver ---------------- */

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
    const int64_t workspace_size) {
    (void)workspace;
    (void)workspace_size;
    const int64_t n0 = NX * NY * NZ;
    const int64_t nnz0 = (3 * NX - 2) * (3 * NY - 2) * (3 * NZ - 2);
    const int64_t zpad = (3 * (nnz0 + n0)) / 2;
    const int64_t npad = n0;

#define ALLOC64(n) ((int64_t *)malloc((size_t)(n) * sizeof(int64_t)))
#define ALLOCF(n)  ((double *)malloc((size_t)(n) * sizeof(double)))

    int64_t *cur_indptr = ALLOC64(npad + 1);
    int64_t *cur_indices = ALLOC64(zpad);
    double *cur_data = ALLOCF(zpad);
    int64_t *nxt_indptr = ALLOC64(npad + 1);
    int64_t *nxt_indices = ALLOC64(zpad);
    double *nxt_data = ALLOCF(zpad);
    int64_t *s_indptr = ALLOC64(npad + 1);
    int64_t *s_indices = ALLOC64(zpad);
    int64_t *p_indptr = ALLOC64(npad + 1);
    int64_t *p_indices = ALLOC64(zpad);
    double *p_data = ALLOCF(zpad);
    int64_t *pt_indptr = ALLOC64(npad + 1);
    int64_t *pt_indices = ALLOC64(zpad);
    double *pt_data = ALLOCF(zpad);
    int64_t *ap_indptr = ALLOC64(npad + 1);
    int64_t *ap_indices = ALLOC64(zpad);
    double *ap_data = ALLOCF(zpad);

    double *diag = ALLOCF(npad);
    int64_t *agg = ALLOC64(npad);
    double *cnt = ALLOCF(npad);
    double *tscale = ALLOCF(npad);
    double *vv = ALLOCF(npad);
    double *ww = ALLOCF(npad);
    int64_t *scnt = ALLOC64(npad);

    for (int64_t i = 0; i <= n0; i++) cur_indptr[i] = A_indptr[i];
#pragma omp parallel for schedule(static)
    for (int64_t k = 0; k < nnz0; k++) {
        cur_indices[k] = A_indices[k];
        cur_data[k] = A_data[k];
    }

    for (int64_t lv = 0; lv < LMAX; lv++) { level_n[lv] = 0; level_nnz[lv] = 0; }
    int64_t n = n0;
    dbg_level = 0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int64_t it = 0; it < LMAX - 1; it++) {
        if (n > MAX_COARSE) {
            dbg_level++;
            row_diag(cur_indptr, cur_indices, cur_data, n, diag);
            strength_count(cur_indptr, cur_indices, cur_data, diag, n, theta, scnt);
            strength_fill(cur_indptr, cur_indices, cur_data, diag, scnt, theta, n, s_indptr,
                          s_indices);
            int64_t na = aggregate(s_indptr, s_indices, n, agg);
            if (na < n) {
                if (depth == 1) {
#pragma omp parallel for schedule(static)
                    for (int64_t i = 0; i < n0; i++) agg0[i] = agg[i];
                }
                /* tentative scale (integer counts: serial pass, cheap) */
                for (int64_t c = 0; c < na; c++) cnt[c] = 0.0;
                for (int64_t i = 0; i < n; i++) cnt[agg[i]] += 1.0;
#pragma omp parallel for schedule(static)
                for (int64_t i = 0; i < n; i++) tscale[i] = 1.0 / sqrt(cnt[agg[i]]);

                double rho = spectral_radius(cur_indptr, cur_indices, cur_data, diag, n, vv, ww);
                double omega = SMOOTH_NUM / (SMOOTH_DEN * rho);

                smooth_prolong(cur_indptr, cur_indices, cur_data, diag, agg, tscale, n, na, n0,
                               omega, p_indptr, p_indices, p_data, scnt, zpad);

                csr_matmul(cur_indptr, cur_indices, cur_data, p_indptr, p_indices, p_data, n,
                           na, n0, ap_indptr, ap_indices, ap_data, scnt, zpad);

                csr_transpose(p_indptr, p_indices, p_data, n, na, pt_indptr, pt_indices, pt_data);

                csr_matmul(pt_indptr, pt_indices, pt_data, ap_indptr, ap_indices, ap_data, na,
                           na, n0, nxt_indptr, nxt_indices, nxt_data, scnt, zpad);

                level_n[depth] = na;
                level_nnz[depth] = nxt_indptr[na];
                depth++;
#pragma omp parallel for schedule(static)
                for (int64_t i = 0; i <= na; i++) cur_indptr[i] = nxt_indptr[i];
                {
                    int64_t nz = nxt_indptr[na];
#pragma omp parallel for schedule(static)
                    for (int64_t k = 0; k < nz; k++) {
                        cur_indices[k] = nxt_indices[k];
                        cur_data[k] = nxt_data[k];
                    }
                }
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;

    free(cur_indptr); free(cur_indices); free(cur_data);
    free(nxt_indptr); free(nxt_indices); free(nxt_data);
    free(s_indptr); free(s_indices);
    free(p_indptr); free(p_indices); free(p_data);
    free(pt_indptr); free(pt_indices); free(pt_data);
    free(ap_indptr); free(ap_indices); free(ap_data);
    free(diag); free(agg); free(cnt); free(tscale);
    free(vv); free(ww); free(scnt);
}
