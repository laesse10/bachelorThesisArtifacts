/*
 * Mixed-precision iterative refinement (dsgesv / HPL-MxP style), fp64 driver.
 *
 * Factor A ONCE in fp32 (right-looking Doolittle with partial pivoting, full
 * row swaps, elementwise IEEE arithmetic so the factors are bit-identical to
 * the NumPy/NumpyToX reference), then refine: residual r = b - A x in fp64,
 * correction = L U \ r in fp32, x += correction.  Stop when ||r|| < tol ||b||.
 *
 * OpenBLAS (the harness links it) is the bit source for everything the
 * reference gets from BLAS: 1-D dots (cblas_sdot / cblas_ddot) and the
 * residual mat-vec, which NumPy computes as a single row-major cblas_dgemv
 * call (order=RowMajor, trans=Trans, N x N) -- verified bit-equal to NumPy's
 * A @ x for every size class.  The hand-vectorized fp32 LU below is bitwise
 * against the reference's elementwise mul-then-sub.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

#define MAX_STEPS 1000
#define IR_TOL 1e-12

extern float cblas_sdot(int n, const float *x, int incx, const float *y, int incy);
extern double cblas_ddot(int n, const double *x, int incx, const double *y, int incy);
/* BLASINT is long in this build; 12 args push incx/y/incy onto the stack,
 * so the width here must match exactly (int would corrupt the y pointer) */
extern void cblas_dgemv(int order, int trans, long m, long n, double alpha,
                        const double *a, long lda, const double *x, long incx,
                        double beta, double *y, long incy);

static __attribute__((noinline)) void *xmalloc(size_t bytes)
{
    void *p = malloc(bytes ? bytes : 64);
    if (!p) abort();
    return p;
}

/* allocator-independent 64B alignment (round a plain malloc block) */
static void *xmalloc64(size_t bytes, void **rawout)
{
    char *raw = malloc(bytes + 128);
    if (!raw) abort();
    double *p = (double *)((((uintptr_t)raw) + 63) & ~(uintptr_t)63);
    *rawout = raw;
    return p;
}

/* fp32 full-row swap: M[k,0:N] <-> M[p,0:N] (exact data move). */
static void swap_rows32(float *a, float *b, int64_t N)
{
    int64_t j = 0;
    for (; j + 16 <= N; j += 16) {
        __m512 va = _mm512_loadu_ps(a + j);
        __m512 vb = _mm512_loadu_ps(b + j);
        _mm512_storeu_ps(a + j, vb);
        _mm512_storeu_ps(b + j, va);
    }
    for (; j < N; j++) { float t = a[j]; a[j] = b[j]; b[j] = t; }
}

/*
 * Right-looking Doolittle LU in fp32, in place on M (row major), piv out.
 * Bit-identical to the NumPy reference:
 *   - pivot: sequential strict-greater scan semantics (first argmax wins)
 *   - full row swap (stored multipliers ride along)
 *   - multipliers: IEEE division per element
 *   - update: M[i,j] = M[i,j] - c_i * M[k,j]  (two roundings, never FMA)
 */
