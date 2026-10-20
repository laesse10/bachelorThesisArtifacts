/*
 * Sequential sparse triangular solve (SpTRSV) using level scheduling.
 *
 * This implementation follows the reference algorithm in NumPy: it walks the
 * levels in order and solves each row independently. The diagonal entry is
 * assumed to be the last non‑zero in each CSR row (column indices are sorted).
 *
 * The function signature matches the benchmark's expectation. Unused
 * parameters are explicitly ignored.
 */

#include <stddef.h>
#include <stdint.h>

void sptrsv_level_csr_fp64(
    const double *restrict L_data,          /* CSR L: nonzero values */
    const int64_t *restrict L_indices,      /* CSR L: column indices (ascending) */
    const int64_t *restrict L_indptr,       /* CSR L: row pointers */
    const double *restrict b,               /* RHS vector */
    const int64_t *restrict level_ptr,      /* Level offsets (size N+1) */
    const int64_t *restrict perm,           /* Row permutation in level order */
    double *restrict x,                     /* Solution vector */
    const int64_t N,                        /* Number of rows */
    const int64_t NNZ_L,                    /* Number of nonzeros (unused) */
    uint8_t *restrict workspace,            /* Scratch buffer (unused) */
    const int64_t workspace_size)           /* Scratch size (unused) */
{
    (void)NNZ_L;        /* Unused parameters are ignored */
    (void)workspace;
    (void)workspace_size;

    for (int64_t lvl = 0; lvl < N; ++lvl) {
        int64_t lvl_start = level_ptr[lvl];
        int64_t lvl_end   = level_ptr[lvl + 1];
        for (int64_t idx = lvl_start; idx < lvl_end; ++idx) {
            int64_t row = perm[idx];
            double sum = b[row];

            int64_t row_begin = L_indptr[row];
            int64_t row_end   = L_indptr[row + 1];
            /* Diagonal entry is the last entry because column indices are sorted */
            int64_t diag_pos = row_end - 1;
            double diag = L_data[diag_pos];

            for (int64_t k = row_begin; k < diag_pos; ++k) {
                sum -= L_data[k] * x[L_indices[k]];
            }
            x[row] = sum / diag;
        }
    }
}

