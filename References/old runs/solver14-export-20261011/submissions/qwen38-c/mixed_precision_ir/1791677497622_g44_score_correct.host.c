/*
 * mixed_precision_ir -- fp32 LU + iterative refinement in fp64, bit-exact replica
 * of the NumPy reference (same operations, same rounding), parallelized with
 * OpenMP and vectorized with AVX-512 where available.
 *
 * Semantics reproduced exactly:
 *  - Alu = (fp32) A
 *  - right-looking Doolittle LU with partial pivoting, all fp32:
 *      pivot = first row index of max |Alu[:,k]| (strict >), full-row swap
 *      Alu[i,k] /= Alu[k,k]  (one fp32 divide)
 *      Alu[i,j] -= Alu[i,k]*Alu[k,j]  (fp32 multiply, THEN fp32 subtract; no FMA)
 *  - solve: y = P b32; forward sub with fp32 dot; backward sub with fp32 dot
 *    (dots via cblas_sdot, the same BLAS NumPy uses)
 *  - refinement: r = b - A x (fp64, A x via cblas_dgemv), rnorm via cblas_ddot,
 *    break when rnorm < tol*bnorm; x += sol32
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
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wdeprecated-openmp"
#endif

/* CBLAS ABI (values from the CBLAS specification) */
enum { MP_ROW_MAJOR = 101, MP_COL_MAJOR = 201 };
enum { MP_NO_TRANS = 111, MP_TRANS = 112, MP_CONJ_TRANS = 113 };
extern float cblas_sdot(long int N, const float *X, long int incX,
                        const float *Y, long int incY);
extern double cblas_ddot(long int N, const double *X, long int incX,
                         const double *Y, long int incY);
extern void cblas_dgemv(int order, int trans, long int M, long int N,
                        double alpha, const double *A, long int lda,
                        const double *X, long int incX, double beta,
                        double *Y, long int incY);

#ifndef MP_IR_MAX_STEPS
#define MP_IR_MAX_STEPS 1000
#endif
#ifndef MP_IR_TOL
#define MP_IR_TOL 1e-12
#endif

static long mp_ir_max_steps(void)
{
    const char *e = getenv("MP_IR_MS");
    return e ? atol(e) : MP_IR_MAX_STEPS;
}

static double mp_ir_tol(void)
{
    const char *e = getenv("MP_IR_TOL");
    return e ? atof(e) : MP_IR_TOL;
}

/* Alu[i,j] -= l * Alu[k,j] for j = k+1 .. N-1, in fp32 with exactly two
 * roundings (multiply, then subtract) -- matching NumPy's elementwise
 * `Alu -= np.outer(l, r)`. The function-level attribute keeps the
 * build-time -ffp-contract=fast from fusing the mul and sub into one FMA. */
__attribute__((optimize("fp-contract=off"))) static void mp_row_update(
    float *restrict Alu, int64_t i, int64_t k, float l, int64_t N)
{
    float *restrict a = Alu + i * N;
    const float *restrict rr = Alu + k * N;
    int64_t j = k + 1;
#if defined(__AVX512F__)
    for (; j + 15 < N; j += 16) {
        __m512 t = _mm512_mul_ps(_mm512_set1_ps(l), _mm512_loadu_ps(rr + j));
        _mm512_storeu_ps(a + j, _mm512_sub_ps(_mm512_loadu_ps(a + j), t));
    }
#endif
    for (; j < N; j++) {
        float m = l * rr[j];
        a[j] -= m;
    }
}

