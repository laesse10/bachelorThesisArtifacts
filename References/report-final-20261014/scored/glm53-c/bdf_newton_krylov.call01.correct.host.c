/* Variable-order variable-step BDF integrator over a Newton-Krylov corrector.
 * Faithful port of bdf_newton_krylov_numpy.py.
 * Layout: arrays fed to the Laplacian stencil (u_trial, v_trial, Krylov basis
 * qu/qv) are padded (N+2)x(N+2) with mirrored ghosts so the 5-point stencil is
 * uniform; everything else is plain NxN.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef PARAM_RTOL
#define PARAM_RTOL 1.0e-6
#endif
#ifndef PARAM_ATOL
#define PARAM_ATOL 1.0e-9
#endif
#ifndef PARAM_NEWTON_RTOL
#define PARAM_NEWTON_RTOL 1.0e-12
#endif
#ifndef PARAM_MAX_ORDER
#define PARAM_MAX_ORDER 5
#endif
#ifndef PARAM_MAX_NEWTON
#define PARAM_MAX_NEWTON 12
#endif
#ifndef PARAM_GMRES_RESTART
#define PARAM_GMRES_RESTART 30
#endif
#ifndef PARAM_GMRES_TOL
#define PARAM_GMRES_TOL 1.0e-1
#endif

#define H0 1.0e-3
#define CONV_DEGRADE 0.3
#define SAFETY 0.9
#define MIN_FACTOR 0.2
#define MAX_FACTOR 6.0
#define MAXHIST (PARAM_MAX_ORDER + 1)

static void lagrange_weights(const double *nodes, int k1, int deriv, double *weights) {
    double aug[MAXHIST][MAXHIST + 2];
    for (int row = 0; row < k1; row++) {
        for (int col = 0; col < k1; col++) {
            double power = 1.0;
            for (int e = 0; e < row; e++) power = power * nodes[col];
            aug[row][col] = power;
        }
        aug[row][k1] = 0.0;
    }
    aug[deriv][k1] = 1.0;
    for (int col = 0; col < k1; col++) {
        double pivot = aug[col][col];
        for (int w = 0; w <= k1; w++) aug[col][w] = aug[col][w] / pivot;
        for (int row = 0; row < k1; row++) {
            if (row != col) {
                double factor = aug[row][col];
                for (int w = 0; w <= k1; w++)
                    aug[row][w] = aug[row][w] - factor * aug[col][w];
            }
        }
    }
    for (int j = 0; j < k1; j++) weights[j] = aug[j][k1];
}

static void update_ghosts(double *X, int N, int M) {
    for (int c = 1; c <= N; c++) {
        X[(size_t)0 * M + c] = X[(size_t)2 * M + c];
        X[(size_t)(N + 1) * M + c] = X[(size_t)(N - 1) * M + c];
    }
    for (int r = 1; r <= N; r++) {
        X[(size_t)r * M + 0] = X[(size_t)r * M + 2];
        X[(size_t)r * M + (N + 1)] = X[(size_t)r * M + (N - 1)];
    }
}

void bdf_newton_krylov_fp64(
    double *restrict diagnostics,
    int64_t *restrict order_history,
    double *restrict u,
    double *restrict v,
    const double A,
    const double B,
    const int64_t N_,
    const double alpha,
    const int64_t max_steps,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const int N = (int)N_;
    const int M = N + 2;
    const size_t GG = (size_t)M * (size_t)M;
    const size_t G = (size_t)N * (size_t)N;

    const double rtol = PARAM_RTOL, abstol = PARAM_ATOL, newton_rtol = PARAM_NEWTON_RTOL;
    const int max_order = PARAM_MAX_ORDER, max_newton = PARAM_MAX_NEWTON;
    const int restart = PARAM_GMRES_RESTART;
    const double gmres_tol = PARAM_GMRES_TOL;
    const int R1 = restart + 1;

    double *base = (double *)malloc(sizeof(double) * GG * (2 * MAXHIST + 2 * R1 + 16));
    if (!base) { diagnostics[0] = 0; diagnostics[1] = 0; diagnostics[2] = 0; return; }
    double *p = base;
    double *hist_u[MAXHIST], *hist_v[MAXHIST];
    for (int j = 0; j < MAXHIST; j++) { hist_u[j] = p; p += GG; }
    for (int j = 0; j < MAXHIST; j++) { hist_v[j] = p; p += GG; }
    double *qu = p; p += (size_t)R1 * GG;
    double *qv = p; p += (size_t)R1 * GG;
    double *wu = p; p += GG;
    double *wv = p; p += GG;
    double *uf = p; p += GG;
    double *vf = p; p += GG;
    double *u_trial = p; p += GG;
    double *v_trial = p; p += GG;
    double *u_pred = p; p += GG;
    double *v_pred = p; p += GG;
    double *rhs_u = p; p += GG;
    double *rhs_v = p; p += GG;
    double *f_u = p; p += GG;
    double *f_v = p; p += GG;
    double *res_u = p; p += GG;
    double *res_v = p; p += GG;
    double *nres_u = p; p += GG;
    double *nres_v = p; p += GG;
    double *err_u = p; p += GG;
    double *err_v = p; p += GG;
    double *sdu = p; p += GG;
    double *sdv = p; p += GG;

    double nodes_p[MAXHIST], nodes_c[MAXHIST + 1], w_pred[MAXHIST], c_corr[MAXHIST + 1];
    double *hess = (double *)malloc(sizeof(double) * (size_t)R1 * (size_t)restart);
    double cs[PARAM_GMRES_RESTART], sn[PARAM_GMRES_RESTART];
    double g[PARAM_GMRES_RESTART + 1], y[PARAM_GMRES_RESTART];

    const double h_grid = 1.0 / (double)N;
    const double invh2 = 1.0 / (h_grid * h_grid);
    const double Bp1 = B + 1.0;

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            hist_u[0][(size_t)(i + 1) * M + (j + 1)] = u[(size_t)i * N + j];
            hist_v[0][(size_t)(i + 1) * M + (j + 1)] = v[(size_t)i * N + j];
        }
    int head = 0;
    double hist_t[MAXHIST];
    hist_t[0] = 0.0;
    int n_hist = 1;

    double t = 0.0, h = H0;
    int order = 1, steps_since_order_change = 0, consecutive_reject = 0;
    long njev = 0, nsteps = 0;
    int have_jac = 0;

    while (t < t_end && nsteps < max_steps) {
        if (t + h > t_end) h = t_end - t;
        int k = order < n_hist ? order : n_hist;
        double t_new = t + h;

        int deg = k < (n_hist - 1) ? k : (n_hist - 1);
        for (int j = 0; j < deg + 1; j++)
            nodes_p[j] = hist_t[(head + j) % MAXHIST] - t_new;
        lagrange_weights(nodes_p, deg + 1, 0, w_pred);
        for (size_t idx = 0; idx < G; idx++) { u_pred[idx] = 0.0; v_pred[idx] = 0.0; }
        for (int j = 0; j < deg + 1; j++) {
            const double wj = w_pred[j];
            const double *hu = hist_u[(head + j) % MAXHIST];
            const double *hv = hist_v[(head + j) % MAXHIST];
            for (int i = 0; i < N; i++) {
                const double *sru = hu + (size_t)(i + 1) * M + 1;
                const double *srv = hv + (size_t)(i + 1) * M + 1;
                double *du = u_pred + (size_t)i * N;
                double *dv = v_pred + (size_t)i * N;
                for (int j2 = 0; j2 < N; j2++) {
                    du[j2] = du[j2] + wj * sru[j2];
                    dv[j2] = dv[j2] + wj * srv[j2];
                }
            }
        }

        nodes_c[0] = 0.0;
        for (int j = 0; j < k; j++) nodes_c[j + 1] = hist_t[(head + j) % MAXHIST] - t_new;
        lagrange_weights(nodes_c, k + 1, 1, c_corr);
        const double c0 = c_corr[0];
        const double hbeta0 = 1.0 / c0;
        for (size_t idx = 0; idx < G; idx++) { rhs_u[idx] = 0.0; rhs_v[idx] = 0.0; }
        for (int j = 1; j < k + 1; j++) {
            const double cj = c_corr[j] / c0;
            const double *hu = hist_u[(head + j - 1) % MAXHIST];
            const double *hv = hist_v[(head + j - 1) % MAXHIST];
            for (int i = 0; i < N; i++) {
                const double *sru = hu + (size_t)(i + 1) * M + 1;
                const double *srv = hv + (size_t)(i + 1) * M + 1;
                double *du = rhs_u + (size_t)i * N;
                double *dv = rhs_v + (size_t)i * N;
                for (int j2 = 0; j2 < N; j2++) {
                    du[j2] = du[j2] - cj * sru[j2];
                    dv[j2] = dv[j2] - cj * srv[j2];
                }
            }
        }

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                u_trial[(size_t)(i + 1) * M + (j + 1)] = u_pred[(size_t)i * N + j];
                v_trial[(size_t)(i + 1) * M + (j + 1)] = v_pred[(size_t)i * N + j];
            }
        update_ghosts(u_trial, N, M);
        update_ghosts(v_trial, N, M);
        if (!have_jac) {
            memcpy(uf, u_trial, GG * sizeof(double));
            memcpy(vf, v_trial, GG * sizeof(double));
            njev++;
            have_jac = 1;
        }

        int converged = 0, refactored_this_step = 0;
        double prev_resnorm = -1.0;
        for (int it = 0; it < max_newton; it++) {
            /* brusselator_rhs */
            for (int i = 0; i < N; i++) {
                const double *su = u_trial + (size_t)(i + 1) * M;
                const double *sv = v_trial + (size_t)(i + 1) * M;
                const double *su_up = u_trial + (size_t)i * M;
                const double *sv_up = v_trial + (size_t)i * M;
                const double *su_dn = u_trial + (size_t)(i + 2) * M;
                const double *sv_dn = v_trial + (size_t)(i + 2) * M;
                double *fu = f_u + (size_t)i * N;
                double *fv = f_v + (size_t)i * N;
                for (int j = 0; j < N; j++) {
                    const double uu = su[j + 1], vv = sv[j + 1];
                    const double lu = ((su_up[j + 1] + su_dn[j + 1]) + su[j]) + su[j + 2] - 4.0 * uu;
                    const double lv = ((sv_up[j + 1] + sv_dn[j + 1]) + sv[j]) + sv[j + 2] - 4.0 * vv;
                    fu[j] = A + uu * uu * vv - Bp1 * uu + alpha * lu * invh2;
                    fv[j] = B * uu - uu * uu * vv + alpha * lv * invh2;
                }
            }
            /* residual + wrms norm */
            double resnorm;
            {
                double s = 0.0;
                for (int i = 0; i < N; i++) {
                    const double *su = u_trial + (size_t)(i + 1) * M + 1;
                    const double *sv = v_trial + (size_t)(i + 1) * M + 1;
                    const double *fu = f_u + (size_t)i * N;
                    const double *fv = f_v + (size_t)i * N;
                    const double *ru_in = rhs_u + (size_t)i * N;
                    const double *rv_in = rhs_v + (size_t)i * N;
                    double *ru = res_u + (size_t)i * N;
                    double *rv = res_v + (size_t)i * N;
                    double au = 0.0, av = 0.0;
                    for (int j = 0; j < N; j++) {
                        const double a = su[j] - hbeta0 * fu[j] - ru_in[j];
                        const double b = sv[j] - hbeta0 * fv[j] - rv_in[j];
                        ru[j] = a; rv[j] = b;
                        const double wu_i = newton_rtol + newton_rtol * fabs(su[j]);
                        const double wv_i = newton_rtol + newton_rtol * fabs(sv[j]);
                        const double ta = a / wu_i, tb = b / wv_i;
                        au += ta * ta;
                        av += tb * tb;
                    }
                    s = (s + au) + av;
                }
                resnorm = sqrt(s / (2.0 * (double)G));
            }
            if (resnorm < 1.0) { converged = 1; break; }
            if (prev_resnorm > 0.0) {
                const double rate = resnorm / prev_resnorm;
                if (rate > CONV_DEGRADE && !refactored_this_step) {
                    memcpy(uf, u_trial, GG * sizeof(double));
                    memcpy(vf, v_trial, GG * sizeof(double));
                    njev++;
                    refactored_this_step = 1;
                }
            }
            prev_resnorm = resnorm;

            /* GMRES */
            for (size_t idx = 0; idx < G; idx++) {
                nres_u[idx] = -res_u[idx];
                nres_v[idx] = -res_v[idx];
            }
            double beta = 0.0;
            for (int i = 0; i < N; i++) {
                const double *nu = nres_u + (size_t)i * N;
                const double *nv = nres_v + (size_t)i * N;
                double au = 0.0, av = 0.0;
                for (int j = 0; j < N; j++) { au += nu[j] * nu[j]; av += nv[j] * nv[j]; }
                beta = (beta + au) + av;
            }
            beta = sqrt(beta);
            for (size_t idx = 0; idx < G; idx++) { sdu[idx] = 0.0; sdv[idx] = 0.0; }
            if (beta != 0.0) {
                for (int i = 0; i < N; i++)
                    for (int j = 0; j < N; j++) {
                        qu[(size_t)0 * GG + (size_t)(i + 1) * M + (j + 1)] = nres_u[(size_t)i * N + j] / beta;
                        qv[(size_t)0 * GG + (size_t)(i + 1) * M + (j + 1)] = nres_v[(size_t)i * N + j] / beta;
                    }
                update_ghosts(qu, N, M);
                update_ghosts(qv, N, M);
                g[0] = beta;
                int m_used = restart;
                for (int kk = 0; kk < restart; kk++) {
                    /* w = (I - hbeta0 J) q_kk */
                    {
                        const double *qk_u = qu + (size_t)kk * GG;
                        const double *qk_v = qv + (size_t)kk * GG;
                        for (int i = 0; i < N; i++) {
                            const double *su = qk_u + (size_t)(i + 1) * M;
                            const double *sv = qk_v + (size_t)(i + 1) * M;
                            const double *su_up = qk_u + (size_t)i * M;
                            const double *sv_up = qk_v + (size_t)i * M;
                            const double *su_dn = qk_u + (size_t)(i + 2) * M;
                            const double *sv_dn = qk_v + (size_t)(i + 2) * M;
                            const double *uf_ = uf + (size_t)(i + 1) * M + 1;
                            const double *vf_ = vf + (size_t)(i + 1) * M + 1;
                            double *ou = wu + (size_t)i * N;
                            double *ov = wv + (size_t)i * N;
                            for (int j = 0; j < N; j++) {
                                const double du_ = su[j + 1], dv_ = sv[j + 1];
                                const double lu = ((su_up[j + 1] + su_dn[j + 1]) + su[j]) + su[j + 2] - 4.0 * du_;
                                const double lv = ((sv_up[j + 1] + sv_dn[j + 1]) + sv[j]) + sv[j + 2] - 4.0 * dv_;
                                const double a = uf_[j], b = vf_[j];
                                const double t1 = (2.0 * a * b - Bp1) * du_ + a * a * dv_;
                                const double t2 = (B - 2.0 * a * b) * du_ - a * a * dv_;
                                ou[j] = du_ - hbeta0 * (t1 + alpha * lu * invh2);
                                ov[j] = dv_ - hbeta0 * (t2 + alpha * lv * invh2);
                            }
                        }
                    }
                    /* modified Gram-Schmidt */
                    for (int pp = 0; pp <= kk; pp++) {
                        const double *pu = qu + (size_t)pp * GG;
                        const double *pv = qv + (size_t)pp * GG;
                        double h_pk = 0.0;
                        for (int i = 0; i < N; i++) {
                            const double *pu_i = pu + (size_t)(i + 1) * M + 1;
                            const double *pv_i = pv + (size_t)(i + 1) * M + 1;
                            const double *wu_i = wu + (size_t)i * N;
                            const double *wv_i = wv + (size_t)i * N;
                            double au = 0.0, av = 0.0;
                            for (int j = 0; j < N; j++) { au += pu_i[j] * wu_i[j]; av += pv_i[j] * wv_i[j]; }
                            h_pk = (h_pk + au) + av;
                        }
                        hess[(size_t)pp * restart + kk] = h_pk;
                        for (int i = 0; i < N; i++) {
                            const double *pu_i = pu + (size_t)(i + 1) * M + 1;
                            const double *pv_i = pv + (size_t)(i + 1) * M + 1;
                            double *wu_i = wu + (size_t)i * N;
                            double *wv_i = wv + (size_t)i * N;
                            for (int j = 0; j < N; j++) {
                                wu_i[j] = wu_i[j] - h_pk * pu_i[j];
                                wv_i[j] = wv_i[j] - h_pk * pv_i[j];
                            }
                        }
                    }
                    double h_next = 0.0;
                    for (int i = 0; i < N; i++) {
                        const double *wu_i = wu + (size_t)i * N;
                        const double *wv_i = wv + (size_t)i * N;
                        double au = 0.0, av = 0.0;
                        for (int j = 0; j < N; j++) { au += wu_i[j] * wu_i[j]; av += wv_i[j] * wv_i[j]; }
                        h_next = (h_next + au) + av;
                    }
                    h_next = sqrt(h_next);
                    hess[(size_t)(kk + 1) * restart + kk] = h_next;

                    for (int pp = 0; pp < kk; pp++) {
                        const double temp = cs[pp] * hess[(size_t)pp * restart + kk] + sn[pp] * hess[(size_t)(pp + 1) * restart + kk];
                        hess[(size_t)(pp + 1) * restart + kk] = -sn[pp] * hess[(size_t)pp * restart + kk] + cs[pp] * hess[(size_t)(pp + 1) * restart + kk];
                        hess[(size_t)pp * restart + kk] = temp;
                    }
                    const double hkk = hess[(size_t)kk * restart + kk];
                    const double hk1k = hess[(size_t)(kk + 1) * restart + kk];
                    const double denom = sqrt(hkk * hkk + hk1k * hk1k);
                    cs[kk] = hkk / denom;
                    sn[kk] = hk1k / denom;
                    hess[(size_t)kk * restart + kk] = cs[kk] * hkk + sn[kk] * hk1k;
                    hess[(size_t)(kk + 1) * restart + kk] = 0.0;
                    const double temp = cs[kk] * g[kk];
                    g[kk + 1] = -sn[kk] * g[kk];
                    g[kk] = temp;

                    const double rel = fabs(g[kk + 1]) / beta;
                    if (rel < gmres_tol || h_next < 1.0e-13 || kk == restart - 1) {
                        m_used = kk + 1;
                        break;
                    }
                    {
                        const double inv = 1.0 / h_next;
                        double *qn_u = qu + (size_t)(kk + 1) * GG;
                        double *qn_v = qv + (size_t)(kk + 1) * GG;
                        for (int i = 0; i < N; i++)
                            for (int j = 0; j < N; j++) {
                                qn_u[(size_t)(i + 1) * M + (j + 1)] = wu[(size_t)i * N + j] * inv;
                                qn_v[(size_t)(i + 1) * M + (j + 1)] = wv[(size_t)i * N + j] * inv;
                            }
                        update_ghosts(qn_u, N, M);
                        update_ghosts(qn_v, N, M);
                    }
                }
                /* backsolve R upper-triangular system */
                for (int row = 0; row < m_used; row++) {
                    const int rr = m_used - 1 - row;
                    double s = g[rr];
                    for (int col = rr + 1; col < m_used; col++)
                        s = s - hess[(size_t)rr * restart + col] * y[col];
                    y[rr] = s / hess[(size_t)rr * restart + rr];
                }
                /* combine step */
                for (int pp = 0; pp < m_used; pp++) {
                    const double *pu = qu + (size_t)pp * GG;
                    const double *pv = qv + (size_t)pp * GG;
                    const double yp = y[pp];
                    for (int i = 0; i < N; i++) {
                        const double *pu_i = pu + (size_t)(i + 1) * M + 1;
                        const double *pv_i = pv + (size_t)(i + 1) * M + 1;
                        double *du = sdu + (size_t)i * N;
                        double *dv = sdv + (size_t)i * N;
                        for (int j = 0; j < N; j++) {
                            du[j] = du[j] + yp * pu_i[j];
                            dv[j] = dv[j] + yp * pv_i[j];
                        }
                    }
                }
            }
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++) {
                    u_trial[(size_t)(i + 1) * M + (j + 1)] += sdu[(size_t)i * N + j];
                    v_trial[(size_t)(i + 1) * M + (j + 1)] += sdv[(size_t)i * N + j];
                }
            update_ghosts(u_trial, N, M);
            update_ghosts(v_trial, N, M);
        }

        if (!converged) {
            h = h * 0.25;
            if (order > 1) order -= 1;
            continue;
        }

        const double c_err = 1.0 / (double)(k + 1);
        double err_est;
        {
            double s = 0.0;
            for (int i = 0; i < N; i++) {
                const double *su = u_trial + (size_t)(i + 1) * M + 1;
                const double *sv = v_trial + (size_t)(i + 1) * M + 1;
                double *eu = err_u + (size_t)i * N;
                double *ev = err_v + (size_t)i * N;
                double au = 0.0, av = 0.0;
                for (int j = 0; j < N; j++) {
                    const double a = c_err * (su[j] - u_pred[(size_t)i * N + j]);
                    const double b = c_err * (sv[j] - v_pred[(size_t)i * N + j]);
                    eu[j] = a; ev[j] = b;
                    const double wu_i = abstol + rtol * fabs(su[j]);
                    const double wv_i = abstol + rtol * fabs(sv[j]);
                    const double ta = a / wu_i, tb = b / wv_i;
                    au += ta * ta;
                    av += tb * tb;
                }
                s = (s + au) + av;
            }
            err_est = sqrt(s / (2.0 * (double)G));
        }

        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            order_history[nsteps] = (int64_t)order;
            nsteps++;
            head = (head + MAXHIST - 1) % MAXHIST;
            memcpy(hist_u[head], u_trial, GG * sizeof(double));
            memcpy(hist_v[head], v_trial, GG * sizeof(double));
            hist_t[head] = t;
            if (n_hist < MAXHIST) n_hist++;

            steps_since_order_change++;
            if (order < max_order && steps_since_order_change > order && n_hist > order + 1) {
                order += 1;
                steps_since_order_change = 0;
            } else if (refactored_this_step == 1 && order > 1) {
                order -= 1;
                steps_since_order_change = 0;
            }

            double fac;
            if (err_est > 0.0) fac = SAFETY * pow(err_est, -1.0 / ((double)k + 1.0));
            else fac = MAX_FACTOR;
            fac = fmin(MAX_FACTOR, fmax(MIN_FACTOR, fac));
            h = h * fac;
        } else {
            consecutive_reject++;
            double fac = SAFETY * pow(err_est, -1.0 / ((double)k + 1.0));
            fac = fmin(1.0, fmax(MIN_FACTOR, fac));
            h = h * fac;
            if (consecutive_reject >= 2 && order > 1) {
                order -= 1;
                steps_since_order_change = 0;
            }
        }
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            u[(size_t)i * N + j] = hist_u[head][(size_t)(i + 1) * M + (j + 1)];
            v[(size_t)i * N + j] = hist_v[head][(size_t)(i + 1) * M + (j + 1)];
        }
    diagnostics[0] = (double)nsteps;
    diagnostics[1] = (double)njev;
    diagnostics[2] = t;
    free(hess);
    free(base);
}
