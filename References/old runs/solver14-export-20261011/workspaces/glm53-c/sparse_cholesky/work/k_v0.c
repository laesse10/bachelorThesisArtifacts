#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

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
    double *work = (double *)malloc(N * sizeof(double));
    memset(work, 0, N * sizeof(double));

    for (int64_t j = 0; j < N; j++) {
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
            int64_t row = A_indices[p];
            if (row >= j) work[row] = A_data[p];
        }
        for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
            int64_t k = L_indices[p];
            double ljk = Lc_data[L_to_Lc[p]];
            for (int64_t q = Lc_indptr[k]; q < Lc_indptr[k + 1]; q++) {
                int64_t row = Lc_indices[q];
                if (row >= j) work[row] -= Lc_data[q] * ljk;
            }
        }
        double diag = work[j];
        work[j] = 0.0;
        double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj;
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++) {
            int64_t row = Lc_indices[p];
            Lc_data[p] = work[row] / ljj;
            work[row] = 0.0;
        }
    }

    for (int64_t j = 0; j < N; j++) y[j] = b[j];
    for (int64_t j = 0; j < N; j++) {
        double ljj = Lc_data[Lc_indptr[j]];
        y[j] = y[j] / ljj;
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++) {
            int64_t row = Lc_indices[p];
            y[row] -= Lc_data[p] * y[j];
        }
    }
    for (int64_t jj = 0; jj < N; jj++) {
        int64_t col = N - 1 - jj;
        double ljj = Lc_data[Lc_indptr[col]];
        double s = y[col];
        for (int64_t p = Lc_indptr[col] + 1; p < Lc_indptr[col + 1]; p++) {
            int64_t row = Lc_indices[p];
            s -= Lc_data[p] * y[row];
        }
        y[col] = s / ljj;
    }
    free(work);
}
