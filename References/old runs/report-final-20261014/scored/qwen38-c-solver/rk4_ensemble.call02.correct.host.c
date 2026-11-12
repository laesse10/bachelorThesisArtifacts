/* AVX-512 kernel: 8 systems per 512-bit lane, OpenMP over system blocks.
 *
 * RK4, classic 4-stage form -- every stage reads the ORIGINAL state of the
 * step (base registers UB/VB/WB are never modified during the step):
 *   k1 = f(U0)
 *   k2 = f(U0 + h2*k1)
 *   k3 = f(U0 + h2*k2)
 *   k4 = f(U0 + h*k3)
 *   U1 = U0 + h6*(k1 + 2*k2 + 2*k3 + k4)
 * f(u,v,w):
 *   uu = u*u ; wu = w*u ; tt = a - u - wu ; vu2 = v*uu
 *   du = v*uu + tt = v*u^2 + a - (w+1)*u
 *   dv = wu - vu2
 *   dw = b*invep - (invep*w + wu) = (b-w)/ep - w*u
 * No gather/scatter (this host executes vgatherqpd incorrectly).
 * Block in/out uses scalar set_pd loads / store8 (amortized over NSTEPS).
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

#ifndef TWO_BLOCK
#define TWO_BLOCK 1
#endif

/* B1/B2/B3: base state, read-only. F1/F2/F3: destination, written once (last line).
 * A1/A2/A3: running accumulator, seeded from the base on the first k1 lines.
 * Callers MUST keep the three argument triples in distinct registers. */
#define STEP8(B1, B2, B3, F1, F2, F3, A1, A2, A3) do {                            \
    __m512d uu, wu, tt, vu2, kk, SU, SV, SW;                                       \
    /* --- k1 = f(B1,B2,B3) --- */                                                \
    uu  = _mm512_mul_pd(B1, B1);                                                   \
    wu  = _mm512_mul_pd(B3, B1);                                                   \
    tt  = _mm512_sub_pd(va, B1);                                                   \
    tt  = _mm512_sub_pd(tt, wu);                                                   \
    vu2 = _mm512_mul_pd(B2, uu);                                                   \
    kk  = _mm512_fmadd_pd(vinvep, B3, wu);                                         \
    __m512d k1u = _mm512_fmadd_pd(B2, uu, tt);                                     \
    __m512d k1v = _mm512_sub_pd(wu, vu2);                                          \
    __m512d k1w = _mm512_sub_pd(vbm, kk);                                          \
    A1 = _mm512_fmadd_pd(vh6, k1u, B1);                                            \
    A2 = _mm512_fmadd_pd(vh6, k1v, B2);                                            \
    A3 = _mm512_fmadd_pd(vh6, k1w, B3);                                            \
    SU = _mm512_fmadd_pd(vh2, k1u, B1);                                            \
    SV = _mm512_fmadd_pd(vh2, k1v, B2);                                            \
    SW = _mm512_fmadd_pd(vh2, k1w, B3);                                            \
    /* --- k2 = f(SU,SV,SW) --- */                                                \
    uu  = _mm512_mul_pd(SU, SU);                                                   \
    wu  = _mm512_mul_pd(SW, SU);                                                   \
    tt  = _mm512_sub_pd(va, SU);                                                   \
    tt  = _mm512_sub_pd(tt, wu);                                                   \
    vu2 = _mm512_mul_pd(SV, uu);                                                   \
    kk  = _mm512_fmadd_pd(vinvep, SW, wu);                                         \
    __m512d k2u = _mm512_fmadd_pd(SV, uu, tt);                                     \
    __m512d k2v = _mm512_sub_pd(wu, vu2);                                          \
    __m512d k2w = _mm512_sub_pd(vbm, kk);                                          \
    A1 = _mm512_fmadd_pd(vtw6, k2u, A1);                                           \
    A2 = _mm512_fmadd_pd(vtw6, k2v, A2);                                           \
    A3 = _mm512_fmadd_pd(vtw6, k2w, A3);                                           \
    SU = _mm512_fmadd_pd(vh2, k2u, B1);                                            \
    SV = _mm512_fmadd_pd(vh2, k2v, B2);                                            \
    SW = _mm512_fmadd_pd(vh2, k2w, B3);                                            \
    /* --- k3 = f(SU,SV,SW) --- */                                                \
    uu  = _mm512_mul_pd(SU, SU);                                                   \
    wu  = _mm512_mul_pd(SW, SU);                                                   \
    tt  = _mm512_sub_pd(va, SU);                                                   \
    tt  = _mm512_sub_pd(tt, wu);                                                   \
    vu2 = _mm512_mul_pd(SV, uu);                                                   \
    kk  = _mm512_fmadd_pd(vinvep, SW, wu);                                         \
    __m512d k3u = _mm512_fmadd_pd(SV, uu, tt);                                     \
    __m512d k3v = _mm512_sub_pd(wu, vu2);                                          \
    __m512d k3w = _mm512_sub_pd(vbm, kk);                                          \
    A1 = _mm512_fmadd_pd(vtw6, k3u, A1);                                           \
    A2 = _mm512_fmadd_pd(vtw6, k3v, A2);                                           \
    A3 = _mm512_fmadd_pd(vtw6, k3w, A3);                                           \
    SU = _mm512_fmadd_pd(vh, k3u, B1);                                             \
    SV = _mm512_fmadd_pd(vh, k3v, B2);                                             \
    SW = _mm512_fmadd_pd(vh, k3w, B3);                                             \
    /* --- k4 = f(SU,SV,SW) --- */                                                \
    uu  = _mm512_mul_pd(SU, SU);                                                   \
    wu  = _mm512_mul_pd(SW, SU);                                                   \
    tt  = _mm512_sub_pd(va, SU);                                                   \
    tt  = _mm512_sub_pd(tt, wu);                                                   \
    vu2 = _mm512_mul_pd(SV, uu);                                                   \
    kk  = _mm512_fmadd_pd(vinvep, SW, wu);                                         \
    __m512d k4u = _mm512_fmadd_pd(SV, uu, tt);                                     \
    __m512d k4v = _mm512_sub_pd(wu, vu2);                                          \
    __m512d k4w = _mm512_sub_pd(vbm, kk);                                          \
    F1 = _mm512_fmadd_pd(vh6, k4u, A1);                                            \
    F2 = _mm512_fmadd_pd(vh6, k4v, A2);                                            \
    F3 = _mm512_fmadd_pd(vh6, k4w, A3);                                            \
} while (0)

