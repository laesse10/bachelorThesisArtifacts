#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

#ifndef BATCH
#define BATCH 5
#endif

/* one RK4 step for one 8-system group; accumulates into s0,s1,s2 (weights 1,2,2,1) */
#define STEP(U,V,W,S0,S1,S2) do { \
    __m512d uu = _mm512_mul_pd((U),(U)); \
    __m512d vuu = _mm512_mul_pd((V),uu); \
    __m512d k1_0 = _mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd((W), one), (U))), vuu); \
    __m512d k1_1 = _mm512_sub_pd(_mm512_mul_pd((W),(U)), vuu); \
    __m512d k1_2 = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv,(W)), dEp), _mm512_mul_pd((W),(U))); \
    (S0)=k1_0; (S1)=k1_1; (S2)=k1_2; \
    __m512d m2u=_mm512_fmadd_pd(h2v,k1_0,(U)), m2v=_mm512_fmadd_pd(h2v,k1_1,(V)), m2w=_mm512_fmadd_pd(h2v,k1_2,(W)); \
    __m512d m2uu=_mm512_mul_pd(m2u,m2u), m2vuu=_mm512_mul_pd(m2v,m2uu); \
    __m512d k2_0=_mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(m2w,one),m2u)), m2vuu); \
    __m512d k2_1=_mm512_sub_pd(_mm512_mul_pd(m2w,m2u), m2vuu); \
    __m512d k2_2=_mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv,m2w), dEp), _mm512_mul_pd(m2w,m2u)); \
    (S0)=_mm512_fmadd_pd(two,k2_0,(S0)); (S1)=_mm512_fmadd_pd(two,k2_1,(S1)); (S2)=_mm512_fmadd_pd(two,k2_2,(S2)); \
    __m512d m3u=_mm512_fmadd_pd(h2v,k2_0,(U)), m3v=_mm512_fmadd_pd(h2v,k2_1,(V)), m3w=_mm512_fmadd_pd(h2v,k2_2,(W)); \
    __m512d m3uu=_mm512_mul_pd(m3u,m3u), m3vuu=_mm512_mul_pd(m3v,m3uu); \
    __m512d k3_0=_mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(m3w,one),m3u)), m3vuu); \
    __m512d k3_1=_mm512_sub_pd(_mm512_mul_pd(m3w,m3u), m3vuu); \
    __m512d k3_2=_mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv,m3w), dEp), _mm512_mul_pd(m3w,m3u)); \
    (S0)=_mm512_fmadd_pd(two,k3_0,(S0)); (S1)=_mm512_fmadd_pd(two,k3_1,(S1)); (S2)=_mm512_fmadd_pd(two,k3_2,(S2)); \
    __m512d m4u=_mm512_fmadd_pd(hv,k3_0,(U)), m4v=_mm512_fmadd_pd(hv,k3_1,(V)), m4w=_mm512_fmadd_pd(hv,k3_2,(W)); \
    __m512d m4uu=_mm512_mul_pd(m4u,m4u), m4vuu=_mm512_mul_pd(m4v,m4uu); \
    __m512d k4_0=_mm512_add_pd(_mm512_sub_pd(Av, _mm512_mul_pd(_mm512_add_pd(m4w,one),m4u)), m4vuu); \
    __m512d k4_1=_mm512_sub_pd(_mm512_mul_pd(m4w,m4u), m4vuu); \
    __m512d k4_2=_mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(Bv,m4w), dEp), _mm512_mul_pd(m4w,m4u)); \
    (S0)=_mm512_add_pd((S0),k4_0); (S1)=_mm512_add_pd((S1),k4_1); (S2)=_mm512_add_pd((S2),k4_2); \
    (U)=_mm512_fmadd_pd(h6v,(S0),(U)); (V)=_mm512_fmadd_pd(h6v,(S1),(V)); (W)=_mm512_fmadd_pd(h6v,(S2),(W)); \
} while(0)

#define LOADGRP(J) do { \
    const double *sj = (const double*)y0 + 3*((base)+8*(J)); \
    double as##J[8], bs##J[8], cs##J[8]; \
    for (int i=0;i<8;i++){ as##J[i]=sj[3*i]; bs##J[i]=sj[3*i+1]; cs##J[i]=sj[3*i+2]; } \
    u##J = _mm512_loadu_pd(as##J); v##J = _mm512_loadu_pd(bs##J); w##J = _mm512_loadu_pd(cs##J); \
} while(0)

