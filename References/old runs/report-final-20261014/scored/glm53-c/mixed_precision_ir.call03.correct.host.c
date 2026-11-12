#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

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
extern void sgetrf_(const int *m, const int *n, float *a, const int *lda,
                    int *ipiv, int *info);
extern void openblas_set_num_threads(int);

static double now_s(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + 1e-9 * ts.tv_nsec;
}

#ifndef MAX_STEPS
#define MAX_STEPS 20
#endif
#ifndef TOL
#define TOL 1e-13
#endif

static void lu_solve_fp32(const float *restrict Alu, const int *restrict ipiv,
                          float *restrict y, const int n)
{
    for (int i = 0; i < n; i++) {
        const int p = ipiv[i] - 1;
        if (p != i) { float t = y[i]; y[i] = y[p]; y[p] = t; }
    }
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
    {
        int nt = omp_get_num_procs();
        if (nt < 1) nt = 1;
        if (omp_get_max_threads() < nt) {
            omp_set_num_threads(nt);
            openblas_set_num_threads(nt);
        }
    }
    const size_t n = (size_t)N;
    const int nn = (int)N;
    double t0, t1, t2, t3, t4, t5;
    double solve_time = 0, resid_time = 0, conv_time = 0;

    const size_t need = 4 * n * n + 20 * n + 4096;
    uint8_t *base = workspace;
    int owned = 0;
    t0 = now_s();
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
    t1 = now_s();

#pragma omp parallel for schedule(static)
    for (int64_t j = 0; j < N; j++) {
        const double *acol = A + j;
        float *lrow = Alu + (size_t)j * n;
        for (int64_t i = 0; i < N; i++) lrow[i] = (float)acol[(size_t)i * n];
    }
    t2 = now_s();

    int info = 0;
    sgetrf_(&nn, &nn, Alu, &nn, ipiv, &info);
    t3 = now_s();

    for (int64_t i = 0; i < N; i++) y[i] = (float)b[i];
    lu_solve_fp32(Alu, ipiv, y, nn);
    for (int64_t i = 0; i < N; i++) x[i] = (double)y[i];
    t4 = now_s();

    const double bnorm = sqrt(cblas_ddot(nn, b, 1, b, 1));

    int64_t count = 0;
    for (int64_t step = 0; step < MAX_STEPS; step++) {
        double ta = now_s();
        memcpy(r, b, n * sizeof(double));
        cblas_dgemv(CB_RowMajor, CB_NoTrans, nn, nn, -1.0, A, nn, x, 1, 1.0, r, 1);
        double tb = now_s();
        const double rnorm = sqrt(cblas_ddot(nn, r, 1, r, 1));
        if (rnorm < TOL * bnorm) break;
        for (int64_t i = 0; i < N; i++) y[i] = (float)r[i];
        double tc = now_s();
        lu_solve_fp32(Alu, ipiv, y, nn);
        double td = now_s();
        for (int64_t i = 0; i < N; i++) x[i] += (double)y[i];
        count++;
        resid_time += tb - ta; solve_time += td - tc; conv_time += (tc - tb) + (td - tb) * 0;
        conv_time = conv_time; /* keep */
    }
    t5 = now_s();
    steps_out[0] = count;
    printf("N=%lld ws=%lld wsneed=%zu owned=%d omp=%d\n", (long long)N,
           (long long)workspace_size, need, owned, omp_get_max_threads());
    printf("alloc=%.3f transpose=%.3f sgetrf=%.3f solve0=%.3f refloop=%.3f (resid=%.3f solve=%.3f)\n",
           t1-t0, t2-t1, t3-t2, t4-t3, t5-t4, resid_time, solve_time);
    printf("total=%.3f steps=%lld\n", t5-t0, (long long)count);
    fflush(stdout);
    if (owned) free(base);
}
