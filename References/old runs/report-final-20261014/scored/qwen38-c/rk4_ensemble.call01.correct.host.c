/* Fixed-step RK4 over an ensemble of independent 3-species Brusselator ODEs.
 *
 *   du/dt = a - (w+1)*u + v*u^2
 *   dv/dt = w*u - v*u^2
 *   dw/dt = (b-w)/ep - w*u
 *
 * Each of the NSYS systems is integrated independently, so the outer loop is a
 * pure MAP: OpenMP parallel over systems.
 */
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
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const double h = t_end / (double)NSTEPS;

#pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        double u = y0[3 * n + 0], v = y0[3 * n + 1], w = y0[3 * n + 2];
        for (int64_t s = 0; s < NSTEPS; s++) {
            double k1u = a - (w + 1.0) * u + v * u * u;
            double k1v = w * u - v * u * u;
            double k1w = (b - w) / ep - w * u;
            double u2 = u + 0.5 * h * k1u;
            double v2 = v + 0.5 * h * k1v;
            double w2 = w + 0.5 * h * k1w;
            double k2u = a - (w2 + 1.0) * u2 + v2 * u2 * u2;
            double k2v = w2 * u2 - v2 * u2 * u2;
            double k2w = (b - w2) / ep - w2 * u2;
            double u3 = u + 0.5 * h * k2u;
            double v3 = v + 0.5 * h * k2v;
            double w3 = w + 0.5 * h * k2w;
            double k3u = a - (w3 + 1.0) * u3 + v3 * u3 * u3;
            double k3v = w3 * u3 - v3 * u3 * u3;
            double k3w = (b - w3) / ep - w3 * u3;
            double u4 = u + h * k3u;
            double v4 = v + h * k3v;
            double w4 = w + h * k3w;
            double k4u = a - (w4 + 1.0) * u4 + v4 * u4 * u4;
            double k4v = w4 * u4 - v4 * u4 * u4;
            double k4w = (b - w4) / ep - w4 * u4;
            u = u + (h / 6.0) * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
            v = v + (h / 6.0) * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
            w = w + (h / 6.0) * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
        }
        y[3 * n + 0] = u;
        y[3 * n + 1] = v;
        y[3 * n + 2] = w;
    }
}
