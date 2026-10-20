/* sparse_cholesky: numeric factorization (up-looking column Cholesky over a
 * precomputed pattern) + triangular solves. Direct port of the NumPy reference. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void solve(const double *restrict Lc_data, const int64_t *restrict Lc_indices,
                  const int64_t *restrict Lc_indptr, const double *restrict b,
                  double *restrict y, int64_t N) {
    for (int64_t j = 0; j < N; j++) y[j] = b[j];
    for (int64_t j = 0; j < N; j++) {
        double ljj = Lc_data[Lc_indptr[j]];
        y[j] = y[j] / ljj;
        const double yj = y[j];
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++) {
            y[Lc_indices[p]] -= Lc_data[p] * yj;
        }
    }
    for (int64_t jj = 0; jj < N; jj++) {
        int64_t col = N - 1 - jj;
        double ljj = Lc_data[Lc_indptr[col]];
        double s = y[col];
        for (int64_t p = Lc_indptr[col] + 1; p < Lc_indptr[col + 1]; p++) {
            s -= Lc_data[p] * y[Lc_indices[p]];
        }
        y[col] = s / ljj;
    }
}

void sparse_cholesky_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t *L_indices,
    const int64_t *L_indptr,
    const int64_t *L_to_Lc,
    double *restrict Lc_data,
    const int64_t *Lc_indices,
    const int64_t *Lc_indptr,
    const double *restrict b,
    double *restrict y,
    const int64_t EDGE,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    int64_t N = EDGE * EDGE * EDGE;
    double *work = (double *)malloc((size_t)N * sizeof(double));
    memset(work, 0, (size_t)N * sizeof(double));

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
    solve(Lc_data, Lc_indices, Lc_indptr, b, y, N);
    free(work);
}
