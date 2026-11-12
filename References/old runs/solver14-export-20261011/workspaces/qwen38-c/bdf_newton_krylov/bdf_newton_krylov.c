#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Fixed algorithmic constants (the values the reference is run with). */
#ifndef RTOL_REF
#define RTOL_REF 1e-8
#endif
#ifndef ABSTOL_REF
#define ABSTOL_REF 1e-8
#endif
#ifndef NEWTON_RTOL
#define NEWTON_RTOL 1e-12
#endif
#ifndef MAX_ORDER
#define MAX_ORDER 5
#endif
#ifndef MAX_NEWTON
#define MAX_NEWTON 12
#endif
#ifndef GMRES_RESTART
#define GMRES_RESTART 30
#endif
#ifndef GMRES_TOL
#define GMRES_TOL 1e-1
#endif
#ifndef OMP_CAP
#define OMP_CAP 8
#endif

static const double H0 = 1.0e-3;
static const double CONV_DEGRADE = 0.3;
static const double SAFETY = 0.9;
static const double MIN_FACTOR = 0.2;
static const double MAX_FACTOR = 6.0;

/*
 * Parallelism model: a SINGLE persistent team (created once) drives the whole
 * integration in lockstep. The big N^2 arrays are shared memory written by
 * worksharing `omp for` loops; the small arrays and the step-control scalars are
 * thread-private (every thread recomputes the identical deterministic value).
 * This avoids the ~1e5 team create/destroy (clone3) calls of per-region
 * `parallel for`, which dominated the runtime.
 */

/* 5-point Neumann Laplacian (one-sided doubled boundary). */
static inline void neumann_laplacian(const double *U, int64_t N, double *L) {
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t iu = (i > 0) ? i - 1 : i + 1;
        const int64_t id = (i < N - 1) ? i + 1 : i - 1;
        const double *Uu = U + iu * N, *Ud = U + id * N, *Uc = U + i * N;
        double *Lc = L + i * N;
        for (int64_t j = 0; j < N; ++j) {
            const int64_t iw = (j > 0) ? j - 1 : j + 1;
            const int64_t ie = (j < N - 1) ? j + 1 : j - 1;
            Lc[j] = Uu[j] + Ud[j] + Uc[iw] + Uc[ie] - 4.0 * Uc[j];
        }
    }
}

/* f(t,y): Brusselator reaction-diffusion. h is the spatial grid spacing (1/N). */
static inline void brusselator_rhs(const double *u, const double *v, int64_t N, double h,
                                   double alpha, double A, double B, double *du, double *dv) {
    const double invh2 = 1.0 / (h * h);
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t iu = (i > 0) ? i - 1 : i + 1;
        const int64_t id = (i < N - 1) ? i + 1 : i - 1;
        const double *Uu = u + iu * N, *Ud = u + id * N, *Uc = u + i * N;
        const double *Vu = v + iu * N, *Vd = v + id * N, *Vc = v + i * N;
        double *duc = du + i * N, *dvc = dv + i * N;
        for (int64_t j = 0; j < N; ++j) {
            const int64_t iw = (j > 0) ? j - 1 : j + 1;
            const int64_t ie = (j < N - 1) ? j + 1 : j - 1;
            const double uc = Uc[j], vc = Vc[j];
            const double lap_u = Uu[j] + Ud[j] + Uc[iw] + Uc[ie] - 4.0 * uc;
            const double lap_v = Vu[j] + Vd[j] + Vc[iw] + Vc[ie] - 4.0 * vc;
            duc[j] = A + uc * uc * vc - (B + 1.0) * uc + alpha * lap_u * invh2;
            dvc[j] = B * uc - uc * uc * vc + alpha * lap_v * invh2;
        }
    }
}

