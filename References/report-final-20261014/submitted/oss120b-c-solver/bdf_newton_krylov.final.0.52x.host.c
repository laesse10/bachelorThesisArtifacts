#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * Optimised BDF‑Newton‑Krylov integrator for the 2‑D Brusselator.
 * Mirrors the reference NumPy implementation exactly while re‑using
 * temporaries to avoid repeated malloc/free inside the tight loops.
 */

static const double R_TOL = 1e-6;        /* BDF relative tolerance (match reference) */
static const double ABS_TOL = 1e-8;     /* BDF absolute tolerance (match reference) */
static const double NEWTON_R_TOL = 1e-12;/* Newton corrector tolerance */
static const int MAX_ORDER = 5;         /* Maximum BDF order */
static const int MAX_NEWTON = 12;       /* Max Newton iterations */
static const int GMRES_RESTART = 20;    /* GMRES restart length (reference) */
static const double GMRES_TOL = 0.1;    /* GMRES relative tolerance (reference) */

static const double CONV_DEGRADE = 0.3;
static const double SAFETY = 0.9;
static const double MIN_FACTOR = 0.2;
static const double MAX_FACTOR = 6.0;
static const double H0 = 1.0e-3;         /* Initial step size */

#define IDX(i, j, N) ((i)*(N) + (j))

/* 5‑point Neumann Laplacian */
static void neumann_laplacian(const double *U, int N, double *L) {
    int i, j;
    #pragma omp parallel for private(i,j) collapse(2)
    for (i = 1; i < N-1; ++i) {
        for (j = 1; j < N-1; ++j) {
            L[IDX(i,j,N)] = U[IDX(i-1,j,N)] + U[IDX(i+1,j,N)] +
                            U[IDX(i,j-1,N)] + U[IDX(i,j+1,N)] -
                            4.0 * U[IDX(i,j,N)];
        }
    }
    i = 0;
    #pragma omp parallel for private(j)
    for (j = 1; j < N-1; ++j) {
        L[IDX(i,j,N)] = 2.0 * U[IDX(1,j,N)] + U[IDX(0,j-1,N)] + U[IDX(0,j+1,N)] -
                         4.0 * U[IDX(0,j,N)];
    }
    i = N-1;
    #pragma omp parallel for private(j)
    for (j = 1; j < N-1; ++j) {
        L[IDX(i,j,N)] = 2.0 * U[IDX(N-2,j,N)] + U[IDX(N-1,j-1,N)] + U[IDX(N-1,j+1,N)] -
                         4.0 * U[IDX(N-1,j,N)];
    }
    j = 0;
    #pragma omp parallel for private(i)
    for (i = 1; i < N-1; ++i) {
        L[IDX(i,j,N)] = U[IDX(i-1,0,N)] + U[IDX(i+1,0,N)] + 2.0 * U[IDX(i,1,N)] -
                         4.0 * U[IDX(i,0,N)];
    }
    j = N-1;
    #pragma omp parallel for private(i)
    for (i = 1; i < N-1; ++i) {
        L[IDX(i,j,N)] = U[IDX(i-1,N-1,N)] + U[IDX(i+1,N-1,N)] + 2.0 * U[IDX(i,N-2,N)] -
                         4.0 * U[IDX(i,N-1,N)];
    }
    L[IDX(0,0,N)] = 2.0 * U[IDX(1,0,N)] + 2.0 * U[IDX(0,1,N)] - 4.0 * U[IDX(0,0,N)];
    L[IDX(0,N-1,N)] = 2.0 * U[IDX(1,N-1,N)] + 2.0 * U[IDX(0,N-2,N)] - 4.0 * U[IDX(0,N-1,N)];
    L[IDX(N-1,0,N)] = 2.0 * U[IDX(N-2,0,N)] + 2.0 * U[IDX(N-1,1,N)] - 4.0 * U[IDX(N-1,0,N)];
    L[IDX(N-1,N-1,N)] = 2.0 * U[IDX(N-2,N-1,N)] + 2.0 * U[IDX(N-1,N-2,N)] - 4.0 * U[IDX(N-1,N-1,N)];
}

static void brusselator_rhs(const double *u, const double *v, int N, double h,
                             double alpha, double A, double B,
                             double *du, double *dv,
                             double *lap_u, double *lap_v) {
    double invh2 = 1.0 / (h * h);
    neumann_laplacian(u, N, lap_u);
    neumann_laplacian(v, N, lap_v);
    int i, j;
    #pragma omp parallel for private(i,j) collapse(2)
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            double uij = u[IDX(i,j,N)];
            double vij = v[IDX(i,j,N)];
            double lu = lap_u[IDX(i,j,N)];
            double lv = lap_v[IDX(i,j,N)];
            du[IDX(i,j,N)] = A + uij * uij * vij - (B + 1.0) * uij + alpha * lu * invh2;
            dv[IDX(i,j,N)] = B * uij - uij * uij * vij + alpha * lv * invh2;
        }
    }
}

