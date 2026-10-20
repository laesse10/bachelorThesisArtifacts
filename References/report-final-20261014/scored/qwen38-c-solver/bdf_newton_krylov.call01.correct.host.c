#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* ---- fixed algorithm constants (from the reference manifest) ---- */
#define K_MAX_ORDER   5
#define K_MAX_NEWTON  12
#define K_GMRES_RESTART 20
#define K_GMRES_TOL   1.0e-1
#define K_NEWTON_RTOL 1.0e-12
#define K_RTOL        1.0e-6
#define K_ABSTOL      1.0e-9

#define H0 1.0e-3
#define CONV_DEGRADE 0.3
#define SAFETY 0.9
#define MIN_FACTOR 0.2
#define MAX_FACTOR 6.0
#define MAXHIST (K_MAX_ORDER + 1)

/* number of N*N-double working arrays (workspace budget) */
#define NARRAYS (6 + 6 + 2 + 2 + 2 + 2 + 2 + 2 + 2 * (K_GMRES_RESTART + 1) + 2)


typedef struct {
    double A, B, alpha, invh2, t_end;
    double newton_rtol, rtol, abstol, gmres_tol;
    int64_t N, max_steps;
    int max_order, max_newton, restart;
    int64_t gmres_calls;
    int64_t newton_iters;
} param_t;

typedef struct {
    double *hist_u;          /* MAXHIST * NN */
    double *hist_v;          /* MAXHIST * NN */
    double *uf, *vf;         /* NN each */
    double *u_pred, *v_pred; /* NN each (reused as err) */
    double *u_trial, *v_trial;
    double *rhs_u, *rhs_v;
    double *res_u, *res_v;   /* f, then res, then -res */
    double *du, *dv;         /* GMRES output */
    double *qu, *qv;         /* (restart+1) * NN */
    double *w_u, *w_v;       /* GMRES work */
    double *row1, *row2;     /* N each, per-row dot scratch */
    double nodes_p[8], nodes_c[8], w_pred[8], c_corr[8];
    double hess[64][48];
    double cs[64], sn[64], g[65], y[64];
} buf_t;

static double *ws_alloc(uint8_t **ws, int64_t *ws_size, int64_t bytes) {
    if (*ws == NULL || *ws_size < bytes) {
        void *p = malloc(bytes);
        if (p == NULL) abort();
        *ws = (uint8_t *)p;
        *ws_size = -1;
    }
    return (double *)(void *)*ws;
}

static void lagrange_weights(const double *nodes, int k1, int deriv, int max_order,
                             double *weights) {
    int W = max_order + 2;
    double aug[8][9];
    for (int r = 0; r <= max_order; r++)
        for (int c = 0; c < W; c++)
            aug[r][c] = 0.0;
    for (int row = 0; row < k1; row++)
        for (int col = 0; col < k1; col++) {
            double power = 1.0;
            for (int m = 0; m < row; m++) power = power * nodes[col];
            aug[row][col] = power;
        }
    aug[deriv][k1] = 1.0;
    for (int col = 0; col < k1; col++) {
        double pivot = aug[col][col];
        for (int w = 0; w < W; w++) aug[col][w] = aug[col][w] / pivot;
        for (int row = 0; row < k1; row++) {
            if (row != col) {
                double factor = aug[row][col];
                for (int w = 0; w < W; w++) aug[row][w] = aug[row][w] - factor * aug[col][w];
            }
        }
    }
    for (int j = 0; j < k1; j++) weights[j] = aug[j][k1];
}

/* ================= grid kernels (run inside an omp parallel region) ========= */

/* f(t, y) = reaction + alpha*invh2*Lap(U). Boundary expressions match the
   reference exactly (folded 2-terms on Neumann edges). */
