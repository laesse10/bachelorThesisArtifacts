#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void sparse_cholesky_csr_fp64(
    const double *restrict A_data /* csr A: value of each entry */,
    const int64_t *restrict A_indices /* csr A: COLUMN index of each entry, ascending within a row; 0-based int64 */,
    const int64_t *restrict A_indptr /* csr A: row r's entries are [indptr[r], indptr[r+1]); 0-based int64 */,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const int64_t *restrict L_to_Lc,
    double *restrict Lc_data,
    const int64_t *restrict Lc_indices,
    const int64_t *restrict Lc_indptr,
    const double *restrict b,
    double *restrict y,
    const int64_t EDGE,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    /* Numeric Cholesky factorization and solve for the 7‑point 3‑D Poisson problem.
       The workspace buffer is used as a temporary double array "work" of length N = EDGE³. */
    const int64_t N = EDGE * EDGE * EDGE;
    // Allocate a temporary work array of size N (double). This replaces the optional
    // workspace buffer supplied by the harness. Using a dedicated allocation avoids
    // crashes when the provided workspace is null or too small.
    double *work = (double *)malloc((size_t)N * sizeof(double));
    if (!work) {
        // Allocation failure: abort the kernel.
        return;
    }
    // Initialise the buffer to zero.
    memset(work, 0, (size_t)N * sizeof(double));

    // Factorisation: compute Lc_data (CSC representation of L) column‑wise.
    for (int64_t j = 0; j < N; ++j) {
        // 1) Load the original matrix entries belonging to column j (upper triangle)
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; ++p) {
            int64_t row = A_indices[p];
            if (row >= j) {
                work[row] = A_data[p];
            }
        }
        // 2) Subtract contributions from already computed columns k < j.
        //    L row j stores column indices k (< j) in CSR format; the diagonal entry is stored
        //    as the last column index and must be skipped (hence -1).
        const int64_t row_start = L_indptr[j];
        const int64_t row_end   = L_indptr[j + 1] - 1; // exclude diagonal
        for (int64_t p = row_start; p < row_end; ++p) {
            const int64_t k   = L_indices[p];            // column index k (< j)
            const double  ljk = Lc_data[L_to_Lc[p]];    // L(j,k)
            // Column k of L (stored in CSC) contributes to rows >= k.
            const int64_t col_start = Lc_indptr[k];
            const int64_t col_end   = Lc_indptr[k + 1];
            for (int64_t q = col_start; q < col_end; ++q) {
                const int64_t row = Lc_indices[q];
                if (row >= j) {
                    work[row] -= Lc_data[q] * ljk;
                }
            }
        }
        // 3) Diagonal entry and off‑diagonal values of column j.
        const double diag = work[j];
        work[j] = 0.0;
        const double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj; // diagonal of column j
        const int64_t col_start = Lc_indptr[j] + 1;
        const int64_t col_end   = Lc_indptr[j + 1];
        for (int64_t p = col_start; p < col_end; ++p) {
            const int64_t row = Lc_indices[p];
            Lc_data[p] = work[row] / ljj;
            work[row] = 0.0;
        }
    }

    // Forward substitution: solve L * y = b (L stored column‑wise in Lc_data).
    for (int64_t i = 0; i < N; ++i) {
        y[i] = b[i];
    }
    for (int64_t j = 0; j < N; ++j) {
        const double ljj = Lc_data[Lc_indptr[j]];
        const double yj  = y[j] / ljj;
        y[j] = yj;
        const int64_t col_start = Lc_indptr[j] + 1;
        const int64_t col_end   = Lc_indptr[j + 1];
        for (int64_t p = col_start; p < col_end; ++p) {
            const int64_t row = Lc_indices[p];
            y[row] -= Lc_data[p] * yj;
        }
    }

    // Backward substitution: solve Lᵀ * y = y (result from forward step).
    for (int64_t jj = 0; jj < N; ++jj) {
        const int64_t col = N - 1 - jj;
        const double ljj = Lc_data[Lc_indptr[col]];
        double s = y[col];
        const int64_t col_start = Lc_indptr[col] + 1;
        const int64_t col_end   = Lc_indptr[col + 1];
        for (int64_t p = col_start; p < col_end; ++p) {
            const int64_t row = Lc_indices[p];
            s -= Lc_data[p] * y[row];
        }
        y[col] = s / ljj;
    }
    free(work);
}
