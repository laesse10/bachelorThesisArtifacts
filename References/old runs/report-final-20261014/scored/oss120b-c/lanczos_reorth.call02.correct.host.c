#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>

void lanczos_reorth_csr_fp64(
    const double *restrict A_data /* csr A: value of each entry */,
    const int64_t *restrict A_indices /* csr A: COLUMN index of each entry, ascending within a row; 0-based int64 */,
    const int64_t *restrict A_indptr /* csr A: row r's entries are [indptr[r], indptr[r+1]); 0-based int64 */,
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
    /* Compute total number of rows (grid points) */
    const size_t N = (size_t)NX * (size_t)NY * (size_t)NZ;
    if (N == 0 || m <= 0) return;

    /* Allocate temporary buffers w, q_prev, and dot vector (size m). Use supplied workspace if large enough. */
    const size_t needed = 2 * N * sizeof(double) + (size_t)m * sizeof(double);
    double *w = NULL;
    double *q_prev = NULL;
    double *dot_vec = NULL;
    bool owned = false;
    if (workspace != NULL && (size_t)workspace_size >= needed) {
        w = (double *)workspace;
        q_prev = (double *)(workspace + N * sizeof(double));
        dot_vec = (double *)(workspace + 2 * N * sizeof(double));
    } else {
        w = (double *)malloc(N * sizeof(double));
        q_prev = (double *)malloc(N * sizeof(double));
        dot_vec = (double *)malloc((size_t)m * sizeof(double));
        owned = true;
        if (w == NULL || q_prev == NULL || dot_vec == NULL) {
            if (w) free(w);
            if (q_prev) free(q_prev);
            if (dot_vec) free(dot_vec);
            return;
        }
    }

    /* ---------------------------------------------------------------------
     * 1. Normalise the initial vector b and store it as the first Lanczos vector
     * --------------------------------------------------------------------- */
    double nrm = 0.0;
    #pragma omp parallel for reduction(+:nrm)
    for (size_t i = 0; i < N; ++i) {
        nrm += b[i] * b[i];
    }
    nrm = sqrt(nrm);
    if (nrm == 0.0) {
        if (owned) { free(w); free(q_prev); free(dot_vec); }
        return;
    }
    const double inv_nrm = 1.0 / nrm;
    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        Q[i * (size_t)m + 0] = b[i] * inv_nrm;
    }

    double beta_prev = 0.0;
    /* ---------------------------------------------------------------------
     * Main Lanczos iteration (j = 0 .. m-1)
     * --------------------------------------------------------------------- */
    for (int64_t j = 0; j < m; ++j) {
        /* Sparse matrix–vector product: w = A * Q[:,j] */
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < N; ++i) {
            double acc = 0.0;
            int64_t row_start = A_indptr[i];
            int64_t row_end   = A_indptr[i + 1];
            for (int64_t k = row_start; k < row_end; ++k) {
                int64_t col = A_indices[k];
                acc += A_data[k] * Q[(size_t)col * (size_t)m + j];
            }
            w[i] = acc;
        }

        /* Subtract beta_{j-1} * q_{j-1} if j > 0 */
        if (j > 0) {
            // w = w - beta_prev * q_prev
            cblas_daxpy((int)N, -beta_prev, q_prev, 1, w, 1);
        }

        /* Compute alpha_j = Q[:,j]^T * w */
        double a = cblas_ddot((int)N, Q + j, (int)m, w, 1);
        alpha[j] = a;

        /* w = w - alpha_j * Q[:,j] */
        cblas_daxpy((int)N, -a, Q + j, (int)m, w, 1);

        /* Full re-orthogonalisation, performed twice */
        for (int pass = 0; pass < 2; ++pass) {
            // Compute dot products of existing Lanczos vectors with w
            cblas_dgemv(CblasRowMajor, CblasTrans,
                        (int)N, (int)(j + 1),
                        1.0,
                        Q, (int)m,
                        w, 1,
                        0.0,
                        dot_vec, 1);
            // Subtract the projections: w = w - Q[:,0..j] * dot_vec
            cblas_dgemv(CblasRowMajor, CblasNoTrans,
                        (int)N, (int)(j + 1),
                        -1.0,
                        Q, (int)m,
                        dot_vec, 1,
                        1.0,
                        w, 1);
        }

        /* Compute beta_j = ||w|| */
        double bj = sqrt(cblas_ddot((int)N, w, 1, w, 1));
        beta[j] = bj;

        /* Store current Lanczos vector in q_prev for the next iteration */
        cblas_dcopy((int)N, Q + j, (int)m, q_prev, 1);

        /* Normalise w to become the next Lanczos vector (if any) */
        if (j + 1 < m) {
            if (bj != 0.0) {
                double inv_bj = 1.0 / bj;
                // Copy w into the next column of Q and scale
                cblas_dcopy((int)N, w, 1, Q + (j + 1), (int)m);
                cblas_dscal((int)N, inv_bj, Q + (j + 1), (int)m);
            } else {
                // Zero column if norm vanished
                #pragma omp parallel for
                for (size_t i = 0; i < N; ++i) {
                    Q[i * (size_t)m + (j + 1)] = 0.0;
                }
            }
        }

        beta_prev = bj;
    }

    if (owned) {
        free(w);
        free(q_prev);
        free(dot_vec);
    }
}

