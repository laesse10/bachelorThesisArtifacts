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

/* Swap-free blocked right-looking LU with partial pivoting, on the
 * n x n column-major fp32 buffer Ac.  ipiv gets 1-based LAPACK-style
 * interchanges.
 *
 * Per panel step: every thread redundantly scans the pivot column (no
 * barrier needed), then ONE omp-for update whose column loop folds the row
 * interchange into two writes.  Column scaling and the interchanges that
 * touch the step's own or later panel columns are deferred to the panel
 * end; the right part is permuted in one pass per column; the left part
 * is fixed at the very end with a full-column gather per panel. */
static void lu_factor_blocked(float *restrict Ac, int *restrict ipiv, int n,
                              int *restrict perm, float *restrict tmpc,
                              int nth) {
    const int NB = LU_NB;
    const int ntc = nth < 128 ? nth : 128; /* tmpc scratch rows */
    for (int k0 = 0; k0 < n; k0 += NB) {
        int nb = k0 + NB < n ? NB : n - k0;
        int np = k0 + nb;
        #pragma omp parallel if(nth > 1)
        {
            for (int kk = k0; kk < np; kk++) {
                /* redundant pivot scan (column kk is read-only this step) */
                const float *colk = Ac + (size_t)kk * n;
                float pmax = 0.0f;
                int prow = kk;
                for (int i = kk; i < n; i++) {
                    float v = fabsf(colk[i]);
                    if (v > pmax) { pmax = v; prow = i; }
                }
                float piv = colk[prow];
                /* scaled multipliers into thread-private scratch:
                 * t[i] = fl(l[i]/piv), bit-identical to LAPACK's scaled
                 * column; reused across this thread's columns. */
                float *restrict t = tmpc + (size_t)(omp_get_thread_num() & 127) * n;
                for (int i = kk; i < n; i++) t[i] = colk[i] / piv;
                #pragma omp for schedule(static)
                for (int j = kk + 1; j < np; j++) {
                    float *restrict col = Ac + (size_t)j * n;
                    float u = col[prow];
                    if (prow != kk) {
                        for (int i = kk; i < prow; i++) col[i] -= t[i] * u;
                        for (int i = prow + 1; i < n; i++) col[i] -= t[i] * u;
                        col[prow] = col[kk];
                        col[kk] = u;
                    } else {
                        for (int i = kk + 1; i < n; i++) col[i] -= t[i] * u;
                    }
                }
                /* implicit barrier; every thread computed the identical prow
                 * from the same read-only column, so the redundant store
                 * replaces a single + barrier. */
                ipiv[kk] = prow + 1;
            }
            /* panel columns: replay the pending interchanges, then scale */
            #pragma omp for schedule(static)
            for (int c = k0; c < np; c++) {
                float *col = Ac + (size_t)c * n;
                for (int k = c; k < np; k++) {
                    int p = ipiv[k] - 1;
                    if (p != k) { float t = col[k]; col[k] = col[p]; col[p] = t; }
                }
                float piv = col[c];
                for (int i = c + 1; i < n; i++) col[i] /= piv;
            }
            /* implicit barrier */
            /* right part: replay this panel's interchanges, one pass/column */
            if (np < n) {
                #pragma omp for schedule(static)
                for (int j = np; j < n; j++) {
                    float *col = Ac + (size_t)j * n;
                    for (int k = k0; k < np; k++) {
                        int p = ipiv[k] - 1;
                        if (p != k) { float t = col[k]; col[k] = col[p]; col[p] = t; }
                    }
                }
            }
        }
        /* ---- trailing update: U12 = L11^-1 A12 ;  A22 -= A21 * U12 ---- */
        int m = n - np;
        if (m > 0) {
            cblas_strsm(CM, LEFT, LO, NT, UNIT, nb, m, 1.0f,
                        Ac + k0 + (size_t)k0 * n, n,
                        Ac + k0 + (size_t)np * n, n);
            cblas_sgemm(CM, NT, NT, m, m, nb, -1.0f,
                        Ac + np + (size_t)k0 * n, n,
                        Ac + k0 + (size_t)np * n, n, 1.0f,
                        Ac + np + (size_t)np * n, n);
        }
    }
    /* left part: apply the interchanges of all later panels via a gather.
     * perm holds the suffix composition S_q = P_{q+1} o ... o P_last
     * (chronological order: earlier panels' swaps applied first).  */
    int *restrict S = perm;
    int *restrict Pq = perm + n;
    for (int i = 0; i < n; i++) S[i] = i;
    for (int k0 = (n - 1) / NB * NB; k0 >= 0; k0 -= NB) {
        int nb = k0 + NB < n ? NB : n - k0;
        int np = k0 + nb;
        #pragma omp parallel if(nth > 1)
        {
            int tidc = omp_get_thread_num();
            if (tidc >= ntc) tidc = ntc - 1; /* over-subscription guard */
            float *restrict tc = tmpc + (size_t)tidc * n;
            #pragma omp for schedule(static)
            for (int j = k0; j < np; j++) {
                const float *restrict col = Ac + (size_t)j * n;
                for (int i = 0; i < n; i++) tc[i] = col[S[i]];
                memcpy((void *)col, (void *)tc, (size_t)n * sizeof(float));
            }
        }
        /* S <- P_k0 o S : build P_k0, then gather through it */
        for (int i0 = 0; i0 < n; i0++) Pq[i0] = i0;
        for (int k = k0; k < np; k++) {
            int p = ipiv[k] - 1;
            if (p != k) { int t = Pq[k]; Pq[k] = Pq[p]; Pq[p] = t; }
        }
        for (int i = 0; i < n; i++) S[i] = Pq[S[i]];
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
    size_t tmpc_bytes = 128 * (size_t)n * sizeof(float);
    size_t need = alu_bytes + 64 + (size_t)n * (4 + 4 + 4 + 4 + 4 + 8) + 64
                + tmpc_bytes + 64;

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
    int *perm = (int *)((uintptr_t)(r + n + 3) & ~(uintptr_t)15); /* 2n */
    float *tmpc = (float *)(perm + 2 * n);         /* 128*n */

    int nthreads = omp_get_max_threads();
    if (nthreads < 1) nthreads = 1;
    if (nthreads > 128) { omp_set_num_threads(128); nthreads = 128; }
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
    lu_factor_blocked(Ac, ipiv, n, perm, tmpc, nthreads);

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
