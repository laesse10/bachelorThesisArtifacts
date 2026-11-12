/* JFNK solve of the 2-D Bratu problem:  -Laplacian(u) - lam*exp(u) = 0, u = 0 on boundary.
 *
 * Structure mirrors the reference exactly (Newton with 1e-14*f0 gate, GMRES(restart) with
 * incremental Givens, finite-difference Jacobian with the Pernice-Walker scaled step).
 * The Newton/Arnoldi skeletons stay sequential; every data-parallel grid operation
 * (stencil, reductions, vector updates) is an OpenMP work-sharing region.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define REST 50
#define MAX_NEWTON 20
#define TOL_INNER 1.0e-4
#define NEWTON_RTOL 1.0e-14
#define SQRT_MACHEPS 1.490116119384765625e-08 /* sqrt(2.2204460492503131e-16), exact */

typedef int64_t i64;

/* ---- grid kernels ---- */

static double grid_norm(const double *A, i64 n) {
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (i64 i = 0; i < n; i++) {
        double a = A[i];
        s += a * a;
    }
    return sqrt(s);
}

static double grid_dot(const double *A, const double *B, i64 n) {
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (i64 i = 0; i < n; i++) s += A[i] * B[i];
    return s;
}

/* F = residual(u); boundary rows/cols: F = u. */
static void grid_res(const double *u, double *F, i64 N, double lam) {
    const double ih = (double)(N - 1);
    const double ih2 = ih * ih;
    #pragma omp parallel for schedule(static)
    for (i64 i = 1; i < N - 1; i++) {
        const double *um = u + (i - 1) * N;
        const double *uc = u + i * N;
        const double *un = u + (i + 1) * N;
        double *fc = F + i * N;
        for (i64 j = 1; j < N - 1; j++)
            fc[j] = (4.0 * uc[j] - um[j] - un[j] - uc[j - 1] - uc[j + 1]) * ih2
                    - lam * exp(uc[j]);
    }
    memcpy(F, u, (size_t)N * sizeof(double));
    memcpy(F + (N - 1) * N, u + (N - 1) * N, (size_t)N * sizeof(double));
    for (i64 i = 1; i < N - 1; i++) {
        F[i * N] = u[i * N];
        F[i * N + N - 1] = u[i * N + N - 1];
    }
}

/* w = (residual(up) - Fu) * inv_eps over the full grid. */
static void grid_jvp_body(const double *up, const double *Fu, double *w, i64 N,
                          double lam, double inv_eps) {
    const double ih = (double)(N - 1);
    const double ih2 = ih * ih;
    #pragma omp parallel for schedule(static)
    for (i64 i = 1; i < N - 1; i++) {
        const double *um = up + (i - 1) * N;
        const double *uc = up + i * N;
        const double *un = up + (i + 1) * N;
        const double *fc = Fu + i * N;
        double *wc = w + i * N;
        for (i64 j = 1; j < N - 1; j++)
            wc[j] = ((4.0 * uc[j] - um[j] - un[j] - uc[j - 1] - uc[j + 1]) * ih2
                     - lam * exp(uc[j]) - fc[j]) * inv_eps;
    }
    for (i64 j = 0; j < N; j++) {
        w[j] = (up[j] - Fu[j]) * inv_eps;
        w[(N - 1) * N + j] = (up[(N - 1) * N + j] - Fu[(N - 1) * N + j]) * inv_eps;
    }
    for (i64 i = 1; i < N - 1; i++) {
        w[i * N] = (up[i * N] - Fu[i * N]) * inv_eps;
        w[i * N + N - 1] = (up[i * N + N - 1] - Fu[i * N + N - 1]) * inv_eps;
    }
}

/* Matrix-free J(u) v ~= (F(u + eps v) - F(u)) / eps, scaled eps (Pernice & Walker). */
static void grid_jvp(const double *u, const double *v, const double *Fu, double *w,
                     double *up, i64 N, double lam) {
    const i64 n = N * N;
    double nu = grid_norm(u, n);
    double nv = grid_norm(v, n);
    if (nv == 0.0) {
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) w[i] = 0.0;
        return;
    }
    double eps = SQRT_MACHEPS * (1.0 + nu) / nv;
    double inv_eps = 1.0 / eps;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) up[i] = u[i] + eps * v[i];
    grid_jvp_body(up, Fu, w, N, lam, inv_eps);
}

