/* Variable-order variable-step BDF integrator over a Newton-Krylov corrector.
 * Faithful C port of bdf_newton_krylov_numpy.py.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* ---- module constants of the reference ---- */
#define H0           1.0e-3
#define CONV_DEGRADE 0.3
#define SAFETY       0.9
#define MIN_FACTOR   0.2
#define MAX_FACTOR   6.0

/* ---- scalar knobs handed to the reference entry point ---- */
#ifndef RTOL
#define RTOL         1.0e-6
#endif
#ifndef ABSTOL
#define ABSTOL       1.0e-9
#endif
#ifndef NEWTON_RTOL
#define NEWTON_RTOL  1.0e-12
#endif
#ifndef MAX_ORDER
#define MAX_ORDER    5
#endif
#ifndef MAX_NEWTON
#define MAX_NEWTON   12
#endif
#ifndef GMRES_RESTART
#define GMRES_RESTART 20
#endif
#ifndef GMRES_TOL
#define GMRES_TOL    0.1
#endif

#define MAXHIST (MAX_ORDER + 1)

/* ---------------- lagrange weights (Vandermonde, Gauss-Jordan) --------------- */
static void lagrange_weights(const double *nodes, int k1, int deriv, double *weights)
{
    double aug[MAXHIST][MAXHIST + 1];
    memset(aug, 0, sizeof(aug));
    for (int row = 0; row < k1; row++) {
        for (int col = 0; col < k1; col++) {
            double power = 1.0;
            for (int r = 0; r < row; r++)
                power = power * nodes[col];
            aug[row][col] = power;
        }
    }
    aug[deriv][k1] = 1.0;
    for (int col = 0; col < k1; col++) {
        double pivot = aug[col][col];
        for (int w = 0; w < MAXHIST + 1; w++)
            aug[col][w] = aug[col][w] / pivot;
        for (int row = 0; row < k1; row++) {
            if (row != col) {
                double factor = aug[row][col];
                for (int w = 0; w < MAXHIST + 1; w++)
                    aug[row][w] = aug[row][w] - factor * aug[col][w];
            }
        }
    }
    for (int j = 0; j < k1; j++)
        weights[j] = aug[j][k1];
}

/* ---------------- weighted-RMS norm ---------------- */
static double wrms_norm(const double *restrict au, const double *restrict av,
                        double atol, double rtol,
                        const double *restrict refu, const double *restrict refv,
                        int64_t N)
{
    const int64_t NN = N * N;
    double s = 0.0;
    for (int64_t idx = 0; idx < NN; idx++) {
        double wu = atol + rtol * fabs(refu[idx]);
        double wv = atol + rtol * fabs(refv[idx]);
        double du = au[idx] / wu;
        double dv = av[idx] / wv;
        s += du * du + dv * dv;
    }
    return sqrt(s / (2.0 * (double)NN));
}

/* ---------------- reaction+diffusion right-hand side ----------------
 * Unified Neumann stencil: at a boundary the missing neighbour's weight is 0 and
 * the opposite neighbour's weight is 2 (mirror), which reproduces the reference's
 * per-case formulas bit for bit.
 */
static void compute_rhs(const double *restrict u, const double *restrict v,
                        double *restrict fu, double *restrict fv,
                        int64_t N, double A, double B, double alpha, double invh2)
{
    const double Bp1 = B + 1.0;
    for (int64_t i = 0; i < N; i++) {
        const double *r0 = u + i * N;
        const double *v0 = v + i * N;
        double wup, wdn;
        const double *rm, *rp, *vm, *vp;
        if (i == 0) {
            wup = 0.0; wdn = 2.0; rm = u + N; rp = u + N; vm = v + N; vp = v + N;
        } else if (i == N - 1) {
            wup = 2.0; wdn = 0.0; rm = u + (i - 1) * N; rp = rm; vm = v + (i - 1) * N; vp = vm;
        } else {
            wup = 1.0; wdn = 1.0; rm = u + (i - 1) * N; rp = u + (i + 1) * N;
            vm = v + (i - 1) * N; vp = v + (i + 1) * N;
        }
        double *f0u = fu + i * N;
        double *f0v = fv + i * N;
        /* j = 0 */
        {
            double lu = wup * rm[0] + wdn * rp[0] + 2.0 * r0[1] - 4.0 * r0[0];
            double lv = wup * vm[0] + wdn * vp[0] + 2.0 * v0[1] - 4.0 * v0[0];
            double uu = r0[0], vv = v0[0];
            f0u[0] = A + uu * uu * vv - Bp1 * uu + alpha * lu * invh2;
            f0v[0] = B * uu - uu * uu * vv + alpha * lv * invh2;
        }
        /* j = 1 .. N-2 */
        for (int64_t j = 1; j < N - 1; j++) {
            double lu = wup * rm[j] + wdn * rp[j] + r0[j - 1] + r0[j + 1] - 4.0 * r0[j];
            double lv = wup * vm[j] + wdn * vp[j] + v0[j - 1] + v0[j + 1] - 4.0 * v0[j];
            double uu = r0[j], vv = v0[j];
            f0u[j] = A + uu * uu * vv - Bp1 * uu + alpha * lu * invh2;
            f0v[j] = B * uu - uu * uu * vv + alpha * lv * invh2;
        }
        /* j = N-1 */
        {
            double lu = wup * rm[N - 1] + wdn * rp[N - 1] + 2.0 * r0[N - 2] - 4.0 * r0[N - 1];
            double lv = wup * vm[N - 1] + wdn * vp[N - 1] + 2.0 * v0[N - 2] - 4.0 * v0[N - 1];
            double uu = r0[N - 1], vv = v0[N - 1];
            f0u[N - 1] = A + uu * uu * vv - Bp1 * uu + alpha * lu * invh2;
            f0v[N - 1] = B * uu - uu * uu * vv + alpha * lv * invh2;
        }
    }
}