/* (I - hbeta0 * J) (du,dv), J exact/linear at frozen state (uf,vf). h is the spatial grid spacing. */
static inline void newton_matvec(const double *du, const double *dv, const double *uf, const double *vf,
                                 int64_t N, double h, double alpha, double B, double hbeta0,
                                 double *out_du, double *out_dv) {
    const double invh2 = 1.0 / (h * h);
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t iu = (i > 0) ? i - 1 : i + 1;
        const int64_t id = (i < N - 1) ? i + 1 : i - 1;
        const double *Duu = du + iu * N, *Dud = du + id * N, *Duc = du + i * N;
        const double *Dvu = dv + iu * N, *Dvd = dv + id * N, *Dvc = dv + i * N;
        const double *Ufc = uf + i * N, *Vfc = vf + i * N;
        double *oduc = out_du + i * N, *odvc = out_dv + i * N;
        for (int64_t j = 0; j < N; ++j) {
            const int64_t iw = (j > 0) ? j - 1 : j + 1;
            const int64_t ie = (j < N - 1) ? j + 1 : j - 1;
            const double uc = Ufc[j], vc = Vfc[j], dc = Duc[j], dvj = Dvc[j];
            const double lap_du = Duu[j] + Dud[j] + Duc[iw] + Duc[ie] - 4.0 * dc;
            const double lap_dv = Dvu[j] + Dvd[j] + Dvc[iw] + Dvc[ie] - 4.0 * dvj;
            const double du_part = (2.0 * uc * vc - (B + 1.0)) * dc + (uc * uc) * dvj + alpha * lap_du * invh2;
            const double dv_part = (B - 2.0 * uc * vc) * dc - (uc * uc) * dvj + alpha * lap_dv * invh2;
            oduc[j] = dc - hbeta0 * du_part;
            odvc[j] = dvj - hbeta0 * dv_part;
        }
    }
}

/* Weighted-RMS norm over both fields. */
static inline double wrms_norm(const double *au, const double *av, double abstol, double rtol,
                               const double *refu, const double *refv, int64_t N, double *red) {
    const int tid = omp_get_thread_num();
    red[tid] = 0.0;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const double *auc = au + i * N, *avc = av + i * N, *ruc = refu + i * N, *rvc = refv + i * N;
        for (int64_t j = 0; j < N; ++j) {
            const double wu = abstol + rtol * fabs(ruc[j]);
            const double wv = abstol + rtol * fabs(rvc[j]);
            const double x = auc[j] / wu;
            const double y = avc[j] / wv;
            red[tid] += x * x + y * y;
        }
    }
    double rs = 0.0;
    for (int t = 0; t < omp_get_num_threads(); ++t) rs += red[t];
    return sqrt(rs / (2.0 * N * N));
}

/* BDF / predictor Lagrange weights via Gaussian elimination (no pivoting). */
static inline void lagrange_weights(const double *nodes, int k1, int deriv, int max_order, double *weights) {
    double aug[8][9];
    memset(aug, 0, sizeof(aug));
    for (int row = 0; row < k1; ++row)
        for (int col = 0; col < k1; ++col) {
            double power = 1.0;
            for (int r = 0; r < row; ++r) power = power * nodes[col];
            aug[row][col] = power;
        }
    aug[deriv][k1] = 1.0;
    const int W = max_order + 2;
    for (int col = 0; col < k1; ++col) {
        double pivot = aug[col][col];
        for (int w = 0; w < W; ++w) aug[col][w] = aug[col][w] / pivot;
        for (int row = 0; row < k1; ++row)
            if (row != col) {
                double factor = aug[row][col];
                for (int w = 0; w < W; ++w) aug[row][w] = aug[row][w] - factor * aug[col][w];
            }
    }
    for (int j = 0; j < k1; ++j) weights[j] = aug[j][k1];
}

