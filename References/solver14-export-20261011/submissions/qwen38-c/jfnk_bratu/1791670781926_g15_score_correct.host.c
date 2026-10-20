/* JFNK solve of the 2-D Bratu problem, OpenMP-parallel port of the NumPy reference.
 *
 *   -Laplacian(u) - lam*exp(u) = 0 on (0,1)^2, u = 0 on the boundary.
 *
 * Outer Newton loop is sequential; GMRES(restart) Arnoldi loop is sequential;
 * every grid-level kernel inside (residual stencil, norms, dots, axpys) is
 * data-parallel. Semantics follow jfnk_bratu_numpy.py exactly:
 *   - scaled finite-difference Jacobian (Pernice & Walker 1998)
 *   - GMRES with modified Gram-Schmidt and incremental Givens rotations
 *   - Newton stopping gate  fnow <= 1e-14 * f0
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef RESTART
#define RESTART 50
#endif
#ifndef MAX_NEWTON
#define MAX_NEWTON 20
#endif
#ifndef INNER_TOL
#define INNER_TOL 1.0e-4
#endif

#define SQRT_MACHEPS 1.4901161193847656e-08 /* sqrt(2^-52) */

typedef struct {
    double *F, *du, *up, *Fp, *w;      /* single grid buffers                */
    double *Q;                          /* (RESTART+1) grid Krylov basis     */
    double *H;                          /* (RESTART+1) x RESTART             */
    double *cs, *sn;                    /* RESTART each                      */
    double *g;                          /* RESTART+1                         */
    double *y;                          /* RESTART                           */
} Work;

static inline void *align_up(void *p, size_t a) {
    size_t m = (size_t)p % a;
    return m ? (char *)p + (a - m) : p;
}

/* ---- grid kernels ----------------------------------------------------- */

static double k_norm(const double *A, int64_t n) {
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (int64_t i = 0; i < n; i++) s += A[i] * A[i];
    return sqrt(s);
}

static double k_dot(const double *A, const double *B, int64_t n) {
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (int64_t i = 0; i < n; i++) s += A[i] * B[i];
    return s;
}

static void k_residual(const double *u, double *F, int64_t N, double lam) {
    const double h = 1.0 / (double)(N - 1);
    const double inv_h2 = 1.0 / (h * h);

    #pragma omp parallel for schedule(static)
    for (int64_t i = 1; i < N - 1; i++) {
        const double *uc = u + i * N;
        const double *un = uc - N;
        const double *ud = uc + N;
        double *Fc = F + i * N;
        for (int64_t j = 1; j < N - 1; j++)
            Fc[j] = (4.0 * uc[j] - un[j] - ud[j] - uc[j - 1] - uc[j + 1]) * inv_h2
                    - lam * exp(uc[j]);
    }
    /* Dirichlet boundary: F = u on the frame */
    for (int64_t j = 0; j < N; j++) { F[j] = u[j]; F[(N - 1) * N + j] = u[(N - 1) * N + j]; }
    for (int64_t i = 1; i < N - 1; i++) {
        F[i * N] = u[i * N];
        F[i * N + N - 1] = u[i * N + N - 1];
    }
}

/* w = J_fd(u) Qk  = (F(u + eps*Qk) - Fu) / eps, with the scaled eps */
static void k_jvp(const double *u, const double *v, const double *Fu, double *w,
                  double *up, double *Fp, int64_t N, int64_t n, double lam, double nu) {
    double nv = k_norm(v, n);
    if (nv == 0.0) {
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) w[i] = 0.0;
        return;
    }
    const double eps = SQRT_MACHEPS * (1.0 + nu) / nv;
    const double ie = 1.0 / eps;

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) up[i] = u[i] + eps * v[i];
    k_residual(up, Fp, N, lam);
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) w[i] = (Fp[i] - Fu[i]) * ie;
}

