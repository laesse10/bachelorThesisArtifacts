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
    int64_t T = 0;
    for (int64_t l = 0; l < N; l++)
        if (level_ptr[l + 1] != level_ptr[l]) T = l + 1;

    for (int64_t lvl = 0; lvl < T; lvl++) {
        int64_t a = level_ptr[lvl], e = level_ptr[lvl + 1];
        #pragma omp parallel for schedule(static)
        for (int64_t idx = a; idx < e; idx++) {
            int64_t row = perm[idx];
            double s = b[row];
            double diag = 0.0;
            for (int64_t k = L_indptr[row]; k < L_indptr[row + 1]; k++) {
                int64_t col = L_indices[k];
                if (col < row) s -= L_data[k] * x[col];
                else if (col == row) diag = L_data[k];
            }
            x[row] = s / diag;
        }
    }
}
