// SPDX-License-Identifier: GPL-3.0-or-later
// Optimized level-scheduled sparse triangular solve (forward substitution) for CSR lower triangular matrix.
// Target: C, compiled with -O3 -march=native -fopenmp.

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// Forward substitution with level scheduling.
// L_data   : non-zero values of L in CSR format (size = NNZ_L).
// L_indices: column indices of each non-zero (size = NNZ_L), 0‑based.
// L_indptr : row pointer for CSR (size = N+1).
// b        : right‑hand side vector (size = N).
// level_ptr: marks start of each level in the perm array (size = N+1). level l contains rows
//            with indices perm[level_ptr[l] .. level_ptr[l+1]-1].
// perm     : permutation of rows in level order (size = N).
// x        : output vector (size = N). Must be writable.
// N        : number of rows (and columns) of L.
// NNZ_L    : number of non‑zeros in L (unused inside the kernel).
// workspace, workspace_size: unused, kept for API compatibility.
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
    // OpenMP parallel region: each level is processed sequentially, but rows within a level are
    // independent and can be executed in parallel.
    #pragma omp parallel
    {
        // Process each level sequentially; rows within a level are independent.
        for (int64_t lvl = 0; lvl < N; ++lvl) {
            // Parallel work sharing for rows belonging to this level.
            #pragma omp for schedule(static)
            for (int64_t idx = level_ptr[lvl]; idx < level_ptr[lvl + 1]; ++idx) {
                int64_t row = perm[idx];
                double sum = b[row];
                double diag = 0.0;
                // Iterate over non‑zeros of this row.
                for (int64_t k = L_indptr[row]; k < L_indptr[row + 1]; ++k) {
                    int64_t col = L_indices[k];
                    if (col < row) {
                        sum -= L_data[k] * x[col];
                    } else if (col == row) {
                        diag = L_data[k];
                    }
                }
                // Diagonal entry must be present for a proper triangular solve.
                x[row] = sum / diag;
            }
            // Implicit barrier after the omp for ensures all rows of the current level are
            // finished before proceeding to the next level.
        }
    }
    // workspace is unused – the function signature requires it for compatibility.
    (void)workspace;
    (void)workspace_size;
    (void)NNZ_L;
}