static void newton_matvec(const double *du, const double *dv,
                           const double *uf, const double *vf,
                           int N, double h, double alpha, double B, double hbeta0,
                           double *out_du, double *out_dv,
                           double *lap_du, double *lap_dv) {
    double invh2 = 1.0 / (h * h);
    neumann_laplacian(du, N, lap_du);
    neumann_laplacian(dv, N, lap_dv);
    int i, j;
    #pragma omp parallel for private(i,j) collapse(2)
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            double duij = du[IDX(i,j,N)];
            double dvij = dv[IDX(i,j,N)];
            double ufij = uf[IDX(i,j,N)];
            double vfij = vf[IDX(i,j,N)];
            double lu = lap_du[IDX(i,j,N)];
            double lv = lap_dv[IDX(i,j,N)];
            double term1 = (2.0 * ufij * vfij - (B + 1.0)) * duij;
            double term2 = (ufij * ufij) * dvij;
            double term3 = alpha * lu * invh2;
            out_du[IDX(i,j,N)] = duij - hbeta0 * (term1 + term2 + term3);
            double term4 = (B - 2.0 * ufij * vfij) * duij;
            double term5 = -(ufij * ufij) * dvij;
            double term6 = alpha * lv * invh2;
            out_dv[IDX(i,j,N)] = dvij - hbeta0 * (term4 + term5 + term6);
        }
    }
}

static double wrms_norm(const double *au, const double *av,
                        double abstol, double rtol,
                        const double *refu, const double *refv,
                        int N) {
    double s = 0.0;
    int i, j;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            double wu = abstol + rtol * fabs(refu[IDX(i,j,N)]);
            double wv = abstol + rtol * fabs(refv[IDX(i,j,N)]);
            double du = au[IDX(i,j,N)] / wu;
            double dv = av[IDX(i,j,N)] / wv;
            s += du * du + dv * dv;
        }
    }
    return sqrt(s / (2.0 * N * N));
}

static void lagrange_weights(const double *nodes, int k1, int deriv,
                             int max_order, double *weights) {
    int dim = max_order + 1;
    int cols = max_order + 2;
    double *aug = (double *)calloc(dim * cols, sizeof(double));
    int row, col, i;
    for (row = 0; row < k1; ++row) {
        for (col = 0; col < k1; ++col) {
            double power = 1.0;
            for (i = 0; i < row; ++i) power *= nodes[col];
            aug[row * cols + col] = power;
        }
    }
    aug[deriv * cols + k1] = 1.0;
    for (col = 0; col < k1; ++col) {
        double pivot = aug[col * cols + col];
        for (i = 0; i < cols; ++i) aug[col * cols + i] /= pivot;
        for (row = 0; row < k1; ++row) {
            if (row == col) continue;
            double factor = aug[row * cols + col];
            if (factor == 0.0) continue;
            for (i = 0; i < cols; ++i) {
                aug[row * cols + i] -= factor * aug[col * cols + i];
            }
        }
    }
    for (i = 0; i < k1; ++i) weights[i] = aug[i * cols + k1];
    free(aug);
}