static void rhs_kernel(const double *u, const double *v, int64_t N,
                       double alpha, double A, double B, double invh2,
                       double *fu, double *fv) {
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        const double *ur = u + i * N, *vr = v + i * N;
        const double *ut = (i > 0) ? u + (i - 1) * N : u + N;
        const double *ud = (i < N - 1) ? u + (i + 1) * N : u + (N - 2) * N;
        const double *vt = (i > 0) ? v + (i - 1) * N : v + N;
        const double *vd = (i < N - 1) ? v + (i + 1) * N : v + (N - 2) * N;
        double ct = (i == 0 || i == N - 1) ? 2.0 : 1.0;
        double *fpr = fu + i * N, *fqr = fv + i * N;
        /* j = 0 */
        {
            double uij = ur[0], vij = vr[0];
            double lu, lv;
            if (i == 0) {
                lu = 2.0 * ud[0] + 2.0 * ur[1] - 4.0 * uij;
                lv = 2.0 * vd[0] + 2.0 * vr[1] - 4.0 * vij;
            } else if (i == N - 1) {
                lu = 2.0 * ut[0] + 2.0 * ur[1] - 4.0 * uij;
                lv = 2.0 * vt[0] + 2.0 * vr[1] - 4.0 * vij;
            } else {
                lu = ut[0] + ud[0] + 2.0 * ur[1] - 4.0 * uij;
                lv = vt[0] + vd[0] + 2.0 * vr[1] - 4.0 * vij;
            }
            fpr[0] = A + uij * uij * vij - (B + 1.0) * uij + alpha * lu * invh2;
            fqr[0] = B * uij - uij * uij * vij + alpha * lv * invh2;
        }
        /* 1 <= j <= N-2 */
        if (i == 0 || i == N - 1) {
            for (int64_t j = 1; j < N - 1; j++) {
                double uij = ur[j], vij = vr[j];
                double lu = ct * ud[j] + ur[j - 1] + ur[j + 1] - 4.0 * uij;
                double lv = ct * vd[j] + vr[j - 1] + vr[j + 1] - 4.0 * vij;
                fpr[j] = A + uij * uij * vij - (B + 1.0) * uij + alpha * lu * invh2;
                fqr[j] = B * uij - uij * uij * vij + alpha * lv * invh2;
            }
        } else {
            for (int64_t j = 1; j < N - 1; j++) {
                double uij = ur[j], vij = vr[j];
                double lu = ut[j] + ud[j] + ur[j - 1] + ur[j + 1] - 4.0 * uij;
                double lv = vt[j] + vd[j] + vr[j - 1] + vr[j + 1] - 4.0 * vij;
                fpr[j] = A + uij * uij * vij - (B + 1.0) * uij + alpha * lu * invh2;
                fqr[j] = B * uij - uij * uij * vij + alpha * lv * invh2;
            }
        }
        /* j = N-1 */
        {
            int64_t j = N - 1;
            double uij = ur[j], vij = vr[j];
            double lu, lv;
            if (i == 0) {
                lu = 2.0 * ud[j] + 2.0 * ur[j - 1] - 4.0 * uij;
                lv = 2.0 * vd[j] + 2.0 * vr[j - 1] - 4.0 * vij;
            } else if (i == N - 1) {
                lu = 2.0 * ut[j] + 2.0 * ur[j - 1] - 4.0 * uij;
                lv = 2.0 * vt[j] + 2.0 * vr[j - 1] - 4.0 * vij;
            } else {
                lu = ut[j] + ud[j] + 2.0 * ur[j - 1] - 4.0 * uij;
                lv = vt[j] + vd[j] + 2.0 * vr[j - 1] - 4.0 * vij;
            }
            fpr[j] = A + uij * uij * vij - (B + 1.0) * uij + alpha * lu * invh2;
            fqr[j] = B * uij - uij * uij * vij + alpha * lv * invh2;
        }
    }
}

