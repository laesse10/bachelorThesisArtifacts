#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Tunables (must match the judge's reference configuration). */
#define MAX_NEWTON 20
#define INNER_TOL 1.0e-4
#define RESTART 50

static inline void bratu_residual(const double *restrict u, double *restrict F,
                                  int64_t N, double lam)
{
    const double h = 1.0 / (double)(N - 1);
    const double hh = h * h;
    const int64_t nm1 = N - 1;
#pragma omp parallel for schedule(static, 1)
    for (int64_t i = 0; i < N; i++) {
        double *Fr = F + i * N;
        const double *ur = u + i * N;
        if (i == 0 || i == nm1) {
            for (int64_t j = 0; j < N; j++)
                Fr[j] = ur[j];
            continue;
        }
        const double *ur_u = u + (i - 1) * N;
        const double *ur_d = u + (i + 1) * N;
        Fr[0] = ur[0];
        Fr[nm1] = ur[nm1];
        for (int64_t j = 1; j < nm1; j++) {
            Fr[j] = (4.0 * ur[j] - ur_u[j] - ur_d[j] - ur[j - 1] - ur[j + 1]) /
                        hh -
                    lam * exp(ur[j]);
        }
    }
}

static inline double bratu_norm(const double *A, int64_t N)
{
    double s = 0.0;
#pragma omp parallel for reduction(+:s) schedule(static, 1)
    for (int64_t i = 0; i < N; i++) {
        const double *r = A + i * N;
        double ss = 0.0;
        for (int64_t j = 0; j < N; j++)
            ss += r[j] * r[j];
        s += ss;
    }
    return sqrt(s);
}

static inline double bratu_dot(const double *restrict A, const double *restrict B,
                               int64_t N)
{
    double s = 0.0;
#pragma omp parallel for reduction(+:s) schedule(static, 1)
    for (int64_t i = 0; i < N; i++) {
        const double *ra = A + i * N;
        const double *rb = B + i * N;
        double ss = 0.0;
        for (int64_t j = 0; j < N; j++)
            ss += ra[j] * rb[j];
        s += ss;
    }
    return s;
}

static inline void axpy_(double *restrict y, const double *restrict x, int64_t n,
                         double a)
{
#pragma omp parallel for schedule(static, 1)
    for (int64_t i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}

static inline void scal_(double *y, int64_t n, double a)
{
#pragma omp parallel for schedule(static, 1)
    for (int64_t i = 0; i < n; i++)
        y[i] *= a;
}

/* GMRES(restart), one cycle, Saad Alg. 6.9 with incremental Givens. */
static void bratu_gmres(const double *restrict u, const double *restrict F,
                        double *restrict du, int64_t N, double lam, int restart,
                        double tol, double *Q, double *H, double *cs, double *sn,
                        double *g, double *y, double *w, double *up, double *Fp,
                        double nu_cached, int64_t n2)
{
    const double beta = bratu_norm(F, N);
    /* Q[0] = -F / beta */
#pragma omp parallel for schedule(static, 1)
    for (int64_t i = 0; i < n2; i++)
        Q[i] = -F[i] / beta;
    g[0] = beta;

    const double nu = nu_cached; /* u is constant inside this cycle */
    int m_used = restart;
    const double macheps = 2.220446049250313e-16;

    for (int k = 0; k < restart; k++) {
        const double *Qk = Q + (int64_t)k * n2;
        /* JVP: J v ~= (F(u + eps v) - F(u)) / eps */
        const double nv = bratu_norm(Qk, N);
        if (nv == 0.0) {
            scal_(w, n2, 0.0);
        } else {
            const double eps = sqrt(macheps) * (1.0 + nu) / nv;
            for (int64_t i = 0; i < n2; i++)
                up[i] = u[i] + eps * Qk[i];
            bratu_residual(up, Fp, N, lam);
            for (int64_t i = 0; i < n2; i++)
                w[i] = (Fp[i] - F[i]) / eps;
        }

        for (int p = 0; p <= k; p++) {
            const double h_pk = bratu_dot(Q + (int64_t)p * n2, w, N);
            H[p * restart + k] = h_pk;
            axpy_(w, Q + (int64_t)p * n2, n2, -h_pk);
        }

        const double h_next = bratu_norm(w, N);
        H[(k + 1) * restart + k] = h_next;

        for (int p = 0; p < k; p++) {
            const double temp = cs[p] * H[p * restart + k] + sn[p] * H[(p + 1) * restart + k];
            H[(p + 1) * restart + k] = -sn[p] * H[p * restart + k] + cs[p] * H[(p + 1) * restart + k];
            H[p * restart + k] = temp;
        }

        const double denom =
            sqrt(H[k * restart + k] * H[k * restart + k] + H[(k + 1) * restart + k] *
                                                         H[(k + 1) * restart + k]);
        cs[k] = H[k * restart + k] / denom;
        sn[k] = H[(k + 1) * restart + k] / denom;
        H[k * restart + k] = cs[k] * H[k * restart + k] + sn[k] * H[(k + 1) * restart + k];
        H[(k + 1) * restart + k] = 0.0;

        const double temp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = temp;

        const double rel = fabs(g[k + 1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart - 1) {
            m_used = k + 1;
            break;
        }
        for (int64_t i = 0; i < n2; i++)
            Q[(int64_t)(k + 1) * n2 + i] = w[i] / h_next;
    }

    for (int row = 0; row < m_used; row++) {
        const int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; col++)
            s -= H[rr * restart + col] * y[col];
        y[rr] = s / H[rr * restart + rr];
    }

    for (int64_t i = 0; i < n2; i++)
        du[i] = 0.0;
    for (int p = 0; p < m_used; p++)
        axpy_(du, Q + (int64_t)p * n2, n2, y[p]);
}

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace_size;
    const int64_t n2 = N * N;
    double *F = (double *)workspace;
    double *du = (double *)(workspace + 8 * n2);
    double *up = (double *)(workspace + 16 * n2);
    double *Fp = (double *)(workspace + 24 * n2);
    double *w = (double *)(workspace + 32 * n2);
    double *Q = (double *)(workspace + 40 * n2);
    double *H = Q + (int64_t)(RESTART + 1) * n2;
    double *cs = H + (int64_t)(RESTART + 1) * RESTART;
    double *sn = cs + RESTART;
    double *g = sn + RESTART;
    double *y = g + RESTART + 1;

    bratu_residual(u, F, N, lam);
    double f0 = bratu_norm(F, N);
    const double newton_rtol = 1.0e-14;

    for (int step = 0; step < MAX_NEWTON; step++) {
        const double fnow = bratu_norm(F, N);
        if (fnow <= newton_rtol * f0)
            break;
        const double nu = bratu_norm(u, N);
        bratu_gmres(u, F, du, N, lam, RESTART, INNER_TOL, Q, H, cs, sn, g, y, w, up, Fp,
                    nu, n2);
        for (int64_t i = 0; i < n2; i++)
            u[i] += du[i];
        bratu_residual(u, F, N, lam);
    }
}
