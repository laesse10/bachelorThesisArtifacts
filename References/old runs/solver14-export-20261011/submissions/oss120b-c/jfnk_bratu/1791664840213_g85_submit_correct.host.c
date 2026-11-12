/*
 * Optimized C implementation of the Jacobian‑free Newton‑Krylov (JFNK) solver for the
 * 2‑D Bratu problem.
 *
 * The reference implementation lives in Python (jfnk_bratu_numpy.py). The algorithm
 * consists of an outer Newton loop and an inner GMRES iteration that evaluates the
 * Jacobian‑vector product using a finite‑difference approximation. All heavy work is
 * performed on the N×N grid and is parallelised with OpenMP.
 *
 * The kernel entry point follows the signature required by the benchmark:
 *
 *   void jfnk_bratu_fp64(double *restrict u,
 *                        const int64_t N,
 *                        const double lam,
 *                        uint8_t *restrict workspace,
 *                        const int64_t workspace_size);
 *
 * `u` contains the initial guess (the interior is perturbed but the boundaries are zero).
 * On return `u` holds the converged solution. The workspace pointer is provided for scratch
 * memory; this implementation allocates its own buffers with `malloc` because the benchmark
 * does not enforce a particular allocation strategy. All allocations are freed before the
 * function returns.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <omp.h>

/* -------------------------------------------------------------------------- */
/* Helper functions operating on flat row‑major N×N arrays. */
/* -------------------------------------------------------------------------- */

static inline size_t grid_len(const int N) {
    return (size_t)N * (size_t)N;
}

/* Compute the Bratu residual.
   For interior points (1..N-2) we apply the 5‑point stencil; on the boundary we simply
   copy the Dirichlet value (u) into the residual array. */
static void bratu_residual(const double *restrict u, double *restrict F, const int N, const double lam) {
    const double h = 1.0 / (N - 1);
    const double h2 = h * h;
    const int Nminus1 = N - 1;

    /* Boundary rows */
    #pragma omp parallel for
    for (int j = 0; j < N; ++j) {
        F[j] = u[j];                               // top row i = 0
        F[Nminus1 * N + j] = u[Nminus1 * N + j];   // bottom row i = N-1
    }
    /* Boundary columns */
    #pragma omp parallel for
    for (int i = 0; i < N; ++i) {
        F[i * N] = u[i * N];                       // left column j = 0
        F[i * N + Nminus1] = u[i * N + Nminus1];   // right column j = N-1
    }
    /* Interior points */
    #pragma omp parallel for schedule(static)
    for (int i = 1; i < Nminus1; ++i) {
        const int im1 = (i - 1) * N;
        const int ip1 = (i + 1) * N;
        const int iN  = i * N;
        for (int j = 1; j < Nminus1; ++j) {
            double center = u[iN + j];
            double val = (4.0 * center
                          - u[im1 + j]    // north
                          - u[ip1 + j]    // south
                          - u[iN + j - 1] // west
                          - u[iN + j + 1] // east) / h2
                          ) / h2;
            val -= lam * exp(center);
            F[iN + j] = val;
        }
    }
}

/* Euclidean norm of a flattened N×N array. */
static double bratu_norm(const double *restrict A, const int N) {
    const size_t len = grid_len(N);
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (size_t i = 0; i < len; ++i) {
        double v = A[i];
        sum += v * v;
    }
    return sqrt(sum);
}

/* Dot product of two flattened N×N arrays. */
static double bratu_dot(const double *restrict A, const double *restrict B, const int N) {
    const size_t len = grid_len(N);
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (size_t i = 0; i < len; ++i) {
        sum += A[i] * B[i];
    }
    return sum;
}

/* Matrix‑free Jacobian‑vector product using a scaled finite‑difference step. */
static void bratu_jvp(const double *restrict u, const double *restrict v,
                      const double *restrict Fu, double *restrict Jv,
                      double *restrict up, double *restrict Fp,
                      const int N, const double lam) {
    const double macheps = DBL_EPSILON; // machine epsilon for double precision
    double nu = bratu_norm(u, N);
    double nv = bratu_norm(v, N);
    if (nv == 0.0) {
        const size_t len = grid_len(N);
        #pragma omp parallel for
        for (size_t i = 0; i < len; ++i) {
            Jv[i] = 0.0;
        }
        return;
    }
    double eps = sqrt(macheps) * (1.0 + nu) / nv;
    const size_t len = grid_len(N);
    /* up = u + eps * v */
    #pragma omp parallel for
    for (size_t i = 0; i < len; ++i) {
        up[i] = u[i] + eps * v[i];
    }
    bratu_residual(up, Fp, N, lam);
    double inv_eps = 1.0 / eps;
    #pragma omp parallel for
    for (size_t i = 0; i < len; ++i) {
        Jv[i] = (Fp[i] - Fu[i]) * inv_eps;
    }
}

