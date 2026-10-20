#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>
#include <immintrin.h>

#ifndef MS
#define MS 20
#endif
#ifndef TOL
#define TOL 1e-13
#endif

/* c - l*u with exactly two IEEE fp32 roundings (never FMA-fused), matching NumPy's
   outer-product-then-inplace-subtract semantics. */
static inline float nf_mul_sub(float c, float l, float u) {
    __m128 v = _mm_sub_ss(_mm_set1_ps(c), _mm_mul_ss(_mm_set1_ps(l), _mm_set1_ps(u)));
    return _mm_cvtss_f32(v);
}
/* Product then subtract as two separate IEEE roundings. The asm volatile barrier
   stops GCC's -ffp-contract=fast from fusing mul+sub into a single FMA, which would
   round differently than NumPy's outer-then-subtract and flip partial-pivot choices. */
static void rank1_update_vec(float *C, const float *U, float l, int64_t n) {
#if defined(__AVX512F__)
    for (int64_t j = 0; j < n; j += 16) {
        int64_t cnt = n - j;
        __mmask16 m = (cnt >= 16) ? 0xFFFFu : (__mmask16)((1u << cnt) - 1u);
        __m512 c = _mm512_maskz_loadu_ps(m, C + j);
        __m512 u = _mm512_maskz_loadu_ps(m, U + j);
        __m512 t = _mm512_mul_ps(u, _mm512_set1_ps(l));
        __asm__ __volatile__("" : "+v"(t) : : );
        c = _mm512_sub_ps(c, t);
        _mm512_mask_storeu_ps(C + j, m, c);
    }
#elif defined(__AVX2__)
    {
        int64_t j = 0;
        for (; j + 8 <= n; j += 8) {
            __m256 c = _mm256_loadu_ps(C + j);
            __m256 u = _mm256_loadu_ps(U + j);
            __m256 t = _mm256_mul_ps(u, _mm256_set1_ps(l));
            __asm__ __volatile__("" : "+v"(t) : : );
            c = _mm256_sub_ps(c, t);
            _mm256_storeu_ps(C + j, c);
        }
        for (; j < n; j++) C[j] = nf_mul_sub(C[j], l, U[j]);
    }
#else
    for (int64_t j = 0; j < n; j++) C[j] = nf_mul_sub(C[j], l, U[j]);
#endif
}

static void lu_solve_exact(const float *Alu, const int64_t *piv, const float *rhs,
                           float *y, float *sol32, int64_t N)
{
    int64_t i;
    for (i = 0; i < N; i++) y[i] = rhs[piv[i]];
    for (i = 0; i < N; i++) {
        float d = (i > 0) ? cblas_sdot((int)i, Alu + i*N, 1, y, 1) : 0.0f;
        y[i] = y[i] - d;
    }
    for (i = N - 1; i >= 0; i--) {
        float d = (i < N-1) ? cblas_sdot((int)(N-1-i), Alu + i*N + i + 1, 1, sol32 + i + 1, 1) : 0.0f;
        sol32[i] = (y[i] - d) / Alu[i*N + i];
    }
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
    const int64_t need = (4*N*N + 8*N + 12*N + 8*N + 128) & ~(int64_t)63;
    uint8_t *ws = workspace;
    bool own = false;
    if (ws == NULL || workspace_size < need) {
        ws = (uint8_t*)aligned_alloc(64, (size_t)need);
        if (ws == NULL) { ws = (uint8_t*)malloc(need); if (!ws) return; }
        own = true;
    }
    float *Alu = (float*)ws;
    int64_t *piv = (int64_t*)(ws + 4*N*N);
    float *y = (float*)(ws + 4*N*N + 8*N);
    float *rhs32 = y + N;
    float *sol32 = y + 2*N;
    double *r = (double*)(ws + 4*N*N + 8*N + 12*N);

    /* Alu = (float32) A, exact per-element conversion; piv = identity */
    {
        int64_t i;
        #pragma omp parallel for
        for (i = 0; i < N; i++) piv[i] = i;
    }
    {
        int64_t i;
        #pragma omp parallel for
        for (i = 0; i < N*N; i++) Alu[i] = (float)A[i];
    }

    /* ---- LU factorization, right-looking Doolittle, fp32, exact per-element semantics ---- */
    {
        int64_t k;
        for (k = 0; k < N; k++) {
            int64_t i;
            int64_t prow = k;
            float pmax = fabsf(Alu[k*N + k]);
            for (i = k + 1; i < N; i++) {
                float v = fabsf(Alu[i*N + k]);
                if (v > pmax) { pmax = v; prow = i; }
            }
            if (prow != k) {
                float *r1 = Alu + k*N;
                float *r2 = Alu + prow*N;
                for (i = 0; i < N; i++) { float t = r1[i]; r1[i] = r2[i]; r2[i] = t; }
                int64_t tp = piv[k]; piv[k] = piv[prow]; piv[prow] = tp;
            }
            float dk = Alu[k*N + k];
            for (i = k + 1; i < N; i++) Alu[i*N + k] /= dk;
            float *Urow = Alu + k*N + k + 1;
            #pragma omp parallel for schedule(static, 128)
            for (i = k + 1; i < N; i++) {
                float *C = Alu + i*N + k + 1;
                float l = Alu[i*N + k];
                rank1_update_vec(C, Urow, l, N - k - 1);
            }
        }
    }

    /* initial solve: rhs32 = (float32) b */
    {
        int64_t i;
        for (i = 0; i < N; i++) rhs32[i] = (float)b[i];
    }
    lu_solve_exact(Alu, piv, rhs32, y, sol32, N);
    {
        int64_t i;
        for (i = 0; i < N; i++) x[i] = (double)sol32[i];
    }

    double bnorm = sqrt((double)cblas_ddot((int)N, b, 1, b, 1));
    double thr = TOL * bnorm;
    int64_t count = 0;
    for (int step = 0; step < MS; step++) {
        cblas_dgemv(CblasRowMajor, CblasNoTrans, (int)N, (int)N, 1.0, A, (int)N, x, 1, 0.0, r, 1);
        {
            int64_t i;
            for (i = 0; i < N; i++) r[i] = b[i] - r[i];
        }
        double rnorm = sqrt((double)cblas_ddot((int)N, r, 1, r, 1));
        if (rnorm < thr) break;
        {
            int64_t i;
            for (i = 0; i < N; i++) rhs32[i] = (float)r[i];
        }
        lu_solve_exact(Alu, piv, rhs32, y, sol32, N);
        {
            int64_t i;
            for (i = 0; i < N; i++) x[i] += (double)sol32[i];
        }
        count++;
    }
    steps_out[0] = count;

    if (own) free(ws);
}
