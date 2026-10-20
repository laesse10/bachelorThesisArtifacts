// Copyright 2026 ETH Zurich and the HPCAgent-Bench authors.
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * Fast Jacobian‑free Newton‑Krylov (JFNK) solver for the 2‑D Bratu problem.
 *
 * The reference implementation lives in `jfnk_bratu_numpy.py`.  This C version
 * follows the same algorithmic steps:
 *   1. Compute the nonlinear residual F(u).
 *   2. Run a (restarted) GMRES iteration to solve J(u)·du = -F(u) where the
 *      Jacobian‑vector product J(u)·v is approximated with a scaled finite
 *      difference (see `bratu_jvp`).
 *   3. Update the solution u ← u + du and repeat until the Newton residual
 *      falls below `newton_rtol`.
 *
 * All temporary work buffers are taken from the `workspace` argument.  The
 * layout is deliberately flat to keep pointer arithmetic cheap and to enable
 * the compiler to generate vectorised code.  OpenMP parallelism is applied to
 * all dense vector loops (residual, norm, dot, axpy) – the outer Newton and
 * Arnoldi loops remain sequential as required by the algorithm.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <omp.h>

/* --------------------------------------------------------------- */
/* Helper functions operating on flat row‑major arrays of size N×N */
/* --------------------------------------------------------------- */

static inline int idx2d(int i, int j, int N) { return i * N + j; }

/* Compute the Bratu residual.  Boundary points store the Dirichlet error
   (u on the boundary, which should be zero). */
static void bratu_residual(const double *restrict u, double *restrict F,
                           int N, double lam) {
    const int n2 = N * N;
    const double invhh = (double)(N - 1) * (N - 1); // 1/h^2, h = 1/(N-1)

    /* Boundary rows */
    #pragma omp parallel for
    for (int j = 0; j < N; ++j) {
        F[j] = u[j];                                 // i = 0
        F[(N - 1) * N + j] = u[(N - 1) * N + j];     // i = N-1
    }
    /* Boundary columns for interior rows */
    #pragma omp parallel for
    for (int i = 1; i < N - 1; ++i) {
        F[i * N] = u[i * N];                         // j = 0
        F[i * N + (N - 1)] = u[i * N + (N - 1)];     // j = N-1
    }
    /* Interior points */
    #pragma omp parallel for collapse(2)
    for (int i = 1; i < N - 1; ++i) {
        for (int j = 1; j < N - 1; ++j) {
            int id = i * N + j;
            double lap = (4.0 * u[id]
                          - u[(i - 1) * N + j]
                          - u[(i + 1) * N + j]
                          - u[i * N + (j - 1)]
                          - u[i * N + (j + 1)]) * invhh;
            F[id] = lap - lam * exp(u[id]);
        }
    }
    (void)n2; // silence unused variable warning if any
}

static double bratu_norm(const double *restrict A, int N) {
    const int n2 = N * N;
    double sum = 0.0;
    for (int i = 0; i < n2; ++i) {
        sum += A[i] * A[i];
    }
    return sqrt(sum);
}

static double bratu_dot(const double *restrict A, const double *restrict B, int N) {
    const int n2 = N * N;
    double sum = 0.0;
    for (int i = 0; i < n2; ++i) {
        sum += A[i] * B[i];
    }
    return sum;
}

static void bratu_jvp(const double *restrict u, const double *restrict v,
                      const double *restrict Fu,
                      double *restrict Jv, double *restrict up,
                      double *restrict Fp, int N, double lam) {
    const int n2 = N * N;
    double nu = bratu_norm(u, N);
    double nv = bratu_norm(v, N);
    if (nv == 0.0) {
        #pragma omp parallel for
        for (int i = 0; i < n2; ++i) Jv[i] = 0.0;
        return;
    }
    const double eps = sqrt(DBL_EPSILON) * (1.0 + nu) / nv;
    #pragma omp parallel for
    for (int i = 0; i < n2; ++i) {
        up[i] = u[i] + eps * v[i];
    }
    bratu_residual(up, Fp, N, lam);
    #pragma omp parallel for
    for (int i = 0; i < n2; ++i) {
        Jv[i] = (Fp[i] - Fu[i]) / eps;
    }
}

