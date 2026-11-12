#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void rb_sor_fp64(
    const double *restrict f,
    double *restrict u,
    const int64_t N,
    const int64_t TSTEPS,
    const double omega,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om = omega;
    const double som = 1.0 - omega;
    const int64_t N2 = N - 2;

    #pragma omp parallel
    {
        const int nt = omp_get_num_threads();
        const int tid = omp_get_thread_num();
        /* contiguous block of interior rows [1, N-2] per thread */
        const int64_t rows = N2 - 1 + 1; /* = N-2 interior rows, i in [1, N-2] */
        int64_t r0 = 1 + (rows * tid) / nt;
        int64_t r1 = 1 + (rows * (tid + 1)) / nt;

        for (int64_t t = 0; t < TSTEPS; ++t) {
            for (int parity = 0; parity < 2; ++parity) {
                for (int64_t i = r0; i < r1; ++i) {
                    const double *up   = u + (i - 1) * N;
                    const double *dn   = u + (i + 1) * N;
                    double *row  = u + i * N;
                    const double *frow = f + i * N;
                    int64_t j0 = (((parity ^ (int)(i & 1))) & 1) ? 1 : 2;
                    for (int64_t j = j0; j <= N2; j += 2) {
                        const double g = (up[j] + dn[j] + row[j - 1] + row[j + 1] + h2 * frow[j]) * 0.25;
                        row[j] = som * row[j] + om * g;
                    }
                }
                #pragma omp barrier
            }
        }
    }
}