static void k_gmres(const double *u, const double *Fu, double *du, int64_t N,
                    double lam, Work *W, int64_t n) {
    double *Q = W->Q, *H = W->H, *cs = W->cs, *sn = W->sn, *g = W->g, *y = W->y;
    double *w = W->w, *up = W->up, *Fp = W->Fp;

    const double beta = k_norm(Fu, n);
    const double ib = -1.0 / beta;

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) Q[i] = Fu[i] * ib;
    g[0] = beta;

    const double nu = k_norm(u, n);
    int m_used = RESTART;
    for (int k = 0; k < RESTART; k++) {
        double *Qk = Q + (int64_t)k * n;

        /* Arnoldi: w = J_fd Qk */
        k_jvp(u, Qk, Fu, w, up, Fp, N, n, lam, nu);

        /* modified Gram-Schmidt against Q[0..k] */
        for (int p = 0; p <= k; p++) {
            double *Qp = Q + (int64_t)p * n;
            const double hpk = k_dot(Qp, w, n);
            H[(int64_t)p * RESTART + k] = hpk;
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < n; i++) w[i] -= hpk * Qp[i];
        }

        const double hnext = k_norm(w, n);
        H[(int64_t)(k + 1) * RESTART + k] = hnext;

        /* previously accumulated Givens rotations */
        for (int p = 0; p < k; p++) {
            const double a = H[(int64_t)p * RESTART + k];
            const double b = H[(int64_t)(p + 1) * RESTART + k];
            H[(int64_t)(p + 1) * RESTART + k] = -sn[p] * a + cs[p] * b;
            H[(int64_t)p * RESTART + k] = cs[p] * a + sn[p] * b;
        }
        const double denom = sqrt(H[(int64_t)k * RESTART + k] * H[(int64_t)k * RESTART + k]
                                  + hnext * hnext);
        cs[k] = H[(int64_t)k * RESTART + k] / denom;
        sn[k] = hnext / denom;
        H[(int64_t)k * RESTART + k] = cs[k] * H[(int64_t)k * RESTART + k] + sn[k] * hnext;
        H[(int64_t)(k + 1) * RESTART + k] = 0.0;

        const double gt = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = gt;

        const double rel = fabs(g[k + 1]) / beta;
        if (rel < INNER_TOL || hnext < 1.0e-13 || k == RESTART - 1) {
            m_used = k + 1;
            break;
        }
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) Q[(int64_t)(k + 1) * n + i] = w[i] / hnext;
    }

    /* least-squares back substitution on the upper-triangular R */
    for (int row = 0; row < m_used; row++) {
        const int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; col++)
            s -= H[(int64_t)rr * RESTART + col] * y[col];
        y[rr] = s / H[(int64_t)rr * RESTART + rr];
    }

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) du[i] = 0.0;
    for (int p = 0; p < m_used; p++) {
        const double *Qp = Q + (int64_t)p * n;
        const double yp = y[p];
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) du[i] += Qp[i] * yp;
    }
}

/* ---- entry point ------------------------------------------------------- */

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size) {
    const int64_t n = N * N;
    /* grid buffers: Q (RESTART+1) + F,du,up,Fp,w (5)  ; small: H,cs,sn,g,y */
    const size_t need =
        (size_t)(RESTART + 6) * n * sizeof(double)
        + ((size_t)(RESTART + 1) * RESTART + 3 * RESTART + (RESTART + 1)) * sizeof(double);

    void *own = NULL;
    uint8_t *ws = workspace;
    if ((size_t)workspace_size < need) {
        own = malloc(need);
        ws = (uint8_t *)own;
    }
    double *p = (double *)align_up(ws, 64);

    Work W;
    W.F  = p;
    W.du = p + n;
    W.up = p + 2 * n;
    W.Fp = p + 3 * n;
    W.w  = p + 4 * n;
    W.Q  = p + 5 * n;
    p += (size_t)(RESTART + 6) * n;
    W.H  = p; p += (size_t)(RESTART + 1) * RESTART;
    W.cs = p; p += RESTART;
    W.sn = p; p += RESTART;
    W.g  = p; p += RESTART + 1;
    W.y  = p;

    double *F = W.F;
    k_residual(u, F, N, lam);
    const double f0 = k_norm(F, n);

    for (int step = 0; step < MAX_NEWTON; step++) {
        const double fnow = k_norm(F, n);
        if (fnow <= 1.0e-14 * f0) break;
        k_gmres(u, F, W.du, N, lam, &W, n);
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) u[i] += W.du[i];
        k_residual(u, F, N, lam);
    }

    if (own) free(own);
}