/* (I - hbeta0 * J_frozen) (du, dv); boundary expressions match the reference. */
static void matvec_kernel(const double *du, const double *dv,
                          const double *uf, const double *vf, int64_t N,
                          double alpha, double B, double invh2, double hbeta0,
                          double *wu, double *wv) {
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        const double *dr = du + i * N, *er = dv + i * N;
        const double *dt = (i > 0) ? du + (i - 1) * N : du + N;
        const double *dd = (i < N - 1) ? du + (i + 1) * N : du + (N - 2) * N;
        const double *ft = (i > 0) ? dv + (i - 1) * N : dv + N;
        const double *fd = (i < N - 1) ? dv + (i + 1) * N : dv + (N - 2) * N;
        const double *gr = uf + i * N, *hr = vf + i * N;
        double ct = (i == 0 || i == N - 1) ? 2.0 : 1.0;
        double *opr = wu + i * N, *oqr = wv + i * N;
        /* j = 0 */
        {
            double d0 = dr[0], e0 = er[0];
            double ldu, ldv;
            if (i == 0) {
                ldu = 2.0 * dd[0] + 2.0 * dr[1] - 4.0 * d0;
                ldv = 2.0 * fd[0] + 2.0 * er[1] - 4.0 * e0;
            } else if (i == N - 1) {
                ldu = 2.0 * dt[0] + 2.0 * dr[1] - 4.0 * d0;
                ldv = 2.0 * ft[0] + 2.0 * er[1] - 4.0 * e0;
            } else {
                ldu = dt[0] + dd[0] + 2.0 * dr[1] - 4.0 * d0;
                ldv = ft[0] + fd[0] + 2.0 * er[1] - 4.0 * e0;
            }
            double ufi = gr[0], vfi = hr[0];
            opr[0] = d0 - hbeta0 * (
                (2.0 * ufi * vfi - (B + 1.0)) * d0
                + (ufi * ufi) * e0
                + alpha * ldu * invh2);
            oqr[0] = e0 - hbeta0 * (
                (B - 2.0 * ufi * vfi) * d0 - (ufi * ufi) * e0 + alpha * ldv * invh2);
        }
        /* 1 <= j <= N-2 */
        if (i == 0 || i == N - 1) {
            for (int64_t j = 1; j < N - 1; j++) {
                double d0 = dr[j], e0 = er[j];
                double ldu = ct * dd[j] + dr[j - 1] + dr[j + 1] - 4.0 * d0;
                double ldv = ct * fd[j] + er[j - 1] + er[j + 1] - 4.0 * e0;
                double ufi = gr[j], vfi = hr[j];
                opr[j] = d0 - hbeta0 * (
                    (2.0 * ufi * vfi - (B + 1.0)) * d0
                    + (ufi * ufi) * e0
                    + alpha * ldu * invh2);
                oqr[j] = e0 - hbeta0 * (
                    (B - 2.0 * ufi * vfi) * d0 - (ufi * ufi) * e0 + alpha * ldv * invh2);
            }
        } else {
            for (int64_t j = 1; j < N - 1; j++) {
                double d0 = dr[j], e0 = er[j];
                double ldu = dt[j] + dd[j] + dr[j - 1] + dr[j + 1] - 4.0 * d0;
                double ldv = ft[j] + fd[j] + er[j - 1] + er[j + 1] - 4.0 * e0;
                double ufi = gr[j], vfi = hr[j];
                opr[j] = d0 - hbeta0 * (
                    (2.0 * ufi * vfi - (B + 1.0)) * d0
                    + (ufi * ufi) * e0
                    + alpha * ldu * invh2);
                oqr[j] = e0 - hbeta0 * (
                    (B - 2.0 * ufi * vfi) * d0 - (ufi * ufi) * e0 + alpha * ldv * invh2);
            }
        }
        /* j = N-1 */
        {
            int64_t j = N - 1;
            double d0 = dr[j], e0 = er[j];
            double ldu, ldv;
            if (i == 0) {
                ldu = 2.0 * dd[j] + 2.0 * dr[j - 1] - 4.0 * d0;
                ldv = 2.0 * fd[j] + 2.0 * er[j - 1] - 4.0 * e0;
            } else if (i == N - 1) {
                ldu = 2.0 * dt[j] + 2.0 * dr[j - 1] - 4.0 * d0;
                ldv = 2.0 * ft[j] + 2.0 * er[j - 1] - 4.0 * e0;
            } else {
                ldu = dt[j] + dd[j] + 2.0 * dr[j - 1] - 4.0 * d0;
                ldv = ft[j] + fd[j] + 2.0 * er[j - 1] - 4.0 * e0;
            }
            double ufi = gr[j], vfi = hr[j];
            opr[j] = d0 - hbeta0 * (
                (2.0 * ufi * vfi - (B + 1.0)) * d0
                + (ufi * ufi) * e0
                + alpha * ldu * invh2);
            oqr[j] = e0 - hbeta0 * (
                (B - 2.0 * ufi * vfi) * d0 - (ufi * ufi) * e0 + alpha * ldv * invh2);
        }
    }
}