static void store8(double *d, __m512d uu, __m512d vv, __m512d ww)
{
    double u[8], v[8], w[8];
    _mm512_storeu_pd(u, uu);
    _mm512_storeu_pd(v, vv);
    _mm512_storeu_pd(w, ww);
    for (int i = 0; i < 8; i++) {
        d[3 * i + 0] = u[i];
        d[3 * i + 1] = v[i];
        d[3 * i + 2] = w[i];
    }
}

static void step_systems(int64_t base, const double *y0, double *y,
                         int64_t NSTEPS, double h, double h2, double h6,
                         double a, double b, double ep)
{
    const double *src = y0 + 3 * base;
    __m512d va     = _mm512_set1_pd(a);
    __m512d vbm    = _mm512_set1_pd(b / ep);
    __m512d vinvep = _mm512_set1_pd(1.0 / ep);
    __m512d vh     = _mm512_set1_pd(h);
    __m512d vh2    = _mm512_set1_pd(h2);
    __m512d vh6    = _mm512_set1_pd(h6);
    __m512d vtw6   = _mm512_set1_pd(2.0 * h6);
    (void)b;

    __m512d B1 = _mm512_set_pd(src[21], src[18], src[15], src[12],
                               src[9],  src[6],  src[3],  src[0]);
    __m512d B2 = _mm512_set_pd(src[22], src[19], src[16], src[13],
                               src[10], src[7],  src[4],  src[1]);
    __m512d B3 = _mm512_set_pd(src[23], src[20], src[17], src[14],
                               src[11], src[8],  src[5],  src[2]);
    __m512d A1, A2, A3, F1, F2, F3;
    for (int64_t s = 0; s < NSTEPS; s++) {
        STEP8(B1, B2, B3, F1, F2, F3, A1, A2, A3);
        B1 = F1; B2 = F2; B3 = F3;
    }
    store8(y + 3 * base, B1, B2, B3);
}

