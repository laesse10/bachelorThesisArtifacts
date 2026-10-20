#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void rb_sor_fp64(const double *restrict f,
                 double *restrict u,
                 const int64_t N,
                 const int64_t TSTEPS,
                 const double omega,
                 uint8_t *restrict workspace,
                 const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double one_minus_omega = 1.0 - omega;
    for (int64_t t = 0; t < TSTEPS; ++t) {
        // red
        for (int64_t i = 1; i < N - 1; ++i) {
            const double *restrict u_up = u + (i - 1) * N;
            const double *restrict u_down = u + (i + 1) * N;
            double *restrict u_cur = u + i * N;
            const double *restrict f_cur = f + i * N;
            int64_t j_start = (i % 2 == 0) ? 2 : 1;
            for (int64_t j = j_start; j < N - 1; j += 2) {
                double g = (u_up[j] + u_down[j] + u_cur[j - 1] + u_cur[j + 1] + h2 * f_cur[j]) * 0.25;
                u_cur[j] = one_minus_omega * u_cur[j] + omega * g;
            }
        }
        // black
        for (int64_t i = 1; i < N - 1; ++i) {
            const double *restrict u_up = u + (i - 1) * N;
            const double *restrict u_down = u + (i + 1) * N;
            double *restrict u_cur = u + i * N;
            const double *restrict f_cur = f + i * N;
            int64_t j_start = (i % 2 == 0) ? 1 : 2;
            for (int64_t j = j_start; j < N - 1; j += 2) {
                double g = (u_up[j] + u_down[j] + u_cur[j - 1] + u_cur[j + 1] + h2 * f_cur[j]) * 0.25;
                u_cur[j] = one_minus_omega * u_cur[j] + omega * g;
            }
        }
    }
}