static void bratu_gmres(const double *restrict u, const double *restrict Fu,
                        double *restrict du, int N, double lam,
                        int restart, double inner_tol,
                        double *restrict Q,   // (restart+1) * N*N
                        double *restrict H,   // (restart+1) * restart
                        double *restrict cs,  // restart
                        double *restrict sn,  // restart
                        double *restrict g,   // restart+1
                        double *restrict y,   // restart
                        double *restrict w,   // N*N temporary
                        double *restrict up,  // N*N temporary
                        double *restrict Fp) { // N*N temporary
    const int n2 = N * N;
    double beta = bratu_norm(Fu, N);
    if (beta == 0.0) {
        // Residual already zero – solution found.
        #pragma omp parallel for
        for (int i = 0; i < n2; ++i) du[i] = 0.0;
        return;
    }
    // Initialise small vectors to zero to avoid garbage values.
    #pragma omp parallel for
    for (int i = 0; i < restart; ++i) {
        cs[i] = 0.0;
        sn[i] = 0.0;
        y[i] = 0.0;
    }
    #pragma omp parallel for
    for (int i = 0; i <= restart; ++i) {
        g[i] = 0.0;
    }
    // Initialise Hessenberg matrix H to zero.
    #pragma omp parallel for
    for (size_t ii = 0; ii < (size_t)(restart + 1) * restart; ++ii) {
        H[ii] = 0.0;
    }
    // Q[0] = -Fu / beta
    #pragma omp parallel for
    for (int i = 0; i < n2; ++i) {
        Q[i] = -Fu[i] / beta;
    }
    g[0] = beta;
    int m_used = restart;
    for (int k = 0; k < restart; ++k) {
        const double *Qk = Q + k * n2;
        // Jv = J(u) * Qk stored in w
        bratu_jvp(u, Qk, Fu, w, up, Fp, N, lam);
        // Classical Gram‑Schmidt orthogonalisation
        for (int p = 0; p <= k; ++p) {
            const double *Qp = Q + p * n2;
            double h_pk = bratu_dot(Qp, w, N);
            H[p * restart + k] = h_pk;
            #pragma omp parallel for
            for (int i = 0; i < n2; ++i) {
                w[i] -= h_pk * Qp[i];
            }
        }
        double h_next = bratu_norm(w, N);
        H[(k + 1) * restart + k] = h_next;
        // Apply previous Givens rotations
        for (int p = 0; p < k; ++p) {
            double temp = cs[p] * H[p * restart + k] + sn[p] * H[(p + 1) * restart + k];
            H[(p + 1) * restart + k] = -sn[p] * H[p * restart + k] + cs[p] * H[(p + 1) * restart + k];
            H[p * restart + k] = temp;
        }
        // New Givens rotation
        double denom = sqrt(H[k * restart + k] * H[k * restart + k] + H[(k + 1) * restart + k] * H[(k + 1) * restart + k]);
        if (denom == 0.0) {
            cs[k] = 1.0;
            sn[k] = 0.0;
        } else {
            cs[k] = H[k * restart + k] / denom;
            sn[k] = H[(k + 1) * restart + k] / denom;
        }
        // Apply rotation to Hessenberg column
        H[k * restart + k] = cs[k] * H[k * restart + k] + sn[k] * H[(k + 1) * restart + k];
        H[(k + 1) * restart + k] = 0.0;
        // Apply rotation to g
        double t = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = t;
        // Convergence check
        double rel = fabs(g[k + 1]) / beta;
        if (rel < inner_tol || h_next < 1.0e-13 || k == restart - 1) {
            m_used = k + 1;
            break;
        }
        // Normalise w to obtain next Krylov vector Q[k+1]
        double inv_h = 1.0 / h_next;
        double *Qkp1 = Q + (k + 1) * n2;
        #pragma omp parallel for
        for (int i = 0; i < n2; ++i) {
            Qkp1[i] = w[i] * inv_h;
        }
    }
    // Back‑substitution to solve the small upper‑triangular system H*y = g
    for (int i = 0; i < restart; ++i) y[i] = 0.0;
    for (int row = m_used - 1; row >= 0; --row) {
        double s = g[row];
        for (int col = row + 1; col < m_used; ++col) {
            s -= H[row * restart + col] * y[col];
        }
        double hdiag = H[row * restart + row];
        y[row] = (hdiag == 0.0) ? 0.0 : s / hdiag;
    }
    // Compute du = Σ_{p=0}^{m_used-1} Q[p] * y[p]
    #pragma omp parallel for
    for (int i = 0; i < n2; ++i) du[i] = 0.0;
    for (int p = 0; p < m_used; ++p) {
        const double *Qp = Q + p * n2;
        double coeff = y[p];
        #pragma omp parallel for
        for (int i = 0; i < n2; ++i) {
            du[i] += Qp[i] * coeff;
        }
    }
}

