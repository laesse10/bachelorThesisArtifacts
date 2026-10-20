// Copyright 2026 ETH Zurich and the HPCAgent-Bench authors.
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Mixed-precision iterative refinement kernel for double-precision linear systems.
 *
 * The algorithm follows the reference implementation in mixed_precision_ir_numpy.py:
 *   1. Copy the matrix A (double) to a single-precision copy Alu and factor it once
 *      using LU decomposition with partial pivoting (single precision).
 *   2. Solve the system A_fp32 * x_fp32 = b_fp32 to obtain an initial solution x.
 *   3. Iteratively refine the solution in double precision:
 *        r = b - A * x                (double precision residual)
 *        if ||r|| < tol * ||b|| break
 *        Solve Alu * delta = r (converted to float) to obtain a correction term.
 *        x = x + delta (cast delta back to double)
 *   4. Store the number of refinement steps performed in steps_out[0].
 *
 * The kernel receives its inputs via the signature described in signature.json.
 * It may use the provided workspace buffer for temporary storage. If the workspace is
 * not large enough, the kernel allocates the required memory dynamically.
 *
 * The implementation uses LAPACK routines sgetrf (LU factorization) and sgetrs (solve)
 * from OpenBLAS, which are multi-threaded and provide good performance.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* LAPACK prototypes (Fortran symbols with trailing underscores). */
extern void sgetrf_(int *m, int *n, float *a, int *lda, int *ipiv, int *info);
extern void sgetrs_(char *trans, int *n, int *nrhs, const float *a, int *lda,
                     const int *ipiv, float *b, int *ldb, int *info);

static void solve_fp32(const float *A_lu, const int *pivot, float *rhs_buf, int n_dim) {
    char trans = 'N';
    int nrhs = 1;
    int ldb = n_dim;
    int info_s = 0;
    sgetrs_(&trans, &n_dim, &nrhs, A_lu, &n_dim, pivot, rhs_buf, &ldb, &info_s);
    (void)info_s;
}

void mixed_precision_ir_fp64(const double *restrict A,
                             const double *restrict b,
                             int64_t *restrict steps_out,
                             double *restrict x,
                             const int64_t N,
                             uint8_t *restrict workspace,
                             const int64_t workspace_size) {
    // Early exit for degenerate sizes.
    if (N <= 0) {
        if (steps_out) steps_out[0] = 0;
        return;
    }

    // Constants for the iterative refinement.
    const int max_steps = 10;          // Upper bound on refinement iterations.
    const double tol = 1e-12;         // Relative tolerance for the residual norm.

    size_t n = (size_t)N;
    size_t mat_elems = n * n;
    size_t alu_bytes = mat_elems * sizeof(float);
    size_t piv_bytes = n * sizeof(int);
    size_t work_bytes = n * sizeof(float);
    size_t required = alu_bytes + piv_bytes + work_bytes;

    // Allocate temporary buffers.
    float *Alu = NULL;   // Single-precision copy of A in column-major order.
    int *piv = NULL;     // Pivot indices for LAPACK.
    float *buf = NULL;   // Buffer used for RHS and solution (float).
    bool allocated = false;

    if (workspace && (size_t)workspace_size >= required) {
        // Use the provided workspace.
        uint8_t *ptr = workspace;
        Alu = (float *)ptr;
        ptr += alu_bytes;
        piv = (int *)ptr;
        ptr += piv_bytes;
        buf = (float *)ptr;
    } else {
        // Allocate manually.
        Alu = (float *)malloc(alu_bytes);
        piv = (int *)malloc(piv_bytes);
        buf = (float *)malloc(work_bytes);
        allocated = true;
    }

    if (!Alu || !piv || !buf) {
        // Allocation failure – abort.
        if (steps_out) steps_out[0] = -1;
        if (allocated) {
            free(Alu);
            free(piv);
            free(buf);
        }
        return;
    }

    // ---------------------------------------------------------------------
    // 1) Copy A (double, row-major) to Alu (float, column-major).
    // ---------------------------------------------------------------------
    #pragma omp parallel for schedule(static)
    for (size_t j = 0; j < n; ++j) {
        // Column j of Alu is contiguous.
        float *col = Alu + j * n;
        const double *src = A + j; // first element of column j in row-major layout.
        for (size_t i = 0; i < n; ++i) {
            col[i] = (float)src[i * n];
        }
    }

    // ---------------------------------------------------------------------
    // 2) LU factorization of Alu (single precision) using LAPACK sgetrf.
    // ---------------------------------------------------------------------
    int info = 0;
    int n_int = (int)N;
    sgetrf_(&n_int, &n_int, Alu, &n_int, piv, &info);
    if (info != 0) {
        // Factorization failed – abort.
        if (steps_out) steps_out[0] = -1;
        if (allocated) {
            free(Alu);
            free(piv);
            free(buf);
        }
        return;
    }

    // solve_fp32 defined at file scope
    // ---------------------------------------------------------------------
    // 3) Initial solve: x = solve_fp32(b).
    // ---------------------------------------------------------------------
    // Copy b (double) to buf (float).
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; ++i) {
        buf[i] = (float)b[i];
    }
    solve_fp32(Alu, piv, buf, n_int);
    // Copy solution back to double vector x.
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; ++i) {
        x[i] = (double)buf[i];
    }

    // Compute norm of b (||b||_2) once.
    double bnorm2 = 0.0;
    #pragma omp parallel for reduction(+:bnorm2) schedule(static)
    for (size_t i = 0; i < n; ++i) {
        bnorm2 += b[i] * b[i];
    }
    double bnorm = sqrt(bnorm2);

    int64_t steps = 0;
    // ---------------------------------------------------------------------
    // 4) Iterative refinement loop.
    // ---------------------------------------------------------------------
    for (int iter = 0; iter < max_steps; ++iter) {
        // Compute residual r = b - A * x (double precision) and its norm.
        double rnorm2 = 0.0;
        // Compute each component of r, store as float in buf for the next solve.
        #pragma omp parallel for reduction(+:rnorm2) schedule(static)
        for (size_t i = 0; i < n; ++i) {
            double sum = 0.0;
            const double *row = A + i * n;
            for (size_t j = 0; j < n; ++j) {
                sum += row[j] * x[j];
            }
            double ri = b[i] - sum;
            rnorm2 += ri * ri;
            buf[i] = (float)ri; // reuse buf for RHS of correction solve.
        }
        double rnorm = sqrt(rnorm2);
        if (rnorm < tol * bnorm) {
            break;
        }
        // Solve correction delta = Alu^{-1} * r (single precision).
        solve_fp32(Alu, piv, buf, n_int);
        // Update solution: x = x + delta (delta in buf as float).
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < n; ++i) {
            x[i] += (double)buf[i];
        }
        ++steps;
    }
    if (steps_out) {
        steps_out[0] = steps;
    }

    if (allocated) {
        free(Alu);
        free(piv);
        free(buf);
    }
    // End of kernel.
}
