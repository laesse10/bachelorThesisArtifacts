#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)nnz; (void)workspace_size;
    if (N <= 0) return;

    double *restrict row_val = (double *)workspace;
    uint32_t *restrict mark = (uint32_t *)(workspace + 8 * (size_t)N);
    double *restrict inv_diag = (double *)(workspace + 8 * (size_t)N + 4 * (size_t)N);
    int64_t *upper_start = (int64_t *)(workspace + 8 * (size_t)N + 4 * (size_t)N + 8 * (size_t)N);

    /* check sortedness + precompute upper_start[k] = first p in row k with A_indices[p] > k */
    int sorted = 1;
    for (int64_t k = 0; k < N; k++) {
        int64_t us = A_indptr[k + 1]; /* default: no upper entries */
        int64_t rs = A_indptr[k], re = A_indptr[k + 1];
        for (int64_t p = rs; p < re; p++) {
            if (p > rs && A_indices[p] < A_indices[p - 1]) sorted = 0;
            if (A_indices[p] > k) { us = p; break; }
        }
        upper_start[k] = us;
    }

    for (int64_t i = 0; i < N; i++) {
        uint32_t gen = (uint32_t)(i + 1);
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        for (int64_t p = rs; p < re; p++) { const int64_t c = A_indices[p]; row_val[c] = A_data[p]; mark[c] = gen; }
        for (int64_t p = rs; p < re; p++) {
            const int64_t k = A_indices[p];
            if (k >= i) continue;
            const double l = row_val[k] * inv_diag[k];
            row_val[k] = l;
            if (sorted) {
                const int64_t qs = upper_start[k], qe = A_indptr[k + 1];
                for (int64_t q = qs; q < qe; q++) {
                    const int64_t cj = A_indices[q];
                    if (mark[cj] == gen) row_val[cj] -= l * A_data[q];
                }
            } else {
                const int64_t qs = A_indptr[k], qe = A_indptr[k + 1];
                for (int64_t q = qs; q < qe; q++) {
                    const int64_t cj = A_indices[q];
                    if (cj > k && mark[cj] == gen) row_val[cj] -= l * A_data[q];
                }
            }
        }
        inv_diag[i] = 1.0 / row_val[i];
        for (int64_t p = rs; p < re; p++) { const int64_t c = A_indices[p]; A_data[p] = row_val[c]; }
    }
}