/* ---------------- frozen Jacobian matvec (I - h*beta0*J) ---------------- */
static void newton_matvec(const double *restrict du, const double *restrict dv,
                           double *restrict odu, double *restrict odv,
                           const double *restrict c1, const double *restrict c2,
                           const double *restrict c3,
                           int64_t N, double alpha, double hbeta0, double invh2)
{
    for (int64_t i = 0; i < N; i++) {
        const double *r0 = du + i * N;
        const double *s0 = dv + i * N;
        double wup, wdn;
        const double *rm, *rp, *sm, *sp;
        if (i == 0) {
            wup = 0.0; wdn = 2.0; rm = du + N; rp = rm; sm = dv + N; sp = sm;
        } else if (i == N - 1) {
            wup = 2.0; wdn = 0.0; rm = du + (i - 1) * N; rp = rm; sm = dv + (i - 1) * N; sp = sm;
        } else {
            wup = 1.0; wdn = 1.0; rm = du + (i - 1) * N; rp = du + (i + 1) * N;
            sm = dv + (i - 1) * N; sp = dv + (i + 1) * N;
        }
        double *o0u = odu + i * N;
        double *o0v = odv + i * N;
        const double *c1r = c1 + i * N;
        const double *c2r = c2 + i * N;
        const double *c3r = c3 + i * N;
        /* j = 0 */
        {
            double ldu = wup * rm[0] + wdn * rp[0] + 2.0 * r0[1] - 4.0 * r0[0];
            double ldv = wup * sm[0] + wdn * sp[0] + 2.0 * s0[1] - 4.0 * s0[0];
            double t1 = c1r[0] * r0[0] + c2r[0] * s0[0];
            double t2 = alpha * ldu * invh2;
            o0u[0] = r0[0] - hbeta0 * (t1 + t2);
            double t3 = c3r[0] * r0[0] - c2r[0] * s0[0];
            double t4 = alpha * ldv * invh2;
            o0v[0] = s0[0] - hbeta0 * (t3 + t4);
        }
        for (int64_t j = 1; j < N - 1; j++) {
            double ldu = wup * rm[j] + wdn * rp[j] + r0[j - 1] + r0[j + 1] - 4.0 * r0[j];
            double ldv = wup * sm[j] + wdn * sp[j] + s0[j - 1] + s0[j + 1] - 4.0 * s0[j];
            double t1 = c1r[j] * r0[j] + c2r[j] * s0[j];
            double t2 = alpha * ldu * invh2;
            o0u[j] = r0[j] - hbeta0 * (t1 + t2);
            double t3 = c3r[j] * r0[j] - c2r[j] * s0[j];
            double t4 = alpha * ldv * invh2;
            o0v[j] = s0[j] - hbeta0 * (t3 + t4);
        }
        /* j = N-1 */
        {
            double ldu = wup * rm[N - 1] + wdn * rp[N - 1] + 2.0 * r0[N - 2] - 4.0 * r0[N - 1];
            double ldv = wup * sm[N - 1] + wdn * sp[N - 1] + 2.0 * s0[N - 2] - 4.0 * s0[N - 1];
            double t1 = c1r[N - 1] * r0[N - 1] + c2r[N - 1] * s0[N - 1];
            double t2 = alpha * ldu * invh2;
            o0u[N - 1] = r0[N - 1] - hbeta0 * (t1 + t2);
            double t3 = c3r[N - 1] * r0[N - 1] - c2r[N - 1] * s0[N - 1];
            double t4 = alpha * ldv * invh2;
            o0v[N - 1] = s0[N - 1] - hbeta0 * (t3 + t4);
        }
    }
}

