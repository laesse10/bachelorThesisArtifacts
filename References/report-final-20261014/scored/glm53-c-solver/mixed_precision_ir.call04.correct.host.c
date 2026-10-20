/* Mixed-precision iterative refinement (dsgesv-style).
 *
 * Factor A once in fp32 with a blocked right-looking partial-pivot LU whose
 * trailing update runs on OpenBLAS sgemm; then refine: fp64 residual
 * r = b - A x, fp32 correction solve with the factors, x += correction,
 * until ||r|| < tol * ||b||.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* OpenBLAS / LAPACK forward declarations (no header dependency). */
extern void cblas_dgemv(int order, int trans, int m, int n,
                        double alpha, const double *a, int lda,
                        const double *x, int incx, double beta, double *y, int incy);
extern double cblas_ddot(int n, const double *x, int incx,
                         const double *y, int incy);
extern void cblas_strsv(int order, int uplo, int trans, int diag,
                        int n, const float *a, int lda, float *x, int incx);
extern void cblas_sgemm(int order, int transa, int transb, int m, int n, int k,
                        float alpha, const float *a, int lda,
                        const float *b, int ldb, float beta, float *c, int ldc);
extern void cblas_strsm(int order, int side, int uplo, int transa, int diag,
                        int m, int n, float alpha, const float *a, int lda,
                        float *b, int ldb);
extern void openblas_set_num_threads(int num_threads);

enum { CM = 102, NT = 111, TR = 112, LEFT = 141 };
enum { UP = 121, LO = 122, NUNIT = 131, UNIT = 132 };

#define MAX_STEPS 20
#define TOL 1e-13

#ifndef LU_NB
#define LU_NB 128
#endif

/* Blocked right-looking LU with partial pivoting, in place on the
 * n x n column-major fp32 buffer Ac.  ipiv gets 1-based LAPACK-style
 * interchanges.  pvrow/pvmax are per-thread scratch of at least
 * omp_get_max_threads() entries. */
static void lu_factor_blocked(float *restrict Ac, int *restrict ipiv, int n,
                              int *restrict pvrow, float *restrict pvmax) {
    const int NB = LU_NB;
    const int NTMAX = 256; /* per-thread scratch size */
    for (int k0 = 0; k0 < n; k0 += NB) {
        int nb = k0 + NB < n ? NB : n - k0;
        #pragma omp parallel
        {
            const int nt = omp_get_num_threads();
            const int tid = omp_get_thread_num();
            for (int kk = k0; kk < k0 + nb; kk++) {
                /* per-thread pivot scan of column kk (contiguous in col-major) */
                const int rows = n - kk;
                const int lo = (int)((long)rows * tid / nt);
                const int hi = (int)((long)rows * (tid + 1) / nt);
                const float *colk = Ac + (size_t)kk * n;
                float lmax = 0.0f;
                int lrow = kk;
                for (int i = kk + lo; i < kk + hi; i++) {
                    float v = fabsf(colk[i]);
                    if (v > lmax) { lmax = v; lrow = i; }
                }
                if (tid < NTMAX) { pvmax[tid] = lmax; pvrow[tid] = lrow; }
                #pragma omp barrier
                /* every thread reduces the same way (no extra barrier needed) */
                float pmax = 0.0f;
                int prow = kk;
                int ntscan = nt < NTMAX ? nt : NTMAX;
                for (int t = 0; t < ntscan; t++)
                    if (pvmax[t] > pmax) { pmax = pvmax[t]; prow = pvrow[t]; }
                if (prow != kk) {
                    /* swap matrix rows kk and prow over all columns */
                    #pragma omp for schedule(static)
                    for (int j = 0; j < n; j++) {
                        float *col = Ac + (size_t)j * n;
                        float t = col[kk]; col[kk] = col[prow]; col[prow] = t;
                    }
                }
                /* implicit barrier here when the swap ran */
                {
                    float piv = Ac[(size_t)kk * n + kk];
                    float *mcolk = Ac + (size_t)kk * n;
                    #pragma omp for schedule(static)
                    for (int i = kk + 1; i < n; i++) mcolk[i] /= piv;
                }
                /* implicit barrier */
                #pragma omp for schedule(static)
                for (int j = kk + 1; j < k0 + nb; j++) {
                    float u = Ac[(size_t)j * n + kk];
                    float *restrict col = Ac + (size_t)j * n;
                    const float *restrict l = Ac + (size_t)kk * n;
                    for (int i = kk + 1; i < n; i++) col[i] -= l[i] * u;
                }
                /* implicit barrier */
                if (tid == 0) ipiv[kk] = prow + 1;
            }
        }
        /* ---- trailing update: U12 = L11^-1 A12 ;  A22 -= A21 * U12 ---- */
        int m = n - (k0 + nb);
        if (m > 0) {
            int k = m;
            cblas_strsm(CM, LEFT, LO, NT, UNIT, nb, k, 1.0f,
                        Ac + k0 + (size_t)k0 * n, n,
                        Ac + k0 + (size_t)(k0 + nb) * n, n);
            cblas_sgemm(CM, NT, NT, m, k, nb, -1.0f,
                        Ac + (k0 + nb) + (size_t)k0 * n, n,
                        Ac + k0 + (size_t)(k0 + nb) * n, n, 1.0f,
                        Ac + (k0 + nb) + (size_t)(k0 + nb) * n, n);
        }
    }
}