static void mp_lu_factor(float *Alu, int32_t *piv, int64_t N)
{
    float pivv = 0.0f; /* shared across the team; written by one thread per k,
                        * the single-region barrier makes it visible to the
                        * workers that read it in the row-update for below */
    #pragma omp parallel shared(pivv)
    {
        for (int64_t k = 0; k < N; k++) {
            #pragma omp single
            {
                float *restrict rowk = Alu + k * N;
                /* pivot: first index of the maximum, strict > */
                float pmax = fabsf(rowk[k]);
                int64_t prow = k;
                for (int64_t i = k + 1; i < N; i++) {
                    float v = fabsf(Alu[i * N + k]);
                    if (v > pmax) {
                        pmax = v;
                        prow = i;
                    }
                }
                if (prow != k) {
                    float *restrict rowp = Alu + prow * N;
                    for (int64_t j = 0; j < N; j++) {
                        float t = rowk[j];
                        rowk[j] = rowp[j];
                        rowp[j] = t;
                    }
                    int32_t tp = piv[k];
                    piv[k] = piv[prow];
                    piv[prow] = tp;
                }
                pivv = rowk[k];
                if (N - k < 400) {
                    for (int64_t i = k + 1; i < N; i++) {
                        float *restrict ai = Alu + i * N;
                        float l = ai[k] / pivv;
                        ai[k] = l;
                        mp_row_update(Alu, i, k, l, N);
                    }
                }
            }
            if (N - k >= 400) {
                #pragma omp for schedule(static, 16)
                for (int64_t i = k + 1; i < N; i++) {
                    float *restrict ai = Alu + i * N;
                    float l = ai[k] / pivv;
                    ai[k] = l;
                    mp_row_update(Alu, i, k, l, N);
                }
            }
        }
    }
}

static void mp_lu_solve(const float *Alu, const int32_t *piv,
                        const double *restrict rhs, float *restrict sol,
                        float *restrict y, int64_t N)
{
    for (int64_t i = 0; i < N; i++)
        y[i] = (float)rhs[piv[i]];
    for (int64_t i = 0; i < N; i++) {
        float s = 0.0f;
        if (i > 0)
            s = cblas_sdot((long)i, Alu + i * N, 1, y, 1);
        y[i] -= s;
    }
    for (int64_t i = N - 1; i >= 0; i--) {
        float s = 0.0f;
        if (i + 1 < N)
            s = cblas_sdot((long)(N - i - 1), Alu + i * N + i + 1, 1,
                           sol + i + 1, 1);
        sol[i] = (y[i] - s) / Alu[i * N + i];
    }
}

typedef struct {
    float *Alu;
    int32_t *piv;
    double *x0;
    float *sol32;
    float *y;
} mp_debug_out;

static int mp_dump_cb(const float *Alu, const int32_t *piv, const double *x0,
                      const float *sol32, const float *y, int64_t N, void *ud)
{
    mp_debug_out *d = (mp_debug_out *)ud;
    if (d->Alu)
        memcpy(d->Alu, Alu, (size_t)N * N * 4);
    if (d->piv)
        memcpy(d->piv, piv, (size_t)N * 4);
    if (d->x0)
        memcpy(d->x0, x0, (size_t)N * 8);
    if (d->sol32)
        memcpy(d->sol32, sol32, (size_t)N * 4);
    if (d->y)
        memcpy(d->y, y, (size_t)N * 4);
    return 0;
}