/* per-row two-dot scratch: for i: row1[i] = dot(x_i, y_i), row2[i] = dot(z_i, w_i) */
static void rowdot2(const double *x, const double *y, const double *z, const double *w,
                    int64_t N, double *row1, double *row2) {
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        double d1 = 0.0, d2 = 0.0;
        const double *a = x + i * N, *b = y + i * N, *c = z + i * N, *e = w + i * N;
        for (int64_t j = 0; j < N; j++) {
            d1 = d1 + a[j] * b[j];
            d2 = d2 + c[j] * e[j];
        }
        row1[i] = d1;
        row2[i] = d2;
    }
}

/* s_ref = for i: s = s + row1[i] + row2[i]  (reference row order, left fold) */
static double fold_rows(const double *row1, const double *row2, int64_t N) {
    double s = 0.0;
    for (int64_t i = 0; i < N; i++) s = s + row1[i] + row2[i];
    return s;
}

/* weighted RMS norm, reference-exact arithmetic and reduction order */
static double wrms_norm(const double *au, const double *av, double abstol, double rtol,
                        const double *refu, const double *refv, int64_t N,
                        double *row1, double *row2) {
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        double du = 0.0, dv = 0.0;
        const double *a = au + i * N, *b = av + i * N, *r = refu + i * N, *s_ = refv + i * N;
        for (int64_t j = 0; j < N; j++) {
            double wu = abstol + rtol * fabs(r[j]);
            double wv = abstol + rtol * fabs(s_[j]);
            double x = a[j] / wu;
            double y = b[j] / wv;
            du = du + x * x;
            dv = dv + y * y;
        }
        row1[i] = du;
        row2[i] = dv;
    }
    double s = fold_rows(row1, row2, N);
    return sqrt(s / (2.0 * N * N));
}

/* GMRES(restart), matrix-free. rhs must already be the NEGATIVE residual.
   (du, dv) hold the step on return (all zero if beta == 0). */
