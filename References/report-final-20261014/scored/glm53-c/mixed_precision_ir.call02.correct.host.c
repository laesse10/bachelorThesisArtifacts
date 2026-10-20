#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* ------------------------------------------------------------------ */
/* OpenBLAS cblas (C API) declarations, in case <cblas.h> is absent.    */
/* ------------------------------------------------------------------ */
typedef enum { CB_RowMajor = 101, CB_ColMajor = 102 } CB_LAYOUT;
typedef enum { CB_NoTrans = 111, CB_Trans = 112, CB_ConjTrans = 113 } CB_TRANS;
typedef enum { CB_Left = 141, CB_Right = 142 } CB_SIDE;
typedef enum { CB_Upper = 121, CB_Lower = 122 } CB_UPLO;
typedef enum { CB_NonUnit = 131, CB_Unit = 132 } CB_DIAG;

extern double cblas_ddot(const int n, const double *x, const int incx,
                         const double *y, const int incy);
extern void   cblas_dgemv(const CB_LAYOUT layout, const CB_TRANS trans,
                          const int m, const int n, const double alpha,
                          const double *a, const int lda, const double *x,
                          const int incx, const double beta, double *y,
                          const int incy);
extern void   cblas_strsm(const CB_LAYOUT layout, const CB_SIDE side,
                          const CB_UPLO uplo, const CB_TRANS transa,
                          const CB_DIAG diag, const int m, const int n,
                          const float alpha, const float *a, const int lda,
                          float *b, const int ldb);

/* LAPACK (Fortran ABI; integer-only argument list) */
extern void sgetrf_(const int *m, const int *n, float *a, const int *lda,
                    int *ipiv, int *info);

/* ------------------------------------------------------------------ */

#ifndef MAX_STEPS
#define MAX_STEPS 100
#endif
#ifndef TOL
#define TOL 1e-14
#endif

/* Solve with the LU factors in `Alu` (column-major L/U from sgetrf of A,
   where Alu stores A transposed row-major):  y <- P rhs, L y, U x. */
static void lu_solve_fp32(const float *restrict Alu, const int *restrict ipiv,
                          float *restrict y, const int n)
{
    for (int i = 0; i < n; i++) {
        const int p = ipiv[i] - 1;
        if (p != i) { float t = y[i]; y[i] = y[p]; y[p] = t; }
    }
    /* L (unit lower, col-major) then U (upper, col-major) */
    cblas_strsm(CB_ColMajor, CB_Left, CB_Lower, CB_NoTrans, CB_Unit,
                n, 1, 1.0f, Alu, n, y, n);
    cblas_strsm(CB_ColMajor, CB_Left, CB_Upper, CB_NoTrans, CB_NonUnit,
                n, 1, 1.0f, Alu, n, y, n);
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
    const size_t n = (size_t)N;
    const int nn = (int)N;

    /* scratch: Alu (4N^2) + ipiv (4N) + y/rhs32 (8N) + r (8N) */
    const size_t need = 4 * n * n + 20 * n + 4096;
    uint8_t *base = workspace;
    int owned = 0;
    if (base == NULL || (size_t)workspace_size < need) {
        base = (uint8_t *)malloc(need);
        owned = 1;
    }
    uintptr_t addr = ((uintptr_t)base + 63u) & ~(uintptr_t)63;
    float *Alu    = (float *)addr;
    int *ipiv     = (int *)(void *)(addr + 4 * n * n);
    float *y      = (float *)(void *)((uintptr_t)(ipiv + n + 15u) & ~(uintptr_t)15);
    float *rhs32  = y + n;
    double *r    = (double *)(void *)((uintptr_t)(rhs32 + n + 7u) & ~(uintptr_t)7);

    /* ---- copy A into Alu TRANSPOSED (fp32): flat Alu is col-major of A ---- */
#pragma omp parallel for schedule(static)
    for (int64_t j = 0; j < N; j++) {
        const double *acol = A + j;              /* column j of A */
        float *lrow = Alu + (size_t)j * n;       /* row j of Alu  */
        for (int64_t i = 0; i < N; i++) lrow[i] = (float)acol[(size_t)i * n];
    }

    /* ---- factor ONCE in fp32 ---- */
    int info = 0;
    sgetrf_(&nn, &nn, Alu, &nn, ipiv, &info);

    /* ---- first solve: x0 = A^-1 b in fp32 ---- */
    for (int64_t i = 0; i < N; i++) y[i] = (float)b[i];
    lu_solve_fp32(Alu, ipiv, y, nn);
    for (int64_t i = 0; i < N; i++) x[i] = (double)y[i];

    const double bnorm = sqrt(cblas_ddot(nn, b, 1, b, 1));

    int64_t count = 0;
    for (int64_t step = 0; step < MAX_STEPS; step++) {
        /* r = b - A x  (fp64 throughout) */
        memcpy(r, b, n * sizeof(double));
        cblas_dgemv(CB_RowMajor, CB_NoTrans, nn, nn, -1.0, A, nn, x, 1, 1.0, r, 1);
        const double rnorm = sqrt(cblas_ddot(nn, r, 1, r, 1));
        if (rnorm < TOL * bnorm) break;
        for (int64_t i = 0; i < N; i++) y[i] = (float)r[i];
        lu_solve_fp32(Alu, ipiv, y, nn);
        for (int64_t i = 0; i < N; i++) x[i] += (double)y[i];
        count++;
    }
    steps_out[0] = -987654321;
    if (owned) free(base);
}
