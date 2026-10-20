#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *restrict b,
    const int64_t *restrict level_ptr,
    const int64_t *restrict perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)NNZ_L; (void)workspace; (void)workspace_size;
    if (N <= 0) return;

    int64_t lo = 0, hi = N;
    while (lo < hi) {
        int64_t mid = (lo + hi) >> 1;
        if (level_ptr[mid] >= N) hi = mid; else lo = mid + 1;
    }
    int64_t L = lo;
    if (L < 1) L = 1;
    if (L > N) L = N;

    #pragma omp parallel
    {
        for (int64_t lvl = 0; lvl < L; lvl++) {
            int64_t a = level_ptr[lvl];
            int64_t c = level_ptr[lvl + 1];
            #pragma omp for schedule(static)
            for (int64_t p = a; p < c; p++) {
                int64_t row = perm[p];
                int64_t s0 = L_indptr[row];
                int64_t s1 = L_indptr[row + 1];
                double s = b[row];
                double diag;
                if (s1 > s0 && L_indices[s1 - 1] == row) {
                    diag = L_data[s1 - 1];
                    const double *d = L_data + s0;
                    const int64_t *ic = L_indices + s0;
                    int64_t m = s1 - 1 - s0;
                    for (int64_t k = 0; k < m; k++)
                        s -= d[k] * x[ic[k]];
                } else {
                    diag = 0.0;
                    for (int64_t k = s0; k < s1; k++) {
                        int64_t col = L_indices[k];
                        if (col < row) s -= L_data[k] * x[col];
                        else if (col == row) diag = L_data[k];
                    }
                }
                x[row] = s / diag;
            }
        }
    }
}