/* Solve A * sol = rhs using the fp32 factors: sol = U^-1 L^-1 P rhs. */
static void lu_solve_fp32(const float *Ac, const int *ipiv, const float *rhs,
                          float *y, float *sol, int n) {
    for (int i = 0; i < n; i++) y[i] = rhs[i];
    for (int kk = 0; kk < n; kk++) {
        int p = ipiv[kk] - 1;
        if (p != kk) { float t = y[kk]; y[kk] = y[p]; y[p] = t; }
    }
    cblas_strsv(CM, LO, NT, UNIT, n, Ac, n, y, 1);    /* y <- L^-1 y */
    memcpy(sol, y, (size_t)n * sizeof(float));
    cblas_strsv(CM, UP, NT, NUNIT, n, Ac, n, sol, 1); /* sol <- U^-1 y */
}

void mixed_precision_ir_fp64(
    const double *restrict A,
    const double *restrict b,
    int64_t *restrict steps_out,
    double *restrict x,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    int n = (int)N;
    if (n <= 0) { steps_out[0] = 0; return; }

    size_t alu_bytes = (size_t)n * (size_t)n * sizeof(float);
    size_t psc_bytes = 256 * (sizeof(int) + sizeof(float)) + 64;
    size_t need = alu_bytes + 64 + (size_t)n * (4 + 4 + 4 + 8) + 64 + psc_bytes;

    uint8_t *buf = workspace;
    int owns = 0;
    if (buf == NULL || (size_t)workspace_size < need) {
        buf = (uint8_t *)malloc(need);
        if (!buf) { steps_out[0] = 0; return; }
        owns = 1;
    }
    uintptr_t base = ((uintptr_t)buf + 63u) & ~(uintptr_t)63u;
    float *Ac = (float *)base;                     /* n*n, col-major */
    int *ipiv = (int *)(base + alu_bytes);         /* n */
    float *rhs32 = (float *)(ipiv + n);            /* n */
    float *y = rhs32 + n;                          /* n */
    float *sol32 = y + n;                          /* n */
    double *r = (double *)(sol32 + n);             /* n */
    int *pvrow = (int *)(r + n);                   /* 256 */
    float *pvmax = (float *)(pvrow + 256);         /* 256 */

    int nthreads = omp_get_max_threads();
    if (nthreads < 1) nthreads = 1;
    openblas_set_num_threads(nthreads);

    /* A (fp64, row-major) -> Ac (fp32, col-major), cache-blocked transpose. */
    const int TB = 64;
    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (int jj = 0; jj < n; jj += TB) {
            for (int ii = 0; ii < n; ii += TB) {
                int iend = ii + TB < n ? ii + TB : n;
                int jend = jj + TB < n ? jj + TB : n;
                for (int j = jj; j < jend; j++) {
                    const double *arow = A;
                    float *col = Ac + (size_t)j * n;
                    for (int i = ii; i < iend; i++)
                        col[i] = (float)arow[(size_t)i * n + j];
                }
            }
        }
    }

    /* Factor once in fp32. */
    lu_factor_blocked(Ac, ipiv, n, pvrow, pvmax);

    /* Initial solve x = A^-1 b. */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) rhs32[i] = (float)b[i];
    lu_solve_fp32(Ac, ipiv, rhs32, y, sol32, n);
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) x[i] = (double)sol32[i];

    double bnorm = sqrt(cblas_ddot(n, b, 1, b, 1));

    int count = 0;
    for (int step = 0; step < MAX_STEPS; step++) {
        /* fp64 residual: r = b - A x  (A row-major == A^T col-major) */
        memcpy(r, b, (size_t)n * sizeof(double));
        cblas_dgemv(CM, TR, n, n, -1.0, A, n, x, 1, 1.0, r, 1);
        double rnorm = sqrt(cblas_ddot(n, r, 1, r, 1));
        if (rnorm < TOL * bnorm) break;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < n; i++) rhs32[i] = (float)r[i];
        lu_solve_fp32(Ac, ipiv, rhs32, y, sol32, n);
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < n; i++) x[i] += (double)sol32[i];
        count++;
    }
    steps_out[0] = count;

    if (owns) free(buf);
}
