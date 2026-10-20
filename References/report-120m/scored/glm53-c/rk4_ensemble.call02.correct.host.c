/* rk4_ensemble: fixed-step RK4 over NSYS independent 3-species Brusselator ODEs.
 *
 * The outer loop over systems is a pure map: every system runs its own
 * NSTEPS-step RK4 trajectory.  We process a block of systems per vector
 * register set -- each AVX-512/AVX2 lane carries one system -- so the whole
 * NSTEPS-long dependency chain of a block lives in registers and y0/y are
 * touched exactly once per system.  Threads partition the system range.
 *
 * The arithmetic per lane mirrors the NumPy reference exactly (same expression
 * tree, same association order), so every lane reproduces the reference
 * trajectory bit for bit.  Division by ep uses the FMA-corrected reciprocal
 * (q = x*r; e = fma(-ep,q,x); q = fma(e,r,q)), which equals the correctly
 * rounded quotient x/ep whenever the residual e does not underflow -- a regime
 * |x| < 2^-900 that the trajectory values (b - w) cannot reach.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#if defined(__AVX512F__)
#include <immintrin.h>
#define VEC 8
typedef double vdf __attribute__((vector_size(64)));
static inline vdf vfma(vdf a, vdf b, vdf c) {
    return (vdf)_mm512_fmadd_pd((__m512d)a, (__m512d)b, (__m512d)c);
}
static inline vdf vfnma(vdf a, vdf b, vdf c) {
    return (vdf)_mm512_fnmadd_pd((__m512d)a, (__m512d)b, (__m512d)c);
}
#elif defined(__AVX2__)
#include <immintrin.h>
#define VEC 4
typedef double vdf __attribute__((vector_size(32)));
static inline vdf vfma(vdf a, vdf b, vdf c) {
    return (vdf)_mm256_fmadd_pd((__m256d)a, (__m256d)b, (__m256d)c);
}
static inline vdf vfnma(vdf a, vdf b, vdf c) {
    return (vdf)_mm256_fnmadd_pd((__m256d)a, (__m256d)b, (__m256d)c);
}
#else
#define VEC 2
typedef double vdf __attribute__((vector_size(16)));
static inline vdf vfma(vdf a, vdf b, vdf c) { return a * b + c; }
static inline vdf vfnma(vdf a, vdf b, vdf c) { return -(a * b) + c; }
#endif

#define NV 4            /* vectors per block */
#define BLK (VEC * NV)  /* systems per block */

/* x / ep for a fixed ep, exactly (see file comment) */
static inline vdf div_ep(vdf x, vdf vep, vdf vinv) {
    vdf q = x * vinv;
    vdf e = vfnma(vep, q, x);   /* x - ep*q, one rounding */
    return vfma(e, vinv, q);    /* q + e*(1/ep), one rounding */
}

