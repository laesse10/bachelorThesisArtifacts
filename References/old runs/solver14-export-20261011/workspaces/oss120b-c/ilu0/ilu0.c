// Copyright 2026 ETH Zurich and the HPCAgent-Bench authors.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * Optimized ILU(0) factorization for a CSR matrix.
 *
 * The reference algorithm (see ilu0_numpy.py) performs the factorization in three steps:
 *   1. Scatter the current row into a dense temporary row (row_val) and mark the
 *      columns present in this row (in_row).
 *   2. For each non‑zero column k < i in the current row, compute the multiplier
 *      L(i,k) = row_val[k] / diag[k] and use it to update the remaining entries
 *      of the row that also appear in row k (the pattern of A).
 *   3. Store the diagonal entry into diag[i] and write the completed row back
 *      into the CSR data array.
 *
 * The algorithm is inherently sequential in the outer row loop because each
 * row depends on all previous rows.  The implementation therefore does not
 * parallelise the outer loop, but it avoids per‑row allocations by reusing a
 * workspace buffer supplied by the benchmark harness.
 */

void ilu0_fp64(
    double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t N,
    const int64_t nnz,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    /* Allocate temporary arrays: diag (double[N]), row_val (double[N]),
       in_row (uint8_t[N]).  Use the provided workspace if it is large enough;
       otherwise fall back to malloc. */
    const int64_t bytes_diag = N * (int64_t)sizeof(double);
    const int64_t bytes_row  = N * (int64_t)sizeof(double);
    const int64_t bytes_mark = N * (int64_t)sizeof(uint8_t);
    const int64_t needed = bytes_diag + bytes_row + bytes_mark;

    double *diag = NULL;
    double *row_val = NULL;
    uint8_t *in_row = NULL;
    int allocated = 0;

    if (workspace != NULL && workspace_size >= needed) {
        diag    = (double *)(workspace);
        row_val = (double *)(workspace + bytes_diag);
        in_row  = (uint8_t *)(workspace + bytes_diag + bytes_row);
    } else {
        diag    = (double *)malloc((size_t)bytes_diag);
        row_val = (double *)malloc((size_t)bytes_row);
        in_row  = (uint8_t *)malloc((size_t)bytes_mark);
        allocated = 1;
    }

    // Initialise diag and the marker array to zero.
    for (int64_t i = 0; i < N; ++i) {
        diag[i] = 0.0;
        in_row[i] = 0;
    }

    // Main ILU(0) loop – sequential over rows.
    for (int64_t i = 0; i < N; ++i) {
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];

        // 1) Scatter the current row into the dense temporary storage.
        for (int64_t kk = row_start; kk < row_end; ++kk) {
            const int64_t col = A_indices[kk];
            row_val[col] = A_data[kk];
            in_row[col] = 1;
        }

        // 2) Eliminate using previously factored rows (k < i).
        for (int64_t kk = row_start; kk < row_end; ++kk) {
            const int64_t col_k = A_indices[kk];
            if (col_k >= i) {
                continue; // Upper‑triangular part – nothing to eliminate.
            }
            // Compute L(i,k) = row_val[col_k] / diag[col_k].
            const double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik; // store the multiplier back into L part.

            // Update the remaining columns of row i that also appear in row k.
            const int64_t k_start = A_indptr[col_k];
            const int64_t k_end   = A_indptr[col_k + 1];
            for (int64_t m = k_start; m < k_end; ++m) {
                const int64_t col_j = A_indices[m];
                // Only update entries that are present in the current row (in_row).
                if (col_j > col_k && in_row[col_j]) {
                    row_val[col_j] -= l_ik * A_data[m];
                }
            }
        }

        // Store the diagonal element for future rows.
        diag[i] = row_val[i];

        // 3) Gather the computed values back into the CSR data array and clear markers.
        for (int64_t kk = row_start; kk < row_end; ++kk) {
            const int64_t col = A_indices[kk];
            A_data[kk] = row_val[col];
            in_row[col] = 0;
        }
    }

    // Free any heap allocations performed above.
    if (allocated) {
        free(diag);
        free(row_val);
        free(in_row);
    }
}

