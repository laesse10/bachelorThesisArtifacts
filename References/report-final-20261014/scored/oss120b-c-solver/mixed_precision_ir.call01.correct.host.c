#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>
#include <lapacke.h>

/*
 * Mixed-precision iterative refinement.
 *
 * Solve A * x = b for a symmetric (?) dense matrix A (NxN) using a single-precision
 * LU factorization with partial pivoting (performed once). The residual is computed in
 * double precision, and each refinement step solves a correction system in single
 * precision. The process repeats until the residual norm is reduced below a relative
 * tolerance or a fixed maximum number of refinement steps is reached.
 *
 * The harness provides a pre‑allocated workspace buffer that the kernel must use for
 * all temporary allocations. The required workspace size is:
 *   N*N floats            – copy of A in single precision (LU factors)
 *   N   ints (pivot)      – LAPACK pivot array
 *   N   floats (rhs)      – right‑hand side / solution buffer
 *   N   doubles (residual) – double‑precision residual vector
 *
 * The function returns the number of refinement steps performed via `steps_out`.
 */

void mixed_precision_ir_fp64(
    const double *restrict A,
    const double *restrict b,
    int64_t *restrict steps_out,
    double *restrict x,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    /* Constants – chosen to guarantee convergence for the problem range. */
    const int MAX_STEPS = 20;                /* safety bound; convergence occurs far earlier */
    const double TOL = 1e-12;                /* relative residual tolerance */

    const int n = (int)N; /* BLAS / LAPACK expect int */
    if (n <= 0) {
        if (steps_out) steps_out[0] = 0;
        return;
    }

    /* Compute required workspace layout with proper alignment. */
    size_t offset = 0;
    // 1. Alu: N×N single‑precision matrix (row‑major)
    float *Alu = (float *)(workspace + offset);
    offset += (size_t)n * n * sizeof(float);
    // Align to 8‑byte boundary for the next pointer (int may need 4‑byte but double needs 8)
    size_t align = (8 - (offset % 8)) % 8;
    offset += align;
    // 2. ipiv: pivot indices (int32)
    int *ipiv = (int *)(workspace + offset);
    offset += (size_t)n * sizeof(int);
    align = (8 - (offset % 8)) % 8;
    offset += align;
    // 3. rhs: temporary right‑hand side / solution buffer (single precision)
    float *rhs = (float *)(workspace + offset);
    offset += (size_t)n * sizeof(float);
    align = (8 - (offset % 8)) % 8;
    offset += align;
    // 4. r: residual vector (double precision)
    double *r = (double *)(workspace + offset);
    offset += (size_t)n * sizeof(double);

    /* If the provided workspace is too small, fall back to malloc – this should never happen in the benchmark. */
    if ((int64_t)offset > workspace_size) {
        Alu = (float *)malloc((size_t)n * n * sizeof(float));
        ipiv = (int *)malloc((size_t)n * sizeof(int));
        rhs = (float *)malloc((size_t)n * sizeof(float));
        r = (double *)malloc((size_t)n * sizeof(double));
        if (!Alu || !ipiv || !rhs || !r) {
            /* Allocation failure – give up gracefully. */
            if (steps_out) steps_out[0] = 0;
            free(Alu); free(ipiv); free(rhs); free(r);
            return;
        }
    }

    /* ------------------------------------------------------------------
     * 1. Copy A (double) into Alu (single) and factorise in place.
     * ------------------------------------------------------------------ */
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < (size_t)n * n; ++i) {
        Alu[i] = (float)A[i];
    }

    /* LU factorisation (partial pivoting) in single precision using LAPACK. */
    int info = LAPACKE_sgetrf(LAPACK_ROW_MAJOR, n, n, Alu, n, ipiv);
    if (info != 0) {
        /* Factorisation failed – abort early. */
        if (steps_out) steps_out[0] = 0;
        if ((int64_t)offset > workspace_size) {
            free(Alu); free(ipiv); free(rhs); free(r);
        }
        return;
    }

    /* ------------------------------------------------------------------
     * 2. Initial solve A x = b using the single‑precision factors.
     * ------------------------------------------------------------------ */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        rhs[i] = (float)b[i];
    }
    info = LAPACKE_sgetrs(LAPACK_ROW_MAJOR, 'N', n, 1, Alu, n, ipiv, rhs, 1);
    if (info != 0) {
        if (steps_out) steps_out[0] = 0;
        if ((int64_t)offset > workspace_size) {
            free(Alu); free(ipiv); free(rhs); free(r);
        }
        return;
    }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        x[i] = (double)rhs[i];
    }

    /* Pre‑compute the norm of b (used for the relative stopping criterion). */
    double bnorm = sqrt(cblas_ddot(n, b, 1, b, 1));
    if (bnorm == 0.0) bnorm = 1.0; /* avoid division by zero */

    int64_t count = 0;
    for (int step = 0; step < MAX_STEPS; ++step) {
        /* Compute the double‑precision residual r = b - A x. */
        cblas_dcopy(n, b, 1, r, 1);
        cblas_dgemv(CblasRowMajor, CblasNoTrans, n, n,
                    -1.0, A, n, x, 1,
                    1.0, r, 1);
        double rnorm = sqrt(cblas_ddot(n, r, 1, r, 1));
        if (rnorm <= TOL * bnorm) {
            break;
        }
        /* Convert residual to single precision and solve for the correction. */
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < n; ++i) {
            rhs[i] = (float)r[i];
        }
        info = LAPACKE_sgetrs(LAPACK_ROW_MAJOR, 'N', n, 1, Alu, n, ipiv, rhs, 1);
        if (info != 0) {
            /* Solver failure – abort. */
            break;
        }
        /* Update the solution x = x + delta (delta is in single precision). */
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < n; ++i) {
            x[i] += (double)rhs[i];
        }
        ++count;
    }

    if (steps_out) {
        steps_out[0] = count;
    }

    if ((int64_t)offset > workspace_size) {
        free(Alu);
        free(ipiv);
        free(rhs);
        free(r);
    }
}

