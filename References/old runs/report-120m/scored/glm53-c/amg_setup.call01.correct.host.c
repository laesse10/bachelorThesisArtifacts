/* Smoothed-aggregation AMG setup (parallel C port of the NumPy reference).
 *
 * Exactness strategy: every floating-point accumulation that feeds a discrete
 * decision (strength comparison, aggregate order, output counts) keeps the
 * reference's per-entry accumulation order: loops are parallelized only across
 * independent rows / columns, and the spectral-radius norm stays a sequential
 * sum in reference order.
 */
#include <stdint.h>
#include <stddef.h>
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
    i32 *mark;
    double *acc;
    i32 *touch;
} TLS;

typedef struct {
    i64 n0;
    i32 *cur_ip, *cur_ix; double *cur_d;
    i32 *s_ip, *s_ix;
    i32 *p_ip, *p_ix;  double *p_d;
    i32 *pt_ip, *pt_ix; double *pt_d;
    i32 *ap_ip, *ap_ix; double *ap_d;
    double *diag, *cnt, *tscale, *vv, *ww;
    i32 *agg, *rowcnt;
    TLS *tls;
    int nt;
} Ctx;

/* ---------------- input copy ---------------- */

static void copy_in(Ctx *c, const double *A_data, const i64 *A_indices,
                    const i64 *A_indptr, i64 n0, i64 nnz0)
{
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i <= n0; i++)
        c->cur_ip[i] = (i32)A_indptr[i];
    #pragma omp parallel for schedule(static)
    for (i64 k = 0; k < nnz0; k++) {
        c->cur_ix[k] = (i32)A_indices[k];
        c->cur_d[k] = A_data[k];
    }
}

/* ---------------- diagonal ---------------- */

static void row_diagonal(const Ctx *c, i64 n)
{
    const i32 *restrict ip = c->cur_ip, *restrict ix = c->cur_ix;
    const double *restrict d = c->cur_d;
    double *restrict diag = c->diag;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        double dv = 0.0;
        const i32 I = (i32)i;
        for (i32 k = ip[i]; k < ip[i + 1]; k++)
            if (ix[k] == I)
                dv = fabs(d[k]);
        diag[i] = dv;
    }
}

/* ---------------- strength graph ---------------- */

static void strength_graph(const Ctx *c, i64 n, double theta)
{
    const i32 *restrict ip = c->cur_ip, *restrict ix = c->cur_ix;
    const double *restrict d = c->cur_d, *restrict diag = c->diag;
    i32 *restrict s_ip = c->s_ip, *restrict s_ix = c->s_ix;
    i32 *restrict rowcnt = c->rowcnt;
    const i32 I_n = (i32)n;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        i32 cnt = 0;
        const double di = diag[i];
        for (i32 k = ip[i]; k < ip[i + 1]; k++) {
            i32 j = ix[k];
            if (j != (i32)i && fabs(d[k]) > theta * sqrt(di * diag[j]))
                cnt++;
        }
        rowcnt[i] = cnt;
    }
    s_ip[0] = 0;
    for (i64 i = 0; i < n; i++)
        s_ip[i + 1] = s_ip[i] + rowcnt[i];
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        i32 pos = s_ip[i];
        const double di = diag[i];
        for (i32 k = ip[i]; k < ip[i + 1]; k++) {
            i32 j = ix[k];
            if (j != (i32)i && fabs(d[k]) > theta * sqrt(di * diag[j]))
                s_ix[pos++] = j;
        }
    }
    (void)I_n;
}

/* ---------------- greedy aggregation (sequential; exact port) ---------------- */

