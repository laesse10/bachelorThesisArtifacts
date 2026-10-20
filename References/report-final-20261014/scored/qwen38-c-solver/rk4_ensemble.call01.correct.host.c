/* Fixed-step RK4 over an ensemble of independent 3-species Brusselator ODEs.
 *
 *   du/dt = a - (w+1)*u + v*u^2
 *   dv/dt = w*u - v*u^2
 *   dw/dt = (b-w)/ep - w*u
 *
 * Each of the NSYS systems is integrated independently with NSTEPS classical
 * Runge-Kutta steps of size h = t_end/NSTEPS.  The systems never interact, so
 * the outer loop is a map and is parallelized with OpenMP.
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
    const double h2 = 0.5 * h;
    const double h6 = h / 6.0;

    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        double u = y0[3 * n + 0];
        double v = y0[3 * n + 1];
        double w = y0[3 * n + 2];
        for (int64_t s = 0; s < NSTEPS; s++) {
            double k1u, k1v, k1w, k2u, k2v, k2w, k3u, k3v, k3w, k4u, k4v, k4w;
            double uu, vu2, wu;

            uu  = u * u;
            vu2 = v * uu;
            wu  = w * u;
            k1u = a - (w + 1.0) * u + vu2;
            k1v = wu - vu2;
            k1w = (b - w) / ep - wu;

            double s1u = u + h2 * k1u;
            double s1v = v + h2 * k1v;
            double s1w = w + h2 * k1w;
            uu  = s1u * s1u;
            vu2 = s1v * uu;
            wu  = s1w * s1u;
            k2u = a - (s1w + 1.0) * s1u + vu2;
            k2v = wu - vu2;
            k2w = (b - s1w) / ep - wu;

            double s2u = u + h2 * k2u;
            double s2v = v + h2 * k2v;
            double s2w = w + h2 * k2w;
            uu  = s2u * s2u;
            vu2 = s2v * uu;
            wu  = s2w * s2u;
            k3u = a - (s2w + 1.0) * s2u + vu2;
            k3v = wu - vu2;
            k3w = (b - s2w) / ep - wu;

            double s3u = u + h * k3u;
            double s3v = v + h * k3v;
            double s3w = w + h * k3w;
            uu  = s3u * s3u;
            vu2 = s3v * uu;
            wu  = s3w * s3u;
            k4u = a - (s3w + 1.0) * s3u + vu2;
            k4v = wu - vu2;
            k4w = (b - s3w) / ep - wu;

            u = u + h6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
            v = v + h6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
            w = w + h6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
        }
        y[3 * n + 0] = u;
        y[3 * n + 1] = v;
        y[3 * n + 2] = w;
    }
}
