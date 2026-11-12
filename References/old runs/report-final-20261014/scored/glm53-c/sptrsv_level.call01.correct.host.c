#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *b,
    const int64_t *restrict level_ptr,
    const int64_t *restrict perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    /* number of levels: level_ptr is nondecreasing, level_ptr[N] == N,
       level_ptr[lvl] < N for lvl < nlev */
    int64_t nlev = 0;
    if (N > 0) {
        int64_t lo = 0, hi = N;
        while (hi - lo > 1) {
            int64_t mid = lo + (hi - lo) / 2;
            if (level_ptr[mid] < N) lo = mid; else hi = mid;
        }
        nlev = lo + 1;
    }

#pragma omp parallel
    {
        for (int64_t lvl = 0; lvl < nlev; lvl++) {
            const int64_t start = level_ptr[lvl], end = level_ptr[lvl + 1];
#pragma omp for schedule(static)
            for (int64_t idx = start; idx < end; idx++) {
                const int64_t row = perm[idx];
                double s = b[row];
                double diag = 0.0;
                const int64_t ks = L_indptr[row], ke = L_indptr[row + 1];
                for (int64_t k = ks; k < ke; k++) {
                    const int64_t col = L_indices[k];
                    if (col < row) {
                        s -= L_data[k] * x[col];
                    } else if (col == row) {
                        diag = L_data[k];
                    } else {
                        break;
                    }
                }
                x[row] = s / diag;
            }
        }
    }
}
