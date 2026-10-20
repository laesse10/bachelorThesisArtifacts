/* rk4_ensemble_fp64 -- classical fixed-step RK4 over NSYS independent
 * 3-species Brusselator systems (ARKODE test problem).
 *
 *   du/dt = a - (w+1)u + v u^2
 *   dv/dt = w u     - v u^2
 *   dw/dt = (b-w)/ep - w u
 *
 * The systems never interact, so the batch is a pure map: each thread owns a
 * slice of systems, transposes the slice from AoS to SoA, runs all NSTEPS steps
 * on a tile of systems whose state lives in zmm registers, and stores back.
 * The step loop is the innermost loop -> zero memory traffic inside it.
 *
 * The RHS costs 7 vector ops instead of 10 because
 *   k_u + k_v = a - u                      (exact identity of this RHS)
 *   k_w     = (b-w)/ep - w*u  =  (b-w)*iep - w*u      (iep = 1/ep)
 * and the RK4 combination is formed as (k1+k4) + 2*(k2+k3).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef TV
#define TV 5                 /* zmm vectors per tile (8*TV systems per tile) */
#endif

#define LANE (8 * TV)        /* systems per tile */
#define BLKMAX (8 * TV * 16) /* largest block, also the stack scratch size */

#ifdef __AVX512F__
#include <immintrin.h>

/* Run nsteps RK4 steps for the LANE systems of one tile.  Lanes outside mk are
 * loaded as zero, compute garbage and are never stored. */
