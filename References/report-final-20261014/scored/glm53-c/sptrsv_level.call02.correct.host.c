#include <stdint.h>
#include <stddef.h>
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
    (void)level_ptr; (void)perm; (void)NNZ_L; (void)workspace; (void)workspace_size;
    for (int64_t r = 0; r < N; r++) {
        const int64_t ks = L_indptr[r], ke = L_indptr[r + 1];
        if (ke > ks && L_indices[ke - 1] == r) {
            /* ascending columns + last diagonal is the last entry entry of the row */
            double s = b[r];
            const int64_t klast = ke - 1;
            for (int64_t k = ks; k < klast; k++) s -= L_data[k] * x[L_indices[k]];
            x[r] = s / L_data[klast];
        } else {
            double s = b[r];
            double diag = 0.0;
            for (int64_t k = ks; k < ke; k++) {
                const int64_t col = L_indices[k];
                if (col < r) s -= L_data[k] * x[col];
                else if (col == r) diag = L_data[k];
                else break;
            }
            x[r] = s / diag;
        }
    }
}
