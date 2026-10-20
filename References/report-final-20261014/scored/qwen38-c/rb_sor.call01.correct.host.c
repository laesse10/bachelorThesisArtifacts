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
    const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double c1 = 1.0 - omega;

    for (int64_t t = 0; t < TSTEPS; t++) {
        for (int64_t parity = 0; parity < 2; parity++) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 1; i < N - 1; i++) {
                const int64_t j0 = ((i & 1) == parity) ? 2 : 1;
                const double *up = u + (i - 1) * N;
                const double *dn = u + (i + 1) * N;
                double *row = u + i * N;
                const double *fr = f + i * N;
                for (int64_t j = j0; j < N - 1; j += 2) {
                    const double g = (up[j] + dn[j] + row[j - 1] + row[j + 1]
                                      + h2 * fr[j]) / 4.0;
                    row[j] = c1 * row[j] + omega * g;
                }
            }
        }
    }
}