static void gmres_matfree(const double *rhs_u, const double *rhs_v,
                           const double *uf, const double *vf,
                           int N, double h, double alpha, double B, double hbeta0,
                           int restart, double tol,
                           double *du, double *dv,
                           double *qu, double *qv, double *hess,
                           double *cs, double *sn, double *g,
                           double *wu, double *wv, double *y,
                           double *mv_du, double *mv_dv,
                           double *lap_du, double *lap_dv) {
    int vec_sz = N * N;
    int maxhist = restart + 1;
    double beta = 0.0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int idx = IDX(i,j,N);
            beta += rhs_u[idx] * rhs_u[idx] + rhs_v[idx] * rhs_v[idx];
        }
    }
    beta = sqrt(beta);
    memset(du, 0, vec_sz * sizeof(double));
    memset(dv, 0, vec_sz * sizeof(double));
    if (beta == 0.0) return;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int idx = IDX(i,j,N);
            qu[idx] = rhs_u[idx] / beta;
            qv[idx] = rhs_v[idx] / beta;
        }
    }
    g[0] = beta;
    int m_used = restart;
    for (int k = 0; k < restart; ++k) {
        newton_matvec(&qu[k * vec_sz], &qv[k * vec_sz], uf, vf,
                      N, h, alpha, B, hbeta0,
                      mv_du, mv_dv, lap_du, lap_dv);
        for (int p = 0; p <= k; ++p) {
            double h_pk = 0.0;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    int idx = IDX(i,j,N);
                    h_pk += qu[p * vec_sz + idx] * mv_du[idx] +
                            qv[p * vec_sz + idx] * mv_dv[idx];
                }
            }
            hess[p * restart + k] = h_pk;
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    int idx = IDX(i,j,N);
                    mv_du[idx] -= h_pk * qu[p * vec_sz + idx];
                    mv_dv[idx] -= h_pk * qv[p * vec_sz + idx];
                }
            }
        }
        double h_next = 0.0;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                int idx = IDX(i,j,N);
                h_next += mv_du[idx] * mv_du[idx] + mv_dv[idx] * mv_dv[idx];
            }
        }
        h_next = sqrt(h_next);
        hess[(k+1) * restart + k] = h_next;
        for (int p = 0; p < k; ++p) {
            double temp = cs[p] * hess[p * restart + k] + sn[p] * hess[(p+1) * restart + k];
            hess[(p+1) * restart + k] = -sn[p] * hess[p * restart + k] + cs[p] * hess[(p+1) * restart + k];
            hess[p * restart + k] = temp;
        }
        double denom = sqrt(hess[k * restart + k] * hess[k * restart + k] +
                            hess[(k+1) * restart + k] * hess[(k+1) * restart + k]);
        cs[k] = hess[k * restart + k] / denom;
        sn[k] = hess[(k+1) * restart + k] / denom;
        hess[k * restart + k] = cs[k] * hess[k * restart + k] + sn[k] * hess[(k+1) * restart + k];
        hess[(k+1) * restart + k] = 0.0;
        double tmp = cs[k] * g[k];
        g[k+1] = -sn[k] * g[k];
        g[k] = tmp;
        double rel = fabs(g[k+1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart - 1) { m_used = k + 1; break; }
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                int idx = IDX(i,j,N);
                qu[(k+1) * vec_sz + idx] = mv_du[idx] / h_next;
                qv[(k+1) * vec_sz + idx] = mv_dv[idx] / h_next;
            }
        }
    }
    for (int row = 0; row < m_used; ++row) {
        int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; ++col) {
            s -= hess[rr * restart + col] * y[col];
        }
        y[rr] = s / hess[rr * restart + rr];
    }
    for (int p = 0; p < m_used; ++p) {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                int idx = IDX(i,j,N);
                du[idx] += qu[p * vec_sz + idx] * y[p];
                dv[idx] += qv[p * vec_sz + idx] * y[p];
            }
        }
    }
}

