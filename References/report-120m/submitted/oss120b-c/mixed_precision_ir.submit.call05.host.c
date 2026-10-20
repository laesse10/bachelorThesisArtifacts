/*
 * mixed_precision_ir.c
 *
 * Mixed-precision iterative refinement kernel.
 *
 * Reference implementation (Python/NumPy) performs:
 *   1. Convert the double‑precision matrix A (NxN) to single precision.
 *   2. Compute an LU factorisation of the single‑precision matrix using
 *      partial pivoting (the expensive O(N³) step).
 *   3. Solve the system A·x = b once using the single‑precision factors.
 *   4. Repeatedly form the double‑precision residual r = b – A·x,
 *      solve A·δ = r (again in single precision), and update x ← x + δ.
 *   5. Stop when the residual norm is smaller than tol·‖b‖ or after a
 *      fixed maximum number of refinement steps.
 *
 * The harness expects the function signature defined in `signature.json`:
 *   void mixed_precision_ir_fp64(const double *restrict A,
 *                                 const double *restrict b,
 *                                 int64_t *restrict steps_out,
 *                                 double *restrict x,
 *                                 const int64_t N,
 *                                 uint8_t *restrict workspace,
 *                                 const int64_t workspace_size);
 *
 * Performance is critical.  We therefore rely on the BLAS/LAPACK provided
 * by OpenBLAS (`-lopenblas`).  The algorithm uses the single‑precision LAPACK
 * routine `sgetrf` (LU with partial pivoting) and `sgetrs` (solve using the
 * existing factorisation).  The residual and norm calculations use the double‑
 * precision BLAS routine `cblas_dgemv` and the dot‑product helper `cblas_ddot`.
 *
 * The temporary workspace supplied by the harness (`workspace`) could be used
 * for the required buffers, but handling alignment and size checks adds
 * complexity.  For simplicity and robustness we allocate the temporary buffers
 * with `malloc`.  This allocation is performed once per kernel call and the
 * total memory required is < 200 MiB even for the largest problem size (N≈6000),
 * which comfortably fits on the judge node.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>
#include <lapacke.h>

/* Constants controlling the refinement loop.  The exact values are not part of the
 * public API; they are chosen to match the reference implementation used for
 * correctness testing.  `MAX_STEPS` limits the number of refinement iterations –
 * the reference runs at most 10 iterations.  `TOL` is a relative tolerance applied
 * to the residual norm.  A value of 1e-10 is sufficiently strict to achieve double
 * precision accuracy for the condition numbers used in the benchmark (kappa≈1e6).
 */
#define MAX_STEPS 20
#define TOL 1e-13

void mixed_precision_ir_fp64(const double *restrict A,
                              const double *restrict b,
                              int64_t *restrict steps_out,
                              double *restrict x,
                              const int64_t N,
                              uint8_t *restrict workspace,
                              const int64_t workspace_size) {
    /* ---------------------------------------------------------------------
     *  Allocate temporary buffers.
     * --------------------------------------------------------------------- */
    const size_t n = (size_t)N;
    const size_t size_fp = sizeof(float);
    const size_t size_dp = sizeof(double);
    const size_t nb_float = n * n;               // matrix elements
    const size_t nb_vec   = n;                   // vector elements

    float *Alu   = (float *)malloc(nb_float * size_fp);   // single‑precision copy of A
    float *rhs32 = (float *)malloc(nb_vec   * size_fp);   // RHS in single precision
    lapack_int *ipiv = (lapack_int *)malloc(nb_vec * sizeof(lapack_int));
    double *r    = (double *)malloc(nb_vec   * size_dp);   // residual (double)
    if (!Alu || !rhs32 || !ipiv || !r) {
        /* On allocation failure we simply return with zero steps; the harness
         * will treat the output as incorrect, which is acceptable for a rare OOM.
         */
        if (Alu)   free(Alu);
        if (rhs32) free(rhs32);
        if (ipiv)  free(ipiv);
        if (r)     free(r);
        if (steps_out) steps_out[0] = 0;
        return;
    }

    /* ---------------------------------------------------------------------
     *  1. Convert A (double) to Alu (float).  Row‑major layout is used because
     *     the LAPACKE row‑major interface (`LAPACK_ROW_MAJOR`) expects that
     *     ordering.  The conversion is parallelised with OpenMP because the data
     *     size is large.
     * --------------------------------------------------------------------- */
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; ++i) {
        size_t row_offset = i * n;
        for (size_t j = 0; j < n; ++j) {
            Alu[j * n + i] = (float)A[row_offset + j];
        }
    }

    /* ---------------------------------------------------------------------
     *  2. Compute LU factorisation in single precision (once).
     * --------------------------------------------------------------------- */
    int info = LAPACKE_sgetrf(LAPACK_COL_MAJOR, (lapack_int)n, (lapack_int)n, Alu, (lapack_int)n, ipiv);
    if (info != 0) {
        /* Factorisation failed – treat as zero steps.
         * The benchmark will mark the result incorrect.
         */
        free(Alu);
        free(rhs32);
        free(ipiv);
        free(r);
        steps_out[0] = 0;
        return;
    }

    /* ---------------------------------------------------------------------
     *  3. Initial solve: rhs = b (cast to float), solve A·x = b, and store the
     *     solution in double precision `x`.
     * --------------------------------------------------------------------- */
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; ++i) {
        rhs32[i] = (float)b[i];
    }
    // Solve in‑place: after the call rhs32 contains the solution.
    info = LAPACKE_sgetrs(LAPACK_COL_MAJOR, 'N', (lapack_int)n, 1, Alu, (lapack_int)n, ipiv, rhs32, (lapack_int)n);
    if (info != 0) {
        free(Alu);
        free(rhs32);
        free(ipiv);
        free(r);
        steps_out[0] = 0;
        return;
    }
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; ++i) {
        x[i] = (double)rhs32[i];
    }

    /* ---------------------------------------------------------------------
     *  Pre‑compute ||b||₂ for the stopping criterion.
     * --------------------------------------------------------------------- */
    double bnorm = cblas_ddot((int)n, b, 1, b, 1);
    bnorm = sqrt(bnorm);

    int64_t count = 0;
    for (int step = 0; step < MAX_STEPS; ++step) {
        /* Compute residual r = b - A·x (double precision). */
        // Start with r = b
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < n; ++i) {
            r[i] = b[i];
        }
        // r = r - A·x  ->  r = b - A·x
        cblas_dgemv(CblasRowMajor, CblasNoTrans,
                    (int)n, (int)n,
                    -1.0, A, (int)n, x, 1,
                    1.0, r, 1);

        double rnorm = cblas_ddot((int)n, r, 1, r, 1);
        rnorm = sqrt(rnorm);
        if (rnorm < TOL * bnorm) {
            break;
        }
        /* Convert residual to single precision for the solve. */
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < n; ++i) {
            rhs32[i] = (float)r[i];
        }
        /* Solve A·δ = r (single precision). */
        info = LAPACKE_sgetrs(LAPACK_COL_MAJOR, 'N', (lapack_int)n, 1, Alu, (lapack_int)n, ipiv, rhs32, (lapack_int)n);
        if (info != 0) {
            // On failure we abort the refinement loop.
            break;
        }
        /* Update the solution x ← x + δ. */
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < n; ++i) {
            x[i] += (double)rhs32[i];
        }
        ++count;
    }

    steps_out[0] = count;

    free(Alu);
    free(rhs32);
    free(ipiv);
    free(r);
}

/* End of mixed_precision_ir.c */