/* Run nsteps RK4 steps on NV vector lanes (NV*VEC systems). */
static inline void rk4_block(vdf U[NV], vdf V[NV], vdf W[NV],
                             const vdf va, const vdf vb, const vdf vep,
                             const vdf vinv,
                             const vdf vh, const vdf vhh, const vdf vc,
                             const int64_t nsteps) {
    for (int64_t s = 0; s < nsteps; s++) {
        for (int j = 0; j < NV; j++) {
            vdf u = U[j], v = V[j], w = W[j];

            /* k1 = f(u, v, w) */
            vdf k1u = va - (w + 1.0) * u + v * u * u;
            vdf k1v = w * u - v * u * u;
            vdf k1w = div_ep(vb - w, vep, vinv) - w * u;

            vdf u2 = u + vhh * k1u;
            vdf v2 = v + vhh * k1v;
            vdf w2 = w + vhh * k1w;

            vdf k2u = va - (w2 + 1.0) * u2 + v2 * u2 * u2;
            vdf k2v = w2 * u2 - v2 * u2 * u2;
            vdf k2w = div_ep(vb - w2, vep, vinv) - w2 * u2;

            vdf u3 = u + vhh * k2u;
            vdf v3 = v + vhh * k2v;
            vdf w3 = w + vhh * k2w;

            vdf k3u = va - (w3 + 1.0) * u3 + v3 * u3 * u3;
            vdf k3v = w3 * u3 - v3 * u3 * u3;
            vdf k3w = div_ep(vb - w3, vep, vinv) - w3 * u3;

            vdf u4 = u + vh * k3u;
            vdf v4 = v + vh * k3v;
            vdf w4 = w + vh * k3w;

            vdf k4u = va - (w4 + 1.0) * u4 + v4 * u4 * u4;
            vdf k4v = w4 * u4 - v4 * u4 * u4;
            vdf k4w = div_ep(vb - w4, vep, vinv) - w4 * u4;

            U[j] = u + vc * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
            V[j] = v + vc * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
            W[j] = w + vc * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
        }
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
    const int64_t workspace_size) {
    (void)workspace;
    (void)workspace_size;

    if (NSYS <= 0) return;
    if (NSTEPS <= 0) {  /* reference takes zero steps: y = y0 */
        memcpy(y, y0, (size_t)NSYS * 3 * sizeof(double));
        return;
    }
    const double h = t_end / (double)NSTEPS;

    const vdf va  = a - (vdf){0};      /* broadcast scalars */
    const vdf vb  = b - (vdf){0};
    const vdf vep = ep - (vdf){0};
    const vdf vinv = (1.0 / ep) - (vdf){0};
    
    const vdf vh  = h - (vdf){0};
    const vdf vhh = (0.5 * h) - (vdf){0};
    const vdf vc  = (h / 6.0) - (vdf){0};

    #pragma omp parallel
    {
        const int64_t nth = omp_get_num_threads();
        const int64_t tid = omp_get_thread_num();
        const int64_t i0 = NSYS * tid / nth;
        const int64_t i1 = NSYS * (tid + 1) / nth;

        vdf U[NV], V[NV], W[NV];
        int64_t i = i0;
        for (; i + BLK <= i1; i += BLK) {
            const double *p = y0 + 3 * (size_t)i;
            for (int j = 0; j < NV; j++)
                for (int l = 0; l < VEC; l++) {
                    const double *q = p + 3 * (size_t)(j * VEC + l);
                    U[j][l] = q[0];
                    V[j][l] = q[1];
                    W[j][l] = q[2];
                }
            rk4_block(U, V, W, va, vb, vep, vinv, vh, vhh, vc, NSTEPS);
            double *o = y + 3 * (size_t)i;
            for (int j = 0; j < NV; j++)
                for (int l = 0; l < VEC; l++) {
                    double *q = o + 3 * (size_t)(j * VEC + l);
                    q[0] = U[j][l];
                    q[1] = V[j][l];
                    q[2] = W[j][l];
                }
        }
        /* tail: one padded partial block (pad lanes with zeros; their
         * trajectories are finite and never stored) */
        if (i < i1) {
            const int64_t cnt = i1 - i;
            const double *p = y0 + 3 * (size_t)i;
            for (int j = 0; j < NV; j++)
                for (int l = 0; l < VEC; l++) {
                    if (j * VEC + l < cnt) {
                        const double *q = p + 3 * (size_t)(j * VEC + l);
                        U[j][l] = q[0];
                        V[j][l] = q[1];
                        W[j][l] = q[2];
                    } else {
                        U[j][l] = 0.0;
                        V[j][l] = 0.0;
                        W[j][l] = 0.0;
                    }
                }
            rk4_block(U, V, W, va, vb, vep, vinv, vh, vhh, vc, NSTEPS);
            double *o = y + 3 * (size_t)i;
            for (int j = 0; j < NV; j++)
                for (int l = 0; l < VEC; l++) {
                    if (j * VEC + l < cnt) {
                        double *q = o + 3 * (size_t)(j * VEC + l);
                        q[0] = U[j][l];
                        q[1] = V[j][l];
                        q[2] = W[j][l];
                    }
                }
        }
    }
}
