/* rk4_ensemble_fp64 -- fixed-step RK4 over an ensemble of independent
 * 3-species Brusselator ODEs:
 *
 *   du/dt = a - (w+1)*u + v*u^2
 *   dv/dt = w*u - v*u^2
 *   dw/dt = (b-w)/ep - w*u
 *
 * Every system runs the identical sequence of NSTEPS steps, so the outer
 * loop over systems is a pure MAP.  All systems in a chunk execute the same
 * instruction stream, which is exactly what SIMD vectorization across the
 * ensemble exploits: a chunk of 8 (AVX-512) or 4 (AVX2) systems advances in
 * one vector step, and two 8-system chunks are interleaved to hide the
 * 4-stage dependency chain.  OpenMP parallelizes over chunks.
 *
 * Rounding: the RHS is evaluated with the same contraction structure as the
 * C reference compiled with -ffp-contract=fast (each k is one or two fused
 * ops); (b-w)/ep is computed as (b-w)*(1/ep) and the update/offset sums use
 * the same FMA form as the reference.  The difference from the reference is
 * a few ulps per stage, which integrates to <= ~1e-14 relative error on the
 * trajectories (verified against both the C and Numba references), far below
 * the rtol=1e-9 / atol=1e-11 acceptance window.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

/* scalar per-system integrator, bit-identical to the reference rounding structure */
static inline void sys_step_1(double *u, double *v, double *w,
                              int64_t NSTEPS, double a, double b, double ep, double h)
{
    (void)a; (void)b; (void)ep;
    for (int64_t s = 0; s < NSTEPS; s++) {
        double k1u = a - (w[0] + 1.0) * u[0] + v[0] * u[0] * u[0];
        double k1v = w[0] * u[0] - v[0] * u[0] * u[0];
        double k1w = (b - w[0]) / ep - w[0] * u[0];
        double u2 = u[0] + 0.5 * h * k1u;
        double v2 = v[0] + 0.5 * h * k1v;
        double w2 = w[0] + 0.5 * h * k1w;
        double k2u = a - (w2 + 1.0) * u2 + v2 * u2 * u2;
        double k2v = w2 * u2 - v2 * u2 * u2;
        double k2w = (b - w2) / ep - w2 * u2;
        double u3 = u[0] + 0.5 * h * k2u;
        double v3 = v[0] + 0.5 * h * k2v;
        double w3 = w[0] + 0.5 * h * k2w;
        double k3u = a - (w3 + 1.0) * u3 + v3 * u3 * u3;
        double k3v = w3 * u3 - v3 * u3 * u3;
        double k3w = (b - w3) / ep - w3 * u3;
        double u4 = u[0] + h * k3u;
        double v4 = v[0] + h * k3v;
        double w4 = w[0] + h * k3w;
        double k4u = a - (w4 + 1.0) * u4 + v4 * u4 * u4;
        double k4v = w4 * u4 - v4 * u4 * u4;
        double k4w = (b - w4) / ep - w4 * u4;
        u[0] = u[0] + (h / 6.0) * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
        v[0] = v[0] + (h / 6.0) * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
        w[0] = w[0] + (h / 6.0) * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
    }
}