#if TWO_BLOCK
static void step_systems2(int64_t base, const double *y0, double *y,
                          int64_t NSTEPS, double h, double h2, double h6,
                          double a, double b, double ep)
{
    const double *src = y0 + 3 * base;
    __m512d va     = _mm512_set1_pd(a);
    __m512d vbm    = _mm512_set1_pd(b / ep);
    __m512d vinvep = _mm512_set1_pd(1.0 / ep);
    __m512d vh     = _mm512_set1_pd(h);
    __m512d vh2    = _mm512_set1_pd(h2);
    __m512d vh6    = _mm512_set1_pd(h6);
    __m512d vtw6   = _mm512_set1_pd(2.0 * h6);
    (void)b;

    __m512d B1 = _mm512_set_pd(src[21], src[18], src[15], src[12],
                               src[9],  src[6],  src[3],  src[0]);
    __m512d B2 = _mm512_set_pd(src[22], src[19], src[16], src[13],
                               src[10], src[7],  src[4],  src[1]);
    __m512d B3 = _mm512_set_pd(src[23], src[20], src[17], src[14],
                               src[11], src[8],  src[5],  src[2]);
    const double *src2 = src + 24;
    __m512d C1 = _mm512_set_pd(src2[21], src2[18], src2[15], src2[12],
                               src2[9],  src2[6],  src2[3],  src2[0]);
    __m512d C2 = _mm512_set_pd(src2[22], src2[19], src2[16], src2[13],
                               src2[10], src2[7],  src2[4],  src2[1]);
    __m512d C3 = _mm512_set_pd(src2[23], src2[20], src2[17], src2[14],
                               src2[11], src2[8],  src2[5],  src2[2]);
    __m512d A1, A2, A3, F1, F2, F3, D1, D2, D3, G1, G2, G3;
    for (int64_t s = 0; s < NSTEPS; s++) {
        STEP8(B1, B2, B3, F1, F2, F3, A1, A2, A3);
        B1 = F1; B2 = F2; B3 = F3;
        STEP8(C1, C2, C3, G1, G2, G3, D1, D2, D3);
        C1 = G1; C2 = G2; C3 = G3;
    }
    store8(y + 3 * base, B1, B2, B3);
    store8(y + 3 * base + 24, C1, C2, C3);
}
#endif

static void step_one(int64_t n, const double *restrict y0, double *restrict y,
                     int64_t NSTEPS, double h, double h2, double h6,
                     double a, double b, double ep)
{
    double u = y0[3 * n + 0], v = y0[3 * n + 1], w = y0[3 * n + 2];
    double au = 0.0, av = 0.0, aw = 0.0;
    double h62 = 2.0 * h6;
    for (int64_t s = 0; s < NSTEPS; s++) {
        double uu, wu, tt, vu2;
        uu = u * u; wu = w * u; tt = a - u - wu; vu2 = v * uu;
        double k1u = v * uu + tt, k1v = wu - vu2, k1w = (b - w) / ep - wu;
        double su = u + h2 * k1u, sv = v + h2 * k1v, sw = w + h2 * k1w;
        au = u + h6 * k1u; av = v + h6 * k1v; aw = w + h6 * k1w;
        uu = su * su; wu = sw * su; tt = a - su - wu; vu2 = sv * uu;
        double k2u = sv * uu + tt, k2v = wu - vu2, k2w = (b - sw) / ep - wu;
        su = u + h2 * k2u; sv = v + h2 * k2v; sw = w + h2 * k2w;
        au += h62 * k2u; av += h62 * k2v; aw += h62 * k2w;
        uu = su * su; wu = sw * su; tt = a - su - wu; vu2 = sv * uu;
        double k3u = sv * uu + tt, k3v = wu - vu2, k3w = (b - sw) / ep - wu;
        su = u + h * k3u; sv = v + h * k3v; sw = w + h * k3w;
        au += h62 * k3u; av += h62 * k3v; aw += h62 * k3w;
        uu = su * su; wu = sw * su; tt = a - su - wu; vu2 = sv * uu;
        double k4u = sv * uu + tt, k4v = wu - vu2, k4w = (b - sw) / ep - wu;
        u = au + h6 * k4u; v = av + h6 * k4v; w = aw + h6 * k4w;
    }
    y[3 * n + 0] = u; y[3 * n + 1] = v; y[3 * n + 2] = w;
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
    (void)workspace; (void)workspace_size;
    if (NSYS <= 0 || NSTEPS <= 0) return;
    const double h = t_end / (double)NSTEPS;
    const double h2 = 0.5 * h;
    const double h6 = h / 6.0;

#if TWO_BLOCK
    const int64_t nblk = NSYS / 16;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < nblk; i++)
        step_systems2(16 * i, y0, y, NSTEPS, h, h2, h6, a, b, ep);
    int64_t i = 16 * nblk;
    if (NSYS - i >= 8) {
        step_systems(i, y0, y, NSTEPS, h, h2, h6, a, b, ep);
        i += 8;
    }
    for (; i < NSYS; i++)
        step_one(i, y0, y, NSTEPS, h, h2, h6, a, b, ep);
#else
    const int64_t nblk = NSYS / 8;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < nblk; i++)
        step_systems(8 * i, y0, y, NSTEPS, h, h2, h6, a, b, ep);
    for (int64_t i = 8 * nblk; i < NSYS; i++)
        step_one(i, y0, y, NSTEPS, h, h2, h6, a, b, ep);
#endif
}
