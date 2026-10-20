/* Level-scheduled sparse lower-triangular solve: x = L^-1 b.
 * Rows within one level are independent; levels run in order. */
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
    const double *b,
    const int64_t *level_ptr,
    const int64_t *perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)NNZ_L; (void)workspace; (void)workspace_size;

    /* last non-empty level: first lvl with level_ptr[lvl] == N (monotone prefix sums) */
    int64_t hi = N;
    while (hi > 0 && level_ptr[hi] == level_ptr[hi - 1]) hi--;

    const int64_t nlv = hi; /* levels 0 .. nlv-1 non-empty (level_ptr[nlv] == level_ptr[hi]) */

    #pragma omp parallel
    {
        for (int64_t lvl = 0; lvl < nlv; lvl++) {
            const int64_t lo = level_ptr[lvl];
            const int64_t hi2 = level_ptr[lvl + 1];
            #pragma omp for schedule(static)
            for (int64_t idx = lo; idx < hi2; idx++) {
                const int64_t row = perm[idx];
                const int64_t p = L_indptr[row];
                const int64_t e = L_indptr[row + 1];
                double s = b[row];
                double diag = 0.0;
                for (int64_t k = p; k < e; k++) {
                    const int64_t col = L_indices[k];
                    if (col < row) s -= L_data[k] * x[col];
                    else if (col == row) diag = L_data[k];
                }
                x[row] = s / diag;
            }
        }
    }
}
