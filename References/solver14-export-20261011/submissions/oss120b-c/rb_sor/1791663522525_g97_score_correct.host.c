#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * Red-Black Successive Over-Relaxation (SOR) kernel.
 *
 * f : input source term (NxN), read‑only
 * u : solution array (NxN), updated in place (interior only)
 * N : grid dimension (including boundary); must be >= 3 and even
 * TSTEPS : number of red‑black sweeps (each sweep = red + black)
 * omega : relaxation factor (1.0 = Gauss‑Seidel)
 * workspace, workspace_size : unused (present for ABI compatibility)
 */

void rb_sor_fp64(const double *restrict f,
                 double *restrict u,
                 const int64_t N,
                 const int64_t TSTEPS,
                 const double omega,
                 uint8_t *restrict workspace,
                 const int64_t workspace_size) {
    // Silence unused‑parameter warnings
    (void)workspace;
    (void)workspace_size;

    // Grid spacing squared (h^2). The domain is [0,1]x[0,1] with Dirichlet boundaries.
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;

    // Pre‑compute the factor (1‑omega) since it does not change.
    const double one_minus_omega = 1.0 - omega;

    // Main time‑stepping loop.
    for (int64_t t = 0; t < TSTEPS; ++t) {
        // ------------------------------------------------------------
        // Red sweep  (parity = 0, i + j even)
        // ------------------------------------------------------------
        #pragma omp parallel for schedule(static)
        for (int64_t i = 1; i < N - 1; ++i) {
            const double *restrict u_up   = u + (i - 1) * N;
            const double *restrict u_down = u + (i + 1) * N;
            double *restrict       u_cur  = u + i * N;
            const double *restrict f_cur  = f + i * N;
            // Compute the first column index that satisfies (i + j) % 2 == 0.
            int64_t j_start = ((i + 0) % 2 == 0) ? 2 : 1;
            // The interior columns stop at N-2 (inclusive).
            for (int64_t j = j_start; j < N - 1; j += 2) {
                double g = (u_up[j] + u_down[j] + u_cur[j - 1] + u_cur[j + 1] + h2 * f_cur[j]) * 0.25;
                u_cur[j] = one_minus_omega * u_cur[j] + omega * g;
            }
        }

        // ------------------------------------------------------------
        // Black sweep (parity = 1, i + j odd)
        // ------------------------------------------------------------
        #pragma omp parallel for schedule(static)
        for (int64_t i = 1; i < N - 1; ++i) {
            const double *restrict u_up   = u + (i - 1) * N;
            const double *restrict u_down = u + (i + 1) * N;
            double *restrict       u_cur  = u + i * N;
            const double *restrict f_cur  = f + i * N;
            // First column index with (i + j) % 2 == 1.
            int64_t j_start = ((i + 1) % 2 == 0) ? 2 : 1; // equivalent to ((i + 1) % 2 != 0) ? 1 : 2
            // Simpler: j_start = ((i + 1) & 1) ? 1 : 2;
            // Use the expression directly for clarity.
            j_start = ((i + 1) & 1) ? 1 : 2;
            for (int64_t j = j_start; j < N - 1; j += 2) {
                double g = (u_up[j] + u_down[j] + u_cur[j - 1] + u_cur[j + 1] + h2 * f_cur[j]) * 0.25;
                u_cur[j] = one_minus_omega * u_cur[j] + omega * g;
            }
        }
    }
}

