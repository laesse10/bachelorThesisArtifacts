// Optimized implementation of full-reorthogonalization Lanczos (CSR matrix, double precision)
// Target function signature matches the benchmark specification.

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>
#include <stdlib.h>

// The function signature is defined by the benchmark JSON.
void lanczos_reorth_csr_fp64(const double *restrict A_data,
                              const int64_t *restrict A_indices,
                              const int64_t *restrict A_indptr,
                              double *restrict Q,
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
    if (N <= 0 || m <= 0) return; // nothing to do
    // Limit OpenBLAS to single thread to avoid oversubscription.
    #if defined(__unix__) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    extern void openblas_set_num_threads(int);
    openblas_set_num_threads(1);
    #endif

    // Workspace for temporary vector w. Use provided workspace if big enough, else malloc.
    const size_t w_bytes = (size_t)N * sizeof(double);
    double *w = NULL;
    if (workspace != NULL && workspace_size >= (int64_t)w_bytes) {
        w = (double *)workspace; // reuse provided buffer
    } else {
        w = (double *)malloc(w_bytes);
        if (w == NULL) return; // allocation failure leads to early exit
    }

    // Allocate a temporary buffer for dot products during reorthogonalization.
        // Its size is at most m (the Krylov dimension).
        double *dot_buf = (double *)malloc((size_t)m * sizeof(double));
        if (dot_buf == NULL) {
            // Clean up w if it was allocated here.
            if (workspace == NULL || workspace_size < (int64_t)w_bytes) {
                free(w);
            }
            return;
        }

        // Compute norm of initial vector b using BLAS dot product.
    double nrm = sqrt(cblas_ddot((int)N, b, 1, b, 1));
    if (nrm == 0.0) nrm = 1.0; // avoid division by zero, though unlikely.

    // Initialize first Lanczos vector: Q[:,0] = b / nrm
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        Q[i * m + 0] = b[i] / nrm;
    }

    double beta_prev = 0.0;

    // Main Lanczos loop over Krylov dimension m.
    for (int64_t j = 0; j < m; ++j) {
        // Sparse matrix-vector product: w = A * Q[:,j]
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            double acc = 0.0;
            for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; ++k) {
                // column index of the nonzero entry
                int64_t col = A_indices[k];
                acc += A_data[k] * Q[col * m + j];
            }
            w[i] = acc;
        }

        // If not the first iteration, subtract beta_{j-1} * Q[:,j-1] from w
        if (j > 0) {
            // w = w - beta_prev * Q[:,j-1]
            // Q column stride is 'm'
            // w = w - beta_prev * Q[:,j-1]
            // Q column stride is 'm'
            cblas_daxpy((int)N, -beta_prev, Q + (j - 1), (int)m, w, 1);
        }

        // Compute alpha_j = Q[:,j]^T * w
        double a = cblas_ddot((int)N, Q + j, (int)m, w, 1);
        alpha[j] = a;

        // w = w - a * Q[:,j]
#if 0
        #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                w[i] -= a * Q[i * m + j];
            }

        #endif
            cblas_daxpy((int)N, -a, Q + j, (int)m, w, 1);
// Full reorthogonalization, performed twice using BLAS level-2 operations.
        // Compute dot products Q[:,0:j]ᵀ * w and subtract the projection in a fused manner.
        for (int pass = 0; pass < 2; ++pass) {
            // dot = Q[0:j]^T * w (size j+1)
            cblas_dgemv(CblasRowMajor, CblasTrans,
                        (int)N, (int)(j + 1),
                        1.0, Q, (int)m,
                        w, 1,
                        0.0, dot_buf, 1);
            // w = w - Q[0:j] * dot
            cblas_dgemv(CblasRowMajor, CblasNoTrans,
                        (int)N, (int)(j + 1),
                        -1.0, Q, (int)m,
                        dot_buf, 1,
                        1.0, w, 1);
        }

        // Compute beta_j = ||w||_2
        double bj_sq = 0.0;
        #pragma omp parallel for reduction(+:bj_sq)
        for (int64_t i = 0; i < N; ++i) {
            bj_sq += w[i] * w[i];
        }
        double bj = sqrt(bj_sq);
        beta[j] = bj;

        // Normalize w to obtain next Lanczos vector if needed.
        if (j + 1 < m) {
            // Q[:,j+1] = w / bj
            double inv_bj = bj != 0.0 ? 1.0 / bj : 0.0;
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                Q[i * m + (j + 1)] = w[i] * inv_bj;
            }
        }

        // Prepare beta for next iteration.
        beta_prev = bj;
    }

    // Free allocated workspace if we used malloc.
    if (workspace == NULL || workspace_size < (int64_t)w_bytes) {
        free(w);
    }
    // Free the dot product buffer.
    free(dot_buf);
}