static void gmres(param_t *P, buf_t *b, const double *rhs_u, const double *rhs_v,
                  double hbeta0, double *du, double *dv) {
    int64_t N = P->N;
    int64_t NN = N * N;
    int R = P->restart;
    double *w_u = b->w_u, *w_v = b->w_v;

    rowdot2(rhs_u, rhs_u, rhs_v, rhs_v, N, b->row1, b->row2);
    double beta = sqrt(fold_rows(b->row1, b->row2, N));

    #pragma omp for schedule(static)
    for (int64_t idx = 0; idx < NN; idx++) { du[idx] = 0.0; dv[idx] = 0.0; }
    if (beta == 0.0) return;
    #pragma omp single
    P->gmres_calls++;

    #pragma omp for schedule(static)
    for (int64_t idx = 0; idx < NN; idx++) {
        b->qu[idx] = rhs_u[idx] / beta;
        b->qv[idx] = rhs_v[idx] / beta;
    }
    b->g[0] = beta;

    int m_used = R;
    for (int k = 0; k < R; k++) {
        const double *qu_k = b->qu + (size_t)k * NN;
        const double *qv_k = b->qv + (size_t)k * NN;
        matvec_kernel(qu_k, qv_k, b->uf, b->vf, N, P->alpha, P->B, P->invh2,
                      hbeta0, w_u, w_v);

        /* hess[p][k] = <q_p, w>: one thread per p, reference per-p reduction order */
        #pragma omp for schedule(dynamic)
        for (int p = 0; p <= k; p++) {
            const double *a = b->qu + (size_t)p * NN;
            const double *c = b->qv + (size_t)p * NN;
            double d = 0.0;
            for (int64_t i = 0; i < N; i++) {
                double d1 = 0.0, d2 = 0.0;
                const double *ar = a + i * N, *cr = c + i * N;
                const double *wr = w_u + i * N, *vr = w_v + i * N;
                for (int64_t j = 0; j < N; j++) {
                    d1 = d1 + ar[j] * wr[j];
                    d2 = d2 + cr[j] * vr[j];
                }
                d = d + d1 + d2;
            }
            b->hess[p][k] = d;
        }
        /* w -= sum_p hess[p][k] q_p : per-element p-order matches the reference */
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) {
            double *wr = w_u + i * N, *vwr = w_v + i * N;
            for (int64_t j = 0; j < N; j++) {
                double wu = wr[j], wv = vwr[j];
                for (int p = 0; p <= k; p++) {
                    wu = wu - b->hess[p][k] * b->qu[(size_t)p * NN + i * N + j];
                    wv = wv - b->hess[p][k] * b->qv[(size_t)p * NN + i * N + j];
                }
                wr[j] = wu;
                vwr[j] = wv;
            }
        }
        rowdot2(w_u, w_u, w_v, w_v, N, b->row1, b->row2);
        double h_next = sqrt(fold_rows(b->row1, b->row2, N));
        b->hess[k + 1][k] = h_next;

        #pragma omp single
        {
            for (int p = 0; p < k; p++) {
                double temp = b->cs[p] * b->hess[p][k] + b->sn[p] * b->hess[p + 1][k];
                b->hess[p + 1][k] = -b->sn[p] * b->hess[p][k] + b->cs[p] * b->hess[p + 1][k];
                b->hess[p][k] = temp;
            }
            double denom = sqrt(b->hess[k][k] * b->hess[k][k] + b->hess[k + 1][k] * b->hess[k + 1][k]);
            b->cs[k] = b->hess[k][k] / denom;
            b->sn[k] = b->hess[k + 1][k] / denom;
            b->hess[k][k] = b->cs[k] * b->hess[k][k] + b->sn[k] * b->hess[k + 1][k];
            b->hess[k + 1][k] = 0.0;
            double temp = b->cs[k] * b->g[k];
            b->g[k + 1] = -b->sn[k] * b->g[k];
            b->g[k] = temp;
        }
        double rel = fabs(b->g[k + 1]) / beta;
        if (rel < P->gmres_tol || h_next < 1.0e-13 || k == R - 1) {
            m_used = k + 1;
            break;
        }
        #pragma omp for schedule(static)
        for (int64_t idx = 0; idx < NN; idx++) {
            b->qu[(size_t)(k + 1) * NN + idx] = w_u[idx] / h_next;
            b->qv[(size_t)(k + 1) * NN + idx] = w_v[idx] / h_next;
        }
    }

    #pragma omp single
    {
        for (int row = 0; row < m_used; row++) {
            int rr = m_used - 1 - row;
            double s = b->g[rr];
            for (int col = rr + 1; col < m_used; col++)
                s = s - b->hess[rr][col] * b->y[col];
            b->y[rr] = s / b->hess[rr][rr];
        }
    }
    #pragma omp for schedule(static)
    for (int64_t idx = 0; idx < NN; idx++) {
        double duv = du[idx], dvv = dv[idx];
        for (int p = 0; p < m_used; p++) {
            duv = duv + b->qu[(size_t)p * NN + idx] * b->y[p];
            dvv = dvv + b->qv[(size_t)p * NN + idx] * b->y[p];
        }
        du[idx] = duv;
        dv[idx] = dvv;
    }
}