/* ---------------- GMRES(restart), matrix-free ---------------- */
static void gmres_matfree(const double *restrict rhs_u, const double *restrict rhs_v,
                           const double *restrict c1, const double *restrict c2,
                           const double *restrict c3,
                           int64_t N, double alpha, double hbeta0, double invh2,
                           double tol,
                           double *restrict du, double *restrict dv,
                           double *restrict qu, double *restrict qv,
                           double *restrict wu, double *restrict wv,
                           double *restrict hess, double *restrict cs,
                           double *restrict sn, double *restrict g, double *restrict y)
{
    const int restart = GMRES_RESTART;
    const int64_t NN = N * N;
    double beta = 0.0;
    for (int64_t idx = 0; idx < NN; idx++) {
        double a = rhs_u[idx], b = rhs_v[idx];
        beta += a * a + b * b;
    }
    beta = sqrt(beta);
    for (int64_t idx = 0; idx < NN; idx++) {
        du[idx] = 0.0;
        dv[idx] = 0.0;
    }
    if (beta == 0.0)
        return;
    /* the caller hands the residual; the reference solves for its negation */
    for (int64_t idx = 0; idx < NN; idx++) {
        qu[idx] = -rhs_u[idx] / beta;
        qv[idx] = -rhs_v[idx] / beta;
    }
    g[0] = beta;

    int m_used = restart;
    for (int k = 0; k < restart; k++) {
        newton_matvec(qu + (size_t)k * NN, qv + (size_t)k * NN, wu, wv,
                      c1, c2, c3, N, alpha, hbeta0, invh2);
        for (int p = 0; p <= k; p++) {
            const double *pu = qu + (size_t)p * NN;
            const double *pv = qv + (size_t)p * NN;
            double h_pk = 0.0;
            for (int64_t idx = 0; idx < NN; idx++) {
                double a = pu[idx], b = wu[idx];
                double c = pv[idx], d = wv[idx];
                h_pk += a * b + c * d;
            }
            hess[p * restart + k] = h_pk;
            for (int64_t idx = 0; idx < NN; idx++) {
                wu[idx] = wu[idx] - h_pk * pu[idx];
                wv[idx] = wv[idx] - h_pk * pv[idx];
            }
        }
        double h_next = 0.0;
        for (int64_t idx = 0; idx < NN; idx++) {
            double a = wu[idx], b = wv[idx];
            h_next += a * a + b * b;
        }
        h_next = sqrt(h_next);
        hess[(k + 1) * restart + k] = h_next;

        for (int p = 0; p < k; p++) {
            double temp = cs[p] * hess[p * restart + k] + sn[p] * hess[(p + 1) * restart + k];
            hess[(p + 1) * restart + k] = -sn[p] * hess[p * restart + k] + cs[p] * hess[(p + 1) * restart + k];
            hess[p * restart + k] = temp;
        }
        double hkk = hess[k * restart + k];
        double hk1 = hess[(k + 1) * restart + k];
        double denom = sqrt(hkk * hkk + hk1 * hk1);
        cs[k] = hkk / denom;
        sn[k] = hk1 / denom;
        hess[k * restart + k] = cs[k] * hkk + sn[k] * hk1;
        hess[(k + 1) * restart + k] = 0.0;
        double temp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = temp;

        double rel = fabs(g[k + 1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart - 1) {
            m_used = k + 1;
            break;
        }
        for (int64_t idx = 0; idx < NN; idx++) {
            qu[(size_t)(k + 1) * NN + idx] = wu[idx] / h_next;
            qv[(size_t)(k + 1) * NN + idx] = wv[idx] / h_next;
        }
    }

    for (int row = 0; row < m_used; row++) {
        int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; col++)
            s = s - hess[rr * restart + col] * y[col];
        y[rr] = s / hess[rr * restart + rr];
    }
    for (int p = 0; p < m_used; p++) {
        double yp = y[p];
        const double *pu = qu + (size_t)p * NN;
        const double *pv = qv + (size_t)p * NN;
        for (int64_t idx = 0; idx < NN; idx++) {
            du[idx] = du[idx] + pu[idx] * yp;
            dv[idx] = dv[idx] + pv[idx] * yp;
        }
    }
}