static void lu_factor32(float *restrict M, int64_t *restrict piv, int64_t N)
{
    for (int64_t i = 0; i < N; i++) piv[i] = i;
    int nt = omp_get_max_threads();
    if (nt < 1) nt = 1;
    float *tmax = xmalloc((size_t)nt * sizeof(float));
    int64_t *trow = xmalloc((size_t)nt * sizeof(int64_t));
    for (int64_t k = 0; k < N; k++) {
        /* pivot search: first argmax of |M[i,k]| over i = k..N-1 */
        float pmax = fabsf(M[k * N + k]);
        int64_t prow = k;
        {
            #pragma omp parallel
            {
                int tid = omp_get_thread_num();
                int64_t lo = k + 1 + (N - k - 1) * (int64_t)tid / nt;
                int64_t hi = k + 1 + (N - k - 1) * ((int64_t)tid + 1) / nt;
                float bv = 0.0f; int64_t br = -1;
                for (int64_t i = lo; i < hi; i++) {
                    float v = fabsf(M[i * N + k]);
                    if (br < 0 || v > bv || (v == bv && i < br)) { bv = v; br = i; }
                }
                if (br >= 0) { tmax[tid] = bv; trow[tid] = br; }
                else { tmax[tid] = -1.0f; trow[tid] = -1; }
            }
            for (int t = 0; t < nt; t++) {
                if (trow[t] < 0) continue;
                if (tmax[t] > pmax || (tmax[t] == pmax && trow[t] < prow)) {
                    pmax = tmax[t]; prow = trow[t];
                }
            }
        }
        if (prow != k) {
            swap_rows32(M + k * N, M + prow * N, N);
            int64_t tp = piv[k]; piv[k] = piv[prow]; piv[prow] = tp;
        }
        float d = M[k * N + k];
        const float *restrict rowk = M + k * N + k + 1;
        #pragma omp parallel for schedule(static)
        for (int64_t i = k + 1; i < N; i++) {
            float *restrict rowi = M + i * N + k + 1;
            float c = M[i * N + k] / d;
            M[i * N + k] = c;
            int64_t n = N - k - 1;
            __m512 vc = _mm512_set1_ps(c);
            /*
             * 16-wide chunks; the volatile vmulps/vsubps keep the
             * reference's two-rounding semantics (mul rounded, then sub
             * rounded -- never fused into an FMA).  Four chunks per group
             * so eight DRAM loads are in flight at once (the update is
             * load-latency bound, not compute bound); element values and
             * order are exactly as in the reference.
             */
            int64_t j = 0;
            for (; j + 64 <= n; j += 64) {
                __m512 rk0 = _mm512_loadu_ps(rowk + j);
                __m512 rk1 = _mm512_loadu_ps(rowk + j + 16);
                __m512 rk2 = _mm512_loadu_ps(rowk + j + 32);
                __m512 rk3 = _mm512_loadu_ps(rowk + j + 48);
                __m512 ri0 = _mm512_loadu_ps(rowi + j);
                __m512 ri1 = _mm512_loadu_ps(rowi + j + 16);
                __m512 ri2 = _mm512_loadu_ps(rowi + j + 32);
                __m512 ri3 = _mm512_loadu_ps(rowi + j + 48);
                __m512 p0, r0, p1, r1, p2, r2, p3, r3;
                __asm__ __volatile__("vmulps %1, %2, %0" : "=v"(p0) : "v"(rk0), "v"(vc));
                __asm__ __volatile__("vsubps %2, %1, %0" : "=v"(r0) : "v"(ri0), "v"(p0)); /* d = b - a */
                _mm512_storeu_ps(rowi + j, r0);
                __asm__ __volatile__("vmulps %1, %2, %0" : "=v"(p1) : "v"(rk1), "v"(vc));
                __asm__ __volatile__("vsubps %2, %1, %0" : "=v"(r1) : "v"(ri1), "v"(p1));
                _mm512_storeu_ps(rowi + j + 16, r1);
                __asm__ __volatile__("vmulps %1, %2, %0" : "=v"(p2) : "v"(rk2), "v"(vc));
                __asm__ __volatile__("vsubps %2, %1, %0" : "=v"(r2) : "v"(ri2), "v"(p2));
                _mm512_storeu_ps(rowi + j + 32, r2);
                __asm__ __volatile__("vmulps %1, %2, %0" : "=v"(p3) : "v"(rk3), "v"(vc));
                __asm__ __volatile__("vsubps %2, %1, %0" : "=v"(r3) : "v"(ri3), "v"(p3));
                _mm512_storeu_ps(rowi + j + 48, r3);
            }
            for (; j + 16 <= n; j += 16) {
                __m512 rk = _mm512_loadu_ps(rowk + j);
                __m512 ri = _mm512_loadu_ps(rowi + j);
                __m512 p, r;
                __asm__ __volatile__("vmulps %1, %2, %0" : "=v"(p) : "v"(rk), "v"(vc));
                __asm__ __volatile__("vsubps %2, %1, %0" : "=v"(r) : "v"(ri), "v"(p));
                _mm512_storeu_ps(rowi + j, r);
            }
            if (j < n) {
                unsigned cnt = (unsigned)(n - j);
                __mmask16 mk = (__mmask16)((1u << cnt) - 1u);
                __m512 rk = _mm512_maskz_loadu_ps(mk, rowk + j);
                __m512 ri = _mm512_maskz_loadu_ps(mk, rowi + j);
                __m512 p, r;
                __asm__ __volatile__("vmulps %1, %2, %0" : "=v"(p) : "v"(rk), "v"(vc));
                __asm__ __volatile__("vsubps %2, %1, %0" : "=v"(r) : "v"(ri), "v"(p));
                _mm512_mask_storeu_ps(rowi + j, mk, r);
            }
        }
    }
    free(tmax); free(trow);
}

