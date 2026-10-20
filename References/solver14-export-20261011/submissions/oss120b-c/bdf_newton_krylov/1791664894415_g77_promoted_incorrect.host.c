/* BDF Newton-Krylov integrator kernel (fp64) */
/* This implementation follows the reference algorithm in bdf_newton_krylov_numpy.py.
   All parameters are fixed to the benchmark defaults. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>

static const double H0 = 1.0e-3;
static const double SAFETY = 0.9;
static const double MIN_FACTOR = 0.2;
static const double MAX_FACTOR = 6.0;
static const double CONV_DEGRADE = 0.3;
static const double R_TOL = 1.0e-5;
static const double ABS_TOL = 1.0e-8;
static const double NEWTON_R_TOL = 1.0e-12;
static const int MAX_ORDER = 5;
static const int MAX_NEWTON = 12;
static const int GMRES_RESTART = 30;
static const double GMRES_TOL = 1.0e-5;

static double *gmres_qu = NULL;
static double *gmres_qv = NULL;
static double *gmres_hess = NULL;
static double *gmres_cs = NULL;
static double *gmres_sn = NULL;
static double *gmres_g = NULL;
static double *gmres_wu = NULL;
static double *gmres_wv = NULL;
static double *gmres_y = NULL;
static double *gmres_lap_du = NULL;
static double *gmres_lap_dv = NULL;

static void neumann_laplacian(const double *U, int64_t N, double *L) {
    int64_t i, j;
    // interior points
    #pragma omp parallel for private(i,j)
    for (i = 1; i < N-1; ++i) {
        for (j = 1; j < N-1; ++j) {
            L[i*N + j] = U[(i-1)*N + j] + U[(i+1)*N + j] +
                         U[i*N + (j-1)] + U[i*N + (j+1)] - 4.0 * U[i*N + j];
        }
    }
    // top boundary (i = 0, j = 1..N-2)
    #pragma omp parallel for private(j)
    for (j = 1; j < N-1; ++j) {
        L[0*N + j] = 2.0*U[1*N + j] + U[0*N + (j-1)] + U[0*N + (j+1)] - 4.0*U[0*N + j];
    }
    // bottom boundary (i = N-1)
    #pragma omp parallel for private(j)
    for (j = 1; j < N-1; ++j) {
        L[(N-1)*N + j] = 2.0*U[(N-2)*N + j] + U[(N-1)*N + (j-1)] + U[(N-1)*N + (j+1)] - 4.0*U[(N-1)*N + j];
    }
    // left boundary (j = 0, i = 1..N-2)
    #pragma omp parallel for private(i)
    for (i = 1; i < N-1; ++i) {
        L[i*N + 0] = U[(i-1)*N + 0] + U[(i+1)*N + 0] + 2.0*U[i*N + 1] - 4.0*U[i*N + 0];
    }
    // right boundary (j = N-1)
    #pragma omp parallel for private(i)
    for (i = 1; i < N-1; ++i) {
        L[i*N + (N-1)] = U[(i-1)*N + (N-1)] + U[(i+1)*N + (N-1)] + 2.0*U[i*N + (N-2)] - 4.0*U[i*N + (N-1)];
    }
    // corners
    L[0] = 2.0*U[1] + 2.0*U[N] - 4.0*U[0];
    L[N-1] = 2.0*U[N + (N-1)] + 2.0*U[N-2] - 4.0*U[N-1];
    L[(N-1)*N] = 2.0*U[(N-2)*N] + 2.0*U[(N-1)*N + 1] - 4.0*U[(N-1)*N];
    L[N*N - 1] = 2.0*U[(N-2)*N + (N-1)] + 2.0*U[(N-1)*N + (N-2)] - 4.0*U[N*N - 1];
}

static void brusselator_rhs(const double *u, const double *v,
                            int64_t N, double h_grid, double alpha,
                            double A, double B,
                            double *du, double *dv,
                            double *lap_u, double *lap_v) {
    neumann_laplacian(u, N, lap_u);
    neumann_laplacian(v, N, lap_v);
    double invh2 = 1.0 / (h_grid * h_grid);
        for (int64_t i = 0; i < N*N; ++i) {
        double ui = u[i];
        double vi = v[i];
        du[i] = A + ui*ui*vi - (B + 1.0)*ui + alpha * lap_u[i] * invh2;
        dv[i] = B * ui - ui*ui*vi + alpha * lap_v[i] * invh2;
    }
}

static void newton_matvec(const double *du, const double *dv,
                          const double *uf, const double *vf,
                          int64_t N, double h, double alpha,
                          double B, double hbeta0,
                          double *out_du, double *out_dv,
                          double *lap_du, double *lap_dv) {
    neumann_laplacian(du, N, lap_du);
    neumann_laplacian(dv, N, lap_dv);
    double invh2 = 1.0 / (h * h);
        for (int64_t i = 0; i < N*N; ++i) {
        double ufi = uf[i];
        double vfi = vf[i];
        double du_i = du[i];
        double dv_i = dv[i];
        double term_u = (2.0 * ufi * vfi - (B + 1.0)) * du_i
                      + (ufi * ufi) * dv_i
                      + alpha * lap_du[i] * invh2;
        double term_v = (B - 2.0 * ufi * vfi) * du_i
                      - (ufi * ufi) * dv_i
                      + alpha * lap_dv[i] * invh2;
        out_du[i] = du_i - hbeta0 * term_u;
        out_dv[i] = dv_i - hbeta0 * term_v;
    }
}

static double wrms_norm(const double *au, const double *av,
                        double abstol, double rtol,
                        const double *refu, const double *refv,
                        int64_t N) {
    double s = 0.0;
    for (int64_t i = 0; i < N; ++i) {
        int64_t row = i * N;
        for (int64_t j = 0; j < N; ++j) {
            double w_u = abstol + rtol * fabs(refu[row + j]);
            double w_v = abstol + rtol * fabs(refv[row + j]);
            double du = au[row + j] / w_u;
            double dv = av[row + j] / w_v;
            s += du*du + dv*dv;
        }
    }
    return sqrt(s / (2.0 * N * N));
}

static void lagrange_weights(const double *nodes, int k1, int deriv,
                             int max_order, double *weights) {
    int rows = max_order + 1;
    int cols = max_order + 2;
    double *aug = (double*)calloc(rows * cols, sizeof(double));
    for (int row = 0; row < k1; ++row) {
        for (int col = 0; col < k1; ++col) {
            double power = 1.0;
            for (int p = 0; p < row; ++p) {
                power *= nodes[col];
            }
            aug[row * cols + col] = power;
        }
    }
    aug[deriv * cols + k1] = 1.0;
    for (int col = 0; col < k1; ++col) {
        double pivot = aug[col * cols + col];
        for (int w = 0; w < cols; ++w) {
            aug[col * cols + w] /= pivot;
        }
        for (int row = 0; row < k1; ++row) {
            if (row == col) continue;
            double factor = aug[row * cols + col];
            if (factor == 0.0) continue;
            for (int w = 0; w < cols; ++w) {
                aug[row * cols + w] -= factor * aug[col * cols + w];
            }
        }
    }
    for (int j = 0; j < k1; ++j) {
        weights[j] = aug[j * cols + k1];
    }
    free(aug);
}
static void gmres_matfree(const double *rhs_u, const double *rhs_v,
                          const double *uf, const double *vf,
                          int64_t N, double h, double alpha, double B,
                          double hbeta0,
                          int restart, double tol,
                          double *step_du, double *step_dv) {
    int64_t nn = N * N;
    if (gmres_qu == NULL) {
        gmres_qu = (double*)malloc((restart+1) * nn * sizeof(double));
        gmres_qv = (double*)malloc((restart+1) * nn * sizeof(double));
        gmres_hess = (double*)malloc((restart+1) * restart * sizeof(double));
        gmres_cs = (double*)malloc(restart * sizeof(double));
        gmres_sn = (double*)malloc(restart * sizeof(double));
        gmres_g = (double*)malloc((restart+1) * sizeof(double));
        gmres_wu = (double*)malloc(nn * sizeof(double));
        gmres_wv = (double*)malloc(nn * sizeof(double));
        gmres_y = (double*)malloc(restart * sizeof(double));
        gmres_lap_du = (double*)malloc(nn * sizeof(double));
        gmres_lap_dv = (double*)malloc(nn * sizeof(double));
    }
    double *qu = gmres_qu;
    double *qv = gmres_qv;
    double *hess = gmres_hess;
    double *cs = gmres_cs;
    double *sn = gmres_sn;
    double *g = gmres_g;
    double *wu = gmres_wu;
    double *wv = gmres_wv;
    double *y = gmres_y;
    double *lap_du = gmres_lap_du;
    double *lap_dv = gmres_lap_dv;
    double beta = 0.0;
        for (int64_t i = 0; i < N; ++i) {
        int64_t row = i * N;
        double sum = 0.0;
        for (int64_t j = 0; j < N; ++j) {
            double ru = rhs_u[row + j];
            double rv = rhs_v[row + j];
            sum += ru*ru + rv*rv;
        }
        beta += sum;
    }
    beta = sqrt(beta);
    for (int64_t i = 0; i < nn; ++i) { step_du[i] = 0.0; step_dv[i] = 0.0; }
    if (beta == 0.0) {
        /* buffers retained */
        /* buffers retained */
        return;
    }
        for (int64_t i = 0; i < nn; ++i) {
        qu[i] = rhs_u[i] / beta;
        qv[i] = rhs_v[i] / beta;
    }
    g[0] = beta;
    int m_used = restart;
    for (int k = 0; k < restart; ++k) {
        newton_matvec(&qu[k*nn], &qv[k*nn], uf, vf, N, h, alpha, B, hbeta0, wu, wv, lap_du, lap_dv);
        for (int p = 0; p <= k; ++p) {
            double dot_u = cblas_ddot(nn, &qu[p*nn], 1, wu, 1);
            double dot_v = cblas_ddot(nn, &qv[p*nn], 1, wv, 1);
            double h_pk = dot_u + dot_v;
            hess[p*restart + k] = h_pk;
            cblas_daxpy(nn, -h_pk, &qu[p*nn], 1, wu, 1);
            cblas_daxpy(nn, -h_pk, &qv[p*nn], 1, wv, 1);
        }
        double h_next = 0.0;
                for (int64_t i = 0; i < N; ++i) {
            int64_t row = i * N;
            double sum_u = 0.0, sum_v = 0.0;
            for (int64_t j = 0; j < N; ++j) {
                double uu = wu[row + j];
                double vv = wv[row + j];
                sum_u += uu * uu;
                sum_v += vv * vv;
            }
            h_next += sum_u + sum_v;
        }
        h_next = sqrt(h_next);
        hess[(k+1)*restart + k] = h_next;
        for (int p = 0; p < k; ++p) {
            double temp = cs[p] * hess[p*restart + k] + sn[p] * hess[(p+1)*restart + k];
            hess[(p+1)*restart + k] = -sn[p] * hess[p*restart + k] + cs[p] * hess[(p+1)*restart + k];
            hess[p*restart + k] = temp;
        }
        double denom = sqrt(hess[k*restart + k] * hess[k*restart + k] + h_next * h_next);
        if (denom == 0.0) { cs[k] = 1.0; sn[k] = 0.0; } else { cs[k] = hess[k*restart + k] / denom; sn[k] = h_next / denom; }
        hess[k*restart + k] = cs[k] * hess[k*restart + k] + sn[k] * h_next;
        hess[(k+1)*restart + k] = 0.0;
        double temp = cs[k] * g[k];
        g[k+1] = -sn[k] * g[k];
        g[k] = temp;
        double rel = fabs(g[k+1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart-1) { m_used = k + 1; break; }
                for (int64_t i = 0; i < nn; ++i) { qu[(k+1)*nn + i] = wu[i] / h_next; qv[(k+1)*nn + i] = wv[i] / h_next; }
    }
    for (int row = 0; row < m_used; ++row) {
        int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; ++col) {
            s -= hess[rr*restart + col] * y[col];
        }
        y[rr] = s / hess[rr*restart + rr];
    }
    for (int p = 0; p < m_used; ++p) {
        double coeff = y[p];
                for (int64_t i = 0; i < nn; ++i) { step_du[i] += qu[p*nn + i] * coeff; step_dv[i] += qv[p*nn + i] * coeff; }
    }
    /* buffers retained */
    /* buffers retained */
}

