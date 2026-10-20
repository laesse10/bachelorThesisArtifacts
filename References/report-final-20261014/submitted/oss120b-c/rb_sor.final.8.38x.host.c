#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void rb_sor_fp64(const double *restrict f,
                 double *restrict u,
                 const int64_t N,
                 const int64_t TSTEPS,
                 const double omega,
                 uint8_t *restrict workspace,
                 const int64_t workspace_size) {
    (void)workspace;
    (void)workspace_size;
    // Early exit for degenerate cases
    if (N <= 2 || TSTEPS <= 0) {
        return;
    }
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om1 = 1.0 - omega;
    // Main time stepping loop.
    for (int64_t t = 0; t < TSTEPS; ++t) {
        // Red half-sweep: (i + j) % 2 == 0  -> i%2 == j%2
        #pragma omp parallel for schedule(static)
        for (int64_t i = 1; i < N - 1; ++i) {
            // Choose start column so that (i + j) % 2 == 0.
            int64_t j_start = ((i & 1) == 0) ? 2 : 1;
            for (int64_t j = j_start; j < N - 1; j += 2) {
                const int64_t idx = i * N + j;
                // Neighbour sum
                double g = (u[idx - N] + u[idx + N] + u[idx - 1] + u[idx + 1] + h2 * f[idx]) * 0.25;
                u[idx] = om1 * u[idx] + omega * g;
            }
        }
        // Black half-sweep: (i + j) % 2 == 1  -> i%2 != j%2
        #pragma omp parallel for schedule(static)
        for (int64_t i = 1; i < N - 1; ++i) {
            int64_t j_start = ((i & 1) == 0) ? 1 : 2;
            for (int64_t j = j_start; j < N - 1; j += 2) {
                const int64_t idx = i * N + j;
                double g = (u[idx - N] + u[idx + N] + u[idx - 1] + u[idx + 1] + h2 * f[idx]) * 0.25;
                u[idx] = om1 * u[idx] + omega * g;
            }
        }
    }
}
