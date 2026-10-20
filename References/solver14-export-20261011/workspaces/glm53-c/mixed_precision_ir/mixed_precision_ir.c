/* Mixed-precision iterative refinement: fp32 LU factors, fp64 residual.
 * Mirrors the NumPy reference (right-looking partial-pivoted LU, blocked
 * with sgemm trailing updates; refinement residual formed in fp64).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef MPIR_MAX_STEPS
#define MPIR_MAX_STEPS 20
#endif
#ifndef MPIR_TOL
#define MPIR_TOL 1e-13
#endif

#define MPIR_ROWMAJOR 101
#define MPIR_NOTRANS  111

extern void cblas_sgemm(const int layout, const int transa, const int transb,
                        const int m, const int n, const int k,
                        const float alpha, const float *a, const int lda,
                        const float *b, const int ldb,
                        const float beta, float *c, const int ldc);
extern void cblas_strsm(const int layout, const int side, const int uplo,
                        const int transa, const int diag,
                        const int m, const int n, const float alpha,
                        const float *a, const int lda,
                        float *b, const int ldb);

#define MPIR_LEFT   141
#define MPIR_LOWER  122
#define MPIR_UNIT   132

static int mpir_imin(int a, int b) { return a < b ? a : b; }

/* ---- factorization: fp32, row-major, right-looking blocked LU ----
 * piv[i] ends holding the ORIGINAL row index now in row i (reference conv).
 */
static void mpir_lu_factor(int N, float *restrict const Alu,
                           int *restrict const piv, int nb, int nth)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) piv[i] = i;

    for (int k0 = 0; k0 < N; k0 += nb) {
        const int k1 = mpir_imin(k0 + nb, N);

        for (int k = k0; k < k1; ++k) {
            /* pivot: FIRST index attaining max |Alu[i][k]| for i in [k,N) */
            float bestv = -1.0f;
            int besti = N;
            #pragma omp parallel default(none) shared(bestv,besti,Alu,N,k)
            {
                float lv = -1.0f;
                int li = -1;
                #pragma omp for schedule(static) nowait
                for (int i = k; i < N; ++i) {
                    const float v = fabsf(Alu[(size_t)i * N + k]);
                    if (v > lv) { lv = v; li = i; }
                }
                #pragma omp critical
                {
                    if (lv > bestv || (lv == bestv && li >= 0 && li < besti)) {
                        bestv = lv; besti = li;
                    }
                }
            }
            const int p = besti;
            if (p != k) {
                #pragma omp parallel for schedule(static) num_threads(nth) if(N > 256)
                for (int j = 0; j < N; ++j) {
                    const float t = Alu[(size_t)p * N + j];
                    Alu[(size_t)p * N + j] = Alu[(size_t)k * N + j];
                    Alu[(size_t)k * N + j] = t;
                }
                const int tp = piv[p];
                piv[p] = piv[k];
                piv[k] = tp;
            }

            const float pval = Alu[(size_t)k * N + k];

            /* scale column below pivot: true division, like the reference */
            #pragma omp parallel for schedule(static) num_threads(nth) if(N-k > 256)
            for (int i = k + 1; i < N; ++i)
                Alu[(size_t)i * N + k] /= pval;

            /* panel rank-1 update: cols (k+1, k1) only */
            #pragma omp parallel for schedule(static) num_threads(nth) if(k1-k > 32)
            for (int i = k + 1; i < N; ++i) {
                float *restrict const ai = Alu + (size_t)i * N;
                const float *restrict const ak = Alu + (size_t)k * N;
                const float lik = ai[k];
                #pragma omp simd
                for (int j = k + 1; j < k1; ++j)
                    ai[j] -= lik * ak[j];
            }
        }

        /* block row of U: U12 := L11^-1 U12 (unit lower L11) */
        const int nc = N - k1;
        if (nc > 0)
            cblas_strsm(MPIR_ROWMAJOR, MPIR_LEFT, MPIR_LOWER, MPIR_NOTRANS,
                         MPIR_UNIT, k1 - k0, nc, 1.0f,
                         Alu + (size_t)k0 * N + k0, N,
                         Alu + (size_t)k0 * N + k1, N);

        /* trailing submatrix update: A22 -= L21 * U12 */
        const int m = N - k1;
        const int n = N - k1;
        if (m > 0 && n > 0)
            cblas_sgemm(MPIR_ROWMAJOR, MPIR_NOTRANS, MPIR_NOTRANS,
                        m, n, k1 - k0,
                        -1.0f, Alu + (size_t)k1 * N + k0, N,
                                Alu + (size_t)k0 * N + k1, N,
                        1.0f,  Alu + (size_t)k1 * N + k1, N);
    }
}