/* ---------------- main integrator ---------------- */
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
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const int64_t NN = N * N;
    const int maxhist = MAXHIST;
    const size_t nb = (size_t)NN * sizeof(double);

    double *mem = (double *)malloc(((size_t)(2 * maxhist + 3 + 12 + 2 * (GMRES_RESTART + 1) + 2) * (size_t)NN
                                    + (size_t)(GMRES_RESTART + 1) * GMRES_RESTART + 3 * GMRES_RESTART + 2)
                                   * sizeof(double));
    if (!mem)
        return;
    double *p = mem;
    double *hist_u = p; p += (size_t)maxhist * NN;
    double *hist_v = p; p += (size_t)maxhist * NN;
    double *c1 = p;     p += NN;
    double *c2 = p;     p += NN;
    double *c3 = p;     p += NN;
    double *u_pred = p; p += NN;
    double *v_pred = p; p += NN;
    double *u_trial = p; p += NN;
    double *v_trial = p; p += NN;
    double *rhs_u = p;  p += NN;
    double *rhs_v = p;  p += NN;
    double *f_u = p;    p += NN;
    double *f_v = p;    p += NN;
    double *res_u = p;  p += NN;
    double *res_v = p;  p += NN;
    double *err_u = p;  p += NN;
    double *err_v = p;  p += NN;
    double *step_du = p; p += NN;
    double *step_dv = p; p += NN;
    double *qu = p;     p += (size_t)(GMRES_RESTART + 1) * NN;
    double *qv = p;     p += (size_t)(GMRES_RESTART + 1) * NN;
    double *wu = p;     p += NN;
    double *wv = p;     p += NN;
    double *hess = p;   p += (size_t)(GMRES_RESTART + 1) * GMRES_RESTART;
    double *cs = p;     p += GMRES_RESTART;
    double *sn = p;     p += GMRES_RESTART;
    double *g = p;      p += GMRES_RESTART + 1;
    double *y = p;      p += GMRES_RESTART;
    double nodes_p[MAXHIST], nodes_c[MAXHIST], w_pred[MAXHIST], c_corr[MAXHIST];
    double hist_t[MAXHIST];

    memcpy(hist_u, u, nb);
    memcpy(hist_v, v, nb);
    hist_t[0] = 0.0;
    int n_hist = 1;
    int ring = 0;

    double t = 0.0;
    double h = H0;
    int order = 1;
    int steps_since_order_change = 0;
    int consecutive_reject = 0;
    int64_t njev = 0;
    int64_t nsteps = 0;
    int have_jac = 0;
    const double h_grid = 1.0 / (double)N;
    const double invh2 = 1.0 / (h_grid * h_grid);

