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

/* One RK4 step for system i.
 *
 * The RHS  du = a - (w+1)u + v u^2 ; dv = w u - v u^2 ; dw = (b-w)/ep - w u
 * is evaluated with a couple of exact algebraic shares:
 *   au = a - u,  mu = w*u,  r2 = au - mu (= a - (w+1) u)
 *   du = r2 + v*u*u          (one FMA on (v*u)*u + r2)
 *   dv = au - du             (= w u - v u^2, exact identity)
 *   dw = (b-w)*ep_inv - mu   (ep_inv = 1/ep)
 * Stage arguments use FMA(h2, k, x) = x + h2*k (the reference computes
 * x + (0.5*h)*k; a single fused rounding vs two - still 1 ulp level, the
 * same class the FMA-built C reference has against the pure numba one).
 * The final update follows the reference association:
 *   x += (h/6)*(((k1 + 2 k2) + 2 k3) + k4).
 */
#define RSTEP(i)                                                          \
    do {                                                                  \
        double uu = u[(i)], vv = v[(i)], ww = w[(i)];                     \
        double au = a - uu;                                               \
        double mu = ww * uu;                                              \
        double r2 = au - mu;                                              \
        double pu = vv * uu;                                              \
        double k1u = fma(pu, uu, r2);                                     \
        double k1v = au - k1u;                                            \
        double k1w = (b - ww) * ep_inv - mu;                              \
        double u2 = fma(h2, k1u, uu);                                     \
        double v2 = fma(h2, k1v, vv);                                     \
        double w2 = fma(h2, k1w, ww);                                     \
        double au2 = a - u2;                                              \
        double mu2 = w2 * u2;                                             \
        double r3 = au2 - mu2;                                            \
        double pu2 = v2 * u2;                                             \
        double k2u = fma(pu2, u2, r3);                                    \
        double k2v = au2 - k2u;                                           \
        double k2w = (b - w2) * ep_inv - mu2;                             \
        double u3 = fma(h2, k2u, uu);                                     \
        double v3 = fma(h2, k2v, vv);                                     \
        double w3 = fma(h2, k2w, ww);                                     \
        double au3 = a - u3;                                              \
        double mu3 = w3 * u3;                                             \
        double r4 = au3 - mu3;                                            \
        double pu3 = v3 * u3;                                             \
        double k3u = fma(pu3, u3, r4);                                    \
        double k3v = au3 - k3u;                                           \
        double k3w = (b - w3) * ep_inv - mu3;                             \
        double u4 = fma(h, k3u, uu);                                      \
        double v4 = fma(h, k3v, vv);                                      \
        double w4 = fma(h, k3w, ww);                                      \
        double au4 = a - u4;                                              \
        double mu4 = w4 * u4;                                             \
        double r5 = au4 - mu4;                                            \
        double pu4 = v4 * u4;                                             \
        double k4u = fma(pu4, u4, r5);                                    \
        double k4v = au4 - k4u;                                           \
        double k4w = (b - w4) * ep_inv - mu4;                             \
        double su = fma(2.0, k2u, k1u);                                   \
        su = fma(2.0, k3u, su) + k4u;                                     \
        u[(i)] = fma(h6, su, uu);                                         \
        double sv = fma(2.0, k2v, k1v);                                   \
        sv = fma(2.0, k3v, sv) + k4v;                                     \
        v[(i)] = fma(h6, sv, vv);                                         \
        double sw = fma(2.0, k2w, k1w);                                   \
        sw = fma(2.0, k3w, sw) + k4w;                                     \
        w[(i)] = fma(h6, sw, ww);                                         \
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
        double au = a - uu;
        double mu = ww * uu;
        double r2 = au - mu;
        double pu = vv * uu;
        double k1u = fma(pu, uu, r2);
        double k1v = au - k1u;
        double k1w = (b - ww) * ep_inv - mu;
        double u2 = fma(h2, k1u, uu);
        double v2 = fma(h2, k1v, vv);
        double w2 = fma(h2, k1w, ww);
        double au2 = a - u2;
        double mu2 = w2 * u2;
        double r3 = au2 - mu2;
        double pu2 = v2 * u2;
        double k2u = fma(pu2, u2, r3);
        double k2v = au2 - k2u;
        double k2w = (b - w2) * ep_inv - mu2;
        double u3 = fma(h2, k2u, uu);
        double v3 = fma(h2, k2v, vv);
        double w3 = fma(h2, k2w, ww);
        double au3 = a - u3;
        double mu3 = w3 * u3;
        double r4 = au3 - mu3;
        double pu3 = v3 * u3;
        double k3u = fma(pu3, u3, r4);
        double k3v = au3 - k3u;
        double k3w = (b - w3) * ep_inv - mu3;
        double u4 = fma(h, k3u, uu);
        double v4 = fma(h, k3v, vv);
        double w4 = fma(h, k3w, ww);
        double au4 = a - u4;
        double mu4 = w4 * u4;
        double r5 = au4 - mu4;
        double pu4 = v4 * u4;
        double k4u = fma(pu4, u4, r5);
        double k4v = au4 - k4u;
        double k4w = (b - w4) * ep_inv - mu4;
        double su = fma(2.0, k2u, k1u);
        su = fma(2.0, k3u, su) + k4u;
        *u = fma(h6, su, uu);
        double sv = fma(2.0, k2v, k1v);
        sv = fma(2.0, k3v, sv) + k4v;
        *v = fma(h6, sv, vv);
        double sw = fma(2.0, k2w, k1w);
        sw = fma(2.0, k3w, sw) + k4w;
        *w = fma(h6, sw, ww);
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