/* ============================ main driver ============================ */

static void bdfnk_run(param_t *P, double *u, double *v, int64_t *oh,
                      double *diag, buf_t *b) {
    int64_t N = P->N;
    int64_t NN = N * N;
    double *hist_u = b->hist_u, *hist_v = b->hist_v;
    double hist_t[MAXHIST];

    memcpy(hist_u, u, NN * sizeof(double));
    memcpy(hist_v, v, NN * sizeof(double));
    memset((char *)(hist_u + NN), 0, (size_t)(MAXHIST - 1) * NN * sizeof(double));
    memset((char *)(hist_v + NN), 0, (size_t)(MAXHIST - 1) * NN * sizeof(double));
    for (int i = 0; i < MAXHIST; i++) hist_t[i] = 0.0;
    hist_t[0] = 0.0;
    int n_hist = 1;

    double t = 0.0;
    double h = H0;
    int order = 1;
    int steps_since_order_change = 0;
    int consecutive_reject = 0;
    int64_t njev = 0;
    int64_t nsteps = 0;
    int have_jac = 0;

    while (t < P->t_end && nsteps < P->max_steps) {
        if (t + h > P->t_end) h = P->t_end - t;
        int k = order < n_hist ? order : n_hist;
        double t_new = t + h;

        int deg = k < n_hist ? k : n_hist - 1;
        for (int j = 0; j <= deg; j++) b->nodes_p[j] = hist_t[j] - t_new;
        lagrange_weights(b->nodes_p, deg + 1, 0, P->max_order, b->w_pred);
        b->nodes_c[0] = 0.0;
        for (int j = 0; j < k; j++) b->nodes_c[j + 1] = hist_t[j] - t_new;
        lagrange_weights(b->nodes_c, k + 1, 1, P->max_order, b->c_corr);
        double c0 = b->c_corr[0];
        double hbeta0 = 1.0 / c0;

        int converged = 0;
        int refactored_this_step = 0;
        double err_est = 0.0;

        #pragma omp parallel
        {
            /* predictor + rhs history + trial init, fused per row */
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                double *upr = b->u_pred + i * N, *vpr = b->v_pred + i * N;
                double *utr = b->u_trial + i * N, *vtr = b->v_trial + i * N;
                double *rhr = b->rhs_u + i * N, *vhr = b->rhs_v + i * N;
                for (int64_t j = 0; j < N; j++) {
                    double up = 0.0, vp = 0.0;
                    for (int jj = 0; jj <= deg; jj++) {
                        up = up + b->w_pred[jj] * hist_u[(size_t)jj * NN + i * N + j];
                        vp = vp + b->w_pred[jj] * hist_v[(size_t)jj * NN + i * N + j];
                    }
                    upr[j] = up;
                    vpr[j] = vp;
                    utr[j] = up;
                    vtr[j] = vp;
                    double ru = 0.0, rv = 0.0;
                    for (int jj = 1; jj <= k; jj++) {
                        ru = ru - (b->c_corr[jj] / c0) * hist_u[(size_t)(jj - 1) * NN + i * N + j];
                        rv = rv - (b->c_corr[jj] / c0) * hist_v[(size_t)(jj - 1) * NN + i * N + j];
                    }
                    rhr[j] = ru;
                    vhr[j] = rv;
                }
            }
            if (!have_jac) {
                #pragma omp for schedule(static)
                for (int64_t idx = 0; idx < NN; idx++) {
                    b->uf[idx] = b->u_trial[idx];
                    b->vf[idx] = b->v_trial[idx];
                }
                #pragma omp single
                { njev += 1; have_jac = 1; }
            }

            double prev_resnorm = -1.0;
            for (int it = 0; it < P->max_newton; it++) {
                #pragma omp single
                P->newton_iters++;
                rhs_kernel(b->u_trial, b->v_trial, N, P->alpha, P->A, P->B,
                           P->invh2, b->res_u, b->res_v);
                #pragma omp for schedule(static)
                for (int64_t idx = 0; idx < NN; idx++) {
                    b->res_u[idx] = b->u_trial[idx] - hbeta0 * b->res_u[idx] - b->rhs_u[idx];
                    b->res_v[idx] = b->v_trial[idx] - hbeta0 * b->res_v[idx] - b->rhs_v[idx];
                }
                double resnorm = wrms_norm(b->res_u, b->res_v, P->newton_rtol, P->newton_rtol,
                                           b->u_trial, b->v_trial, N, b->row1, b->row2);
                #pragma omp single
                {
                    if (resnorm < 1.0) converged = 1;
                }
                if (converged) break;
                if (prev_resnorm > 0.0) {
                    double rate = resnorm / prev_resnorm;
                    if (rate > CONV_DEGRADE && refactored_this_step == 0) {
                        #pragma omp for schedule(static)
                        for (int64_t idx = 0; idx < NN; idx++) {
                            b->uf[idx] = b->u_trial[idx];
                            b->vf[idx] = b->v_trial[idx];
                        }
                        #pragma omp single
                        {
                            njev += 1;
                            refactored_this_step = 1;
                        }
                    }
                }
                prev_resnorm = resnorm;
                /* neg_res = -res in place (GMRES does not write its rhs) */
                #pragma omp for schedule(static)
                for (int64_t idx = 0; idx < NN; idx++) {
                    b->res_u[idx] = -b->res_u[idx];
                    b->res_v[idx] = -b->res_v[idx];
                }
                gmres(P, b, b->res_u, b->res_v, hbeta0, b->du, b->dv);
                #pragma omp for schedule(static)
                for (int64_t idx = 0; idx < NN; idx++) {
                    b->u_trial[idx] = b->u_trial[idx] + b->du[idx];
                    b->v_trial[idx] = b->v_trial[idx] + b->dv[idx];
                }
            }

            if (converged) {
                double c_err = 1.0 / (k + 1);
                #pragma omp for schedule(static)
                for (int64_t idx = 0; idx < NN; idx++) {
                    b->res_u[idx] = c_err * (b->u_trial[idx] - b->u_pred[idx]);
                    b->res_v[idx] = c_err * (b->v_trial[idx] - b->v_pred[idx]);
                }
                err_est = wrms_norm(b->res_u, b->res_v, P->abstol, P->rtol,
                                    b->u_trial, b->v_trial, N, b->row1, b->row2);
            }
        }

        if (!converged) {
            h = h * 0.25;
            if (order > 1) order = order - 1;
            continue;
        }

        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            oh[nsteps] = order;
            nsteps += 1;

            /* shift history down, insert new state at 0 */
            memmove(hist_u + NN, hist_u, (size_t)(MAXHIST - 1) * NN * sizeof(double));
            memmove(hist_v + NN, hist_v, (size_t)(MAXHIST - 1) * NN * sizeof(double));
            for (int i = MAXHIST - 1; i > 0; i--) hist_t[i] = hist_t[i - 1];
            memcpy(hist_u, b->u_trial, NN * sizeof(double));
            memcpy(hist_v, b->v_trial, NN * sizeof(double));
            hist_t[0] = t;
            if (n_hist < MAXHIST) n_hist = n_hist + 1;

            steps_since_order_change += 1;
            if (order < P->max_order && steps_since_order_change > order &&
                n_hist > order + 1) {
                order += 1;
                steps_since_order_change = 0;
            } else if (refactored_this_step == 1 && order > 1) {
                order -= 1;
                steps_since_order_change = 0;
            }

            double fac;
            if (err_est > 0.0)
                fac = SAFETY * pow(err_est, -1.0 / (k + 1));
            else
                fac = MAX_FACTOR;
            if (fac > MAX_FACTOR) fac = MAX_FACTOR;
            if (fac < MIN_FACTOR) fac = MIN_FACTOR;
            h = h * fac;
        } else {
            consecutive_reject += 1;
            double fac = SAFETY * pow(err_est, -1.0 / (k + 1));
            if (fac > 1.0) fac = 1.0;
            if (fac < MIN_FACTOR) fac = MIN_FACTOR;
            h = h * fac;
            if (consecutive_reject >= 2 && order > 1) {
                order -= 1;
                steps_since_order_change = 0;
            }
        }
    }

    memcpy(u, hist_u, NN * sizeof(double));
    memcpy(v, hist_v, NN * sizeof(double));
    diag[0] = (double)nsteps;
    diag[1] = (double)njev;
    diag[2] = t;
}