static void rk4_tile(double *restrict up, double *restrict vp, double *restrict wp,
                     const int64_t nsteps, const __mmask8 mk,
                     const double a, const double b, const double iep,
                     const double h2, const double h, const double c6)
{
    const __m512d av = _mm512_set1_pd(a);
    const __m512d bv = _mm512_set1_pd(b);
    const __m512d iepv = _mm512_set1_pd(iep);
    const __m512d h2v = _mm512_set1_pd(h2);
    const __m512d hv = _mm512_set1_pd(h);
    const __m512d c6v = _mm512_set1_pd(c6);
    const __m512d two = _mm512_set1_pd(2.0);

    __m512d U[TV], V[TV], W[TV];
    for (int j = 0; j < TV; j++) {
        U[j] = _mm512_maskz_loadu_pd(mk, up + 8 * (size_t)j);
        V[j] = _mm512_maskz_loadu_pd(mk, vp + 8 * (size_t)j);
        W[j] = _mm512_maskz_loadu_pd(mk, wp + 8 * (size_t)j);
    }

    for (int64_t s = 0; s < nsteps; s++) {
        __m512d su[TV], sv[TV], sw[TV];
        __m512d tu[TV], tv[TV], tw[TV];
        __m512d cu[TV], cv[TV], cw[TV];

        /* ---- stage 1 ---- */
        for (int j = 0; j < TV; j++) {
            const __m512d u = U[j], v = V[j], w = W[j];
            const __m512d au = _mm512_sub_pd(av, u);
            const __m512d p1 = _mm512_mul_pd(u, u);
            const __m512d wu = _mm512_mul_pd(w, u);
            const __m512d kv = _mm512_fnmadd_pd(v, p1, wu);
            const __m512d ku = _mm512_sub_pd(au, kv);
            const __m512d kw = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(bv, w), iepv), wu);
            su[j] = ku; sv[j] = kv; sw[j] = kw;
            cu[j] = _mm512_fmadd_pd(h2v, ku, u);
            cv[j] = _mm512_fmadd_pd(h2v, kv, v);
            cw[j] = _mm512_fmadd_pd(h2v, kw, w);
        }
        /* ---- stage 2 ---- */
        for (int j = 0; j < TV; j++) {
            const __m512d u = cu[j], v = cv[j], w = cw[j];
            const __m512d au = _mm512_sub_pd(av, u);
            const __m512d p1 = _mm512_mul_pd(u, u);
            const __m512d wu = _mm512_mul_pd(w, u);
            const __m512d kv = _mm512_fnmadd_pd(v, p1, wu);
            const __m512d ku = _mm512_sub_pd(au, kv);
            const __m512d kw = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(bv, w), iepv), wu);
            tu[j] = ku; tv[j] = kv; tw[j] = kw;
            cu[j] = _mm512_fmadd_pd(h2v, ku, U[j]);
            cv[j] = _mm512_fmadd_pd(h2v, kv, V[j]);
            cw[j] = _mm512_fmadd_pd(h2v, kw, W[j]);
        }
        /* ---- stage 3 ---- */
        for (int j = 0; j < TV; j++) {
            const __m512d u = cu[j], v = cv[j], w = cw[j];
            const __m512d au = _mm512_sub_pd(av, u);
            const __m512d p1 = _mm512_mul_pd(u, u);
            const __m512d wu = _mm512_mul_pd(w, u);
            const __m512d kv = _mm512_fnmadd_pd(v, p1, wu);
            const __m512d ku = _mm512_sub_pd(au, kv);
            const __m512d kw = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(bv, w), iepv), wu);
            tu[j] = _mm512_add_pd(tu[j], ku);
            tv[j] = _mm512_add_pd(tv[j], kv);
            tw[j] = _mm512_add_pd(tw[j], kw);
            cu[j] = _mm512_fmadd_pd(hv, ku, U[j]);
            cv[j] = _mm512_fmadd_pd(hv, kv, V[j]);
            cw[j] = _mm512_fmadd_pd(hv, kw, W[j]);
        }
        /* ---- stage 4 + combination ---- */
        for (int j = 0; j < TV; j++) {
            const __m512d u = cu[j], v = cv[j], w = cw[j];
            const __m512d au = _mm512_sub_pd(av, u);
            const __m512d p1 = _mm512_mul_pd(u, u);
            const __m512d wu = _mm512_mul_pd(w, u);
            const __m512d kv = _mm512_fnmadd_pd(v, p1, wu);
            const __m512d ku = _mm512_sub_pd(au, kv);
            const __m512d kw = _mm512_sub_pd(_mm512_mul_pd(_mm512_sub_pd(bv, w), iepv), wu);
            U[j] = _mm512_fmadd_pd(c6v,
                        _mm512_fmadd_pd(two, tu[j], _mm512_add_pd(su[j], ku)), U[j]);
            V[j] = _mm512_fmadd_pd(c6v,
                        _mm512_fmadd_pd(two, tv[j], _mm512_add_pd(sv[j], kv)), V[j]);
            W[j] = _mm512_fmadd_pd(c6v,
                        _mm512_fmadd_pd(two, tw[j], _mm512_add_pd(sw[j], kw)), W[j]);
        }
    }

    for (int j = 0; j < TV; j++) {
        _mm512_mask_storeu_pd(up + 8 * (size_t)j, mk, U[j]);
        _mm512_mask_storeu_pd(vp + 8 * (size_t)j, mk, V[j]);
        _mm512_mask_storeu_pd(wp + 8 * (size_t)j, mk, W[j]);
    }
}
#endif /* __AVX512F__ */

/* portable sweep: one RK4 step over cnt systems held in SoA scratch arrays.
 * Uses the reference's (b-w)/ep form. */