/* ---- triangular solves (fp32), blocked, row-major ---- */
static void mpir_lu_solve(int N, const float *restrict const Alu,
                          const int *restrict const piv,
                          const float *restrict const rhs,
                          float *restrict const sol,
                          float *restrict const y, int nth)
{
    const int bs = 64;

    for (int i = 0; i < N; ++i) y[i] = rhs[piv[i]];

    /* forward: y = L^-1 y (unit diagonal) */
    for (int js = 0; js < N; js += bs) {
        const int je = mpir_imin(js + bs, N);

        for (int i = js; i < je; ++i) {
            const float *restrict const ai = Alu + (size_t)i * N;
            float s = y[i];
            #pragma omp simd
            for (int j = js; j < i; ++j) s -= ai[j] * y[j];
            y[i] = s;
        }

        #pragma omp parallel for schedule(static) num_threads(nth) if(N-je > 512)
        for (int i = je; i < N; ++i) {
            const float *restrict const ai = Alu + (size_t)i * N;
            float s = y[i];
            #pragma omp simd
            for (int j = js; j < je; ++j) s -= ai[j] * y[j];
            y[i] = s;
        }
    }

    /* back: sol = U^-1 y */
    for (int je = N; je > 0; je -= bs) {
        const int js = (je - bs > 0) ? je - bs : 0;

        for (int i = je - 1; i >= js; --i) {
            const float *restrict const ai = Alu + (size_t)i * N;
            float s = y[i];
            #pragma omp simd
            for (int j = i + 1; j < je; ++j) s -= ai[j] * sol[j];
            sol[i] = s / ai[i];
        }

        #pragma omp parallel for schedule(static) num_threads(nth) if(js > 512)
        for (int i = 0; i < js; ++i) {
            const float *restrict const ai = Alu + (size_t)i * N;
            float s = y[i];
            #pragma omp simd
            for (int j = js; j < je; ++j) s -= ai[j] * sol[j];
            y[i] = s;
        }
    }
}

/* ---- residual r = b - A * x and dot products, all fp64 ---- */
static void mpir_residual(int N, const double *restrict const A,
                           const double *restrict const b,
                           const double *restrict const x,
                           double *restrict const r, int nth)
{
    #pragma omp parallel for schedule(static) num_threads(nth) if(N > 64)
    for (int i = 0; i < N; ++i) {
        const double *restrict const arow = A + (size_t)i * N;
        double s = b[i];
        #pragma omp simd reduction(+:s)
        for (int j = 0; j < N; ++j) s -= arow[j] * x[j];
        r[i] = s;
    }
}

static double mpir_sumsq64(const double *restrict const v, int N, int nth)
{
    double total = 0.0;
    #pragma omp parallel for schedule(static) reduction(+:total) num_threads(nth) if(N > 512)
    for (int i = 0; i < N; ++i) total += v[i] * v[i];
    return total;
}

/* ---- scratch carving ---- */
static inline uint8_t *mpir_bump(uint8_t **pc, size_t bytes)
{
    uintptr_t a = (uintptr_t)*pc;
    a = (a + 63u) & ~(uintptr_t)63u;
    *pc = (uint8_t *)(a + bytes);
    return (uint8_t *)a;
}

void mixed_precision_ir_fp64(
    const double *restrict A,
    const double *restrict b,
    int64_t *restrict steps_out,
    double *restrict x,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    if (N <= 0) { steps_out[0] = 0; return; }

    const int n = (int)N;
    const int nth = (int)omp_get_max_threads();

    const size_t need =
        (size_t)4 * (size_t)n * (size_t)n +   /* Alu fp32 */
        64 + 4 * (size_t)n +                  /* piv */
        64 + 4 * (size_t)n +                  /* y */
        64 + 4 * (size_t)n +                  /* rhs32 */
        64 + 4 * (size_t)n +                  /* sol32 */
        64 + 8 * (size_t)n +                  /* r */
        256;

    uint8_t *heap = NULL;
    uint8_t *base = workspace;
    if (!base || (size_t)workspace_size < need) {
        heap = (uint8_t *)malloc(need);
        if (!heap) { steps_out[0] = 0; return; }
        base = heap;
    }

    uint8_t *c = base;
    float *const Alu  = (float *)mpir_bump(&c, (size_t)4 * n * n);
    int   *const piv  = (int   *)mpir_bump(&c, (size_t)4 * n);
    float *const y    = (float *)mpir_bump(&c, (size_t)4 * n);
    float *const rhs32 = (float *)mpir_bump(&c, (size_t)4 * n);
    float *const sol32 = (float *)mpir_bump(&c, (size_t)4 * n);
    double *const r    = (double *)mpir_bump(&c, (size_t)8 * n);

    /* 1. narrow A (row-major) into Alu */
    {
        const size_t total = (size_t)n * n;
        #pragma omp parallel for schedule(static) num_threads(nth) if(total > 4096)
        for (size_t idx = 0; idx < total; ++idx)
            Alu[idx] = (float)A[idx];
    }

    /* 2. factor once, in fp32 */
    mpir_lu_factor(n, Alu, piv, 128, nth);

    /* 3. initial solve */
    #pragma omp parallel for schedule(static) num_threads(nth) if(n > 64)
    for (int i = 0; i < n; ++i) rhs32[i] = (float)b[i];

    mpir_lu_solve(n, Alu, piv, rhs32, sol32, y, nth);

    #pragma omp parallel for schedule(static) num_threads(nth) if(n > 64)
    for (int i = 0; i < n; ++i) x[i] = (double)sol32[i];

    /* 4. refinement */
    const double bnorm = sqrt(mpir_sumsq64(b, n, nth));
    int64_t count = 0;

    for (int step = 0; step < MPIR_MAX_STEPS; ++step) {
        mpir_residual(n, A, b, x, r, nth);
        const double rnorm = sqrt(mpir_sumsq64(r, n, nth));
        if (rnorm < MPIR_TOL * bnorm) break;
        #pragma omp parallel for schedule(static) num_threads(nth) if(n > 64)
        for (int i = 0; i < n; ++i) rhs32[i] = (float)r[i];
        mpir_lu_solve(n, Alu, piv, rhs32, sol32, y, nth);
        #pragma omp parallel for schedule(static) num_threads(nth) if(n > 64)
        for (int i = 0; i < n; ++i) x[i] += (double)sol32[i];
        ++count;
    }

    steps_out[0] = count;
    free(heap);
}
