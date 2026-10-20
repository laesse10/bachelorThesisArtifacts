/*
 * Optimized sparse Cholesky factorization and triangular solve.
 *
 * This implementation follows the reference NumPy version (see
 * sparse_cholesky_numpy.py) but rewrites the inner loops for better cache
 * usage and adds limited OpenMP parallelism in the solve phase. The algorithm
 * is an up‑looking column implementation that uses the pre‑computed sparsity
 * pattern (Lc, L) supplied by the symbolic phase.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

/**
 * sparse_cholesky_csr_fp64 – numeric phase of a sparse Cholesky factorisation.
 *
 * Parameters
 * ----------
 * A_data, A_indices, A_indptr – CSR representation of the permuted matrix P·A·Pᵀ.
 * L_indices, L_indptr            – CSR representation of the lower‑triangular
 *                                 pattern of L (structural part only).
 * L_to_Lc                       – Mapping from CSR entries (L) to the CSC
 *                                 storage (Lc) where the actual numeric values
 *                                 are kept.
 * Lc_data, Lc_indices, Lc_indptr – CSC (column‑wise) storage for the numeric
 *                                  values of L.
 * b, y                          – Right‑hand side and solution vectors.
 * EDGE                          – Grid edge length; the matrix dimension is
 *                                 N = EDGE³.
 * workspace, workspace_size      – Temporary workspace of at least N·sizeof(double)
 *                                 bytes. It is used as a double work array.
 */
void sparse_cholesky_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const int64_t *restrict L_to_Lc,
    double *restrict Lc_data,
    const int64_t *restrict Lc_indices,
    const int64_t *restrict Lc_indptr,
    const double *restrict b,
    double *restrict y,
    const int64_t EDGE,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    const int64_t N = EDGE * EDGE * EDGE;

    /* Workspace for temporary column accumulation. Use the provided buffer if it
     * is large enough; otherwise allocate a temporary buffer on the heap. */
    double *restrict work = NULL;
    const int64_t required_bytes = N * (int64_t)sizeof(double);
    bool allocated_here = false;
    if (workspace != NULL && workspace_size >= required_bytes) {
        work = (double *restrict) workspace;
    } else {
        work = (double *restrict) malloc((size_t)required_bytes);
        allocated_here = true;
    }
    if (work == NULL) {
        return; /* out of memory */
    }
    memset(work, 0, (size_t)required_bytes);

    /* -------------------------------------------------------------------- */
    /* Numeric factorisation (up‑looking column algorithm)                 */
    /* -------------------------------------------------------------------- */
    for (int64_t j = 0; j < N; ++j) {
        /* 1) Copy relevant part of A into work (columns >= j). The CSR column
         *    indices are not guaranteed to be sorted after permutation, so we
         *    must test each entry.
         */
        const int64_t a_start = A_indptr[j];
        const int64_t a_end   = A_indptr[j + 1];
        for (int64_t p = a_start; p < a_end; ++p) {
            const int64_t col = A_indices[p];
            if (col >= j) {
                work[col] = A_data[p];
            }
        }

        /* 2) Subtract contributions from previously computed columns k where L[j,k] != 0. */
        const int64_t l_row_start = L_indptr[j];
        const int64_t l_row_end   = L_indptr[j + 1] - 1; /* exclude diagonal */
        for (int64_t p = l_row_start; p < l_row_end; ++p) {
            const int64_t k   = L_indices[p];
            const double ljk  = Lc_data[L_to_Lc[p]]; /* L[j,k] */
            const int64_t col_start = Lc_indptr[k];
            const int64_t col_end   = Lc_indptr[k + 1];
            for (int64_t q = col_start; q < col_end; ++q) {
                const int64_t row = Lc_indices[q];
                if (row >= j) {
                    work[row] -= Lc_data[q] * ljk;
                }
            }
        }

        /* 3) Diagonal entry */
        const double diag = work[j];
        work[j] = 0.0;
        const double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj;

        /* 4) Off‑diagonal entries of column j. */
        const int64_t off_start = Lc_indptr[j] + 1;
        const int64_t off_end   = Lc_indptr[j + 1];
        #pragma omp parallel for schedule(static)
        for (int64_t p = off_start; p < off_end; ++p) {
            const int64_t row = Lc_indices[p];
            const double val = work[row] / ljj;
            Lc_data[p] = val;
            work[row] = 0.0;
        }
    }

    /* -------------------------------------------------------------------- */
    /* Forward substitution: solve L * z = b (result stored in y)          */
    /* -------------------------------------------------------------------- */
    for (int64_t i = 0; i < N; ++i) {
        y[i] = b[i];
    }
    for (int64_t j = 0; j < N; ++j) {
        const double ljj = Lc_data[Lc_indptr[j]];
        const double yj  = y[j] / ljj;
        y[j] = yj;
        const int64_t col_start = Lc_indptr[j] + 1;
        const int64_t col_end   = Lc_indptr[j + 1];
        #pragma omp parallel for schedule(static)
        for (int64_t p = col_start; p < col_end; ++p) {
            const int64_t row = Lc_indices[p];
            y[row] -= Lc_data[p] * yj;
        }
    }

    /* -------------------------------------------------------------------- */
    /* Backward substitution: solve Lᵀ * y = z (y already holds z)          */
    /* -------------------------------------------------------------------- */
    for (int64_t jj = 0; jj < N; ++jj) {
        const int64_t col = N - 1 - jj;
        const double ljj = Lc_data[Lc_indptr[col]];
        double sum = 0.0;
        const int64_t col_start = Lc_indptr[col] + 1;
        const int64_t col_end   = Lc_indptr[col + 1];
        #pragma omp parallel for reduction(+:sum) schedule(static)
        for (int64_t p = col_start; p < col_end; ++p) {
            const int64_t row = Lc_indices[p];
            sum += Lc_data[p] * y[row];
        }
        y[col] = (y[col] - sum) / ljj;
    }

    if (allocated_here) {
        free(work);
    }
}