#if 0
static void gmres_matfree(const double *rhs_u, const double *rhs_v,
                          const double *uf, const double *vf,
                          int64_t N, double h, double alpha, double B,
                          double hbeta0,
                          int restart, double tol,
                          double *step_du, double *step_dv) {
    int64_t nn = N * N;
    if (gmres_qu == NULL) {
        gmres_qu = (double*)malloc((restart+1) * nn * sizeof(double));
        gmres_qv = (double*)malloc((restart+1) * nn * sizeof(double));
        gmres_hess = (double*)malloc((restart+1) * restart * sizeof(double));
        gmres_cs = (double*)malloc(restart * sizeof(double));
        gmres_sn = (double*)malloc(restart * sizeof(double));
        gmres_g = (double*)malloc((restart+1) * sizeof(double));
        gmres_wu = (double*)malloc(nn * sizeof(double));
        gmres_wv = (double*)malloc(nn * sizeof(double));
        gmres_y = (double*)malloc(restart * sizeof(double));
        gmres_lap_du = (double*)malloc(nn * sizeof(double));
        gmres_lap_dv = (double*)malloc(nn * sizeof(double));
    }
    double *qu = gmres_qu;
    double *qv = gmres_qv;
    double *hess = gmres_hess;
    double *cs = gmres_cs;
    double *sn = gmres_sn;
    double *g = gmres_g;
    double *wu = gmres_wu;
    double *wv = gmres_wv;
    double *y = gmres_y;
    double *lap_du = gmres_lap_du;
    double *lap_dv = gmres_lap_dv;
    double beta = 0.0;
        for (int64_t i = 0; i < N; ++i) {
        int64_t row = i * N;
        double sum = 0.0;
        for (int64_t j = 0; j < N; ++j) {
            double ru = rhs_u[row + j];
            double rv = rhs_v[row + j];
            sum += ru*ru + rv*rv;
        }
        beta += sum;
    }
    beta = sqrt(beta);
    for (int64_t i = 0; i < nn; ++i) { step_du[i] = 0.0; step_dv[i] = 0.0; }
    if (beta == 0.0) {
        /* buffers retained */
        /* buffers retained */
        return;
    }
        for (int64_t i = 0; i < nn; ++i) {
        qu[i] = rhs_u[i] / beta;
        qv[i] = rhs_v[i] / beta;
    }
    g[0] = beta;
    int m_used = restart;
    for (int k = 0; k < restart; ++k) {
        newton_matvec(&qu[k*nn], &qv[k*nn], uf, vf, N, h, alpha, B, hbeta0, wu, wv, lap_du, lap_dv);
        for (int p = 0; p <= k; ++p) {
            double dot_u = cblas_ddot(nn, &qu[p*nn], 1, wu, 1);
            double dot_v = cblas_ddot(nn, &qv[p*nn], 1, wv, 1);
            double h_pk = dot_u + dot_v;
            hess[p*restart + k] = h_pk;
            cblas_daxpy(nn, -h_pk, &qu[p*nn], 1, wu, 1);
            cblas_daxpy(nn, -h_pk, &qv[p*nn], 1, wv, 1);
        }
        double h_next = cblas_dnrm2(nn, wu, 1);
        double wv_norm = cblas_dnrm2(nn, wv, 1);
        h_next = sqrt(h_next*h_next + wv_norm*wv_norm);
        hess[(k+1)*restart + k] = h_next;
        for (int p = 0; p < k; ++p) {
            double temp = cs[p] * hess[p*restart + k] + sn[p] * hess[(p+1)*restart + k];
            hess[(p+1)*restart + k] = -sn[p] * hess[p*restart + k] + cs[p] * hess[(p+1)*restart + k];
            hess[p*restart + k] = temp;
        }
        double denom = sqrt(hess[k*restart + k] * hess[k*restart + k] + h_next * h_next);
        if (denom == 0.0) { cs[k] = 1.0; sn[k] = 0.0; } else { cs[k] = hess[k*restart + k] / denom; sn[k] = h_next / denom; }
        hess[k*restart + k] = cs[k] * hess[k*restart + k] + sn[k] * h_next;
        hess[(k+1)*restart + k] = 0.0;
        double temp = cs[k] * g[k];
        g[k+1] = -sn[k] * g[k];
        g[k] = temp;
        double rel = fabs(g[k+1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart-1) { m_used = k + 1; break; }
                for (int64_t i = 0; i < nn; ++i) { qu[(k+1)*nn + i] = wu[i] / h_next; qv[(k+1)*nn + i] = wv[i] / h_next; }
    }
    for (int row = 0; row < m_used; ++row) {
        int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; ++col) {
            s -= hess[rr*restart + col] * y[col];
        }
        y[rr] = s / hess[rr*restart + rr];
    }
    for (int p = 0; p < m_used; ++p) {
        double coeff = y[p];
        cblas_daxpy(nn, coeff, &qu[p*nn], 1, step_du, 1);
        cblas_daxpy(nn, coeff, &qv[p*nn], 1, step_dv, 1);
    }
    /* buffers retained */
    /* buffers retained */
}
#endif

