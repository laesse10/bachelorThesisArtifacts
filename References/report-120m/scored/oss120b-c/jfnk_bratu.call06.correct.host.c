#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <omp.h>
#include <stdio.h>

/* Helper function to compute linear index */
static inline int64_t idx(int64_t i, int64_t j, int64_t N) {
    return i * N + j;
}

/* Compute the Bratu residual: F = Laplace(u) - lam*exp(u) on interior, copy boundary values */
static void bratu_residual(const double *restrict u, double *restrict F, int64_t N, double lam) {
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    /* Boundary: Dirichlet u=0, but we copy the current u value (which should be zero) */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        F[idx(i, 0, N)] = u[idx(i, 0, N)];
        F[idx(i, N - 1, N)] = u[idx(i, N - 1, N)];
    }
    #pragma omp parallel for schedule(static)
    for (int64_t j = 0; j < N; ++j) {
        F[idx(0, j, N)] = u[idx(0, j, N)];
        F[idx(N - 1, j, N)] = u[idx(N - 1, j, N)];
    }
    /* Interior stencil */
    #pragma omp parallel for collapse(2) schedule(static)
    for (int64_t i = 1; i < N - 1; ++i) {
        for (int64_t j = 1; j < N - 1; ++j) {
            double val = (4.0 * u[idx(i, j, N)]
                          - u[idx(i - 1, j, N)]
                          - u[idx(i + 1, j, N)]
                          - u[idx(i, j - 1, N)]
                          - u[idx(i, j + 1, N)]) / h2
                         - lam * exp(u[idx(i, j, N)]);
            F[idx(i, j, N)] = val;
        }
    }
}

/* Euclidean (L2) norm of an N x N array */
static double bratu_norm(const double *restrict A, int64_t N) {
    const int64_t M = N * N;
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (int64_t i = 0; i < M; ++i) {
        sum += A[i] * A[i];
    }
    return sqrt(sum);
}

/* Dot product of two N x N arrays */
static double bratu_dot(const double *restrict A, const double *restrict B, int64_t N) {
    const int64_t M = N * N;
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for (int64_t i = 0; i < M; ++i) {
        sum += A[i] * B[i];
    }
    return sum;
}

/* Matrix‑free Jacobian‑vector product using a scaled finite‑difference step */
static void bratu_jvp(const double *restrict u, const double *restrict v,
                      const double *restrict Fu, double *restrict Jv,
                      double *restrict up, double *restrict Fp,
                      int64_t N, double lam) {
    double nu = bratu_norm(u, N);
    double nv = bratu_norm(v, N);
    if (nv == 0.0) {
        const int64_t M = N * N;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < M; ++i) {
            Jv[i] = 0.0;
        }
        return;
    }
    double eps = sqrt(DBL_EPSILON) * (1.0 + nu) / nv;
    const int64_t M = N * N;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; ++i) {
        up[i] = u[i] + eps * v[i];
    }
    bratu_residual(up, Fp, N, lam);
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; ++i) {
        Jv[i] = (Fp[i] - Fu[i]) / eps;
    }
}