/* GMRES with restart and incremental Givens rotations. */
static void bratu_gmres(const double *restrict u, const double *restrict Fu,
                        double *restrict du, const int N, const double lam,
                        const int restart, const double tol,
                        double *restrict Q, double *restrict H,
                        double *restrict cs, double *restrict sn,
                        double *restrict g, double *restrict y,
                        double *restrict w, double *restrict up,
                        double *restrict Fp) {
    const size_t len = grid_len(N);
    const double beta = bratu_norm(Fu, N);
    /* Q[0] = -Fu / beta */
    #pragma omp parallel for
    for (size_t i = 0; i < len; ++i) {
        Q[i] = -Fu[i] / beta;
    }
    g[0] = beta;
    int m_used = restart; // will be reduced if convergence occurs earlier
    for (int k = 0; k < restart; ++k) {
        /* Jv = J(u) * Q[k] */
        bratu_jvp(u, Q + k * len, Fu, w, up, Fp, N, lam);
        /* Classical Gram‑Schmidt orthogonalisation */
        for (int p = 0; p <= k; ++p) {
            double h_pk = bratu_dot(Q + p * len, w, N);
            H[p * restart + k] = h_pk;
            #pragma omp parallel for
            for (size_t i = 0; i < len; ++i) {
                w[i] -= h_pk * Q[p * len + i];
            }
        }
        double h_next = bratu_norm(w, N);
        H[(k + 1) * restart + k] = h_next;
        /* Apply previous Givens rotations to the new column */
        for (int p = 0; p < k; ++p) {
            double temp = cs[p] * H[p * restart + k] + sn[p] * H[(p + 1) * restart + k];
            H[(p + 1) * restart + k] = -sn[p] * H[p * restart + k] + cs[p] * H[(p + 1) * restart + k];
            H[p * restart + k] = temp;
        }
        /* New Givens rotation to eliminate H[k+1,k] */
        double denom = hypot(H[k * restart + k], H[(k + 1) * restart + k]);
        cs[k] = H[k * restart + k] / denom;
        sn[k] = H[(k + 1) * restart + k] / denom;
        /* Apply rotation to H */
        H[k * restart + k] = cs[k] * H[k * restart + k] + sn[k] * H[(k + 1) * restart + k];
        H[(k + 1) * restart + k] = 0.0;
        /* Apply rotation to the RHS vector g */
        double tmp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = tmp;
        double rel = fabs(g[k + 1]) / beta;
        if (rel < tol || h_next < 1.0e-13 || k == restart - 1) {
            m_used = k + 1;
            break;
        }
        /* Normalise the new Arnoldi vector */
        #pragma omp parallel for
        for (size_t i = 0; i < len; ++i) {
            Q[(k + 1) * len + i] = w[i] / h_next;
        }
    }
    /* Back substitution to solve the small upper‑triangular system */
    for (int row = m_used - 1; row >= 0; --row) {
        double sum = g[row];
        for (int col = row + 1; col < m_used; ++col) {
            sum -= H[row * restart + col] * y[col];
        }
        y[row] = sum / H[row * restart + row];
    }
    /* Form the update du = Σ Q[p] * y[p] */
    #pragma omp parallel for
    for (size_t i = 0; i < len; ++i) {
        double accum = 0.0;
        for (int p = 0; p < m_used; ++p) {
            accum += Q[p * len + i] * y[p];
        }
        du[i] = accum;
    }
}

/* -------------------------------------------------------------------------- */
/* Kernel entry point. */
/* -------------------------------------------------------------------------- */

void jfnk_bratu_fp64(double *restrict u,
                     const int64_t N,
                     const double lam,
                     uint8_t *restrict workspace,
                     const int64_t workspace_size) {
    /* Tunable parameters – these match the values used by the reference implementation. */
    const int max_newton = 20;          /* Upper bound on Newton iterations */
    const double newton_rtol = 1e-14;   /* Relative tolerance for the Newton residual */
    const double inner_tol = 1e-4;     /* GMRES convergence tolerance (looser as in reference) */
    const int gmres_restart = 50;       /* Restart length for GMRES */

    const size_t len = grid_len((int)N);

    /* Allocate temporaries. */
    double *F   = (double *)malloc(len * sizeof(double));
    double *du  = (double *)malloc(len * sizeof(double));
    double *up  = (double *)malloc(len * sizeof(double));
    double *Fp  = (double *)malloc(len * sizeof(double));
    double *w   = (double *)malloc(len * sizeof(double));
    double *Q   = (double *)malloc((size_t)(gmres_restart + 1) * len * sizeof(double));
    double *H   = (double *)malloc((size_t)(gmres_restart + 1) * (size_t)gmres_restart * sizeof(double));
    double *cs  = (double *)malloc((size_t)gmres_restart * sizeof(double));
    double *sn  = (double *)malloc((size_t)gmres_restart * sizeof(double));
    double *g   = (double *)malloc((size_t)(gmres_restart + 1) * sizeof(double));
    double *y   = (double *)malloc((size_t)gmres_restart * sizeof(double));
    if (!F || !du || !up || !Fp || !w || !Q || !H || !cs || !sn || !g || !y) {
        exit(EXIT_FAILURE);
    }

    /* Initial residual */
    bratu_residual(u, F, (int)N, lam);
    double f0 = bratu_norm(F, (int)N);

    for (int step = 0; step < max_newton; ++step) {
        double fnow = bratu_norm(F, (int)N);
        if (fnow <= newton_rtol * f0) {
            break;
        }
        bratu_gmres(u, F, du, (int)N, lam, gmres_restart, inner_tol,
                    Q, H, cs, sn, g, y, w, up, Fp);
        /* u = u + du */
        #pragma omp parallel for
        for (size_t i = 0; i < len; ++i) {
            u[i] += du[i];
        }
        /* Compute the new residual */
        bratu_residual(u, F, (int)N, lam);
    }

    /* Clean up */
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
    (void)workspace; // unused – kept to match required signature
    (void)workspace_size;
}

