#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct { int32_t ustart; int32_t uend; } uinfo_t;

/* variants: bit0 = drop inner update loop; bit1 = multiply by reciprocal; bit2 = full inner loop over whole pivot row (no ustart skip) */
void ilu0_exp(double *restrict A_data, const int64_t *restrict A_indices,
              const int64_t *restrict A_indptr, const int64_t N,
              uinfo_t *restrict uinfo, double *restrict diag,
              uint32_t *restrict pos, double *restrict local)
{
    const int noinner = getenv("ILU0_EXP") ? (atoi(getenv("ILU0_EXP")) & 1) : 0;
    const int recip   = getenv("ILU0_EXP") ? (atoi(getenv("ILU0_EXP")) & 2) : 0;
    const int fullrow = getenv("ILU0_EXP") ? (atoi(getenv("ILU0_EXP")) & 4) : 0;
    for (int64_t i = 0; i < N; i++) {
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        const int64_t deg = re - rs;
        const int64_t *restrict rowi = A_indices + rs;
        double *restrict rowd = A_data + rs;
        int64_t diagpos = -1;
        for (int64_t j = 0; j < deg; j++) {
            pos[rowi[j]] = (uint32_t)j;
            local[j] = rowd[j];
            diagpos = (rowi[j] == i) ? j : diagpos;
        }
        for (int64_t k = 0; k < deg; k++) {
            const int64_t col_k = rowi[k];
            if (col_k >= i) break;
            double l_ik;
            if (recip) l_ik = local[k] * (1.0 / diag[col_k]);
            else l_ik = local[k] / diag[col_k];
            local[k] = l_ik;
            const uinfo_t ui = uinfo[col_k];
            if (fullrow) {
                for (int64_t m = A_indptr[col_k]; m < A_indptr[col_k + 1]; m++) {
                    const int64_t col_j = A_indices[m];
                    if (col_j > col_k) {
                        const uint32_t p = pos[col_j];
                        if (p < (uint32_t)deg && rowi[p] == col_j)
                            local[p] -= l_ik * A_data[m];
                    }
                }
            } else if (!noinner) {
                for (int64_t m = ui.ustart; m < ui.uend; m++) {
                    const int64_t col_j = A_indices[m];
                    const uint32_t p = pos[col_j];
                    if (p < (uint32_t)deg && rowi[p] == col_j)
                        local[p] -= l_ik * A_data[m];
                }
            }
        }
        diag[i] = local[diagpos];
        for (int64_t j = 0; j < deg; j++) rowd[j] = local[j];
    }
    (void)no_inner_placeholder_unused; 
}
