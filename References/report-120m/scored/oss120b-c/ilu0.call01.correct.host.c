// Optimized ILU(0) implementation in C
// Reference algorithm derived from ilu0_numpy.py.
// The workspace buffer is used to store three auxiliary arrays:
//   diag    : double[N]    (diagonal entries of U)
//   row_val : double[N]    (dense scratch row for current i)
//   in_row  : int8_t[N]   (boolean marker for columns present in row i)
// The buffer layout: [diag][row_val][in_row].
// The kernel computes the in-place ILU(0) factor of a CSR matrix (A_data).
// The factor retains the original sparsity pattern.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>

void ilu0_fp64(
    double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t N,
    const int64_t nnz,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    // Compute required workspace size.
    const int64_t diag_bytes = N * (int64_t)sizeof(double);
    const int64_t row_val_bytes = N * (int64_t)sizeof(double);
    const int64_t in_row_bytes = N * (int64_t)sizeof(int8_t);
    const int64_t needed_bytes = diag_bytes + row_val_bytes + in_row_bytes;
    // If the provided workspace is insufficient, allocate on the heap.
    double *diag;
    double *row_val;
    int8_t *in_row;
    if ((int64_t)workspace_size >= needed_bytes && workspace != NULL) {
        diag    = (double *) (workspace);
        row_val = (double *) (workspace + diag_bytes);
        in_row  = (int8_t *) (workspace + diag_bytes + row_val_bytes);
        // Initialize in_row to zeros once.
        memset(in_row, 0, (size_t)in_row_bytes);
    } else {
        // Fallback allocation (should not happen in the benchmark environment).
        diag    = (double *)malloc(diag_bytes);
        row_val = (double *)malloc(row_val_bytes);
        in_row = (int8_t *)malloc(in_row_bytes);
        if (!diag || !row_val || !in_row) {
            // Allocation failed – abort.
            return;
        }
        memset(in_row, 0, (size_t)in_row_bytes);
    }

    // Main sequential loop over rows (cannot be parallel due to data dependencies).
    for (int64_t i = 0; i < N; ++i) {
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];

        // --- Scatter current row into dense workspace.
        for (int64_t k = row_start; k < row_end; ++k) {
            const int64_t col = A_indices[k];
            row_val[col] = A_data[k];
            in_row[col] = 1; // mark column as present in this row
        }

        // --- Eliminate against already factored rows (k < i) that appear in the pattern.
        for (int64_t k = row_start; k < row_end; ++k) {
            const int64_t col_k = A_indices[k];
            if (col_k >= i) {
                continue; // only columns left of the diagonal participate in elimination
            }
            // Compute multiplier l_ik = row_val[col_k] / diag[col_k]
            const double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik; // store L entry back into the row

            // Update remaining entries of the current row.
            const int64_t k_start = A_indptr[col_k];
            const int64_t k_end   = A_indptr[col_k + 1];
            for (int64_t m = k_start; m < k_end; ++m) {
                const int64_t col_j = A_indices[m];
                // Update only if column j is right of col_k and present in row i.
                if (col_j > col_k && in_row[col_j]) {
                    row_val[col_j] -= l_ik * A_data[m];
                }
            }
        }

        // Diagonal entry of U for this row.
        diag[i] = row_val[i];

        // --- Gather the factored row back into CSR data and clear markers.
        for (int64_t k = row_start; k < row_end; ++k) {
            const int64_t col = A_indices[k];
            A_data[k] = row_val[col];
            in_row[col] = 0; // reset marker for next row
        }
    }

    // If we allocated on the heap, free the temporary buffers.
    if ((int64_t)workspace_size < needed_bytes || workspace == NULL) {
        free(diag);
        free(row_val);
        free(in_row);
    }
}