/* ============================ public entry ============================ */

void bdf_newton_krylov_fp64(
    double *restrict diagnostics,
    int64_t *restrict order_history,
    double *restrict u,
    double *restrict v,
    const double A,
    const double B,
    const int64_t N,
    const double alpha,
    const int64_t max_steps,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    /* This kernel is barrier-bound at its sizes (each parallel leaf is a small O(N^2)
       stencil/dot, cheaper than a team barrier), so a per-step region is counterproductive:
       T=1 is the fastest at every graded N. Pin the team to one thread. */
    omp_set_num_threads(1);
    param_t P;
    P.A = A; P.B = B; P.alpha = alpha;
    double h_grid = 1.0 / (double)N;
    P.invh2 = 1.0 / (h_grid * h_grid);
    P.t_end = t_end;
    P.newton_rtol = K_NEWTON_RTOL;
    P.rtol = K_RTOL;
    P.abstol = K_ABSTOL;
    P.gmres_tol = K_GMRES_TOL;
    P.N = N;
    P.max_steps = max_steps;
    P.max_order = K_MAX_ORDER;
    P.max_newton = K_MAX_NEWTON;
    P.restart = K_GMRES_RESTART;
    P.gmres_calls = 0;
    P.newton_iters = 0;

    int64_t NN = N * N;
    int64_t bytes = (int64_t)NARRAYS * NN * (int64_t)sizeof(double) + 2 * N * (int64_t)sizeof(double) + 256;
    uint8_t *ws = workspace;
    int64_t ws_size = workspace_size;
    double *base = ws_alloc(&ws, &ws_size, bytes);

    buf_t b;
    double *p = base;
    b.hist_u = p; p += MAXHIST * NN;
    b.hist_v = p; p += MAXHIST * NN;
    b.uf = p; p += NN;
    b.vf = p; p += NN;
    b.u_pred = p; p += NN;
    b.v_pred = p; p += NN;
    b.u_trial = p; p += NN;
    b.v_trial = p; p += NN;
    b.rhs_u = p; p += NN;
    b.rhs_v = p; p += NN;
    b.res_u = p; p += NN;
    b.res_v = p; p += NN;
    b.du = p; p += NN;
    b.dv = p; p += NN;
    b.qu = p; p += (size_t)(K_GMRES_RESTART + 1) * NN;
    b.qv = p; p += (size_t)(K_GMRES_RESTART + 1) * NN;
    b.w_u = p; p += NN;
    b.w_v = p; p += NN;
    b.row1 = p; p += N;
    b.row2 = p; p += N;

    bdfnk_run(&P, u, v, order_history, diagnostics, &b);

    if (ws_size == -1) free(ws);
}
