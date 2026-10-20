/* RK4 ensemble of independent 3-species Brusselator systems.
 * Parallelized over systems with OpenMP; vectorized across systems by
 * running a block of CH systems through the identical step sequence. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define CH 16

static void rk4_block(double *restrict yp, const double *restrict y0p,
                      int64_t NSTEPS, double a, double b, double ep,
                      double h, int m)
{
    double u[CH], v[CH], w[CH];
    for (int i = 0; i < m; ++i) {
        u[i] = y0p[3 * i + 0];
        v[i] = y0p[3 * i + 1];
        w[i] = y0p[3 * i + 2];
    }
    const double c = 0.5 * h;   /* == 0.5*h exactly as the reference groups it */
    const double h6 = h / 6.0;

    for (int64_t s = 0; s < NSTEPS; ++s) {
        double k1u[CH], k1v[CH], k1w[CH];
        double k2u[CH], k2v[CH], k2w[CH];
        double k3u[CH], k3v[CH], k3w[CH];
        double k4u[CH], k4v[CH], k4w[CH];
        /* accumulate k1 + 2*k2 + 2*k3 + k4 in the reference's left-assoc order */
        double su[CH], sv[CH], sw[CH];

        for (int i = 0; i < m; ++i) {
            k1u[i] = a - (w[i] + 1.0) * u[i] + v[i] * u[i] * u[i];
            k1v[i] = w[i] * u[i] - v[i] * u[i] * u[i];
            k1w[i] = (b - w[i]) / ep - w[i] * u[i];
            su[i] = k1u[i];
            sv[i] = k1v[i];
            sw[i] = k1w[i];
        }
        for (int i = 0; i < m; ++i) {
            double un = u[i] + c * k1u[i];
            double vn = v[i] + c * k1v[i];
            double wn = w[i] + c * k1w[i];
            k2u[i] = a - (wn + 1.0) * un + vn * un * un;
            k2v[i] = wn * un - vn * un * un;
            k2w[i] = (b - wn) / ep - wn * un;
            su[i] = su[i] + 2.0 * k2u[i];
            sv[i] = sv[i] + 2.0 * k2v[i];
            sw[i] = sw[i] + 2.0 * k2w[i];
        }
        for (int i = 0; i < m; ++i) {
            double un = u[i] + c * k2u[i];
            double vn = v[i] + c * k2v[i];
            double wn = w[i] + c * k2w[i];
            k3u[i] = a - (wn + 1.0) * un + vn * un * un;
            k3v[i] = wn * un - vn * un * un;
            k3w[i] = (b - wn) / ep - wn * un;
            su[i] = su[i] + 2.0 * k3u[i];
            sv[i] = sv[i] + 2.0 * k3v[i];
            sw[i] = sw[i] + 2.0 * k3w[i];
        }
        for (int i = 0; i < m; ++i) {
            double un = u[i] + h * k3u[i];
            double vn = v[i] + h * k3v[i];
            double wn = w[i] + h * k3w[i];
            k4u[i] = a - (wn + 1.0) * un + vn * un * un;
            k4v[i] = wn * un - vn * un * un;
            k4w[i] = (b - wn) / ep - wn * un;
            su[i] = su[i] + k4u[i];
            sv[i] = sv[i] + k4v[i];
            sw[i] = sw[i] + k4w[i];
        }
        for (int i = 0; i < m; ++i) {
            u[i] = u[i] + h6 * su[i];
            v[i] = v[i] + h6 * sv[i];
            w[i] = w[i] + h6 * sw[i];
        }
    }
    for (int i = 0; i < m; ++i) {
        yp[3 * i + 0] = u[i];
        yp[3 * i + 1] = v[i];
        yp[3 * i + 2] = w[i];
    }
}

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
    if (NSYS <= 0 || NSTEPS <= 0)
        return;
    const double h = t_end / (double)NSTEPS;

    const int64_t nblk = (NSYS + CH - 1) / CH;
#pragma omp parallel for schedule(static)
    for (int64_t blk = 0; blk < nblk; ++blk) {
        const int64_t base = blk * CH;
        const int m = (int)((base + CH <= NSYS) ? CH : (NSYS - base));
        rk4_block(y + 3 * base, y0 + 3 * base, NSTEPS, a, b, ep, h, m);
    }
}
