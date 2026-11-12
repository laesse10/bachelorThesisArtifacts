#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * BDF-Newton-Krylov integrator for the 2D Brusselator reaction-diffusion problem.
 * This implementation follows the reference Python version closely, using the same
 * default parameters (step size control, tolerances, limits). The workspace buffer
 * is ignored – temporary memory is allocated with malloc; the buffer is provided
 * only to satisfy the required signature.
 */

/* Default algorithmic parameters (chosen to match the reference implementation). */
static const double H0 = 1.0e-3;                /* initial step size */
static const double CONV_DEGRADE = 0.3;        /* Newton convergence‑rate threshold */
static const double SAFETY = 0.9;              /* PI‑free BDF controller safety factor */
static const double MIN_FACTOR = 0.2;          /* min step‑size change factor */
static const double MAX_FACTOR = 6.0;          /* max step‑size change factor */

static const int MAX_ORDER = 5;                /* maximum BDF order */
static const int MAX_NEWTON = 12;              /* maximum Newton iterations */
static const int GMRES_RESTART = 20;           /* GMRES restart dimension */
static const double GMRES_TOL = 1e-6;          /* GMRES relative tolerance */

static const double R_TOL = 1e-6;              /* BDF local error relative tolerance */
static const double A_TOL = 0.0;               /* BDF local error absolute tolerance */
static const double NEWTON_R_TOL = 1e-12;      /* Newton corrector tolerance */

/* Helper: compute 5‑point Laplacian with zero‑flux (Neumann) mirrored ghosts. */
static void neumann_laplacian(const double *U, int N, double *L) {
    int i, j;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            double u_center = U[i * N + j];
            double u_im1 = (i == 0) ? U[(i + 1) * N + j] : U[(i - 1) * N + j];
            double u_ip1 = (i == N - 1) ? U[(i - 1) * N + j] : U[(i + 1) * N + j];
            double u_jm1 = (j == 0) ? U[i * N + (j + 1)] : U[i * N + (j - 1)];
            double u_jp1 = (j == N - 1) ? U[i * N + (j - 1)] : U[i * N + (j + 1)];
            L[i * N + j] = u_im1 + u_ip1 + u_jm1 + u_jp1 - 4.0 * u_center;
        }
    }
}

/* Compute RHS of the reaction‑diffusion system. */
static void brusselator_rhs(const double *u, const double *v, int N, double h_grid,
                             double alpha, double A, double B,
                             double *du, double *dv) {
    int i, j;
    double invh2 = 1.0 / (h_grid * h_grid);
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            int idx = i * N + j;
            double u_val = u[idx];
            double v_val = v[idx];
            /* Laplacians are computed on‑the‑fly to avoid extra temporaries. */
            double u_im1 = (i == 0) ? u[(i + 1) * N + j] : u[(i - 1) * N + j];
            double u_ip1 = (i == N - 1) ? u[(i - 1) * N + j] : u[(i + 1) * N + j];
            double u_jm1 = (j == 0) ? u[i * N + (j + 1)] : u[i * N + (j - 1)];
            double u_jp1 = (j == N - 1) ? u[i * N + (j - 1)] : u[i * N + (j + 1)];
            double lap_u = u_im1 + u_ip1 + u_jm1 + u_jp1 - 4.0 * u_val;

            double v_im1 = (i == 0) ? v[(i + 1) * N + j] : v[(i - 1) * N + j];
            double v_ip1 = (i == N - 1) ? v[(i - 1) * N + j] : v[(i + 1) * N + j];
            double v_jm1 = (j == 0) ? v[i * N + (j + 1)] : v[i * N + (j - 1)];
            double v_jp1 = (j == N - 1) ? v[i * N + (j - 1)] : v[i * N + (j + 1)];
            double lap_v = v_im1 + v_ip1 + v_jm1 + v_jp1 - 4.0 * v_val;

            du[idx] = A + u_val * u_val * v_val - (B + 1.0) * u_val + alpha * lap_u * invh2;
            dv[idx] = B * u_val - u_val * u_val * v_val + alpha * lap_v * invh2;
        }
    }
}

