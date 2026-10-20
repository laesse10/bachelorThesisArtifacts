#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef GGROUP
#define GGROUP 16
#endif

#define RSTEP(i)                                                        \
    do {                                                                \
        double uu = u[(i)], vv = v[(i)], ww = w[(i)];                   \
        double k1u = a - (ww + 1.0) * uu + vv * uu * uu;                \
        double k1v = ww * uu - vv * uu * uu;                            \
        double k1w = (b - ww) * ep_inv - ww * uu;                       \
        double u2 = uu + h2 * k1u;                                      \
        double v2 = vv + h2 * k1v;                                      \
        double w2 = ww + h2 * k1w;                                      \
        double k2u = a - (w2 + 1.0) * u2 + v2 * u2 * u2;                \
        double k2v = w2 * u2 - v2 * u2 * u2;                            \
        double k2w = (b - w2) * ep_inv - w2 * u2;                       \
        double u3 = uu + h2 * k2u;                                      \
        double v3 = vv + h2 * k2v;                                      \
        double w3 = ww + h2 * k2w;                                      \
        double k3u = a - (w3 + 1.0) * u3 + v3 * u3 * u3;                \
        double k3v = w3 * u3 - v3 * u3 * u3;                            \
        double k3w = (b - w3) * ep_inv - w3 * u3;                       \
        double u4 = uu + h * k3u;                                       \
        double v4 = vv + h * k3v;                                       \
        double w4 = ww + h * k3w;                                       \
        double k4u = a - (w4 + 1.0) * u4 + v4 * u4 * u4;                \
        double k4v = w4 * u4 - v4 * u4 * u4;                            \
        double k4w = (b - w4) * ep_inv - w4 * u4;                       \
        u[(i)] = uu + h6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);         \
        v[(i)] = vv + h6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);         \
        w[(i)] = ww + h6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);         \
    } while (0)

static void integrate_group(double *u, double *v, double *w, int64_t NSTEPS,
                            double a, double b, double ep_inv, double h,
                            double h2, double h6)
{
    for (int64_t s = 0; s < NSTEPS; s++) {
        for (int i = 0; i < GGROUP; i++) {
            RSTEP(i);
        }
    }
}

static void integrate_one(double *u, double *v, double *w, int64_t NSTEPS,
                          double a, double b, double ep_inv, double h,
                          double h2, double h6)
{
    for (int64_t s = 0; s < NSTEPS; s++) {
        double uu = *u, vv = *v, ww = *w;
        double k1u = a - (ww + 1.0) * uu + vv * uu * uu;
        double k1v = ww * uu - vv * uu * uu;
        double k1w = (b - ww) * ep_inv - ww * uu;
        double u2 = uu + h2 * k1u;
        double v2 = vv + h2 * k1v;
        double w2 = ww + h2 * k1w;
        double k2u = a - (w2 + 1.0) * u2 + v2 * u2 * u2;
        double k2v = w2 * u2 - v2 * u2 * u2;
        double k2w = (b - w2) * ep_inv - w2 * u2;
        double u3 = uu + h2 * k2u;
        double v3 = vv + h2 * k2v;
        double w3 = ww + h2 * k2w;
        double k3u = a - (w3 + 1.0) * u3 + v3 * u3 * u3;
        double k3v = w3 * u3 - v3 * u3 * u3;
        double k3w = (b - w3) * ep_inv - w3 * u3;
        double u4 = uu + h * k3u;
        double v4 = vv + h * k3v;
        double w4 = ww + h * k3w;
        double k4u = a - (w4 + 1.0) * u4 + v4 * u4 * u4;
        double k4v = w4 * u4 - v4 * u4 * u4;
        double k4w = (b - w4) * ep_inv - w4 * u4;
        *u = uu + h6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
        *v = vv + h6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
        *w = ww + h6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
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
    const double h = t_end / (double)NSTEPS;
    const double h2 = 0.5 * h;
    const double h6 = h / 6.0;
    const double ep_inv = 1.0 / ep;
    const int64_t full = (NSYS / GGROUP) * GGROUP;
    (void)workspace; (void)workspace_size;

    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < full; n += GGROUP) {
        double u[GGROUP], v[GGROUP], w[GGROUP];
        const double *src = y0 + 3 * n;
        for (int i = 0; i < GGROUP; i++) {
            u[i] = src[3 * i];
            v[i] = src[3 * i + 1];
            w[i] = src[3 * i + 2];
        }
        integrate_group(u, v, w, NSTEPS, a, b, ep_inv, h, h2, h6);
        double *dst = y + 3 * n;
        for (int i = 0; i < GGROUP; i++) {
            dst[3 * i] = u[i];
            dst[3 * i + 1] = v[i];
            dst[3 * i + 2] = w[i];
        }
    }

    #pragma omp parallel for schedule(static)
    for (int64_t n = full; n < NSYS; n++) {
        double u = y0[3 * n];
        double v = y0[3 * n + 1];
        double w = y0[3 * n + 2];
        integrate_one(&u, &v, &w, NSTEPS, a, b, ep_inv, h, h2, h6);
        y[3 * n] = u;
        y[3 * n + 1] = v;
        y[3 * n + 2] = w;
    }
}
