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
    (void)workspace; (void)workspace_size;
    const int64_t N = EDGE * EDGE * EDGE;
    const int64_t nnzL = Lc_indptr[N];

    double *work = (double *)malloc(N * sizeof(double));
    int32_t *sc_row = (int32_t *)malloc(nnzL * sizeof(int32_t));
    double *sc_val = (double *)malloc(nnzL * sizeof(double));
    int32_t *t_of_p = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int32_t *tstart = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int64_t *cursor = (int64_t *)malloc((N + 1) * sizeof(int64_t));
    memset(work, 0, N * sizeof(double));
    memcpy(cursor, Lc_indptr, (N + 1) * sizeof(int64_t));
    for (int64_t k = 0; k < N; k++) sc_row[Lc_indptr[k]] = (int32_t)k;
    for (int64_t i = 0; i < N; i++) {
        for (int64_t p = L_indptr[i]; p < L_indptr[i + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int64_t t = cursor[k]++;
            sc_row[t] = (int32_t)i;
            t_of_p[L_to_Lc[p]] = (int32_t)t;
            tstart[p] = (int32_t)t;
        }
    }

    for (int64_t j = 0; j < N; j++) {
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
            int64_t row = A_indices[p];
            if (row >= j) work[row] = A_data[p];
        }
        for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int32_t ts = tstart[p];
            double ljk = sc_val[ts];
            int32_t te = (int32_t)Lc_indptr[k + 1];
            for (int32_t t = ts; t < te; t++) {
                work[sc_row[t]] -= sc_val[t] * ljk;
            }
        }
        double diag = work[j];
        work[j] = 0.0;
        double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj;
        sc_val[Lc_indptr[j]] = ljj;
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++) {
            int64_t row = Lc_indices[p];
            double v = work[row] / ljj;
            work[row] = 0.0;
            Lc_data[p] = v;
            sc_val[t_of_p[p]] = v;
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
    free(work); free(sc_row); free(sc_val); free(t_of_p); free(tstart); free(cursor);
}