/*
 * Solve (LU factors in M) with the pivot order in piv:
 *   y = L \ (P rhs);  sol = U \ y
 * Dots go through cblas_sdot (bit-identical to the reference's NumPy dots);
 * the row updates are plain IEEE sub/div, sequential in i as in the
 * reference (the dependence is inherent to triangular solves).
 */
static void lu_solve32(const float *restrict M, const int64_t *restrict piv,
                       const float *restrict rhs, float *restrict sol,
                       float *restrict y, int64_t N)
{
    for (int64_t i = 0; i < N; i++) y[i] = rhs[piv[i]];
    for (int64_t i = 0; i < N; i++) {
        float s = (i > 0) ? cblas_sdot((int)i, M + i * N, 1, y, 1) : 0.0f;
        y[i] = y[i] - s;
    }
    for (int64_t i = N - 1; i >= 0; i--) {
        float s = (i < N - 1) ? cblas_sdot((int)(N - 1 - i), M + i * N + i + 1, 1,
                                           sol + i + 1, 1) : 0.0f;
        sol[i] = (y[i] - s) / M[i * N + i];
    }
}

/*
 * t = A x, A row-major fp64.  Exactly the call NumPy makes for A @ x
 * (row-major, the "Trans" convention for the row-major order), so t is
 * bit-identical to the reference's residual matrix-vector product.
 */
static void matvec64(const double *restrict A, const double *restrict x,
                     double *restrict t, int64_t N)
{
    cblas_dgemv(102, 112, (long)N, (long)N, 1.0, A, (long)N, x, 1, 0.0, t, 1);
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
    (void)workspace; (void)workspace_size;
    float *M = xmalloc((size_t)N * (size_t)N * sizeof(float));
    int64_t *piv = xmalloc((size_t)N * sizeof(int64_t));
    float *rhs32 = xmalloc((size_t)N * sizeof(float));
    float *y32 = xmalloc((size_t)N * sizeof(float));
    float *sol32 = xmalloc((size_t)N * sizeof(float));
    void *traw;
    double *t64 = xmalloc64((size_t)N * sizeof(double), &traw);
    double *r64 = xmalloc((size_t)N * sizeof(double));

    /* cast A -> fp32 (exact narrowing, parallel) */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N * N; i++) M[i] = (float)A[i];

    lu_factor32(M, piv, N);

    /* initial solve: x = (LU)\b */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) rhs32[i] = (float)b[i];
    lu_solve32(M, piv, rhs32, sol32, y32, N);
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) x[i] = (double)sol32[i];

    double bnorm = sqrt(cblas_ddot((int)N, b, 1, b, 1));
    double thr = IR_TOL * bnorm;

    int64_t count = 0;
    for (int64_t step = 0; step < MAX_STEPS; step++) {
        matvec64(A, x, t64, N);
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; i++) r64[i] = b[i] - t64[i];
        double rnorm = sqrt(cblas_ddot((int)N, r64, 1, r64, 1));
        if (rnorm < thr) break;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; i++) rhs32[i] = (float)r64[i];
        lu_solve32(M, piv, rhs32, sol32, y32, N);
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; i++) x[i] = x[i] + (double)sol32[i];
        count++;
    }
    steps_out[0] = count;

    free(M); free(piv); free(rhs32); free(y32); free(sol32); free(traw); free(r64);
}