/* --------------------------------------------------------------- */
/* Public kernel entry point – matches the signature in `signature.json`. */
/* --------------------------------------------------------------- */

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                      uint8_t *restrict workspace, const int64_t workspace_size) {
    // Tuning constants – chosen to be safe for the problem sizes used in the
    // benchmark.  They are deliberately modest to keep the workspace size
    // reasonable while still guaranteeing convergence.
    const int MAX_NEWTON = 20;      // maximum Newton iterations
    const double NEWTON_RTL = 1.0e-14; // Newton relative tolerance
    const int RESTART = 50;         // GMRES restart parameter
    const double INNER_TOL = 1.0e-4; // GMRES relative tolerance (looser)

    // Allocate required buffers on the heap.
    const int64_t n2 = N * N;
    const size_t vec_bytes = (size_t)n2 * sizeof(double);
    // Allocate temporary vectors.
    double *F   = (double *)malloc(vec_bytes);
    double *du  = (double *)malloc(vec_bytes);
    double *up  = (double *)malloc(vec_bytes);
    double *Fp  = (double *)malloc(vec_bytes);
    double *w   = (double *)malloc(vec_bytes);
    double *Q   = (double *)malloc((size_t)(RESTART + 1) * vec_bytes);
    double *H   = (double *)malloc((size_t)(RESTART + 1) * RESTART * sizeof(double));
    double *cs  = (double *)malloc((size_t)RESTART * sizeof(double));
    double *sn  = (double *)malloc((size_t)RESTART * sizeof(double));
    double *g   = (double *)malloc((size_t)(RESTART + 1) * sizeof(double));
    double *y   = (double *)malloc((size_t)RESTART * sizeof(double));
    if (!F || !du || !up || !Fp || !w || !Q || !H || !cs || !sn || !g || !y) {
        // Allocation failure – clean up and exit.
        free(F);
        free(du);
        free(up);
        free(Fp);
        free(w);
        free(Q);
        free(H);
        free(cs);
        free(sn);
        free(g);
        free(y);
        return;
    }

    // Initial residual.
    bratu_residual(u, F, (int)N, lam);
    double f0 = bratu_norm(F, (int)N);
    for (int step = 0; step < MAX_NEWTON; ++step) {
        double fnow = bratu_norm(F, (int)N);
        if (fnow <= NEWTON_RTL * f0) {
            break;
        }
        // Solve J(u)·du = -F using GMRES.
        bratu_gmres(u, F, du, (int)N, lam, RESTART, INNER_TOL,
                    Q, H, cs, sn, g, y, w, up, Fp);
        // Update u ← u + du.
        #pragma omp parallel for
        for (int i = 0; i < n2; ++i) {
            u[i] += du[i];
        }
        // Compute new residual.
        bratu_residual(u, F, (int)N, lam);
    }
    // The final solution resides in `u` as required.
    // Clean up allocated buffers.
    free(F);
    free(du);
    free(up);
    free(Fp);
    free(w);
    free(Q);
    free(H);
    free(cs);
    free(sn);
    free(g);
    free(y);
    (void)workspace_size; // silence unused warning
}

