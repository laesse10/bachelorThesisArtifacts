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
    (void)workspace;
    (void)workspace_size;
    if (N < 3 || TSTEPS < 1) return;

    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double omw = 1.0 - omega;

    #pragma omp parallel
    {
        for (int64_t t = 0; t < TSTEPS; t++) {
            #pragma omp for schedule(static)
            for (int64_t i = 1; i < N - 1; i++) {
                int64_t j0 = ((i & 1) == 0) ? 2 : 1;
                const double *up = u + (i - 1) * N;
                const double *dn = u + (i + 1) * N;
                double *mid = u + i * N;
                const double *fr = f + i * N;
                for (int64_t j = j0; j <= N - 2; j += 2) {
                    double g = (up[j] + dn[j] + mid[j-1] + mid[j+1] + h2*fr[j]) * 0.25;
                    mid[j] = omw * mid[j] + omega * g;
                }
            }
            #pragma omp for schedule(static)
            for (int64_t i = 1; i < N - 1; i++) {
                int64_t j0 = ((i & 1) == 0) ? 1 : 2;
                const double *up = u + (i - 1) * N;
                const double *dn = u + (i + 1) * N;
                double *mid = u + i * N;
                const double *fr = f + i * N;
                for (int64_t j = j0; j <= N - 2; j += 2) {
                    double g = (up[j] + dn[j] + mid[j-1] + mid[j+1] + h2*fr[j]) * 0.25;
                    mid[j] = omw * mid[j] + omega * g;
                }
            }
        }
    }
}