/* GMRES(restart), matrix-free. qu/qv: (restart+1)*N2. hess flat: hess[p*restart+k]. */
static void gmres_matfree(double *rhs_u, double *rhs_v, const double *uf, const double *vf,
                          int64_t N, double h, double alpha, double B, double hbeta0,
                          int restart, double tol, double *du, double *dv,
                          double *qu, double *qv, double *wu, double *wv,
                          double *hess, double *cs, double *sn, double *g, double *y, double *red) {
    const int64_t N2 = N * N;
    memset(cs, 0, (size_t)restart * sizeof(double));
    memset(sn, 0, (size_t)restart * sizeof(double));
    memset(g, 0, (size_t)(restart + 1) * sizeof(double));
    memset(y, 0, (size_t)restart * sizeof(double));
    memset(hess, 0, (size_t)(restart + 1) * (size_t)restart * sizeof(double));
    const int tid = omp_get_thread_num();

    red[tid] = 0.0;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N2; ++i) red[tid] += rhs_u[i] * rhs_u[i] + rhs_v[i] * rhs_v[i];
    double rs = 0.0;
    for (int t = 0; t < omp_get_num_threads(); ++t) rs += red[t];
    double beta = sqrt(rs);
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N2; ++i) { du[i] = 0.0; dv[i] = 0.0; }
    if (beta == 0.0) return;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N2; ++i) { qu[i] = rhs_u[i] / beta; qv[i] = rhs_v[i] / beta; }
    g[0] = beta;
    int m_used = restart;
    for (int k = 0; k < restart; ++k) {
        newton_matvec(qu + (int64_t)k * N2, qv + (int64_t)k * N2, uf, vf, N, h, alpha, B, hbeta0, wu, wv);
        for (int p = 0; p <= k; ++p) {
            double *qup = qu + (int64_t)p * N2;
            double *qvp = qv + (int64_t)p * N2;
            red[tid] = 0.0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N2; ++i) red[tid] += qup[i] * wu[i] + qvp[i] * wv[i];
            double rpk = 0.0;
            for (int t = 0; t < omp_get_num_threads(); ++t) rpk += red[t];
            hess[p * restart + k] = rpk;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N2; ++i) { wu[i] -= rpk * qup[i]; wv[i] -= rpk * qvp[i]; }
        }
        red[tid] = 0.0;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N2; ++i) red[tid] += wu[i] * wu[i] + wv[i] * wv[i];
        double rhn = 0.0;
        for (int t = 0; t < omp_get_num_threads(); ++t) rhn += red[t];
        const double h_next = sqrt(rhn);
        hess[(k + 1) * restart + k] = h_next;
        for (int p = 0; p < k; ++p) {
            double temp = cs[p] * hess[p * restart + k] + sn[p] * hess[(p + 1) * restart + k];
            hess[(p + 1) * restart + k] = -sn[p] * hess[p * restart + k] + cs[p] * hess[(p + 1) * restart + k];
            hess[p * restart + k] = temp;
        }
        double denom = sqrt(hess[k * restart + k] * hess[k * restart + k] + hess[(k + 1) * restart + k] * hess[(k + 1) * restart + k]);
        cs[k] = hess[k * restart + k] / denom;
        sn[k] = hess[(k + 1) * restart + k] / denom;
        hess[k * restart + k] = cs[k] * hess[k * restart + k] + sn[k] * hess[(k + 1) * restart + k];
        hess[(k + 1) * restart + k] = 0.0;
        double temp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = temp;
        double rel = fabs(g[k + 1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart - 1) { m_used = k + 1; break; }
        double *quk1 = qu + (int64_t)(k + 1) * N2;
        double *qvk1 = qv + (int64_t)(k + 1) * N2;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N2; ++i) { quk1[i] = wu[i] / h_next; qvk1[i] = wv[i] / h_next; }
    }
    for (int row = 0; row < m_used; ++row) {
        int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; ++col) s = s - hess[rr * restart + col] * y[col];
        y[rr] = s / hess[rr * restart + rr];
    }
    for (int p = 0; p < m_used; ++p) {
        double *qup = qu + (int64_t)p * N2;
        double *qvp = qv + (int64_t)p * N2;
        double yp = y[p];
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N2; ++i) { du[i] += qup[i] * yp; dv[i] += qvp[i] * yp; }
    }
}