static inline void rk4_sweep(double *restrict U, double *restrict V, double *restrict W,
                             const int64_t cnt, const double a, const double b,
                             const double ep, const double h2, const double h,
                             const double c6)
{
    for (int64_t i = 0; i < cnt; i++) {
        const double uu = U[i], vv = V[i], ww = W[i];
        const double k1u = a - (ww + 1.0) * uu + vv * uu * uu;
        const double k1v = ww * uu - vv * uu * uu;
        const double k1w = (b - ww) / ep - ww * uu;
        const double u2 = uu + h2 * k1u;
        const double v2 = vv + h2 * k1v;
        const double w2 = ww + h2 * k1w;
        const double k2u = a - (w2 + 1.0) * u2 + v2 * u2 * u2;
        const double k2v = w2 * u2 - v2 * u2 * u2;
        const double k2w = (b - w2) / ep - w2 * u2;
        const double u3 = uu + h2 * k2u;
        const double v3 = vv + h2 * k2v;
        const double w3 = ww + h2 * k2w;
        const double k3u = a - (w3 + 1.0) * u3 + v3 * u3 * u3;
        const double k3v = w3 * u3 - v3 * u3 * u3;
        const double k3w = (b - w3) / ep - w3 * u3;
        const double u4 = uu + h * k3u;
        const double v4 = vv + h * k3v;
        const double w4 = ww + h * k3w;
        const double k4u = a - (w4 + 1.0) * u4 + v4 * u4 * u4;
        const double k4v = w4 * u4 - v4 * u4 * u4;
        const double k4w = (b - w4) / ep - w4 * u4;
        U[i] = uu + c6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
        V[i] = vv + c6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
        W[i] = ww + c6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
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
    if (NSYS <= 0) return;
    if (NSTEPS <= 0) {
        memcpy(y, y0, (size_t)NSYS * 3 * sizeof(double));
        return;
    }
    const double h = t_end / (double)NSTEPS;
    const double h2 = 0.5 * h;
    const double c6 = h / 6.0;
    const double iep = 1.0 / ep;

    /* pick a block size that keeps every thread busy: aim for well over 32
     * blocks per thread so the static schedule stays balanced, and stay a
     * multiple of the tile width so no partial tile is ever needed. */
    int nthr = omp_get_max_threads();
    if (nthr < 1) nthr = 1;
    int64_t want = (NSYS + (int64_t)64 * nthr - 1) / ((int64_t)64 * nthr);
    if (want < LANE) want = LANE;
    if (want > BLKMAX) want = BLKMAX;
    const int64_t blk = ((want + LANE - 1) / LANE) * LANE;
    const int64_t nblk = (NSYS + blk - 1) / blk;

#ifdef __AVX512F__
    const int fast_path = isfinite(iep);
#else
    const int fast_path = 0;
#endif

#pragma omp parallel for schedule(static)
    for (int64_t ib = 0; ib < nblk; ib++) {
        const int64_t lo = ib * blk;
        int64_t cnt = blk;
        if (lo + cnt > NSYS) cnt = NSYS - lo;
        double U[BLKMAX] __attribute__((aligned(64)));
        double V[BLKMAX] __attribute__((aligned(64)));
        double W[BLKMAX] __attribute__((aligned(64)));
        const double *restrict src = y0 + 3 * lo;
        double *restrict dst = y + 3 * lo;
        for (int64_t i = 0; i < cnt; i++) {
            U[i] = src[3 * i];
            V[i] = src[3 * i + 1];
            W[i] = src[3 * i + 2];
        }
#ifdef __AVX512F__
        if (fast_path) {
            const int64_t nt = (cnt + LANE - 1) / LANE;
            for (int64_t t = 0; t < nt; t++) {
                const int64_t r = cnt - t * LANE;
                const __mmask8 mk = (r >= 8) ? (__mmask8)0xFF
                                              : (__mmask8)((1u << r) - 1u);
                rk4_tile(U + LANE * t, V + LANE * t, W + LANE * t,
                         NSTEPS, mk, a, b, iep, h2, h, c6);
            }
        } else
#endif
        {
            for (int64_t s = 0; s < NSTEPS; s++)
                rk4_sweep(U, V, W, cnt, a, b, ep, h2, h, c6);
        }
        for (int64_t i = 0; i < cnt; i++) {
            dst[3 * i] = U[i];
            dst[3 * i + 1] = V[i];
            dst[3 * i + 2] = W[i];
        }
    }
}
