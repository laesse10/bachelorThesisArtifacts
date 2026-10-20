#include <stddef.h>
#include <stdint.h>
#include <omp.h>

/*
 * Red‑black Successive Over‑Relaxation (SOR) kernel for a 2‑D Poisson problem.
 *
 * The algorithm updates the interior of an N×N grid for TSTEPS iterations.
 * Each iteration consists of two half‑sweeps: first all red points
 * ((i + j) % 2 == 0) and then all black points ((i + j) % 2 == 1).
 *
 * The update is
 *   g = (u[i-1,j] + u[i+1,j] + u[i,j-1] + u[i,j+1] + h^2 * f[i,j]) / 4
 *   u[i,j] = (1-omega) * u[i,j] + omega * g
 * where h = 1/(N-1) and h2 = h*h.
 *
 * The arrays f and u are stored in row-major order as flat 1-D arrays of length N*N.
 * Zero Dirichlet boundary conditions are assumed (the outer ring of u stays zero).
 *
 * Parallelism: each half-sweep touches disjoint points, so it can be safely
 * parallelised with OpenMP. The loops avoid the branch used in the reference
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
    (void)workspace; // unused
    (void)workspace_size;

    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double one_minus_omega = 1.0 - omega;

    #pragma omp parallel
    {
        for (int64_t t = 0; t < TSTEPS; ++t) {
            /* red sweep: (i + j) % 2 == 0 */
            #pragma omp for nowait schedule(static)
            for (int64_t i = 1; i < N - 1; ++i) {
                // For a given row i, the column start depends on i parity.
                // If i is odd, the first red column is j=1 (odd); if i is even, j=2 (even).
                int64_t j_start = ((i & 1) ? 1 : 2);
                #pragma omp simd
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
            #pragma omp barrier

            /* black sweep: (i + j) % 2 == 1 */
            #pragma omp for schedule(static)
            for (int64_t i = 1; i < N - 1; ++i) {
                // For black points the start column parity is opposite to red.
                int64_t j_start = ((i & 1) ? 2 : 1);
                #pragma omp simd
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
}