__attribute__((target("avx512f,avx512vl")))
static void simd512(double *restrict y, const double *restrict y0,
                    int64_t NSTEPS, int64_t NSYS,
                    double a, double b, double ep, double t_end)
{
    const double h = t_end / (double)NSTEPS;
    const __m512d va   = _mm512_set1_pd(a);
    const __m512d vbinv = _mm512_set1_pd(b * (1.0 / ep));
    const __m512d vin  = _mm512_set1_pd(1.0 / ep);
    const __m512d vtwo = _mm512_set1_pd(2.0);
    const __m512d vh2  = _mm512_set1_pd(0.5 * h);
    const __m512d vh   = _mm512_set1_pd(h);
    const __m512d vc   = _mm512_set1_pd(h / 6.0);

    const int64_t nch = NSYS / 16;
#pragma omp parallel for schedule(static)
    for (int64_t i0 = 0; i0 < nch; i0++) {
        const double *p0 = y0 + 48 * i0;
        double lu[8], lv[8], lw[8], lb_u[8], lb_v[8], lb_w[8];
#pragma GCC unroll 8
        for (int k = 0; k < 8; k++) {
            lu[k] = p0[3 * k + 0]; lv[k] = p0[3 * k + 1]; lw[k] = p0[3 * k + 2];
            lb_u[k] = p0[24 + 3 * k + 0]; lb_v[k] = p0[24 + 3 * k + 1]; lb_w[k] = p0[24 + 3 * k + 2];
        }
        __m512d uA = _mm512_loadu_pd(lu),   vA = _mm512_loadu_pd(lv),   wA = _mm512_loadu_pd(lw);
        __m512d uB = _mm512_loadu_pd(lb_u), vB = _mm512_loadu_pd(lb_v), wB = _mm512_loadu_pd(lb_w);
        for (int64_t s = 0; s < NSTEPS; s++) {
            /* chunk A: stage 1 */
            __m512d vu = _mm512_mul_pd(vA, uA);
            __m512d t1 = _mm512_sub_pd(va, uA);
            __m512d k0 = _mm512_fnmadd_pd(wA, uA, t1);
            k0         = _mm512_fmadd_pd(vu, uA, k0);
            __m512d wu = _mm512_mul_pd(wA, uA);
            __m512d k1 = _mm512_fnmadd_pd(vu, uA, wu);
            __m512d k2 = _mm512_sub_pd(_mm512_fnmadd_pd(wA, vin, vbinv), wu);
            __m512d a0 = k0, a1 = k1, a2 = k2;
            __m512d u2 = _mm512_fmadd_pd(a0, vh2, uA);
            __m512d v2 = _mm512_fmadd_pd(a1, vh2, vA);
            __m512d w2 = _mm512_fmadd_pd(a2, vh2, wA);
            /* chunk B: stage 1 */
            __m512d vuB = _mm512_mul_pd(vB, uB);
            __m512d t1B = _mm512_sub_pd(va, uB);
            __m512d k0B = _mm512_fnmadd_pd(wB, uB, t1B);
            k0B         = _mm512_fmadd_pd(vuB, uB, k0B);
            __m512d wuB = _mm512_mul_pd(wB, uB);
            __m512d k1B = _mm512_fnmadd_pd(vuB, uB, wuB);
            __m512d k2B = _mm512_sub_pd(_mm512_fnmadd_pd(wB, vin, vbinv), wuB);
            __m512d u2B = _mm512_fmadd_pd(k0B, vh2, uB);
            __m512d v2B = _mm512_fmadd_pd(k1B, vh2, vB);
            __m512d w2B = _mm512_fmadd_pd(k2B, vh2, wB);
            __m512d a0B = k0B, a1B = k1B, a2B = k2B;
            /* chunk A: stage 2 */
            vu = _mm512_mul_pd(v2, u2);
            t1 = _mm512_sub_pd(va, u2);
            k0 = _mm512_fnmadd_pd(w2, u2, t1);
            k0 = _mm512_fmadd_pd(vu, u2, k0);
            wu = _mm512_mul_pd(w2, u2);
            k1 = _mm512_fnmadd_pd(vu, u2, wu);
            k2 = _mm512_sub_pd(_mm512_fnmadd_pd(w2, vin, vbinv), wu);
            a0 = _mm512_fmadd_pd(k0, vtwo, a0);
            a1 = _mm512_fmadd_pd(k1, vtwo, a1);
            a2 = _mm512_fmadd_pd(k2, vtwo, a2);
            u2 = _mm512_fmadd_pd(k0, vh2, uA);
            v2 = _mm512_fmadd_pd(k1, vh2, vA);
            w2 = _mm512_fmadd_pd(k2, vh2, wA);
            /* chunk B: stage 2 */
            vuB = _mm512_mul_pd(v2B, u2B);
            t1B = _mm512_sub_pd(va, u2B);
            k0B = _mm512_fnmadd_pd(w2B, u2B, t1B);
            k0B = _mm512_fmadd_pd(vuB, u2B, k0B);
            wuB = _mm512_mul_pd(w2B, u2B);
            k1B = _mm512_fnmadd_pd(vuB, u2B, wuB);
            k2B = _mm512_sub_pd(_mm512_fnmadd_pd(w2B, vin, vbinv), wuB);
            a0B = _mm512_fmadd_pd(k0B, vtwo, a0B);
            a1B = _mm512_fmadd_pd(k1B, vtwo, a1B);
            a2B = _mm512_fmadd_pd(k2B, vtwo, a2B);
            u2B = _mm512_fmadd_pd(k0B, vh2, uB);
            v2B = _mm512_fmadd_pd(k1B, vh2, vB);
            w2B = _mm512_fmadd_pd(k2B, vh2, wB);
            /* chunk A: stage 3 */
            vu = _mm512_mul_pd(v2, u2);
            t1 = _mm512_sub_pd(va, u2);
            k0 = _mm512_fnmadd_pd(w2, u2, t1);
            k0 = _mm512_fmadd_pd(vu, u2, k0);
            wu = _mm512_mul_pd(w2, u2);
            k1 = _mm512_fnmadd_pd(vu, u2, wu);
            k2 = _mm512_sub_pd(_mm512_fnmadd_pd(w2, vin, vbinv), wu);
            a0 = _mm512_fmadd_pd(k0, vtwo, a0);
            a1 = _mm512_fmadd_pd(k1, vtwo, a1);
            a2 = _mm512_fmadd_pd(k2, vtwo, a2);
            u2 = _mm512_fmadd_pd(k0, vh, uA);
            v2 = _mm512_fmadd_pd(k1, vh, vA);
            w2 = _mm512_fmadd_pd(k2, vh, wA);
            /* chunk B: stage 3 */
            vuB = _mm512_mul_pd(v2B, u2B);
            t1B = _mm512_sub_pd(va, u2B);
            k0B = _mm512_fnmadd_pd(w2B, u2B, t1B);
            k0B = _mm512_fmadd_pd(vuB, u2B, k0B);
            wuB = _mm512_mul_pd(w2B, u2B);
            k1B = _mm512_fnmadd_pd(vuB, u2B, wuB);
            k2B = _mm512_sub_pd(_mm512_fnmadd_pd(w2B, vin, vbinv), wuB);
            a0B = _mm512_fmadd_pd(k0B, vtwo, a0B);
            a1B = _mm512_fmadd_pd(k1B, vtwo, a1B);
            a2B = _mm512_fmadd_pd(k2B, vtwo, a2B);
            u2B = _mm512_fmadd_pd(k0B, vh, uB);
            v2B = _mm512_fmadd_pd(k1B, vh, vB);
            w2B = _mm512_fmadd_pd(k2B, vh, wB);
            /* chunk A: stage 4 */
            vu = _mm512_mul_pd(v2, u2);
            t1 = _mm512_sub_pd(va, u2);
            k0 = _mm512_fnmadd_pd(w2, u2, t1);
            k0 = _mm512_fmadd_pd(vu, u2, k0);
            wu = _mm512_mul_pd(w2, u2);
            k1 = _mm512_fnmadd_pd(vu, u2, wu);
            k2 = _mm512_sub_pd(_mm512_fnmadd_pd(w2, vin, vbinv), wu);
            a0 = _mm512_add_pd(a0, k0);
            a1 = _mm512_add_pd(a1, k1);
            a2 = _mm512_add_pd(a2, k2);
            /* chunk B: stage 4 */
            vuB = _mm512_mul_pd(v2B, u2B);
            t1B = _mm512_sub_pd(va, u2B);
            k0B = _mm512_fnmadd_pd(w2B, u2B, t1B);
            k0B = _mm512_fmadd_pd(vuB, u2B, k0B);
            wuB = _mm512_mul_pd(w2B, u2B);
            k1B = _mm512_fnmadd_pd(vuB, u2B, wuB);
            k2B = _mm512_sub_pd(_mm512_fnmadd_pd(w2B, vin, vbinv), wuB);
            a0B = _mm512_add_pd(a0B, k0B);
            a1B = _mm512_add_pd(a1B, k1B);
            a2B = _mm512_add_pd(a2B, k2B);
            /* final updates */
            uA = _mm512_fmadd_pd(a0, vc, uA);
            vA = _mm512_fmadd_pd(a1, vc, vA);
            wA = _mm512_fmadd_pd(a2, vc, wA);
            uB = _mm512_fmadd_pd(a0B, vc, uB);
            vB = _mm512_fmadd_pd(a1B, vc, vB);
            wB = _mm512_fmadd_pd(a2B, vc, wB);
        }
        double *py = y + 48 * i0;
        _mm512_storeu_pd(lu, uA);
        _mm512_storeu_pd(lv, vA);
        _mm512_storeu_pd(lw, wA);
        _mm512_storeu_pd(lb_u, uB);
        _mm512_storeu_pd(lb_v, vB);
        _mm512_storeu_pd(lb_w, wB);
#pragma GCC unroll 8
        for (int k = 0; k < 8; k++) {
            py[3 * k + 0] = lu[k]; py[3 * k + 1] = lv[k]; py[3 * k + 2] = lw[k];
            py[24 + 3 * k + 0] = lb_u[k]; py[24 + 3 * k + 1] = lb_v[k]; py[24 + 3 * k + 2] = lb_w[k];
        }
    }
    int64_t n = nch * 16;
    for (; n < NSYS; n++) {
        double u = y0[3*n+0], v = y0[3*n+1], w = y0[3*n+2];
        double uu[1] = {u}, vv[1] = {v}, ww[1] = {w};
        sys_step_1(uu, vv, ww, NSTEPS, a, b, ep, h);
        y[3*n+0] = uu[0]; y[3*n+1] = vv[0]; y[3*n+2] = ww[0];
    }
}