/* Apply (I - h*beta0*J) to (du, dv). The Jacobian J is evaluated at frozen state (uf, vf). */
static void newton_matvec(const double *du, const double *dv,
                          const double *uf, const double *vf,
                          int N, double h_grid, double alpha, double B, double hbeta0,
                          double *out_du, double *out_dv) {
    int i, j;
    double invh2 = 1.0 / (h_grid * h_grid);
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            int idx = i * N + j;
            double du_val = du[idx];
            double dv_val = dv[idx];
            double uf_val = uf[idx];
            double vf_val = vf[idx];

            /* Laplacians of du and dv */
            double du_im1 = (i == 0) ? du[(i + 1) * N + j] : du[(i - 1) * N + j];
            double du_ip1 = (i == N - 1) ? du[(i - 1) * N + j] : du[(i + 1) * N + j];
            double du_jm1 = (j == 0) ? du[i * N + (j + 1)] : du[i * N + (j - 1)];
            double du_jp1 = (j == N - 1) ? du[i * N + (j - 1)] : du[i * N + (j + 1)];
            double lap_du = du_im1 + du_ip1 + du_jm1 + du_jp1 - 4.0 * du_val;

            double dv_im1 = (i == 0) ? dv[(i + 1) * N + j] : dv[(i - 1) * N + j];
            double dv_ip1 = (i == N - 1) ? dv[(i - 1) * N + j] : dv[(i + 1) * N + j];
            double dv_jm1 = (j == 0) ? dv[i * N + (j + 1)] : dv[i * N + (j - 1)];
            double dv_jp1 = (j == N - 1) ? dv[i * N + (j - 1)] : dv[i * N + (j + 1)];
            double lap_dv = dv_im1 + dv_ip1 + dv_jm1 + dv_jp1 - 4.0 * dv_val;

            double term_u = (2.0 * uf_val * vf_val - (B + 1.0)) * du_val + (uf_val * uf_val) * dv_val + alpha * lap_du * invh2;
            double term_v = (B - 2.0 * uf_val * vf_val) * du_val - (uf_val * uf_val) * dv_val + alpha * lap_dv * invh2;

            out_du[idx] = du_val - hbeta0 * term_u;
            out_dv[idx] = dv_val - hbeta0 * term_v;
        }
    }
}

/* Weighted RMS norm over both fields (SUNDIALS convention). */
static double wrms_norm(const double *au, const double *av,
                        double abstol, double rtol,
                        const double *refu, const double *refv, int N) {
    double s = 0.0;
    int i, j;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            int idx = i * N + j;
            double wu_i = abstol + rtol * fabs(refu[idx]);
            double wv_i = abstol + rtol * fabs(refv[idx]);
            double du_i = au[idx] / wu_i;
            double dv_i = av[idx] / wv_i;
            s += du_i * du_i + dv_i * dv_i;
        }
    }
    return sqrt(s / (2.0 * N * N));
}

/* Compute Lagrange weights for predictor (deriv=0) or BDF coefficients (deriv=1). */
static void lagrange_weights(const double *nodes, int k1, int deriv,
                             int max_order, double *weights) {
    /* aug is (max_order+1) x (max_order+2) */
    double aug[ MAX_ORDER + 1 ][ MAX_ORDER + 2 ];
    int row, col;
    /* Zero the matrix */
    for (row = 0; row <= max_order; ++row) {
        for (col = 0; col <= max_order + 1; ++col) {
            aug[row][col] = 0.0;
        }
    }
    for (row = 0; row < k1; ++row) {
        for (col = 0; col < k1; ++col) {
            double power = 1.0;
            int p;
            for (p = 0; p < row; ++p) {
                power *= nodes[col];
            }
            aug[row][col] = power;
        }
    }
    aug[deriv][k1] = 1.0;
    for (col = 0; col < k1; ++col) {
        double pivot = aug[col][col];
        /* Scale pivot row */
        for (row = 0; row <= max_order + 1; ++row) {
            aug[col][row] /= pivot;
        }
        for (row = 0; row < k1; ++row) {
            if (row != col) {
                double factor = aug[row][col];
                for (int w = 0; w <= max_order + 1; ++w) {
                    aug[row][w] -= factor * aug[col][w];
                }
            }
        }
    }
    for (int j = 0; j < k1; ++j) {
        weights[j] = aug[j][k1];
    }
}

