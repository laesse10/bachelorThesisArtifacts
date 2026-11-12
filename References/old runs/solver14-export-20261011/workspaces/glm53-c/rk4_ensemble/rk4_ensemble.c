#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* One block of BLK systems is held SoA-style (u[],v[],w[]) so every stage of an RK4
 * step vectorizes across the block and the working set stays in registers. */
#ifndef BLK
#define BLK 16
#endif

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
    if (NSYS <= 0)
        return;

    const double h  = t_end / (double)NSTEPS;
    const double hh = 0.5 * h;   /* reference computes u + 0.5*h*k */
    const double h6 = h / 6.0;

    const int64_t nfull = (NSYS / BLK) * BLK;

    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (int64_t base = 0; base < nfull; base += BLK) {
            double u[BLK], v[BLK], w[BLK];
            double au[BLK], av[BLK], aw[BLK];

            {
                const double *src = y0 + 3 * base;
                for (int i = 0; i < BLK; i++) {
                    u[i] = src[3 * i + 0];
                    v[i] = src[3 * i + 1];
                    w[i] = src[3 * i + 2];
                }
            }

            for (int64_t s = 0; s < NSTEPS; s++) {
                double ku[BLK], kv[BLK], kw[BLK];
                double ku2[BLK], kv2[BLK], kw2[BLK];
                double ku3[BLK], kv3[BLK], kw3[BLK];
                double ku4[BLK], kv4[BLK], kw4[BLK];

                /* stage 1 */
                for (int i = 0; i < BLK; i++) {
                    const double ui = u[i];
                    const double vi = v[i];
                    const double wi = w[i];
                    ku[i] = a - (wi + 1.0) * ui + vi * ui * ui;
                    kv[i] = wi * ui - vi * ui * ui;
                    kw[i] = (b - wi) / ep - wi * ui;
                    au[i] = ku[i];
                    av[i] = kv[i];
                    aw[i] = kw[i];
                }

                /* stage 2 */
                for (int i = 0; i < BLK; i++) {
                    const double un = u[i] + hh * ku[i];
                    const double vn = v[i] + hh * kv[i];
                    const double wn = w[i] + hh * kw[i];
                    ku2[i] = a - (wn + 1.0) * un + vn * un * un;
                    kv2[i] = wn * un - vn * un * un;
                    kw2[i] = (b - wn) / ep - wn * un;
                    au[i] += 2.0 * ku2[i];
                    av[i] += 2.0 * kv2[i];
                    aw[i] += 2.0 * kw2[i];
                }

                /* stage 3 */
                for (int i = 0; i < BLK; i++) {
                    const double un = u[i] + hh * ku2[i];
                    const double vn = v[i] + hh * kv2[i];
                    const double wn = w[i] + hh * kw2[i];
                    ku3[i] = a - (wn + 1.0) * un + vn * un * un;
                    kv3[i] = wn * un - vn * un * un;
                    kw3[i] = (b - wn) / ep - wn * un;
                    au[i] += 2.0 * ku3[i];
                    av[i] += 2.0 * kv3[i];
                    aw[i] += 2.0 * kw3[i];
                }

                /* stage 4 */
                for (int i = 0; i < BLK; i++) {
                    const double un = u[i] + h * ku3[i];
                    const double vn = v[i] + h * kv3[i];
                    const double wn = w[i] + h * kw3[i];
                    ku4[i] = a - (wn + 1.0) * un + vn * un * un;
                    kv4[i] = wn * un - vn * un * un;
                    kw4[i] = (b - wn) / ep - wn * un;
                    au[i] += ku4[i];
                    av[i] += kv4[i];
                    aw[i] += kw4[i];
                }

                for (int i = 0; i < BLK; i++) {
                    u[i] = u[i] + h6 * au[i];
                    v[i] = v[i] + h6 * av[i];
                    w[i] = w[i] + h6 * aw[i];
                }
            }

            {
                double *dst = y + 3 * base;
                for (int i = 0; i < BLK; i++) {
                    dst[3 * i + 0] = u[i];
                    dst[3 * i + 1] = v[i];
                    dst[3 * i + 2] = w[i];
                }
            }
        }

        /* remainder systems, one at a time */
        #pragma omp for schedule(static)
        for (int64_t n = nfull; n < NSYS; n++) {
            double u = y0[3 * n + 0];
            double v = y0[3 * n + 1];
            double w = y0[3 * n + 2];
            for (int64_t s = 0; s < NSTEPS; s++) {
                const double k1u = a - (w + 1.0) * u + v * u * u;
                const double k1v = w * u - v * u * u;
                const double k1w = (b - w) / ep - w * u;
                const double u2 = u + hh * k1u;
                const double v2 = v + hh * k1v;
                const double w2 = w + hh * k1w;
                const double k2u = a - (w2 + 1.0) * u2 + v2 * u2 * u2;
                const double k2v = w2 * u2 - v2 * u2 * u2;
                const double k2w = (b - w2) / ep - w2 * u2;
                const double u3 = u + hh * k2u;
                const double v3 = v + hh * k2v;
                const double w3 = w + hh * k2w;
                const double k3u = a - (w3 + 1.0) * u3 + v3 * u3 * u3;
                const double k3v = w3 * u3 - v3 * u3 * u3;
                const double k3w = (b - w3) / ep - w3 * u3;
                const double u4 = u + h * k3u;
                const double v4 = v + h * k3v;
                const double w4 = w + h * k3w;
                const double k4u = a - (w4 + 1.0) * u4 + v4 * u4 * u4;
                const double k4v = w4 * u4 - v4 * u4 * u4;
                const double k4w = (b - w4) / ep - w4 * u4;
                u = u + h6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
                v = v + h6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
                w = w + h6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
            }
            y[3 * n + 0] = u;
            y[3 * n + 1] = v;
            y[3 * n + 2] = w;
        }
    }
}