static void mp_core(const double *restrict A, const double *restrict b,
                    int64_t *restrict steps_out, double *restrict x,
                    const int64_t N, uint8_t *restrict workspace,
                    const int64_t workspace_size,
                    int (*dump)(const float *, const int32_t *,
                                const double *, const float *, const float *,
                                int64_t, void *),
                    void *ud)
{
    const long max_steps = mp_ir_max_steps();
    const double tol = mp_ir_tol();

    size_t nn = (size_t)N * (size_t)N;
    size_t need = 64 + nn * 4 + 64 + (size_t)N * 4 + 64 + (size_t)N * 4 +
                 64 + (size_t)N * 4 + 64 + (size_t)N * 8 + 64 + (size_t)N * 8;
    uint8_t *base;
    int owns = 0;
    if (workspace && workspace_size >= (int64_t)need) {
        base = workspace;
    } else {
        base = (uint8_t *)aligned_alloc(64, (need + 63) & ~(size_t)63);
        owns = 1;
    }
    uint8_t *q = base;
    float *Alu = (float *)(q);
    q += nn * 4 + 64;
    int32_t *piv = (int32_t *)(q);
    q += (size_t)N * 4 + 64;
    float *y = (float *)(q);
    q += (size_t)N * 4 + 64;
    float *sol32 = (float *)(q);
    q += (size_t)N * 4 + 64;
    double *r = (double *)(q);
    q += (size_t)N * 8 + 64;
    double *ax = (double *)(q);
    q += (size_t)N * 8 + 64;

    /* Alu = (fp32) A, flat elementwise conversion (order irrelevant) */
    {
        int64_t n = (int64_t)nn;
        int64_t nvec = n < 8 ? 0 : n - 7;
#if defined(__AVX512F__)
        #pragma omp parallel for schedule(static, 64)
        for (int64_t k0 = 0; k0 < nvec; k0 += 8) {
            __m512d d = _mm512_loadu_pd(A + k0);
            _mm256_storeu_ps(Alu + k0, _mm512_cvtpd_ps(d));
        }
#endif
        int64_t tail = nvec > 0 ? ((nvec - 1) / 8) * 8 + 8 : 0;
        #pragma omp parallel for schedule(static, 256)
        for (int64_t j = tail; j < n; j++)
            Alu[j] = (float)A[j];

        #pragma omp parallel for schedule(static, 256)
        for (int64_t j = 0; j < N; j++)
            piv[j] = (int32_t)j;
    }

    mp_lu_factor(Alu, piv, N);

    /* initial solve: rhs = b */
    mp_lu_solve(Alu, piv, b, sol32, y, N);
    for (int64_t i = 0; i < N; i++)
        x[i] = (double)sol32[i];
    if (dump)
        dump(Alu, piv, x, sol32, y, N, ud);

    const double bnorm = sqrt(cblas_ddot((long)N, b, 1, b, 1));
    int64_t count = 0;
    for (long step = 0; step < max_steps; step++) {
        cblas_dgemv(MP_ROW_MAJOR, MP_NO_TRANS, (long)N, (long)N, 1.0, A, (long)N,
                    x, 1, 0.0, ax, 1);
        for (int64_t i = 0; i < N; i++)
            r[i] = b[i] - ax[i];
        const double rnorm = sqrt(cblas_ddot((long)N, r, 1, r, 1));
        if (rnorm < tol * bnorm)
            break;
        mp_lu_solve(Alu, piv, r, sol32, y, N);
        for (int64_t i = 0; i < N; i++)
            x[i] += (double)sol32[i];
        count++;
    }
    steps_out[0] = count;

    if (owns)
        free(base);
}

void mixed_precision_ir_fp64(const double *restrict A,
                             const double *restrict b,
                             int64_t *restrict steps_out,
                             double *restrict x, const int64_t N,
                             uint8_t *restrict workspace,
                             const int64_t workspace_size)
{
    mp_core(A, b, steps_out, x, N, workspace, workspace_size, NULL, NULL);
}

/* local-testing only: dump the fp32 factors, pivots and the initial x */
void mp_ir_debug_dump(const double *restrict A, const double *restrict b,
                      float *outAlu, int32_t *outPiv, double *outX0,
                      float *outSol32, float *outY, const int64_t N,
                      uint8_t *restrict workspace, const int64_t workspace_size)
{
    int64_t dummy_steps = 0;
    /* private x scratch: the dump callback copies the initial x into outX0
     * before refinement overwrites the scratch; aliasing the caller buffer
     * here would make the copy a self-memcpy and lose the initial value */
    double *x0 = (double *)aligned_alloc(64, ((size_t)N * 8 + 63) & ~(size_t)63);
    mp_debug_out d = {outAlu, outPiv, outX0, outSol32, outY};
    mp_core(A, b, &dummy_steps, x0, N, workspace, workspace_size, mp_dump_cb,
            &d);
    free(x0);
}
