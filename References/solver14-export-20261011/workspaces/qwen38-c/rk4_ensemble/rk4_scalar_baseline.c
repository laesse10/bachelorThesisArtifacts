#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void rk4_ensemble_fp64(
    double *restrict y,
    const double *restrict y0,
    const int64_t NSTEPS,
    const int64_t NSYS,
    const double a,
    const double b,
    const double ep,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {

    const double h = t_end / (double)NSTEPS;
    const double h2 = 0.5 * h;
    const double h6 = h / 6.0;

    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        const double *restrict src = y0 + 3 * n;
        double u = src[0];
        double v = src[1];
        double w = src[2];

        for (int64_t step = 0; step < NSTEPS; step++) {
            /* k1 */
            double k1_0 = a - (w + 1.0) * u + v * u * u;
            double k1_1 = w * u - v * u * u;
            double k1_2 = (b - w) / ep - w * u;
            /* k2 */
            double u2 = u + h2 * k1_0;
            double v2 = v + h2 * k1_1;
            double w2 = w + h2 * k1_2;
            double k2_0 = a - (w2 + 1.0) * u2 + v2 * u2 * u2;
            double k2_1 = w2 * u2 - v2 * u2 * u2;
            double k2_2 = (b - w2) / ep - w2 * u2;
            /* k3 */
            double u3 = u + h2 * k2_0;
            double v3 = v + h2 * k2_1;
            double w3 = w + h2 * k2_2;
            double k3_0 = a - (w3 + 1.0) * u3 + v3 * u3 * u3;
            double k3_1 = w3 * u3 - v3 * u3 * u3;
            double k3_2 = (b - w3) / ep - w3 * u3;
            /* k4 */
            double u4 = u + h * k3_0;
            double v4 = v + h * k3_1;
            double w4 = w + h * k3_2;
            double k4_0 = a - (w4 + 1.0) * u4 + v4 * u4 * u4;
            double k4_1 = w4 * u4 - v4 * u4 * u4;
            double k4_2 = (b - w4) / ep - w4 * u4;
            /* update */
            u = u + h6 * (k1_0 + 2.0 * k2_0 + 2.0 * k3_0 + k4_0);
            v = v + h6 * (k1_1 + 2.0 * k2_1 + 2.0 * k3_1 + k4_1);
            w = w + h6 * (k1_2 + 2.0 * k2_2 + 2.0 * k3_2 + k4_2);
        }

        double *restrict dst = y + 3 * n;
        dst[0] = u;
        dst[1] = v;
        dst[2] = w;
    }
}