#define STOREGRP(J) do { \
    double as##J[8], bs##J[8], cs##J[8]; \
    _mm512_storeu_pd(as##J, u##J); _mm512_storeu_pd(bs##J, v##J); _mm512_storeu_pd(cs##J, w##J); \
    double *dj = (double*)y + 3*((base)+8*(J)); \
    for (int i=0;i<8;i++){ dj[3*i]=as##J[i]; dj[3*i+1]=bs##J[i]; dj[3*i+2]=cs##J[i]; } \
} while(0)

/* rename STEP args to U/V/W-style via a wrapper so token paste works */
#define STEPJ(J) STEP(u##J,v##J,w##J, sa##J,sb##J,sc##J)

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
    const double inv_ep = 1.0 / ep;

    __m512d Av  = _mm512_set1_pd(a);
    __m512d Bv  = _mm512_set1_pd(b);
    __m512d one = _mm512_set1_pd(1.0);
    __m512d two = _mm512_set1_pd(2.0);
    __m512d h2v = _mm512_set1_pd(h2);
    __m512d hv  = _mm512_set1_pd(h);
    __m512d h6v = _mm512_set1_pd(h6);
    __m512d dEp = _mm512_set1_pd(inv_ep);

    const int64_t stride = 8 * BATCH;
    const int64_t n_full = (NSYS / stride) * stride;

    #pragma omp parallel for schedule(static)
    for (int64_t base = 0; base < n_full; base += stride) {
        __m512d u0,v0,w0,u1,v1,w1,u2,v2,w2,u3,v3,w3,u4,v4,w4,u5,v5,w5,u6,v6,w6,u7,v7,w7;
        __m512d sa0,sb0,sc0,sa1,sb1,sc1,sa2,sb2,sc2,sa3,sb3,sc3,sa4,sb4,sc4,sa5,sb5,sc5,sa6,sb6,sc6,sa7,sb7,sc7;
        (void)u1;(void)v1;(void)w1;(void)u2;(void)v2;(void)w2;(void)u3;(void)v3;(void)w3;(void)u4;(void)v4;(void)w4;(void)u5;(void)v5;(void)w5;(void)u6;(void)v6;(void)w6;(void)u7;(void)v7;(void)w7;
        (void)sa1;(void)sb1;(void)sc1;(void)sa2;(void)sb2;(void)sc2;(void)sa3;(void)sb3;(void)sc3;(void)sa4;(void)sb4;(void)sc4;(void)sa5;(void)sb5;(void)sc5;(void)sa6;(void)sb6;(void)sc6;(void)sa7;(void)sb7;(void)sc7;
        LOADGRP(0); LOADGRP(1);
#if BATCH>=2
        LOADGRP(2);
#endif
#if BATCH>=3
        LOADGRP(3);
#endif
#if BATCH>=4
        LOADGRP(4);
#endif
#if BATCH>=5
        LOADGRP(5);
#endif
#if BATCH>=6
        LOADGRP(6);
#endif
#if BATCH>=7
        LOADGRP(7);
#endif
        for (int64_t step = 0; step < NSTEPS; step++) {
            STEPJ(0); STEPJ(1);
#if BATCH>=2
            STEPJ(2);
#endif
#if BATCH>=3
            STEPJ(3);
#endif
#if BATCH>=4
            STEPJ(4);
#endif
#if BATCH>=5
            STEPJ(5);
#endif
#if BATCH>=6
            STEPJ(6);
#endif
#if BATCH>=7
            STEPJ(7);
#endif
        }
        STOREGRP(0); STOREGRP(1);
#if BATCH>=2
        STOREGRP(2);
#endif
#if BATCH>=3
        STOREGRP(3);
#endif
#if BATCH>=4
        STOREGRP(4);
#endif
#if BATCH>=5
        STOREGRP(5);
#endif
#if BATCH>=6
        STOREGRP(6);
#endif
#if BATCH>=7
        STOREGRP(7);
#endif

    }

    #pragma omp parallel for schedule(static)
    for (int64_t n = n_full; n < NSYS; n++) {
        const double *restrict src = y0 + 3 * n;
        double u = src[0], v = src[1], w = src[2];
        for (int64_t step = 0; step < NSTEPS; step++) {
            double uu = u * u; double vuu = v * uu;
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
