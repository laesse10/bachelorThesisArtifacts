/*
 * Correct sequential implementation of the sparse Cholesky kernel for the HPCAgent benchmark.
 *
 * This version follows the reference algorithm exactly (see sparse_cholesky_numpy.py).
 * It does not attempt sophisticated parallelism; the primary goal is functional
 * correctness.  The code is compiled with the benchmark's flags (-O3 -march=native
 * -fopenmp) which enables vectorisation and other low-level optimisations.
 */

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>

void sparse_cholesky_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
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
    const int64_t workspace_size)
{
    const int64_t N = EDGE * EDGE * EDGE;

    /* Allocate a temporary work buffer. If a suitably sized workspace is provided, reuse it. */
    double *work = NULL;
    if (workspace && workspace_size >= N * (int64_t)sizeof(double)) {
        work = (double *)workspace;
    } else {
        work = (double *)malloc((size_t)N * sizeof(double));
        if (!work) return; /* allocation failure */
    }

    /* Numeric factorisation: up-looking left-looking column algorithm */
    for (int64_t j = 0; j < N; ++j) {
        /* 1) Gather entries of column j from A (rows >= j). */
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; ++p) {
            int64_t row = A_indices[p];
            if (row >= j) {
                work[row] = A_data[p];
            }
        }

        /* 2) Subtract contributions from previously computed columns k where L(j,k) != 0. */
        int64_t l_start = L_indptr[j];
        int64_t l_end   = L_indptr[j + 1] - 1;  /* exclude diagonal */
        for (int64_t p = l_start; p < l_end; ++p) {
            int64_t k   = L_indices[p];
            double ljk = Lc_data[L_to_Lc[p]];
            for (int64_t q = Lc_indptr[k]; q < Lc_indptr[k + 1]; ++q) {
                int64_t row = Lc_indices[q];
                if (row >= j) {
                    work[row] -= Lc_data[q] * ljk;
                }
            }
        }

        /* 3) Diagonal entry */
        double diag = work[j];
        work[j] = 0.0;
        double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj;

        /* 4) Off-diagonal entries of column j */
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; ++p) {
            int64_t row = Lc_indices[p];
            Lc_data[p] = work[row] / ljj;
            work[row] = 0.0; /* clear for next iteration */
        }
    }

    /* Forward substitution: solve L * y = b */
    for (int64_t i = 0; i < N; ++i) {
        y[i] = b[i];
    }
    for (int64_t j = 0; j < N; ++j) {
        double ljj = Lc_data[Lc_indptr[j]];
        y[j] /= ljj;
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; ++p) {
            int64_t row = Lc_indices[p];
            y[row] -= Lc_data[p] * y[j];
        }
    }

    /* Backward substitution: solve L^T * y = y (overwrites y) */
    for (int64_t jj = 0; jj < N; ++jj) {
        int64_t col = N - 1 - jj;
        double ljj = Lc_data[Lc_indptr[col]];
        double s = y[col];
        for (int64_t p = Lc_indptr[col] + 1; p < Lc_indptr[col + 1]; ++p) {
            int64_t row = Lc_indices[p];
            s -= Lc_data[p] * y[row];
        }
        y[col] = s / ljj;
    }

    if (!(workspace && workspace_size >= N * (int64_t)sizeof(double))) {
        free(work);
    }
}
