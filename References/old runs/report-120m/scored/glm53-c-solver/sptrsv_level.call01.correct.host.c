/* serial reference (measurement probe) */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void sptrsv_level_csr_fp64(
    const double *restrict L_data, const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr, const double *b,
    const int64_t *level_ptr, const int64_t *perm, double *restrict x,
    const int64_t N, const int64_t NNZ_L, uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)NNZ_L; (void)workspace; (void)workspace_size;
    if (N <= 0) return;
    int64_t lo = 0, hi = N;
    while (lo < hi) { int64_t m = lo + (hi-lo)/2; if (level_ptr[m] < N) lo = m+1; else hi = m; }
    const int64_t nlev = lo;
    for (int64_t lvl = 0; lvl < nlev; lvl++) {
        for (int64_t idx = level_ptr[lvl]; idx < level_ptr[lvl+1]; idx++) {
            const int64_t row = perm[idx];
            double s = b[row], diag = 0.0;
            for (int64_t k = L_indptr[row]; k < L_indptr[row+1]; k++) {
                const int64_t col = L_indices[k];
                if (col < row) s -= L_data[k]*x[col];
                else if (col == row) diag = L_data[k];
            }
            x[row] = s/diag;
        }
    }
}
