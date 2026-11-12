#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

/* 1 = use (b-w)*inv_ep (fast), 0 = use (b-w)/ep (matches reference exactly) */
#ifndef RK4_USE_RECIP
#define RK4_USE_RECIP 1
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
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const double h = t_end / (double)NSTEPS;
    const double h2 = 0.5 * h;
    const double h6 = h / 6.0;

    __m512d Av  = _mm512_set1_pd(a);
    __m512d Bv  = _mm512_set1_pd(b);
    __m512d one = _mm512_set1_pd(1.0);
    __m512d two = _mm512_set1_pd(2.0);
    __m512d h2v = _mm512_set1_pd(h2);
    __m512d hv  = _mm512_set1_pd(h);
    __m512d h6v = _mm512_set1_pd(h6);
#if RK4_USE_RECIP
    __m512d dEp = _mm512_set1_pd(1.0 / ep);
#else
    __m512d dEp = _mm512_set1_pd(ep);
#endif
    const double inv_ep = 1.0 / ep;

    const int64_t n_full = (NSYS / 8) * 8;

    #pragma omp parallel for schedule(static)
    for (int64_t base = 0; base < n_full; base += 8) {
        const double *src = y0 + 3 * base;
        double us[8], vs[8], ws[8];
        #pragma omp simd
        for (int i = 0; i < 8; i++) { us[i] = src[3*i]; vs[i] = src[3*i+1]; ws[i] = src[3*i+2]; }
        __m512d u = _mm512_loadu_pd(us);
        __m512d v = _mm512_loadu_pd(vs);
        __m512d w = _mm512_loadu_pd(ws);

        for (int64_t step = 0; step < NSTEPS; step++) {
            __m512d uu = _mm512_mul_pd(u, u);
            __m512d vuu = _mm512_mul_pd(v, uu);
            __m512d k1_0 = _mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(w, one), u)), vuu);
            __m512d k1_1 = _mm512_sub_pd(_mm512_mul_pd(w, u), vuu);
            __m512d k1_2 = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv, w), dEp), _mm512_mul_pd(w, u));
            __m512d u2 = _mm512_fmadd_pd(h2v, k1_0, u), v2 = _mm512_fmadd_pd(h2v, k1_1, v), w2 = _mm512_fmadd_pd(h2v, k1_2, w);
            __m512d uu2 = _mm512_mul_pd(u2, u2), vuu2 = _mm512_mul_pd(v2, uu2);
            __m512d k2_0 = _mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(w2, one), u2)), vuu2);
            __m512d k2_1 = _mm512_sub_pd(_mm512_mul_pd(w2, u2), vuu2);
            __m512d k2_2 = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv, w2), dEp), _mm512_mul_pd(w2, u2));
            __m512d u3 = _mm512_fmadd_pd(h2v, k2_0, u), v3 = _mm512_fmadd_pd(h2v, k2_1, v), w3 = _mm512_fmadd_pd(h2v, k2_2, w);
            __m512d uu3 = _mm512_mul_pd(u3, u3), vuu3 = _mm512_mul_pd(v3, uu3);
            __m512d k3_0 = _mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(w3, one), u3)), vuu3);
            __m512d k3_1 = _mm512_sub_pd(_mm512_mul_pd(w3, u3), vuu3);
            __m512d k3_2 = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv, w3), dEp), _mm512_mul_pd(w3, u3));
            __m512d u4 = _mm512_fmadd_pd(hv, k3_0, u), v4 = _mm512_fmadd_pd(hv, k3_1, v), w4 = _mm512_fmadd_pd(hv, k3_2, w);
            __m512d uu4 = _mm512_mul_pd(u4, u4), vuu4 = _mm512_mul_pd(v4, uu4);
            __m512d k4_0 = _mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(w4, one), u4)), vuu4);
            __m512d k4_1 = _mm512_sub_pd(_mm512_mul_pd(w4, u4), vuu4);
            __m512d k4_2 = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv, w4), dEp), _mm512_mul_pd(w4, u4));
            __m512d s0 = _mm512_add_pd(_mm512_fmadd_pd(two, k3_0, _mm512_fmadd_pd(two, k2_0, k1_0)), k4_0);
            u = _mm512_fmadd_pd(h6v, s0, u);
            __m512d s1 = _mm512_add_pd(_mm512_fmadd_pd(two, k3_1, _mm512_fmadd_pd(two, k2_1, k1_1)), k4_1);
            v = _mm512_fmadd_pd(h6v, s1, v);
            __m512d s2 = _mm512_add_pd(_mm512_fmadd_pd(two, k3_2, _mm512_fmadd_pd(two, k2_2, k1_2)), k4_2);
            w = _mm512_fmadd_pd(h6v, s2, w);
        }
        double os[8], ov[8], ow[8];
        _mm512_storeu_pd(os, u);
        _mm512_storeu_pd(ov, v);
        _mm512_storeu_pd(ow, w);
        double *dst = y + 3 * base;
        #pragma omp simd
        for (int i = 0; i < 8; i++) { dst[3*i] = os[i]; dst[3*i+1] = ov[i]; dst[3*i+2] = ow[i]; }
    }

    #pragma omp parallel for schedule(static)
    for (int64_t n = n_full; n < NSYS; n++) {
        const double *restrict src = y0 + 3 * n;
        double u = src[0], v = src[1], w = src[2];
        for (int64_t step = 0; step < NSTEPS; step++) {
            double uu = u * u;
            double vuu = v * uu;
            double k1_0 = a - (w + 1.0) * u + vuu;
            double k1_1 = w * u - vuu;
            double k1_2 = ((b - w) * inv_ep) - w * u;
            double u2 = u + h2 * k1_0, v2 = v + h2 * k1_1, w2 = w + h2 * k1_2;
            double uu2 = u2 * u2, vuu2 = v2 * uu2;
            double k2_0 = a - (w2 + 1.0) * u2 + vuu2;
            double k2_1 = w2 * u2 - vuu2;
            double k2_2 = ((b - w2) * inv_ep) - w2 * u2;
            double u3 = u + h2 * k2_0, v3 = v + h2 * k2_1, w3 = w + h2 * k2_2;
            double uu3 = u3 * u3, vuu3 = v3 * uu3;
            double k3_0 = a - (w3 + 1.0) * u3 + vuu3;
            double k3_1 = w3 * u3 - vuu3;
            double k3_2 = ((b - w3) * inv_ep) - w3 * u3;
            double u4 = u + h * k3_0, v4 = v + h * k3_1, w4 = w + h * k3_2;
            double uu4 = u4 * u4, vuu4 = v4 * uu4;
            double k4_0 = a - (w4 + 1.0) * u4 + vuu4;
            double k4_1 = w4 * u4 - vuu4;
            double k4_2 = ((b - w4) * inv_ep) - w4 * u4;
            u = u + h6 * (k1_0 + 2.0 * k2_0 + 2.0 * k3_0 + k4_0);
            v = v + h6 * (k1_1 + 2.0 * k2_1 + 2.0 * k3_1 + k4_1);
            w = w + h6 * (k1_2 + 2.0 * k2_2 + 2.0 * k3_2 + k4_2);
        }
        double *restrict dst = y + 3 * n;
        dst[0] = u; dst[1] = v; dst[2] = w;
    }
}
