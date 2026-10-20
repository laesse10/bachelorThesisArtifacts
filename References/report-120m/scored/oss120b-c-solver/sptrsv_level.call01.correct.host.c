/*
 * Optimized implementation of the level‑scheduled sparse triangular solve (SpTRSV).
 *
 * The reference implementation in ``sptrsv_level_numpy.py`` follows the algorithm
 *
 *   for lvl in range(N):
 *       for idx in range(level_ptr[lvl], level_ptr[lvl+1]):
 *           row = perm[idx]
 *           s   = b[row]
 *           diag = 0
 *           for k in range(L_indptr[row], L_indptr[row+1]):
 *               col = L_indices[k]
 *               if col < row:   s -= L_data[k] * x[col]
 *               elif col == row: diag = L_data[k]
 *           x[row] = s / diag
 *
 * This version removes the inner ``if`` by exploiting the CSR invariant that the column
 * indices within a row are sorted in ascending order. For a lower‑triangular matrix the
 * diagonal entry (``col == row``) is therefore the *last* entry of each row.  The algorithm
 * becomes:
 *   - treat all entries before the last as strictly off‑diagonal (``col < row``)
 *   - load the diagonal directly from ``L_data[row_end‑1]``
 *
 * The outer level loop is sequential because rows in a later level depend on rows from
 * earlier levels.  The inner row loop within a level, however, is embarrassingly parallel.
 * We use OpenMP to parallelise that loop while preserving the implicit barrier after each
 * ``#pragma omp for`` – this guarantees that the whole level finishes before the next one
 * starts.  The ``workspace`` arguments are unused by the current kernel but are kept in the
 * signature for API compatibility.
 */

#include <stdint.h>
#include <stddef.h>
#include <omp.h>

/*
 * Forward substitution for a lower‑triangular matrix in CSR format using a pre‑computed
 * level schedule.
 *
 * Parameters
 * ----------
 * L_data          – non‑zero values of L (row‑major CSR)
 * L_indices       – column indices for each value in ``L_data`` (0‑based)
 * L_indptr        – row pointer array; entries for row i are in ``[L_indptr[i], L_indptr[i+1])``
 * b               – right‑hand side vector (size N)
 * level_ptr       – start index of each level in the ``perm`` array (size N+1)
 * perm            – permutation of rows such that rows belonging to the same level are contiguous
 * x               – output vector (size N). May be reused as input for subsequent solves.
 * N               – number of rows (and columns) of L
 * NNZ_L           – total number of non‑zeros in L (unused, kept for API compatibility)
 * workspace       – optional temporary buffer (unused here)
 * workspace_size  – size of ``workspace`` in bytes (unused)
 */
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
    // ``workspace`` and ``NNZ_L`` are part of the generic benchmark signature but are not required
    // for this algorithm; they are deliberately ignored.

    #pragma omp parallel
    {
        for (int64_t lvl = 0; lvl < N; ++lvl) {
            int64_t start = level_ptr[lvl];
            int64_t end   = level_ptr[lvl + 1];

            // Parallelise the rows belonging to the current level.
            #pragma omp for schedule(static)
            for (int64_t idx = start; idx < end; ++idx) {
                int64_t row = perm[idx];
                double sum = b[row];

                int64_t row_start = L_indptr[row];
                int64_t row_end   = L_indptr[row + 1];

                // Diagonal is the last entry of the row (CSR stores columns in ascending order).
                int64_t diag_idx = row_end - 1;
                double diag = L_data[diag_idx];

                // Accumulate contributions from already‑computed rows.
                for (int64_t k = row_start; k < diag_idx; ++k) {
                    int64_t col = L_indices[k];
                    sum -= L_data[k] * x[col];
                }

                x[row] = sum / diag;
            }
        }
    }
}