#define RSLOT(j) (((ring) - (j) + MAXHIST) % MAXHIST)

    while (t < t_end && nsteps < max_steps) {
        if (t + h > t_end)
            h = t_end - t;
        int k = order < n_hist ? order : n_hist;
        double t_new = t + h;

        int deg = k < (n_hist - 1) ? k : n_hist - 1;
        for (int j = 0; j <= deg; j++)
            nodes_p[j] = hist_t[RSLOT(j)] - t_new;
        lagrange_weights(nodes_p, deg + 1, 0, w_pred);
        for (int64_t idx = 0; idx < NN; idx++) {
            u_pred[idx] = 0.0;
            v_pred[idx] = 0.0;
        }
        for (int j = 0; j <= deg; j++) {
            const double wj = w_pred[j];
            const double *hu = hist_u + (size_t)RSLOT(j) * NN;
            const double *hv = hist_v + (size_t)RSLOT(j) * NN;
            for (int64_t idx = 0; idx < NN; idx++) {
                u_pred[idx] = u_pred[idx] + wj * hu[idx];
                v_pred[idx] = v_pred[idx] + wj * hv[idx];
            }
        }

        nodes_c[0] = 0.0;
        for (int j = 1; j <= k; j++)
            nodes_c[j] = hist_t[RSLOT(j - 1)] - t_new;
        lagrange_weights(nodes_c, k + 1, 1, c_corr);
        double c0 = c_corr[0];
        double hbeta0 = 1.0 / c0;
        for (int64_t idx = 0; idx < NN; idx++) {
            rhs_u[idx] = 0.0;
            rhs_v[idx] = 0.0;
        }
        for (int j = 1; j <= k; j++) {
            const double f = c_corr[j] / c0;
            const double *hu = hist_u + (size_t)RSLOT(j - 1) * NN;
            const double *hv = hist_v + (size_t)RSLOT(j - 1) * NN;
            for (int64_t idx = 0; idx < NN; idx++) {
                rhs_u[idx] = rhs_u[idx] - f * hu[idx];
                rhs_v[idx] = rhs_v[idx] - f * hv[idx];
            }
        }

        memcpy(u_trial, u_pred, nb);
        memcpy(v_trial, v_pred, nb);
        if (have_jac == 0) {
            for (int64_t idx = 0; idx < NN; idx++) {
                double a = u_trial[idx], b = v_trial[idx];
                double w = 2.0 * a * b;
                c1[idx] = w - (B + 1.0);
                c2[idx] = a * a;
                c3[idx] = B - w;
            }
            njev += 1;
            have_jac = 1;
        }

        int converged = 0;
        int refactored_this_step = 0;
        double prev_resnorm = -1.0;
        for (int it = 0; it < MAX_NEWTON; it++) {
            compute_rhs(u_trial, v_trial, f_u, f_v, N, A, B, alpha, invh2);
            for (int64_t idx = 0; idx < NN; idx++) {
                res_u[idx] = u_trial[idx] - hbeta0 * f_u[idx] - rhs_u[idx];
                res_v[idx] = v_trial[idx] - hbeta0 * f_v[idx] - rhs_v[idx];
            }
            double resnorm = wrms_norm(res_u, res_v, NEWTON_RTOL, NEWTON_RTOL,
                                       u_trial, v_trial, N);
            if (resnorm < 1.0) {
                converged = 1;
                break;
            }
            if (prev_resnorm > 0.0) {
                double rate = resnorm / prev_resnorm;
                if (rate > CONV_DEGRADE && refactored_this_step == 0) {
                    for (int64_t idx = 0; idx < NN; idx++) {
                        double a = u_trial[idx], b = v_trial[idx];
                        double w = 2.0 * a * b;
                        c1[idx] = w - (B + 1.0);
                        c2[idx] = a * a;
                        c3[idx] = B - w;
                    }
                    njev += 1;
                    refactored_this_step = 1;
                }
            }
            prev_resnorm = resnorm;
            gmres_matfree(res_u, res_v, c1, c2, c3, N, alpha, hbeta0, invh2,
                          GMRES_TOL, step_du, step_dv, qu, qv, wu, wv,
                          hess, cs, sn, g, y);
            for (int64_t idx = 0; idx < NN; idx++) {
                u_trial[idx] = u_trial[idx] + step_du[idx];
                v_trial[idx] = v_trial[idx] + step_dv[idx];
            }
        }

        if (converged == 0) {
            h = h * 0.25;
            if (order > 1)
                order = order - 1;
            continue;
        }

        const double c_err = 1.0 / (double)(k + 1);
        for (int64_t idx = 0; idx < NN; idx++) {
            err_u[idx] = c_err * (u_trial[idx] - u_pred[idx]);
            err_v[idx] = c_err * (v_trial[idx] - v_pred[idx]);
        }
        double err_est = wrms_norm(err_u, err_v, ABSTOL, RTOL, u_trial, v_trial, N);

        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            order_history[nsteps] = order;
            nsteps += 1;

            ring = (ring + 1) % MAXHIST;
            memcpy(hist_u + (size_t)ring * NN, u_trial, nb);
            memcpy(hist_v + (size_t)ring * NN, v_trial, nb);
            hist_t[ring] = t;
            if (n_hist < maxhist)
                n_hist = n_hist + 1;

            steps_since_order_change += 1;
            if (order < MAX_ORDER && steps_since_order_change > order && n_hist > order + 1) {
                order += 1;
                steps_since_order_change = 0;
            } else if (refactored_this_step == 1 && order > 1) {
                order -= 1;
                steps_since_order_change = 0;
            }

            double fac;
            if (err_est > 0.0)
                fac = SAFETY * pow(err_est, -1.0 / (double)(k + 1));
            else
                fac = MAX_FACTOR;
            if (fac > MAX_FACTOR) fac = MAX_FACTOR;
            if (fac < MIN_FACTOR) fac = MIN_FACTOR;
            h = h * fac;
        } else {
            consecutive_reject += 1;
            double fac = SAFETY * pow(err_est, -1.0 / (double)(k + 1));
            if (fac > 1.0) fac = 1.0;
            if (fac < MIN_FACTOR) fac = MIN_FACTOR;
            h = h * fac;
            if (consecutive_reject >= 2 && order > 1) {
                order -= 1;
                steps_since_order_change = 0;
            }
        }
    }
#undef RSLOT

    memcpy(u, hist_u + (size_t)ring * NN, nb);
    memcpy(v, hist_v + (size_t)ring * NN, nb);
    diagnostics[0] = (double)nsteps;
    diagnostics[1] = (double)njev;
    diagnostics[2] = t;
    free(mem);
}