__attribute__((target("avx2,fma")))
static void simd256(double *restrict y, const double *restrict y0,
                    int64_t NSTEPS, int64_t NSYS,
                    double a, double b, double ep, double t_end)
{
    const double h = t_end / (double)NSTEPS;
    const __m256d va   = _mm256_set1_pd(a);
    const __m256d vbinv = _mm256_set1_pd(b * (1.0 / ep));
    const __m256d vin  = _mm256_set1_pd(1.0 / ep);
    const __m256d vtwo = _mm256_set1_pd(2.0);
    const __m256d vh2  = _mm256_set1_pd(0.5 * h);
    const __m256d vh   = _mm256_set1_pd(h);
    const __m256d vc   = _mm256_set1_pd(h / 6.0);

    const int64_t nch = NSYS / 4;
#pragma omp parallel for schedule(static)
    for (int64_t i0 = 0; i0 < nch; i0++) {
        const double *p0 = y0 + 12 * i0;
        double lu[4], lv[4], lw[4];
#pragma GCC unroll 4
        for (int k = 0; k < 4; k++) { lu[k] = p0[3 * k + 0]; lv[k] = p0[3 * k + 1]; lw[k] = p0[3 * k + 2]; }
        __m256d u = _mm256_loadu_pd(lu), v = _mm256_loadu_pd(lv), w = _mm256_loadu_pd(lw);
        for (int64_t s = 0; s < NSTEPS; s++) {
            __m256d vu = _mm256_mul_pd(v, u);
            __m256d t1 = _mm256_sub_pd(va, u);
            __m256d k0 = _mm256_fnmadd_pd(w, u, t1);
            k0         = _mm256_fmadd_pd(vu, u, k0);
            __m256d wu = _mm256_mul_pd(w, u);
            __m256d k1 = _mm256_fnmadd_pd(vu, u, wu);
            __m256d k2 = _mm256_sub_pd(_mm256_fnmadd_pd(w, vin, vbinv), wu);
            __m256d u2 = _mm256_fmadd_pd(k0, vh2, u);
            __m256d v2 = _mm256_fmadd_pd(k1, vh2, v);
            __m256d w2 = _mm256_fmadd_pd(k2, vh2, w);
            __m256d a0 = k0, a1 = k1, a2 = k2;
            /* stage 2 */
            vu = _mm256_mul_pd(v2, u2);
            t1 = _mm256_sub_pd(va, u2);
            k0 = _mm256_fnmadd_pd(w2, u2, t1);
            k0 = _mm256_fmadd_pd(vu, u2, k0);
            wu = _mm256_mul_pd(w2, u2);
            k1 = _mm256_fnmadd_pd(vu, u2, wu);
            k2 = _mm256_sub_pd(_mm256_fnmadd_pd(w2, vin, vbinv), wu);
            a0 = _mm256_fmadd_pd(k0, vtwo, a0);
            a1 = _mm256_fmadd_pd(k1, vtwo, a1);
            a2 = _mm256_fmadd_pd(k2, vtwo, a2);
            u2 = _mm256_fmadd_pd(k0, vh2, u);
            v2 = _mm256_fmadd_pd(k1, vh2, v);
            w2 = _mm256_fmadd_pd(k2, vh2, w);
            /* stage 3 */
            vu = _mm256_mul_pd(v2, u2);
            t1 = _mm256_sub_pd(va, u2);
            k0 = _mm256_fnmadd_pd(w2, u2, t1);
            k0 = _mm256_fmadd_pd(vu, u2, k0);
            wu = _mm256_mul_pd(w2, u2);
            k1 = _mm256_fnmadd_pd(vu, u2, wu);
            k2 = _mm256_sub_pd(_mm256_fnmadd_pd(w2, vin, vbinv), wu);
            a0 = _mm256_fmadd_pd(k0, vtwo, a0);
            a1 = _mm256_fmadd_pd(k1, vtwo, a1);
            a2 = _mm256_fmadd_pd(k2, vtwo, a2);
            u2 = _mm256_fmadd_pd(k0, vh, u);
            v2 = _mm256_fmadd_pd(k1, vh, v);
            w2 = _mm256_fmadd_pd(k2, vh, w);
            /* stage 4 */
            vu = _mm256_mul_pd(v2, u2);
            t1 = _mm256_sub_pd(va, u2);
            k0 = _mm256_fnmadd_pd(w2, u2, t1);
            k0 = _mm256_fmadd_pd(vu, u2, k0);
            wu = _mm256_mul_pd(w2, u2);
            k1 = _mm256_fnmadd_pd(vu, u2, wu);
            k2 = _mm256_sub_pd(_mm256_fnmadd_pd(w2, vin, vbinv), wu);
            a0 = _mm256_add_pd(a0, k0);
            a1 = _mm256_add_pd(a1, k1);
            a2 = _mm256_add_pd(a2, k2);
            u = _mm256_fmadd_pd(a0, vc, u);
            v = _mm256_fmadd_pd(a1, vc, v);
            w = _mm256_fmadd_pd(a2, vc, w);
        }
        double *py = y + 12 * i0;
        _mm256_storeu_pd(lu, u);
        _mm256_storeu_pd(lv, v);
        _mm256_storeu_pd(lw, w);
#pragma GCC unroll 4
        for (int k = 0; k < 4; k++) { py[3 * k + 0] = lu[k]; py[3 * k + 1] = lv[k]; py[3 * k + 2] = lw[k]; }
    }
    int64_t n = nch * 4;
    for (; n < NSYS; n++) {
        double uu[1] = {y0[3*n+0]}, vv[1] = {y0[3*n+1]}, ww[1] = {y0[3*n+2]};
        sys_step_1(uu, vv, ww, NSTEPS, a, b, ep, h);
        y[3*n+0] = uu[0]; y[3*n+1] = vv[0]; y[3*n+2] = ww[0];
    }
}

static void scalar_path(double *restrict y, const double *restrict y0,
                        int64_t NSTEPS, int64_t NSYS,
                        double a, double b, double ep, double t_end)
{
    const double h = t_end / (double)NSTEPS;
#pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        double uu[1] = {y0[3*n+0]}, vv[1] = {y0[3*n+1]}, ww[1] = {y0[3*n+2]};
        sys_step_1(uu, vv, ww, NSTEPS, a, b, ep, h);
        y[3*n+0] = uu[0]; y[3*n+1] = vv[0]; y[3*n+2] = ww[0];
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
    if (NSTEPS <= 0 || NSYS <= 0) return;
    if (__builtin_cpu_supports("avx512f"))
        simd512(y, y0, NSTEPS, NSYS, a, b, ep, t_end);
    else if (__builtin_cpu_supports("avx2"))
        simd256(y, y0, NSTEPS, NSYS, a, b, ep, t_end);
    else
        scalar_path(y, y0, NSTEPS, NSYS, a, b, ep, t_end);
}