/* GMRES with restart. All work arrays are supplied by the caller. */
static void bratu_gmres(const double *restrict u, const double *restrict Fu,
                        double *restrict du, int64_t N, double lam,
                        int64_t restart, double inner_tol,
                        double *restrict Q,   /* (restart+1) * M */
                        double *restrict H,   /* (restart+1) * restart */
                        double *restrict cs,  /* restart */
                        double *restrict sn,  /* restart */
                        double *restrict g,   /* restart+1 */
                        double *restrict y,   /* restart */
                        double *restrict w,   /* M */
                        double *restrict up,  /* M */
                        double *restrict Fp) /* M */ {
    const int64_t M = N * N;
    double beta = bratu_norm(Fu, N);
    if (beta == 0.0) {
        // Zero residual, solution update stays zero
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < M; ++i) du[i] = 0.0;
        return;
    }
    /* Q[0] = -Fu / beta */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; ++i) {
        Q[i] = -Fu[i] / beta;
    }
    g[0] = beta;
    int64_t m_used = restart;
    for (int64_t k = 0; k < restart; ++k) {
        double *Qk = Q + k * M;
        /* w = J(u) * Qk  (matrix‑free) */
        bratu_jvp(u, Qk, Fu, w, up, Fp, N, lam);
        /* Modified Gram‑Schmidt */
        for (int64_t p = 0; p <= k; ++p) {
            double *Qp = Q + p * M;
            double h_pk = bratu_dot(Qp, w, N);
            H[p * restart + k] = h_pk;
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < M; ++i) {
                w[i] -= h_pk * Qp[i];
            }
        }
        double h_next = bratu_norm(w, N);
        H[(k + 1) * restart + k] = h_next;
        /* Apply previous Givens rotations to the new column */
        for (int64_t p = 0; p < k; ++p) {
            double temp = cs[p] * H[p * restart + k] + sn[p] * H[(p + 1) * restart + k];
            H[(p + 1) * restart + k] = -sn[p] * H[p * restart + k] + cs[p] * H[(p + 1) * restart + k];
            H[p * restart + k] = temp;
        }
        /* New rotation */
        double a = H[k * restart + k];
        double b = h_next;
        double denom = sqrt(a * a + b * b);
        cs[k] = a / denom;
        sn[k] = b / denom;
        /* Apply rotation to H */
        H[k * restart + k] = cs[k] * a + sn[k] * b; /* becomes denom */
        H[(k + 1) * restart + k] = 0.0;
        /* Apply rotation to g */
        double temp_g = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = temp_g;
        double rel = fabs(g[k + 1]) / beta;
        if (rel < inner_tol || h_next < 1.0e-13 || k == restart - 1) {
            m_used = k + 1;
            break;
        }
        /* Q[k+1] = w / h_next */
        double *Qkp1 = Q + (k + 1) * M;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < M; ++i) {
            Qkp1[i] = w[i] / h_next;
        }
    }
    /* Back‑substitution: solve H * y = g for the first m_used rows/cols */
    for (int64_t row = m_used - 1; row >= 0; --row) {
        double s = g[row];
        for (int64_t col = row + 1; col < m_used; ++col) {
            s -= H[row * restart + col] * y[col];
        }
        y[row] = s / H[row * restart + row];
    }
    /* Compute the solution update du = \sum_{p=0}^{m_used-1} Q[p] * y[p] */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; ++i) du[i] = 0.0;
    for (int64_t p = 0; p < m_used; ++p) {
        double coeff = y[p];
        double *Qp = Q + p * M;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < M; ++i) {
            du[i] += coeff * Qp[i];
        }
    }
}

/* Main entry point required by the benchmark */
void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size) {
    // Fixed solver parameters (as per benchmark configuration)
    const int64_t used_max_newton = 20; // maximum Newton iterations
    const int64_t used_restart = 50;    // GMRES restart length
    const double used_inner_tol = 1e-4; // GMRES tolerance
    const size_t header_bytes = 0; // No header in workspace

    const int64_t M = N * N;
    /* Compute required size in doubles */
    int64_t r = used_restart;
    int64_t required_doubles = M * (r + 6) + r * r + 5 * r + 1;
    size_t required_bytes = (size_t)required_doubles * sizeof(double);

    double *buf = NULL;
    bool allocated = false;
    size_t total_required_bytes = header_bytes + required_bytes;
    printf("DEBUG: required_bytes=%zu total_required_bytes=%zu\n", required_bytes, total_required_bytes);
    fflush(stdout);
    if ((size_t)workspace_size >= total_required_bytes) {
        buf = (double *)((uint8_t *)workspace + header_bytes); /* use provided workspace after header */
    } else {
        buf = (double *)malloc(required_bytes);
        if (!buf) return; /* allocation failure – give up */
        allocated = true;
    }

    /* Slice the buffer */
    double *F  = buf;                             /* M */
    double *du = F  + M;                          /* M */
    double *up = du + M;                          /* M */
    double *Fp = up + M;                          /* M */
    double *w  = Fp + M;                          /* M */
    double *Q  = w  + M;                          /* (r+1)*M */
    double *H  = Q  + (r + 1) * M;                /* (r+1)*r */
    double *cs = H  + (r + 1) * r;                /* r */
    double *sn = cs + r;                         /* r */
    double *g  = sn + r;                         /* r+1 */
    double *y  = g  + (r + 1);                    /* r */

    /* Initial residual */
    bratu_residual(u, F, N, lam);
    double f0 = bratu_norm(F, N);
    const double newton_rtol = 1.0e-14;
    for (int64_t step = 0; step < used_max_newton; ++step) {
        double fnow = bratu_norm(F, N);
        if (fnow <= newton_rtol * f0) break;
        bratu_gmres(u, F, du, N, lam, r, used_inner_tol, Q, H, cs, sn, g, y, w, up, Fp);
        /* u = u + du */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < M; ++i) {
            u[i] += du[i];
        }
        bratu_residual(u, F, N, lam);
    }

    if (allocated) free(buf);
}