void bdf_newton_krylov_fp64(double *restrict diagnostics,
                            int64_t *restrict order_history,
                            double *restrict u,
                            double *restrict v,
                            const double A,
                            const double B,
                            const int64_t N_in,
                            const double alpha,
                            const int64_t max_steps,
                            const double t_end,
                            uint8_t *restrict workspace,
                            const int64_t workspace_size) {
    const int N = (int)N_in;
    const int maxhist = MAX_ORDER + 1;
    const double h_grid = 1.0 / (double)N;
    /* History buffers */
    double *hist_u = (double *)calloc(maxhist * N * N, sizeof(double));
    double *hist_v = (double *)calloc(maxhist * N * N, sizeof(double));
    double *hist_t = (double *)calloc(maxhist, sizeof(double));
    /* Permanent temporaries */
    double *uf = (double *)calloc(N * N, sizeof(double));
    double *vf = (double *)calloc(N * N, sizeof(double));
    double *u_pred = (double *)calloc(N * N, sizeof(double));
    double *v_pred = (double *)calloc(N * N, sizeof(double));
    double *u_trial = (double *)calloc(N * N, sizeof(double));
    double *v_trial = (double *)calloc(N * N, sizeof(double));
    double *rhs_u = (double *)calloc(N * N, sizeof(double));
    double *rhs_v = (double *)calloc(N * N, sizeof(double));
    double *f_u = (double *)calloc(N * N, sizeof(double));
    double *f_v = (double *)calloc(N * N, sizeof(double));
    double *res_u = (double *)calloc(N * N, sizeof(double));
    double *res_v = (double *)calloc(N * N, sizeof(double));
    double *neg_res_u = (double *)calloc(N * N, sizeof(double));
    double *neg_res_v = (double *)calloc(N * N, sizeof(double));
    double *err_u = (double *)calloc(N * N, sizeof(double));
    double *err_v = (double *)calloc(N * N, sizeof(double));
    double *step_du = (double *)calloc(N * N, sizeof(double));
    double *step_dv = (double *)calloc(N * N, sizeof(double));
    double *lap_u = (double *)calloc(N * N, sizeof(double));
    double *lap_v = (double *)calloc(N * N, sizeof(double));
    double *nodes_p = (double *)calloc(maxhist, sizeof(double));
    double *nodes_c = (double *)calloc(maxhist, sizeof(double));
    double *w_pred = (double *)calloc(maxhist, sizeof(double));
    double *c_corr = (double *)calloc(maxhist, sizeof(double));
    /* GMRES buffers (allocated once) */
    int vec_sz = N * N;
    int maxhist_g = GMRES_RESTART + 1;
    double *qu = (double *)calloc(maxhist_g * vec_sz, sizeof(double));
    double *qv = (double *)calloc(maxhist_g * vec_sz, sizeof(double));
    double *hess = (double *)calloc(maxhist_g * GMRES_RESTART, sizeof(double));
    double *cs = (double *)calloc(GMRES_RESTART, sizeof(double));
    double *sn = (double *)calloc(GMRES_RESTART, sizeof(double));
    double *g = (double *)calloc(GMRES_RESTART + 1, sizeof(double));
    double *wu = (double *)calloc(vec_sz, sizeof(double));
    double *wv = (double *)calloc(vec_sz, sizeof(double));
    double *y = (double *)calloc(GMRES_RESTART, sizeof(double));
    double *mv_du = (double *)calloc(vec_sz, sizeof(double));
    double *mv_dv = (double *)calloc(vec_sz, sizeof(double));
    double *lap_du = (double *)calloc(vec_sz, sizeof(double));
    double *lap_dv = (double *)calloc(vec_sz, sizeof(double));

    memcpy(&hist_u[0], u, N * N * sizeof(double));
    memcpy(&hist_v[0], v, N * N * sizeof(double));
    hist_t[0] = 0.0;
    int n_hist = 1;
    double t = 0.0;
    double h = H0;
    int order = 1;
    int steps_since_order_change = 0;
    int consecutive_reject = 0;
    int njev = 0;
    int nsteps = 0;
    int have_jac = 0;
    while (t < t_end && nsteps < max_steps) {
        if (t + h > t_end) h = t_end - t;
        int k = (order < n_hist) ? order : n_hist;
        double t_new = t + h;
        int deg = (k < n_hist) ? k : (n_hist - 1);
        for (int j = 0; j <= deg; ++j) nodes_p[j] = hist_t[j] - t_new;
        lagrange_weights(nodes_p, deg + 1, 0, MAX_ORDER, w_pred);
        memset(u_pred, 0, N * N * sizeof(double));
        memset(v_pred, 0, N * N * sizeof(double));
        for (int j = 0; j <= deg; ++j) {
            double wj = w_pred[j];
            double *hu = &hist_u[j * N * N];
            double *hv = &hist_v[j * N * N];
            for (int idx = 0; idx < N * N; ++idx) {
                u_pred[idx] += wj * hu[idx];
                v_pred[idx] += wj * hv[idx];
            }
        }
        nodes_c[0] = 0.0;
        for (int j = 0; j < k; ++j) nodes_c[j + 1] = hist_t[j] - t_new;
        lagrange_weights(nodes_c, k + 1, 1, MAX_ORDER, c_corr);
        double c0 = c_corr[0];
        double hbeta0 = 1.0 / c0;
        memset(rhs_u, 0, N * N * sizeof(double));
        memset(rhs_v, 0, N * N * sizeof(double));
        for (int j = 1; j <= k; ++j) {
            double coeff = -(c_corr[j] / c0);
            double *hu = &hist_u[(j-1) * N * N];
            double *hv = &hist_v[(j-1) * N * N];
            for (int idx = 0; idx < N * N; ++idx) {
                rhs_u[idx] += coeff * hu[idx];
                rhs_v[idx] += coeff * hv[idx];
            }
        }
        memcpy(u_trial, u_pred, N * N * sizeof(double));
        memcpy(v_trial, v_pred, N * N * sizeof(double));
        if (!have_jac) { memcpy(uf, u_trial, N * N * sizeof(double)); memcpy(vf, v_trial, N * N * sizeof(double)); njev++; have_jac = 1; }
        int converged = 0;
        int refactored_this_step = 0;
        double prev_resnorm = -1.0;
        for (int it = 0; it < MAX_NEWTON; ++it) {
            brusselator_rhs(u_trial, v_trial, N, h_grid, alpha, A, B,
                             f_u, f_v, lap_u, lap_v);
            for (int idx = 0; idx < N * N; ++idx) {
                res_u[idx] = u_trial[idx] - hbeta0 * f_u[idx] - rhs_u[idx];
                res_v[idx] = v_trial[idx] - hbeta0 * f_v[idx] - rhs_v[idx];
            }
            double resnorm = wrms_norm(res_u, res_v, NEWTON_R_TOL, NEWTON_R_TOL,
                                        u_trial, v_trial, N);
            if (resnorm < 1.0) { converged = 1; break; }
            if (prev_resnorm > 0.0) {
                double rate = resnorm / prev_resnorm;
                if (rate > CONV_DEGRADE && !refactored_this_step) {
                    memcpy(uf, u_trial, N * N * sizeof(double));
                    memcpy(vf, v_trial, N * N * sizeof(double));
                    njev++; refactored_this_step = 1;
                }
            }
            prev_resnorm = resnorm;
            for (int idx = 0; idx < N * N; ++idx) {
                neg_res_u[idx] = -res_u[idx];
                neg_res_v[idx] = -res_v[idx];
            }
            gmres_matfree(neg_res_u, neg_res_v, uf, vf,
                          N, h_grid, alpha, B, hbeta0,
                          GMRES_RESTART, GMRES_TOL,
                          step_du, step_dv,
                          qu, qv, hess, cs, sn, g, wu, wv, y,
                          mv_du, mv_dv, lap_du, lap_dv);
            for (int idx = 0; idx < N * N; ++idx) {
                u_trial[idx] += step_du[idx];
                v_trial[idx] += step_dv[idx];
            }
        }
        if (!converged) { h *= 0.25; if (order > 1) order--; continue; }
        double c_err = 1.0 / (k + 1);
        for (int idx = 0; idx < N * N; ++idx) {
            err_u[idx] = c_err * (u_trial[idx] - u_pred[idx]);
            err_v[idx] = c_err * (v_trial[idx] - v_pred[idx]);
        }
        double err_est = wrms_norm(err_u, err_v, ABS_TOL, R_TOL, u_trial, v_trial, N);
        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            order_history[nsteps] = order;
            nsteps++;
            for (int j = maxhist - 1; j >= 1; --j) {
                memcpy(&hist_u[j * N * N], &hist_u[(j-1) * N * N], N * N * sizeof(double));
                memcpy(&hist_v[j * N * N], &hist_v[(j-1) * N * N], N * N * sizeof(double));
                hist_t[j] = hist_t[j-1];
            }
            memcpy(&hist_u[0], u_trial, N * N * sizeof(double));
            memcpy(&hist_v[0], v_trial, N * N * sizeof(double));
            hist_t[0] = t;
            if (n_hist < maxhist) n_hist++;
            steps_since_order_change++;
            if (order < MAX_ORDER && steps_since_order_change > order && n_hist > order + 1) {
                order++; steps_since_order_change = 0;
            } else if (refactored_this_step && order > 1) {
                order--; steps_since_order_change = 0;
            }
            double fac = (err_est > 0.0) ? SAFETY * pow(err_est, -1.0/(k+1)) : MAX_FACTOR;
            fac = fmin(MAX_FACTOR, fmax(MIN_FACTOR, fac));
            h *= fac;
        } else {
            consecutive_reject++;
            double fac = SAFETY * pow(err_est, -1.0/(k+1));
            fac = fmin(1.0, fmax(MIN_FACTOR, fac));
            h *= fac;
            if (consecutive_reject >= 2 && order > 1) { order--; steps_since_order_change = 0; }
        }
    }
    memcpy(u, &hist_u[0], N * N * sizeof(double));
    memcpy(v, &hist_v[0], N * N * sizeof(double));
    diagnostics[0] = (double)nsteps;
    diagnostics[1] = (double)njev;
    diagnostics[2] = t;
    /* Free memory */
    free(hist_u); free(hist_v); free(hist_t);
    free(uf); free(vf);
    free(u_pred); free(v_pred);
    free(u_trial); free(v_trial);
    free(rhs_u); free(rhs_v);
    free(f_u); free(f_v);
    free(res_u); free(res_v);
    free(neg_res_u); free(neg_res_v);
    free(err_u); free(err_v);
    free(step_du); free(step_dv);
    free(lap_u); free(lap_v);
    free(nodes_p); free(nodes_c); free(w_pred); free(c_corr);
    free(qu); free(qv); free(hess); free(cs); free(sn); free(g);
    free(wu); free(wv); free(y);
    free(mv_du); free(mv_dv);
    free(lap_du); free(lap_dv);
}

/* End of file */