static void aggregate(const i32 *restrict s_ip, const i32 *restrict s_ix,
                      i64 n, i32 *restrict agg, double *restrict cnt, i64 *na_out)
{
    for (i64 i = 0; i < n; i++)
        agg[i] = -1;
    i32 na = 0;
    /* pass 1 */
    for (i64 i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int free = 1;
            for (i32 k = s_ip[i]; k < s_ip[i + 1]; k++)
                if (agg[s_ix[k]] >= 0) { free = 0; break; }
            if (free) {
                agg[i] = na;
                double inc = 1.0;
                for (i32 k = s_ip[i]; k < s_ip[i + 1]; k++) {
                    agg[s_ix[k]] = na;
                    inc += 1.0;
                }
                cnt[na] = inc;
                na++;
            }
        }
    }
    /* pass 2 */
    for (i64 i = 0; i < n; i++) {
        if (agg[i] < 0) {
            i32 picked = -1;
            for (i32 k = s_ip[i]; k < s_ip[i + 1]; k++) {
                i32 cc = agg[s_ix[k]];
                if (cc >= 0) { picked = cc; break; }
            }
            if (picked < 0) {
                agg[i] = na;
                cnt[na] = 1.0;
                na++;
            } else {
                agg[i] = picked;
                cnt[picked] += 1.0;
            }
        }
    }
    *na_out = na;
}

/* ---------------- tentative scaling ---------------- */

static void tentative_scale(const Ctx *c, i64 n, i64 na)
{
    const i32 *restrict agg = c->agg;
    const double *restrict cnt = c->cnt;
    double *restrict ts = c->tscale;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++)
        ts[i] = 1.0 / sqrt(cnt[agg[i]]);
    (void)na;
}

/* ---------------- spectral radius (power iteration) ---------------- */

static double spectral_radius(const Ctx *c, i64 n)
{
    const i32 *restrict ip = c->cur_ip, *restrict ix = c->cur_ix;
    const double *restrict d = c->cur_d, *restrict diag = c->diag;
    double *restrict v = c->vv, *restrict w = c->ww;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++)
        v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; it++) {
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) {
            double acc = 0.0;
            for (i32 k = ip[i]; k < ip[i + 1]; k++)
                acc += d[k] * v[ix[k]];
            w[i] = acc / diag[i];
        }
        double nrm = 0.0;
        for (i64 i = 0; i < n; i++)   /* sequential: reference summation order */
            nrm += w[i] * w[i];
        nrm = sqrt(nrm);
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++)
            v[i] = w[i] / nrm;
        rho = nrm;
    }
    return rho;
}

/* ---------------- smoothed prolongation P = (I - omega D^-1 A) T ---------------- */

static void smoothed_prolongation(Ctx *c, i64 n, i64 na, double omega)
{
    const i32 *restrict ip = c->cur_ip, *restrict ix = c->cur_ix;
    const double *restrict d = c->cur_d, *restrict diag = c->diag;
    const i32 *restrict agg = c->agg;
    const double *restrict ts = c->tscale;
    i32 *restrict p_ip = c->p_ip, *restrict p_ix = c->p_ix;
    double *restrict p_d = c->p_d;
    i32 *restrict rowcnt = c->rowcnt;
    const int nt = c->nt;
    const i32 stampoff = (i32)c->n0 + 1;

    #pragma omp parallel for schedule(static) num_threads(nt)
    for (int t = 0; t < nt; t++)
        memset(c->tls[t].mark, 0, sizeof(i32) * (size_t)na);

    /* pass 1: row counts (mark stamped with i+1) */
    #pragma omp parallel num_threads(nt)
    {
        i32 *restrict mark = c->tls[omp_get_thread_num()].mark;
        #pragma omp for schedule(dynamic, 16)
        for (i64 i = 0; i < n; i++) {
            i32 st = (i32)i + 1;
            i32 ntouch = 0;
            i32 c0 = agg[i];
            if (mark[c0] != st) { mark[c0] = st; ntouch++; }
            for (i32 k = ip[i]; k < ip[i + 1]; k++) {
                i32 cj = agg[ix[k]];
                if (mark[cj] != st) { mark[cj] = st; ntouch++; }
            }
            rowcnt[i] = ntouch;
        }
    }
    p_ip[0] = 0;
    for (i64 i = 0; i < n; i++)
        p_ip[i + 1] = p_ip[i] + rowcnt[i];
    /* pass 2: fill (stamp i+1+n0+1 to stay distinct from pass 1) */
    #pragma omp parallel num_threads(nt)
    {
        TLS *tl = &c->tls[omp_get_thread_num()];
        i32 *restrict mark = tl->mark;
        double *restrict acc = tl->acc;
        i32 *restrict touch = tl->touch;
        #pragma omp for schedule(dynamic, 16)
        for (i64 i = 0; i < n; i++) {
            i32 st = (i32)i + 1 + stampoff;
            i32 ntouch = 0;
            i32 pos = p_ip[i];
            i32 c0 = agg[i];
            if (mark[c0] != st) { mark[c0] = st; acc[c0] = 0.0; touch[ntouch++] = c0; }
            acc[c0] += ts[i];
            for (i32 k = ip[i]; k < ip[i + 1]; k++) {
                i32 jj = ix[k];
                i32 cj = agg[jj];
                if (mark[cj] != st) { mark[cj] = st; acc[cj] = 0.0; touch[ntouch++] = cj; }
                acc[cj] -= omega * (d[k] / diag[i]) * ts[jj];
            }
            for (i32 t = 0; t < ntouch; t++) {
                i32 col = touch[t];
                p_ix[pos + t] = col;
                p_d[pos + t] = acc[col];
            }
        }
    }
}

