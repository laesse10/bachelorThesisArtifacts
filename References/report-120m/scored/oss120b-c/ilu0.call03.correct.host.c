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

    // Main sequential loop over rows (cannot be parallel due to dependencies)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];
        const int64_t row_len   = row_end - row_start;

        // Scatter row i into dense workspace using pointer iteration.
        const int64_t *idx_i = &A_indices[row_start];
        const double   *dat_i = &A_data[row_start];
        for (int64_t off = 0; off < row_len; ++off) {
            const int64_t col = idx_i[off];
            row_val[col] = dat_i[off];
            in_row[col] = 1;
        }

        // Eliminate using previously factored rows.
        for (int64_t off = 0; off < row_len; ++off) {
            const int64_t col_k = idx_i[off];
            if (col_k >= i) continue;
            const double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik; // store L entry

            const int64_t k_start = A_indptr[col_k];
            const int64_t k_end   = A_indptr[col_k + 1];
            const int64_t k_len   = k_end - k_start;
            const int64_t *idx_k = &A_indices[k_start];
            const double   *dat_k = &A_data[k_start];
            // Advance to first column greater than col_k (CSR rows are sorted).
            int64_t m = 0;
            while (m < k_len && idx_k[m] <= col_k) ++m;
            for (; m < k_len; ++m) {
                const int64_t col_j = idx_k[m];
                if (in_row[col_j]) {
                    row_val[col_j] -= l_ik * dat_k[m];
                }
            }
        }

        // Store diagonal.
        diag[i] = row_val[i];

        // Gather row back into CSR and clear markers.
        for (int64_t off = 0; off < row_len; ++off) {
            const int64_t col = idx_i[off];
            A_data[row_start + off] = row_val[col];
            in_row[col] = 0;
        }
    }

    // If we allocated on the heap, free the temporary buffers.
    if ((int64_t)workspace_size < needed_bytes || workspace == NULL) {
        free(diag);
        free(row_val);
        free(in_row);
    }
}