void bdf_newton_krylov_fp64(
    double *restrict diagnostics, int64_t *restrict order_history, double *restrict u, double *restrict v,
    const double A, const double B, const int64_t N, const double alpha, const int64_t max_steps,
    const double t_end, uint8_t *restrict workspace, const int64_t workspace_size) {
    const int64_t N2 = N * N;
    const int max_order = MAX_ORDER;
    const int maxhist = max_order + 1;
    const int R = GMRES_RESTART;
    const double h_grid = 1.0 / (double)N;

    int64_t nd = (int64_t)maxhist * 2 * N2 + maxhist + (int64_t)maxhist * 2 + (int64_t)maxhist * 2;
    nd += 2 * N2 * 9;
    nd += (int64_t)(R + 1) * N2 * 2 + 2 * N2;
    nd += (int64_t)(R + 1) * R + R + R + (R + 1) + R;
    const int64_t need_bytes = nd * (int64_t)sizeof(double);
    const int use_ws = (workspace != NULL) && (workspace_size >= need_bytes);
    double *m = use_ws ? (double *)workspace : (double *)malloc(need_bytes);
    if (m == NULL) return;

    int64_t off = 0;
    double *hist_u = m + off; off += (int64_t)maxhist * N2;
    double *hist_v = m + off; off += (int64_t)maxhist * N2;
    double *uf = m + off; off += N2;
    double *vf = m + off; off += N2;
    double *u_pred = m + off; off += N2;
    double *v_pred = m + off; off += N2;
    double *u_trial = m + off; off += N2;
    double *v_trial = m + off; off += N2;
    double *rhs_u = m + off; off += N2;
    double *rhs_v = m + off; off += N2;
    double *f_u = m + off; off += N2;
    double *f_v = m + off; off += N2;
    double *res_u = m + off; off += N2;
    double *res_v = m + off; off += N2;
    double *neg_res_u = m + off; off += N2;
    double *neg_res_v = m + off; off += N2;
    double *err_u = m + off; off += N2;
    double *err_v = m + off; off += N2;
    double *step_du = m + off; off += N2;
    double *step_dv = m + off; off += N2;
    double *qu = m + off; off += (int64_t)(R + 1) * N2;
    double *qv = m + off; off += (int64_t)(R + 1) * N2;
    double *wu = m + off; off += N2;
    double *wv = m + off; off += N2;
    double *red = m + off; off += 16;

    int nt = omp_get_max_threads();
    if (nt > OMP_CAP) nt = OMP_CAP;
    if (nt < 1) nt = 1;

    #pragma omp parallel num_threads(nt)
    {
        /* thread-private small state + step-control scalars (recomputed identically on every thread) */
        double hist_t[8];
        double nodes_p[8], nodes_c[8], w_pred[8], c_corr[8];
        double hess[(R + 1) * R];
        double cs[R], sn[R], g[R + 1], y[R];
        for (int j = 0; j < maxhist; ++j) hist_t[j] = 0.0;

        int n_hist = 1;
        double t = 0.0;
        double h = H0;
        int order = 1;
        int steps_since_order_change = 0;
        int consecutive_reject = 0;
        int64_t njev = 0;
        int64_t nsteps = 0;
        int have_jac = 0;

        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N2; ++i) { hist_u[i] = u[i]; hist_v[i] = v[i]; }

        while (t < t_end && nsteps < max_steps) {
            if (t + h > t_end) h = t_end - t;
            const int k = order < n_hist ? order : n_hist;
            const double t_new = t + h;
            const int deg = k < (n_hist - 1) ? k : (n_hist - 1);
            for (int j = 0; j <= deg; ++j) nodes_p[j] = hist_t[j] - t_new;
            lagrange_weights(nodes_p, deg + 1, 0, max_order, w_pred);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N2; ++i) { u_pred[i] = 0.0; v_pred[i] = 0.0; }
            for (int j = 0; j <= deg; ++j) {
                const double wj = w_pred[j];
                const double *hu = hist_u + (int64_t)j * N2;
                const double *hv = hist_v + (int64_t)j * N2;
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) { u_pred[i] += wj * hu[i]; v_pred[i] += wj * hv[i]; }
            }
            nodes_c[0] = 0.0;
            for (int j = 0; j < k; ++j) nodes_c[j + 1] = hist_t[j] - t_new;
            lagrange_weights(nodes_c, k + 1, 1, max_order, c_corr);
            const double c0 = c_corr[0];
            const double hbeta0 = 1.0 / c0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N2; ++i) { rhs_u[i] = 0.0; rhs_v[i] = 0.0; }
            for (int j = 1; j <= k; ++j) {
                const double cj = c_corr[j] / c0;
                const double *hu = hist_u + (int64_t)(j - 1) * N2;
                const double *hv = hist_v + (int64_t)(j - 1) * N2;
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) { rhs_u[i] -= cj * hu[i]; rhs_v[i] -= cj * hv[i]; }
            }
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N2; ++i) { u_trial[i] = u_pred[i]; v_trial[i] = v_pred[i]; }
            if (have_jac == 0) {
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) { uf[i] = u_trial[i]; vf[i] = v_trial[i]; }
                njev += 1;
                have_jac = 1;
            }

            int converged = 0;
            int refactored_this_step = 0;
            double prev_resnorm = -1.0;
            for (int it = 0; it < MAX_NEWTON; ++it) {
                brusselator_rhs(u_trial, v_trial, N, h_grid, alpha, A, B, f_u, f_v);
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) {
                    res_u[i] = u_trial[i] - hbeta0 * f_u[i] - rhs_u[i];
                    res_v[i] = v_trial[i] - hbeta0 * f_v[i] - rhs_v[i];
                }
                const double resnorm = wrms_norm(res_u, res_v, NEWTON_RTOL, NEWTON_RTOL, u_trial, v_trial, N, red);
                if (resnorm < 1.0) { converged = 1; break; }
                if (prev_resnorm > 0.0) {
                    const double rate = resnorm / prev_resnorm;
                    if (rate > CONV_DEGRADE && refactored_this_step == 0) {
                        #pragma omp for schedule(static)
                        for (int64_t i = 0; i < N2; ++i) { uf[i] = u_trial[i]; vf[i] = v_trial[i]; }
                        njev += 1;
                        refactored_this_step = 1;
                    }
                }
                prev_resnorm = resnorm;
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) { neg_res_u[i] = -res_u[i]; neg_res_v[i] = -res_v[i]; }
                gmres_matfree(neg_res_u, neg_res_v, uf, vf, N, h_grid, alpha, B, hbeta0, R, GMRES_TOL,
                              step_du, step_dv, qu, qv, wu, wv, hess, cs, sn, g, y, red);
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) { u_trial[i] += step_du[i]; v_trial[i] += step_dv[i]; }
            }

            if (converged == 0) {
                h = h * 0.25;
                if (order > 1) order = order - 1;
                continue;
            }
            const double c_err = 1.0 / (k + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N2; ++i) {
                err_u[i] = c_err * (u_trial[i] - u_pred[i]);
                err_v[i] = c_err * (v_trial[i] - v_pred[i]);
            }
            const double err_est = wrms_norm(err_u, err_v, ABSTOL_REF, RTOL_REF, u_trial, v_trial, N, red);

            if (err_est <= 1.0) {
                consecutive_reject = 0;
                t = t_new;
                #pragma omp master
                { order_history[nsteps] = order; }
                nsteps += 1;
                for (int j = maxhist - 1; j > 0; --j) {
                    #pragma omp for schedule(static)
                    for (int64_t i = 0; i < N2; ++i) {
                        hist_u[(int64_t)j * N2 + i] = hist_u[(int64_t)(j - 1) * N2 + i];
                        hist_v[(int64_t)j * N2 + i] = hist_v[(int64_t)(j - 1) * N2 + i];
                    }
                    hist_t[j] = hist_t[j - 1];
                }
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N2; ++i) { hist_u[i] = u_trial[i]; hist_v[i] = v_trial[i]; }
                hist_t[0] = t;
                if (n_hist < maxhist) n_hist = n_hist + 1;
                steps_since_order_change += 1;
                if (order < max_order && steps_since_order_change > order && n_hist > order + 1) {
                    order += 1;
                    steps_since_order_change = 0;
                } else if (refactored_this_step == 1 && order > 1) {
                    order -= 1;
                    steps_since_order_change = 0;
                }
                double fac;
                if (err_est > 0.0) fac = SAFETY * pow(err_est, -1.0 / (k + 1));
                else fac = MAX_FACTOR;
                if (fac > MAX_FACTOR) fac = MAX_FACTOR;
                if (fac < MIN_FACTOR) fac = MIN_FACTOR;
                h = h * fac;
            } else {
                consecutive_reject += 1;
                double fac = SAFETY * pow(err_est, -1.0 / (k + 1));
                if (fac < MIN_FACTOR) fac = MIN_FACTOR;
                if (fac > 1.0) fac = 1.0;
                h = h * fac;
                if (consecutive_reject >= 2 && order > 1) {
                    order -= 1;
                    steps_since_order_change = 0;
                }
            }
        }

        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N2; ++i) { u[i] = hist_u[i]; v[i] = hist_v[i]; }
        #pragma omp master
        {
            diagnostics[0] = (double)nsteps;
            diagnostics[1] = (double)njev;
            diagnostics[2] = t;
        }
    }

    if (!use_ws) free(m);
}