/* ---------------- C = A*B (CSR) by mark/scatter, parallel over rows of A ---------- */

static void csr_matmul(Ctx *c,
                       const i32 *restrict a_ip, const i32 *restrict a_ix, const double *restrict a_d,
                       const i32 *restrict b_ip, const i32 *restrict b_ix, const double *restrict b_d,
                       i64 nrow, i64 ncols,
                       i32 *restrict c_ip, i32 *restrict c_ix, double *restrict c_d)
{
    i32 *restrict rowcnt = c->rowcnt;
    const int nt = c->nt;
    const i32 stampoff = (i32)c->n0 + 1;

    #pragma omp parallel for schedule(static) num_threads(nt)
    for (int t = 0; t < nt; t++)
        memset(c->tls[t].mark, 0, sizeof(i32) * (size_t)ncols);

    /* pass 1: counts */
    #pragma omp parallel num_threads(nt)
    {
        i32 *restrict mark = c->tls[omp_get_thread_num()].mark;
        #pragma omp for schedule(dynamic, 16)
        for (i64 i = 0; i < nrow; i++) {
            i32 st = (i32)i + 1;
            i32 ntouch = 0;
            for (i32 ka = a_ip[i]; ka < a_ip[i + 1]; ka++) {
                i64 j = a_ix[ka];
                for (i32 kb = b_ip[j]; kb < b_ip[j + 1]; kb++) {
                    i32 col = b_ix[kb];
                    if (mark[col] != st) { mark[col] = st; ntouch++; }
                }
            }
            rowcnt[i] = ntouch;
        }
    }
    c_ip[0] = 0;
    for (i64 i = 0; i < nrow; i++)
        c_ip[i + 1] = c_ip[i] + rowcnt[i];
    /* pass 2: values */
    #pragma omp parallel num_threads(nt)
    {
        TLS *tl = &c->tls[omp_get_thread_num()];
        i32 *restrict mark = tl->mark;
        double *restrict acc = tl->acc;
        i32 *restrict touch = tl->touch;
        #pragma omp for schedule(dynamic, 16)
        for (i64 i = 0; i < nrow; i++) {
            i32 st = (i32)i + 1 + stampoff;
            i32 ntouch = 0;
            i32 pos = c_ip[i];
            for (i32 ka = a_ip[i]; ka < a_ip[i + 1]; ka++) {
                i64 j = a_ix[ka];
                double av = a_d[ka];
                for (i32 kb = b_ip[j]; kb < b_ip[j + 1]; kb++) {
                    i32 col = b_ix[kb];
                    if (mark[col] != st) { mark[col] = st; acc[col] = 0.0; touch[ntouch++] = col; }
                    acc[col] += av * b_d[kb];
                }
            }
            for (i32 t = 0; t < ntouch; t++) {
                i32 col = touch[t];
                c_ix[pos + t] = col;
                c_d[pos + t] = acc[col];
            }
        }
    }
}

