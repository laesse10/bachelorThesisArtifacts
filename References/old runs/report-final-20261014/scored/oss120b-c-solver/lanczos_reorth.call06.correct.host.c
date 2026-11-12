// Optimized Lanczos full reorthogonalization kernel for CSR matrix.
// Uses column-major intermediate storage for Q (fast access) and transposes to row-major before returning.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

void lanczos_reorth_csr_fp64(const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    double *restrict Q,               // N x m matrix, initially column-major, returned row-major
    double *restrict alpha,
    const double *restrict b,
    double *restrict beta,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t m,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {

    const int64_t N = NX * NY * NZ;

    // Allocate temporary vectors w and q_prev, using workspace if large enough.
    const size_t required_bytes = (size_t)N * sizeof(double) * 2;
    double *w = NULL;
    double *q_prev = NULL;
    bool allocated = false;
    if (workspace != NULL && workspace_size >= (int64_t)required_bytes) {
        w = (double *)workspace;
        q_prev = (double *)(workspace + N * sizeof(double));
    } else {
        w = (double *)malloc((size_t)N * sizeof(double));
        q_prev = (double *)malloc((size_t)N * sizeof(double));
        allocated = true;
    }

    // 1. Normalize initial vector b and store as first column of Q (column-major layout).
    double norm2 = 0.0;
    #pragma omp parallel for reduction(+:norm2)
    for (int64_t i = 0; i < N; ++i) {
        norm2 += b[i] * b[i];
    }
    double nrm = sqrt(norm2);
    #pragma omp parallel for
    for (int64_t i = 0; i < N; ++i) {
        Q[i] = b[i] / nrm; // column 0 starts at offset 0
    }

    double beta_prev = 0.0;
    for (int64_t j = 0; j < m; ++j) {
        double *Qj = Q + j * N; // column j

        // 2. Sparse matrix-vector multiply w = A * Qj
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            double sum = 0.0;
            for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; ++k) {
                int64_t col = A_indices[k];
                sum += A_data[k] * Qj[col];
            }
            w[i] = sum;
        }

        // 3. Subtract beta_{j-1} * q_prev if j > 0.
        if (j > 0) {
            #pragma omp parallel for
            for (int64_t i = 0; i < N; ++i) {
                w[i] -= beta_prev * q_prev[i];
            }
        }

        // 4. Compute alpha[j] = Qj^T * w.
        double a = 0.0;
        #pragma omp parallel for reduction(+:a)
        for (int64_t i = 0; i < N; ++i) {
            a += Qj[i] * w[i];
        }
        alpha[j] = a;

        // 5. Orthogonalize w against current column.
        #pragma omp parallel for
        for (int64_t i = 0; i < N; ++i) {
            w[i] -= a * Qj[i];
        }

        // 6. Full reorthogonalization (two passes) against all previous columns.
        for (int pass = 0; pass < 2; ++pass) {
            for (int64_t p = 0; p <= j; ++p) {
                double *Qp = Q + p * N;
                double dot = 0.0;
                #pragma omp parallel for reduction(+:dot)
                for (int64_t i = 0; i < N; ++i) {
                    dot += Qp[i] * w[i];
                }
                #pragma omp parallel for
                for (int64_t i = 0; i < N; ++i) {
                    w[i] -= dot * Qp[i];
                }
            }
        }

        // 7. Compute beta[j] = ||w||.
        double beta_j = 0.0;
        #pragma omp parallel for reduction(+:beta_j)
        for (int64_t i = 0; i < N; ++i) {
            beta_j += w[i] * w[i];
        }
        beta_j = sqrt(beta_j);
        beta[j] = beta_j;

        // 8. Store current Lanczos vector as q_prev for next iteration.
        #pragma omp parallel for
        for (int64_t i = 0; i < N; ++i) {
            q_prev[i] = Qj[i];
        }

        // 9. Compute next Lanczos vector if needed.
        if (j + 1 < m) {
            double *Qnext = Q + (j + 1) * N;
            #pragma omp parallel for
            for (int64_t i = 0; i < N; ++i) {
                Qnext[i] = w[i] / beta_j;
            }
        }

        beta_prev = beta_j;
    }

    // Transpose Q from column-major (N x m) to row-major (N x m) in-place via temporary buffer.
    double *tmp = (double *)malloc((size_t)N * m * sizeof(double));
    for (int64_t i = 0; i < N; ++i) {
        for (int64_t j = 0; j < m; ++j) {
            tmp[i * m + j] = Q[i + N * j];
        }
    }
    memcpy(Q, tmp, (size_t)N * m * sizeof(double));
    free(tmp);

    // Clean up temporary buffers.
    if (allocated) {
        free(w);
        free(q_prev);
    }
}