/* GMRES(restart) with incremental Givens rotations. */
static void gmres(const double *u, const double *Fu, double *du, i64 N, double lam,
                  double *Q, double *w, double *up, double *H, double *cs, double *sn,
                  double *g, double *y, i64 n) {
    double beta = grid_norm(Fu, n);
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) Q[i] = -Fu[i] / beta;
    g[0] = beta;
    int m_used = REST;
    for (int k = 0; k < REST; k++) {
        const double *qk = Q + (i64)k * n;
        grid_jvp(u, qk, Fu, w, up, N, lam);
        for (int p = 0; p <= k; p++) {
            const double *qp = Q + (i64)p * n;
            double hpk = grid_dot(qp, w, n);
            H[(i64)p * REST + k] = hpk;
            #pragma omp parallel for schedule(static)
            for (i64 i = 0; i < n; i++) w[i] -= hpk * qp[i];
        }
        double hnext = grid_norm(w, n);
        H[(i64)(k + 1) * REST + k] = hnext;
        for (int p = 0; p < k; p++) {
            double temp = cs[p] * H[(i64)p * REST + k]
                          + sn[p] * H[(i64)(p + 1) * REST + k];
            H[(i64)(p + 1) * REST + k] = -sn[p] * H[(i64)p * REST + k]
                                         + cs[p] * H[(i64)(p + 1) * REST + k];
            H[(i64)p * REST + k] = temp;
        }
        double denom = sqrt(H[(i64)k * REST + k] * H[(i64)k * REST + k]
                            + H[(i64)(k + 1) * REST + k] * H[(i64)(k + 1) * REST + k]);
        cs[k] = H[(i64)k * REST + k] / denom;
        sn[k] = H[(i64)(k + 1) * REST + k] / denom;
        H[(i64)k * REST + k] = cs[k] * H[(i64)k * REST + k]
                               + sn[k] * H[(i64)(k + 1) * REST + k];
        H[(i64)(k + 1) * REST + k] = 0.0;
        double tmp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = tmp;
        double rel = fabs(g[k + 1]) / beta;
        if (rel < TOL_INNER || hnext < 1.0e-13 || k == REST - 1) {
            m_used = k + 1;
            break;
        }
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) Q[(i64)(k + 1) * n + i] = w[i] / hnext;
    }
    for (int row = m_used - 1; row >= 0; row--) {
        double s = g[row];
        for (int col = row + 1; col < m_used; col++)
            s -= H[(i64)row * REST + col] * y[col];
        y[row] = s / H[(i64)row * REST + row];
    }
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) du[i] = y[0] * Q[i];
    for (int p = 1; p < m_used; p++) {
        const double *qp = Q + (i64)p * n;
        double yp = y[p];
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) du[i] += yp * qp[i];
    }
}

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size) {
    const i64 n = N * N;
    const size_t need_d = (size_t)55 * (size_t)n + 2048; /* 5 grids + REST Q + small vecs */
    const size_t need_b = need_d * sizeof(double);

    int mine = 0;
    double *base;
    if (workspace && (size_t)workspace_size >= need_b && ((uintptr_t)workspace & 63) == 0)
        base = (double *)workspace;
    else {
        base = (double *)aligned_alloc(64, (need_b + 63) & ~63ull);
        mine = 1;
    }
    double *F = base;
    double *du = base + n;
    double *up = base + 2 * n;
    double *w = base + 3 * n;
    double *Q = base + 4 * n;
    double *H = Q + (REST + 1) * n;
    double *cs = H + (REST + 1) * REST;
    double *sn = cs + REST;
    double *g = sn + REST;
    double *y = g + REST + 1;

    grid_res(u, F, N, lam);
    double f0 = grid_norm(F, n);
    double fnow = f0;
    for (int step = 0; step < MAX_NEWTON; step++) {
        if (fnow <= NEWTON_RTOL * f0) break;
        if (getenv("JFNK_DBG")) fprintf(stderr, "step %d fnow/f0=%.3e\n", step, fnow / f0);
        gmres(u, F, du, N, lam, Q, w, up, H, cs, sn, g, y, n);
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < n; i++) u[i] += du[i];
        grid_res(u, F, N, lam);
        fnow = grid_norm(F, n);
    }
    if (mine) free(base);
}
