/* Faithful C translation of the NumPy reference (ilu0_numpy.py). */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size) {
    (void)nnz; (void)workspace_size;
    double *diag = (double *)workspace;
    double *row_val = (double *)(workspace + 8 * (size_t)N);
    int64_t *in_row = (int64_t *)(workspace + 16 * (size_t)N);

    for (int64_t i = 0; i < N; i++) {
        int64_t row_start = A_indptr[i], row_end = A_indptr[i + 1];
        for (int64_t k = row_start; k < row_end; k++) {
            int64_t col = A_indices[k];
            row_val[col] = A_data[k];
            in_row[col] = 1;
        }
        for (int64_t k = row_start; k < row_end; k++) {
            int64_t col_k = A_indices[k];
            if (col_k >= i) continue;
            double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik;
            for (int64_t m = A_indptr[col_k]; m < A_indptr[col_k + 1]; m++) {
                int64_t col_j = A_indices[m];
                if (col_j > col_k && in_row[col_j] == 1) {
                    row_val[col_j] = row_val[col_j] - l_ik * A_data[m];
                }
            }
        }
        diag[i] = row_val[i];
        for (int64_t k = row_start; k < row_end; k++) {
            int64_t col = A_indices[k];
            A_data[k] = row_val[col];
            in_row[col] = 0;
        }
    }
}
