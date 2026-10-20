#include <stddef.h>
#include <stdint.h>
#include <omp.h>

/*
 * Red‑black Successive Over‑Relaxation (SOR) kernel for a 2‑D Poisson problem.
 *
 * The algorithm updates the interior of an N×N grid for TSTEPS iterations.
 * Each iteration consists of two half‑sweeps: first all red points
 * ((i + j) % 2 == 0) and then all black points ((i + j) % 2 == 1).
 * The update is
 *   g = (u_{i-1,j} + u_{i+1,j} + u_{i,j-1} + u_{i,j+1} + h^2 * f_{i,j}) / 4
 *   u_{i,j} = (1‑omega) * u_{i,j} + omega * g
 * with h = 1/(N‑1) and h² = h * h.
 *
 * The boundary values of u remain zero (Dirichlet).  The arrays f and u are
 * stored in row‑major order as flat 1‑D arrays of size N * N.
 *
 * Parallelism: each half‑sweep touches disjoint indices, so it can be safely
 * parallelised with OpenMP.  The loops avoid the branch used in the reference
 * implementation by iterating only over the required colour via a step of two.
 */

void rb_sor_fp64(const double *restrict f,
                 double *restrict u,
                 const int64_t N,
                 const int64_t TSTEPS,
                 const double omega,
                 uint8_t *restrict workspace,
                 const int64_t workspace_size)
{
    // The workspace is not needed for this kernel.
    (void)workspace;
    (void)workspace_size;

    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double one_minus_omega = 1.0 - omega;

    // Main time‑stepping loop.
    for (int64_t t = 0; t < TSTEPS; ++t) {
        // ----- Red sweep (parity = 0) -----
        const int parity_red = 0;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 1; i < N - 1; ++i) {
            // Starting column for this row such that (i + j) % 2 == parity_red.
            // Required j parity is parity_red ^ (i & 1).
            int64_t j_start = ((parity_red ^ (i & 1)) ? 1 : 2);
            for (int64_t j = j_start; j < N - 1; j += 2) {
                const int64_t idx = i * N + j;
                const double g = (u[(i - 1) * N + j] +
                                   u[(i + 1) * N + j] +
                                   u[i * N + (j - 1)] +
                                   u[i * N + (j + 1)] +
                                   h2 * f[idx]) * 0.25;
                u[idx] = one_minus_omega * u[idx] + omega * g;
            }
        }

        // ----- Black sweep (parity = 1) -----
        const int parity_black = 1;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 1; i < N - 1; ++i) {
            int64_t j_start = ((parity_black ^ (i & 1)) ? 1 : 2);
            for (int64_t j = j_start; j < N - 1; j += 2) {
                const int64_t idx = i * N + j;
                const double g = (u[(i - 1) * N + j] +
                                   u[(i + 1) * N + j] +
                                   u[i * N + (j - 1)] +
                                   u[i * N + (j + 1)] +
                                   h2 * f[idx]) * 0.25;
                u[idx] = one_minus_omega * u[idx] + omega * g;
            }
        }
    }
}