void bdf_newton_krylov_fp64(double *restrict diagnostics,
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
    const int maxhist = MAX_ORDER + 1;
    omp_set_num_threads(1);
    omp_set_dynamic(0);
    int64_t n_hist = 1;
    double *hist_u = (double*)malloc(maxhist * N * N * sizeof(double));
    double *hist_v = (double*)malloc(maxhist * N * N * sizeof(double));
    double *hist_t = (double*)malloc(maxhist * sizeof(double));
    memcpy(&hist_u[0], u, N*N*sizeof(double));
    memcpy(&hist_v[0], v, N*N*sizeof(double));
    hist_t[0] = 0.0;
    double *uf = (double*)malloc(N*N*sizeof(double));
    double *vf = (double*)malloc(N*N*sizeof(double));
    double *u_pred = (double*)malloc(N*N*sizeof(double));
    double *v_pred = (double*)malloc(N*N*sizeof(double));
    double *u_trial = (double*)malloc(N*N*sizeof(double));
    double *v_trial = (double*)malloc(N*N*sizeof(double));
    double *rhs_u = (double*)malloc(N*N*sizeof(double));
    double *rhs_v = (double*)malloc(N*N*sizeof(double));
    double *f_u = (double*)malloc(N*N*sizeof(double));
    double *f_v = (double*)malloc(N*N*sizeof(double));
    double *res_u = (double*)malloc(N*N*sizeof(double));
    double *res_v = (double*)malloc(N*N*sizeof(double));
    double *neg_res_u = (double*)malloc(N*N*sizeof(double));
    double *neg_res_v = (double*)malloc(N*N*sizeof(double));
    double *err_u = (double*)malloc(N*N*sizeof(double));
    double *err_v = (double*)malloc(N*N*sizeof(double));
    double *step_du = (double*)malloc(N*N*sizeof(double));
    double *step_dv = (double*)malloc(N*N*sizeof(double));
    double *nodes_p = (double*)malloc(maxhist * sizeof(double));
    double *nodes_c = (double*)malloc(maxhist * sizeof(double));
    double *w_pred = (double*)malloc(maxhist * sizeof(double));
    double *c_corr = (double*)malloc(maxhist * sizeof(double));
    double *lap_u = (double*)malloc(N*N*sizeof(double));
    double *lap_v = (double*)malloc(N*N*sizeof(double));
    double *lap_du = (double*)malloc(N*N*sizeof(double));
    double *lap_dv = (double*)malloc(N*N*sizeof(double));
    double t = 0.0;
    double h = H0;
    int order = 1;
    int steps_since_order_change = 0;
    int consecutive_reject = 0;
    int64_t njev = 0;
    int64_t nsteps = 0;
    int have_jac = 0;
    double h_grid = 1.0 / (double)N;
    while (t < t_end && nsteps < max_steps) {
        if (t + h > t_end) h = t_end - t;
        int k = order < n_hist ? order : (int)n_hist;
        double t_new = t + h;
        int deg = (k < (int)n_hist) ? k : ((int)n_hist - 1);
        for (int j = 0; j <= deg; ++j) nodes_p[j] = hist_t[j] - t_new;
        lagrange_weights(nodes_p, deg + 1, 0, MAX_ORDER, w_pred);
        memset(u_pred, 0, N*N*sizeof(double));
        memset(v_pred, 0, N*N*sizeof(double));
        for (int j = 0; j <= deg; ++j) {
            double wj = w_pred[j];
            const double *hu = &hist_u[j * N * N];
            const double *hv = &hist_v[j * N * N];
                        for (int64_t i = 0; i < N*N; ++i) { u_pred[i] += wj * hu[i]; v_pred[i] += wj * hv[i]; }
        }
        nodes_c[0] = 0.0;
        for (int j = 0; j < k; ++j) nodes_c[j+1] = hist_t[j] - t_new;
        lagrange_weights(nodes_c, k + 1, 1, MAX_ORDER, c_corr);
        double c0 = c_corr[0];
        double hbeta0 = 1.0 / c0;
        memset(rhs_u, 0, N*N*sizeof(double));
        memset(rhs_v, 0, N*N*sizeof(double));
        for (int j = 1; j <= k; ++j) {
            double coeff = c_corr[j] / c0;
            const double *hu = &hist_u[(j-1) * N * N];
            const double *hv = &hist_v[(j-1) * N * N];
                        for (int64_t i = 0; i < N*N; ++i) { rhs_u[i] -= coeff * hu[i]; rhs_v[i] -= coeff * hv[i]; }
        }
        memcpy(u_trial, u_pred, N*N*sizeof(double));
        memcpy(v_trial, v_pred, N*N*sizeof(double));
        if (!have_jac) { memcpy(uf, u_trial, N*N*sizeof(double)); memcpy(vf, v_trial, N*N*sizeof(double)); njev++; have_jac = 1; }
        int converged = 0;
        int refactored_this_step = 0;
        double prev_resnorm = -1.0;
        for (int it = 0; it < MAX_NEWTON; ++it) {
            brusselator_rhs(u_trial, v_trial, N, h_grid, alpha, A, B, f_u, f_v, lap_u, lap_v);
                        for (int64_t i = 0; i < N*N; ++i) { res_u[i] = u_trial[i] - hbeta0 * f_u[i] - rhs_u[i]; res_v[i] = v_trial[i] - hbeta0 * f_v[i] - rhs_v[i]; }
            double resnorm = wrms_norm(res_u, res_v, NEWTON_R_TOL, NEWTON_R_TOL, u_trial, v_trial, N);
            if (resnorm < 1.0) { converged = 1; break; }
            if (prev_resnorm > 0.0) { double rate = resnorm / prev_resnorm; if (rate > CONV_DEGRADE && !refactored_this_step) { memcpy(uf, u_trial, N*N*sizeof(double)); memcpy(vf, v_trial, N*N*sizeof(double)); njev++; refactored_this_step = 1; } }
            prev_resnorm = resnorm;
                        for (int64_t i = 0; i < N*N; ++i) { neg_res_u[i] = -res_u[i]; neg_res_v[i] = -res_v[i]; }
            gmres_matfree(neg_res_u, neg_res_v, uf, vf, N, h_grid, alpha, B, hbeta0, GMRES_RESTART, GMRES_TOL, step_du, step_dv);
                        for (int64_t i = 0; i < N*N; ++i) { u_trial[i] += step_du[i]; v_trial[i] += step_dv[i]; }
        }
        if (!converged) { h *= 0.25; if (order > 1) order--; continue; }
        double c_err = 1.0 / (k + 1.0);
                for (int64_t i = 0; i < N*N; ++i) { err_u[i] = c_err * (u_trial[i] - u_pred[i]); err_v[i] = c_err * (v_trial[i] - v_pred[i]); }
        double err_est = wrms_norm(err_u, err_v, ABS_TOL, R_TOL, u_trial, v_trial, N);
        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            order_history[nsteps] = order;
            nsteps++;
            for (int j = maxhist - 1; j > 0; --j) {
                memcpy(&hist_u[j * N * N], &hist_u[(j-1) * N * N], N*N*sizeof(double));
                memcpy(&hist_v[j * N * N], &hist_v[(j-1) * N * N], N*N*sizeof(double));
                hist_t[j] = hist_t[j-1];
            }
            memcpy(&hist_u[0], u_trial, N*N*sizeof(double));
            memcpy(&hist_v[0], v_trial, N*N*sizeof(double));
            hist_t[0] = t;
            if (n_hist < maxhist) n_hist++;
            steps_since_order_change++;
            if (order < MAX_ORDER && steps_since_order_change > order && n_hist > order + 1) { order++; steps_since_order_change = 0; }
            else if (refactored_this_step && order > 1) { order--; steps_since_order_change = 0; }
            double fac;
            if (err_est > 0.0) fac = SAFETY * pow(err_est, -1.0 / (k + 1.0)); else fac = MAX_FACTOR;
            fac = fmin(MAX_FACTOR, fmax(MIN_FACTOR, fac));
            h = h * fac;
        } else {
            consecutive_reject++;
            double fac = SAFETY * pow(err_est, -1.0 / (k + 1.0));
            fac = fmin(1.0, fmax(MIN_FACTOR, fac));
            h = h * fac;
            if (consecutive_reject >= 2 && order > 1) { order--; steps_since_order_change = 0; }
        }
    }
    memcpy(u, &hist_u[0], N*N*sizeof(double));
    memcpy(v, &hist_v[0], N*N*sizeof(double));
    diagnostics[0] = (double)nsteps;
    diagnostics[1] = (double)njev;
    diagnostics[2] = t;
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
    free(nodes_p); free(nodes_c);
    free(w_pred); free(c_corr);
    free(lap_u); free(lap_v); free(lap_du); free(lap_dv);
}