/* GMRES with restarts, matrix‑free using the exact Jacobian operator. */
static void gmres_matfree(const double *rhs_u, const double *rhs_v,
                           const double *uf, const double *vf,
                           int N, double h_grid, double alpha, double B,
                           double hbeta0, int restart, double tol,
                           double *du, double *dv) {
    int i, j, k, p, col, row;
    int max_vecs = restart + 1;
    int NN = N * N;
    /* Allocate Arnoldi basis vectors */
    double *qu = (double *)malloc(max_vecs * NN * sizeof(double));
    double *qv = (double *)malloc(max_vecs * NN * sizeof(double));
    double *hess = (double *)malloc((restart + 1) * restart * sizeof(double));
    double *cs = (double *)malloc(restart * sizeof(double));
    double *sn = (double *)malloc(restart * sizeof(double));
    double *g = (double *)malloc((restart + 1) * sizeof(double));
    double *wu = (double *)malloc(NN * sizeof(double));
    double *wv = (double *)malloc(NN * sizeof(double));
    double *y = (double *)malloc(restart * sizeof(double));

    double beta = 0.0;
    for (i = 0; i < N; ++i) {
        const double *ru = &rhs_u[i * N];
        const double *rv = &rhs_v[i * N];
        for (j = 0; j < N; ++j) {
            double a = ru[j];
            double b = rv[j];
            beta += a * a + b * b;
        }
    }
    beta = sqrt(beta);
    /* Initialise solution to zero */
    for (i = 0; i < NN; ++i) {
        du[i] = 0.0;
        dv[i] = 0.0;
    }
    if (beta == 0.0) {
        /* Clean up and exit */
        free(qu); free(qv); free(hess); free(cs); free(sn); free(g); free(wu); free(wv); free(y);
        return;
    }
    /* Normalise first basis vectors */
    double inv_beta = 1.0 / beta;
    for (i = 0; i < NN; ++i) {
        qu[i] = rhs_u[i] * inv_beta;
        qv[i] = rhs_v[i] * inv_beta;
    }
    g[0] = beta;
    int m_used = restart;
    for (k = 0; k < restart; ++k) {
        /* Apply Jacobian to current basis vector */
        newton_matvec(&qu[k * NN], &qv[k * NN], uf, vf, N, h_grid, alpha, B, hbeta0, wu, wv);
        /* Orthogonalise against previous basis vectors */
        for (p = 0; p <= k; ++p) {
            double h_pk = 0.0;
            const double *qp_u = &qu[p * NN];
            const double *qp_v = &qv[p * NN];
            for (i = 0; i < NN; ++i) {
                h_pk += qp_u[i] * wu[i] + qp_v[i] * wv[i];
            }
            hess[p * restart + k] = h_pk;
            for (i = 0; i < NN; ++i) {
                wu[i] -= h_pk * qp_u[i];
                wv[i] -= h_pk * qp_v[i];
            }
        }
        /* Compute next Hessenberg element */
        double h_next = 0.0;
        for (i = 0; i < NN; ++i) {
            h_next += wu[i] * wu[i] + wv[i] * wv[i];
        }
        h_next = sqrt(h_next);
        hess[(k + 1) * restart + k] = h_next; /* store in row k+1, col k */
        /* Apply previous Givens rotations */
        for (p = 0; p < k; ++p) {
            double temp = cs[p] * hess[p * restart + k] + sn[p] * hess[(p + 1) * restart + k];
            hess[(p + 1) * restart + k] = -sn[p] * hess[p * restart + k] + cs[p] * hess[(p + 1) * restart + k];
            hess[p * restart + k] = temp;
        }
        /* New rotation */
        double denom = sqrt(hess[k * restart + k] * hess[k * restart + k] + hess[(k + 1) * restart + k] * hess[(k + 1) * restart + k]);
        cs[k] = hess[k * restart + k] / denom;
        sn[k] = hess[(k + 1) * restart + k] / denom;
        hess[k * restart + k] = cs[k] * hess[k * restart + k] + sn[k] * hess[(k + 1) * restart + k];
        hess[(k + 1) * restart + k] = 0.0;
        double tmp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = tmp;
        double rel = fabs(g[k + 1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart - 1) {
            m_used = k + 1;
            break;
        }
        /* Normalise next basis vector */
        double inv_hnext = 1.0 / h_next;
        for (i = 0; i < NN; ++i) {
            qu[(k + 1) * NN + i] = wu[i] * inv_hnext;
            qv[(k + 1) * NN + i] = wv[i] * inv_hnext;
        }
    }
    /* Back‑substitution */
    for (row = 0; row < m_used; ++row) {
        int rr = m_used - 1 - row;
        double s_val = g[rr];
        for (col = rr + 1; col < m_used; ++col) {
            s_val -= hess[rr * restart + col] * y[col];
        }
        y[rr] = s_val / hess[rr * restart + rr];
    }
    /* Form the correction vectors */
    for (p = 0; p < m_used; ++p) {
        const double *qp_u = &qu[p * NN];
        const double *qp_v = &qv[p * NN];
        double coeff = y[p];
        for (i = 0; i < NN; ++i) {
            du[i] += qp_u[i] * coeff;
            dv[i] += qp_v[i] * coeff;
        }
    }
    /* Clean up */
    free(qu);
    free(qv);
    free(hess);
    free(cs);
    free(sn);
    free(g);
    free(wu);
    free(wv);
    free(y);
}

void bdf_newton_krylov_fp64(double *restrict diagnostics,
                            int64_t *restrict order_history,
                            double *restrict u,
                            double *restrict v,
                            const double A,
                            const double B,
                            const int64_t N_in,
                            const double alpha,
                            const int64_t max_steps_in,
                            const double t_end,
                            uint8_t *restrict workspace,
                            const int64_t workspace_size) {
    /* Convert integer parameters to int for convenience */
    int N = (int)N_in;
    int max_steps = (int)max_steps_in;
    const int maxhist = MAX_ORDER + 1;
    const int NN = N * N;

    /* Allocate history arrays */
    double *hist_u = (double *)calloc(maxhist * NN, sizeof(double));
    double *hist_v = (double *)calloc(maxhist * NN, sizeof(double));
    double *hist_t = (double *)calloc(maxhist, sizeof(double));
    if (!hist_u || !hist_v || !hist_t) {
        /* Allocation failure – nothing to do */
        free(hist_u);
        free(hist_v);
        free(hist_t);
        return;
    }
    /* Initialise history with the current state */
    memcpy(&hist_u[0], u, NN * sizeof(double));
    memcpy(&hist_v[0], v, NN * sizeof(double));
    hist_t[0] = 0.0;
    int n_hist = 1;

    /* Allocate auxiliary vectors */
    double *uf = (double *)calloc(NN, sizeof(double));
    double *vf = (double *)calloc(NN, sizeof(double));
    double *u_pred = (double *)calloc(NN, sizeof(double));
    double *v_pred = (double *)calloc(NN, sizeof(double));
    double *u_trial = (double *)calloc(NN, sizeof(double));
    double *v_trial = (double *)calloc(NN, sizeof(double));
    double *rhs_u = (double *)calloc(NN, sizeof(double));
    double *rhs_v = (double *)calloc(NN, sizeof(double));
    double *f_u = (double *)calloc(NN, sizeof(double));
    double *f_v = (double *)calloc(NN, sizeof(double));
    double *res_u = (double *)calloc(NN, sizeof(double));
    double *res_v = (double *)calloc(NN, sizeof(double));
    double *neg_res_u = (double *)calloc(NN, sizeof(double));
    double *neg_res_v = (double *)calloc(NN, sizeof(double));
    double *err_u = (double *)calloc(NN, sizeof(double));
    double *err_v = (double *)calloc(NN, sizeof(double));
    double *step_du = (double *)calloc(NN, sizeof(double));
    double *step_dv = (double *)calloc(NN, sizeof(double));
    double *nodes_p = (double *)calloc(maxhist, sizeof(double));
    double *nodes_c = (double *)calloc(maxhist, sizeof(double));
    double *w_pred = (double *)calloc(maxhist, sizeof(double));
    double *c_corr = (double *)calloc(maxhist, sizeof(double));
    if (!uf || !vf || !u_pred || !v_pred || !u_trial || !v_trial ||
        !rhs_u || !rhs_v || !f_u || !f_v || !res_u || !res_v ||
        !neg_res_u || !neg_res_v || !err_u || !err_v ||
        !step_du || !step_dv || !nodes_p || !nodes_c || !w_pred || !c_corr) {
        /* Allocation failure */
        free(hist_u); free(hist_v); free(hist_t);
        free(uf); free(vf); free(u_pred); free(v_pred); free(u_trial); free(v_trial);
        free(rhs_u); free(rhs_v); free(f_u); free(f_v);
        free(res_u); free(res_v); free(neg_res_u); free(neg_res_v);
        free(err_u); free(err_v); free(step_du); free(step_dv);
        free(nodes_p); free(nodes_c); free(w_pred); free(c_corr);
        return;
    }

    double t = 0.0;
    double h = H0;
    int order = 1;
    int steps_since_order_change = 0;
    int consecutive_reject = 0;
    int njev = 0;
    int nsteps = 0;
    int have_jac = 0;
    double h_grid = 1.0 / (double)N;

    while (t < t_end && nsteps < max_steps) {
        if (t + h > t_end) {
            h = t_end - t;
        }
        int k = order < n_hist ? order : n_hist;
        double t_new = t + h;
        /* Predictor nodes */
        int deg = k < (n_hist - 1) ? k : (n_hist - 1);
        for (int j = 0; j <= deg; ++j) {
            nodes_p[j] = hist_t[j] - t_new;
        }
        lagrange_weights(nodes_p, deg + 1, 0, MAX_ORDER, w_pred);
        /* Initialise prediction */
        for (int i = 0; i < NN; ++i) {
            u_pred[i] = 0.0;
            v_pred[i] = 0.0;
        }
        for (int j = 0; j <= deg; ++j) {
            double wj = w_pred[j];
            const double *hu = &hist_u[j * NN];
            const double *hv = &hist_v[j * NN];
            for (int i = 0; i < NN; ++i) {
                u_pred[i] += wj * hu[i];
                v_pred[i] += wj * hv[i];
            }
        }
        /* Corrector nodes */
        nodes_c[0] = 0.0;
        for (int j = 0; j < k; ++j) {
            nodes_c[j + 1] = hist_t[j] - t_new;
        }
        lagrange_weights(nodes_c, k + 1, 1, MAX_ORDER, c_corr);
        double c0 = c_corr[0];
        double hbeta0 = 1.0 / c0;
        /* Compute RHS = -sum_j (c_j/c0) * hist_u[j] */
        for (int i = 0; i < NN; ++i) {
            rhs_u[i] = 0.0;
            rhs_v[i] = 0.0;
        }
        for (int j = 1; j <= k; ++j) {
            double coeff = -(c_corr[j] / c0);
            const double *hu = &hist_u[(j - 1) * NN];
            const double *hv = &hist_v[(j - 1) * NN];
            for (int i = 0; i < NN; ++i) {
                rhs_u[i] += coeff * hu[i];
                rhs_v[i] += coeff * hv[i];
            }
        }
        /* Initialise trial solution */
        memcpy(u_trial, u_pred, NN * sizeof(double));
        memcpy(v_trial, v_pred, NN * sizeof(double));
        if (!have_jac) {
            memcpy(uf, u_trial, NN * sizeof(double));
            memcpy(vf, v_trial, NN * sizeof(double));
            ++njev;
            have_jac = 1;
        }
        int converged = 0;
        int refactored_this_step = 0;
        double prev_resnorm = -1.0;
        for (int it = 0; it < MAX_NEWTON; ++it) {
            brusselator_rhs(u_trial, v_trial, N, h_grid, alpha, A, B, f_u, f_v);
            for (int i = 0; i < NN; ++i) {
                res_u[i] = u_trial[i] - hbeta0 * f_u[i] - rhs_u[i];
                res_v[i] = v_trial[i] - hbeta0 * f_v[i] - rhs_v[i];
            }
            double resnorm = wrms_norm(res_u, res_v, NEWTON_R_TOL, NEWTON_R_TOL, u_trial, v_trial, N);
            if (resnorm < 1.0) {
                converged = 1;
                break;
            }
            if (prev_resnorm > 0.0) {
                double rate = resnorm / prev_resnorm;
                if (rate > CONV_DEGRADE && refactored_this_step == 0) {
                    memcpy(uf, u_trial, NN * sizeof(double));
                    memcpy(vf, v_trial, NN * sizeof(double));
                    ++njev;
                    refactored_this_step = 1;
                }
            }
            prev_resnorm = resnorm;
            for (int i = 0; i < NN; ++i) {
                neg_res_u[i] = -res_u[i];
                neg_res_v[i] = -res_v[i];
            }
            gmres_matfree(neg_res_u, neg_res_v, uf, vf, N, h_grid, alpha, B, hbeta0,
                          GMRES_RESTART, GMRES_TOL, step_du, step_dv);
            for (int i = 0; i < NN; ++i) {
                u_trial[i] += step_du[i];
                v_trial[i] += step_dv[i];
            }
        }
        if (!converged) {
            h *= 0.25;
            if (order > 1) order--;
            continue;
        }
        double c_err = 1.0 / (k + 1);
        for (int i = 0; i < NN; ++i) {
            err_u[i] = c_err * (u_trial[i] - u_pred[i]);
            err_v[i] = c_err * (v_trial[i] - v_pred[i]);
        }
        double err_est = wrms_norm(err_u, err_v, A_TOL, R_TOL, u_trial, v_trial, N);
        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            order_history[nsteps] = order;
            ++nsteps;
            /* Shift history */
            for (int j = maxhist - 1; j > 0; --j) {
                memcpy(&hist_u[j * NN], &hist_u[(j - 1) * NN], NN * sizeof(double));
                memcpy(&hist_v[j * NN], &hist_v[(j - 1) * NN], NN * sizeof(double));
                hist_t[j] = hist_t[j - 1];
            }
            memcpy(&hist_u[0], u_trial, NN * sizeof(double));
            memcpy(&hist_v[0], v_trial, NN * sizeof(double));
            hist_t[0] = t;
            if (n_hist < maxhist) ++n_hist;
            ++steps_since_order_change;
            if (order < MAX_ORDER && steps_since_order_change > order && n_hist > order + 1) {
                ++order;
                steps_since_order_change = 0;
            } else if (refactored_this_step && order > 1) {
                --order;
                steps_since_order_change = 0;
            }
            double fac;
            if (err_est > 0.0) {
                fac = SAFETY * pow(err_est, -1.0 / (k + 1));
            } else {
                fac = MAX_FACTOR;
            }
            fac = fmin(MAX_FACTOR, fmax(MIN_FACTOR, fac));
            h *= fac;
        } else {
            ++consecutive_reject;
            double fac = SAFETY * pow(err_est, -1.0 / (k + 1));
            fac = fmin(1.0, fmax(MIN_FACTOR, fac));
            h *= fac;
            if (consecutive_reject >= 2 && order > 1) {
                --order;
                steps_since_order_change = 0;
            }
        }
    }
    /* Copy final solution back to output arrays */
    memcpy(u, &hist_u[0], NN * sizeof(double));
    memcpy(v, &hist_v[0], NN * sizeof(double));
    diagnostics[0] = (double)nsteps;
    diagnostics[1] = (double)njev;
    diagnostics[2] = t;
    /* Free allocated memory */
    free(hist_u); free(hist_v); free(hist_t);
    free(uf); free(vf); free(u_pred); free(v_pred); free(u_trial); free(v_trial);
    free(rhs_u); free(rhs_v); free(f_u); free(f_v);
    free(res_u); free(res_v); free(neg_res_u); free(neg_res_v);
    free(err_u); free(err_v); free(step_du); free(step_dv);
    free(nodes_p); free(nodes_c); free(w_pred); free(c_corr);
}

