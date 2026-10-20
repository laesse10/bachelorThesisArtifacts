/*
 * Sequential implementation of the level‑scheduled sparse triangular solve (SpTRSV).
 *
 * This version follows the reference algorithm but removes the OpenMP parallelisation
 * which caused a large overhead on the given matrix (many levels, few rows per level).
 * It also takes advantage of the CSR invariant that the column indices inside a row are
 * sorted in ascending order, so the diagonal entry is the last non‑zero of each row.
 *
 * The algorithm:
 *   for each level:
 *       for each row in that level (given by the permutation array `perm`):
 *           sum = b[row];
 *           diag = last value in the row;
 *           sum -= Σ_{k=row_start}^{diag_idx-1} L_data[k] * x[col];
 *           x[row] = sum / diag;
 *
 * All pointer arguments are marked `restrict` to enable aggressive optimisation by the
 * compiler.  The unused arguments ``NNZ_L``, ``workspace`` and ``workspace_size`` are kept for
 * compatibility with the generic benchmark interface.
 */

#include <stdint.h>
#include <stddef.h>

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
    // Parameters ``NNZ_L``, ``workspace`` and ``workspace_size`` are unused – they are part of the
    // benchmark ABI but not required for the algorithm.
    (void)NNZ_L;
    (void)workspace;
    (void)workspace_size;

    for (int64_t lvl = 0; lvl < N; ++lvl) {
        int64_t start = level_ptr[lvl];
        int64_t end   = level_ptr[lvl + 1];
        for (int64_t idx = start; idx < end; ++idx) {
            int64_t row = perm[idx];
            double sum = b[row];

            int64_t row_start = L_indptr[row];
            int64_t row_end   = L_indptr[row + 1];
            int64_t diag_idx = row_end - 1;               // diagonal is the last entry
            double diag = L_data[diag_idx];

            // Accumulate contributions from already‑solved rows (strictly lower part).
            for (int64_t k = row_start; k < diag_idx; ++k) {
                int64_t col = L_indices[k];
                sum -= L_data[k] * x[col];
            }

            x[row] = sum / diag;
        }
    }
}