/* ---------------- transpose by block counting sort (column order preserved) ----- */

static void csr_transpose(Ctx *c,
                          const i32 *restrict ip, const i32 *restrict ix, const double *restrict d,
                          i64 nrow, i64 ncol,
                          i32 *restrict t_ip, i32 *restrict t_ix, double *restrict t_d)
{
    const int ntb = c->nt;
    int nt = ntb < nrow ? ntb : (int)nrow;
    if (nt < 1) nt = 1;
    TLS *tls = c->tls;

    /* per-thread row-block histograms */
    #pragma omp parallel for schedule(static) num_threads(nt)
    for (int t = 0; t < nt; t++)
        memset(tls[t].mark, 0, sizeof(i32) * (size_t)(ncol + 1));

    #pragma omp parallel num_threads(nt)
    {
        int t = omp_get_thread_num();
        i64 r0 = nrow * t / nt, r1 = nrow * (t + 1) / nt;
        i32 *restrict hist = tls[t].mark;
        for (i64 i = r0; i < r1; i++)
            for (i32 k = ip[i]; k < ip[i + 1]; k++)
                hist[ix[k]]++;
    }

    /* totals + exclusive scan (sequential over columns) */
    for (i64 cc = 0; cc < ncol; cc++) {
        i32 s = 0;
        for (int t = 0; t < nt; t++)
            s += tls[t].mark[cc];
        t_ip[cc] = s;
    }
    {
        i32 run = 0;
        for (i64 cc = 0; cc < ncol; cc++) {
            i32 h = t_ip[cc];
            t_ip[cc] = run;
            run += h;
        }
        t_ip[ncol] = run;
    }

    /* per-thread start offsets inside each column */
    #pragma omp parallel num_threads(nt)
    {
        int t = omp_get_thread_num();
        i32 *restrict off = tls[t].touch;
        for (i64 cc = 0; cc < ncol; cc++) {
            i32 s = t_ip[cc];
            for (int t2 = 0; t2 < t; t2++)
                s += tls[t2].mark[cc];
            off[cc] = s;
        }
    }

    /* scatter, preserving row order within each column */
    #pragma omp parallel num_threads(nt)
    {
        int t = omp_get_thread_num();
        i64 r0 = nrow * t / nt, r1 = nrow * (t + 1) / nt;
        i32 *restrict off = tls[t].touch;
        for (i64 i = r0; i < r1; i++) {
            i32 I = (i32)i;
            for (i32 k = ip[i]; k < ip[i + 1]; k++) {
                i64 col = ix[k];
                i32 pos = off[col]++;
                t_ix[pos] = I;
                t_d[pos] = d[k];
            }
        }
    }
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
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const i64 n0 = NX * NY * NZ;
    const i64 nnz0 = A_indptr[n0];
    const i64 zpad = 3 * (nnz0 + n0) / 2;

    memset(level_n, 0, sizeof(int64_t) * LMAX);
    memset(level_nnz, 0, sizeof(int64_t) * LMAX);

    Ctx c;
    memset(&c, 0, sizeof(c));
    c.n0 = n0;
    c.nt = omp_get_max_threads();
    if (c.nt < 1) c.nt = 1;

    c.cur_ip = malloc(sizeof(i32) * (size_t)(n0 + 1));
    c.cur_ix = malloc(sizeof(i32) * (size_t)zpad);
    c.cur_d  = malloc(sizeof(double) * (size_t)zpad);
    c.s_ip   = malloc(sizeof(i32) * (size_t)(n0 + 1));
    c.s_ix   = malloc(sizeof(i32) * (size_t)zpad);
    c.p_ip   = malloc(sizeof(i32) * (size_t)(n0 + 1));
    c.p_ix   = malloc(sizeof(i32) * (size_t)zpad);
    c.p_d    = malloc(sizeof(double) * (size_t)zpad);
    c.pt_ip  = malloc(sizeof(i32) * (size_t)(n0 + 1));
    c.pt_ix = malloc(sizeof(i32) * (size_t)zpad);
    c.pt_d   = malloc(sizeof(double) * (size_t)zpad);
    c.ap_ip  = malloc(sizeof(i32) * (size_t)(n0 + 1));
    c.ap_ix = malloc(sizeof(i32) * (size_t)zpad);
    c.ap_d   = malloc(sizeof(double) * (size_t)zpad);
    c.diag   = malloc(sizeof(double) * (size_t)n0);
    c.cnt    = malloc(sizeof(double) * (size_t)n0);
    c.tscale = malloc(sizeof(double) * (size_t)n0);
    c.vv     = malloc(sizeof(double) * (size_t)n0);
    c.ww     = malloc(sizeof(double) * (size_t)n0);
    c.agg    = malloc(sizeof(i32) * (size_t)n0);
    c.rowcnt = malloc(sizeof(i32) * (size_t)n0);
    c.tls    = malloc(sizeof(TLS) * (size_t)c.nt);
    for (int t = 0; t < c.nt; t++) {
        c.tls[t].mark  = malloc(sizeof(i32) * (size_t)(n0 + 1));
        c.tls[t].acc  = malloc(sizeof(double) * (size_t)(n0 + 1));
        c.tls[t].touch = malloc(sizeof(i32) * (size_t)(n0 + 1));
    }

    copy_in(&c, A_data, A_indices, A_indptr, n0, nnz0);

    i64 n = n0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    i64 depth = 1;
    i64 na = 0;

    for (int lv = 0; lv < LMAX - 1; lv++) {
        if (n > MAX_COARSE) {
            row_diagonal(&c, n);
            strength_graph(&c, n, theta);
            aggregate(c.s_ip, c.s_ix, n, c.agg, c.cnt, &na);
            if (na < n) {
                if (depth == 1) {
                    const i32 *restrict agg = c.agg;
                    #pragma omp parallel for schedule(static)
                    for (i64 i = 0; i < n0; i++)
                        agg0[i] = agg[i];
                }
                tentative_scale(&c, n, na);
                double rho = spectral_radius(&c, n);
                double omega = 4.0 / (3.0 * rho);
                smoothed_prolongation(&c, n, na, omega);
                csr_matmul(&c, c.cur_ip, c.cur_ix, c.cur_d,
                           c.p_ip, c.p_ix, c.p_d, n, na,
                           c.ap_ip, c.ap_ix, c.ap_d);
                csr_transpose(&c, c.p_ip, c.p_ix, c.p_d, n, na,
                              c.pt_ip, c.pt_ix, c.pt_d);
                csr_matmul(&c, c.pt_ip, c.pt_ix, c.pt_d,
                           c.ap_ip, c.ap_ix, c.ap_d, na, na,
                           c.cur_ip, c.cur_ix, c.cur_d);
                level_n[depth] = na;
                level_nnz[depth] = c.cur_ip[na];
                depth++;
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;

    for (int t = 0; t < c.nt; t++) {
        free(c.tls[t].mark); free(c.tls[t].acc); free(c.tls[t].touch);
    }
    free(c.cur_ip); free(c.cur_ix); free(c.cur_d);
    free(c.s_ip); free(c.s_ix);
    free(c.p_ip); free(c.p_ix); free(c.p_d);
    free(c.pt_ip); free(c.pt_ix); free(c.pt_d);
    free(c.ap_ip); free(c.ap_ix); free(c.ap_d);
    free(c.diag); free(c.cnt); free(c.tscale); free(c.vv); free(c.ww);
    free(c.agg); free(c.rowcnt); free(c.tls);
}
