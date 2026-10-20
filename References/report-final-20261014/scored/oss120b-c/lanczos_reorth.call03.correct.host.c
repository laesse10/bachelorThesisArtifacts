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
    const size_t N = (size_t)NX * (size_t)NY * (size_t)NZ;
    if (N == 0 || m <= 0) return;

    /* Allocate temporary buffers: w, q_prev, dot_vec (size m). Prefer workspace if enough. */
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
        if (!w || !q_prev || !dot_vec) {
            if (w) free(w);
            if (q_prev) free(q_prev);
            if (dot_vec) free(dot_vec);
            return;
        }
    }

    /* 1. Normalise the starting vector b and store as first Lanczos vector */
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

    for (int64_t j = 0; j < m; ++j) {
        /* Sparse matrix-vector multiply: w = A * Q[:,j] */
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

        if (j > 0) {
            // w = w - beta_prev * q_prev
            cblas_daxpy((int)N, -beta_prev, q_prev, 1, w, 1);
        }

        // alpha_j = Q[:,j]^T * w
        double a = cblas_ddot((int)N, Q + j, (int)m, w, 1);
        alpha[j] = a;
        // w = w - a * Q[:,j]
        cblas_daxpy((int)N, -a, Q + j, (int)m, w, 1);

        // Full reorthogonalisation, twice
        for (int pass = 0; pass < 2; ++pass) {
            // Compute dot products of all previous Lanczos vectors with w
            for (int64_t p = 0; p <= j; ++p) {
                dot_vec[p] = 0.0;
            }
            #pragma omp parallel for reduction(+:dot_vec[:j+1])
            for (size_t i = 0; i < N; ++i) {
                double wi = w[i];
                // Unrolled inner loop over p may help, but keep simple
                for (int64_t p = 0; p <= j; ++p) {
                    dot_vec[p] += Q[i * (size_t)m + p] * wi;
                }
            }
            // Subtract the projections
            #pragma omp parallel for
            for (size_t i = 0; i < N; ++i) {
                double sum = 0.0;
                for (int64_t p = 0; p <= j; ++p) {
                    sum += dot_vec[p] * Q[i * (size_t)m + p];
                }
                w[i] -= sum;
            }
        }

        // beta_j = ||w||
        double bj = sqrt(cblas_ddot((int)N, w, 1, w, 1));
        beta[j] = bj;

        // Save current Lanczos vector for next iteration
        cblas_dcopy((int)N, Q + j, (int)m, q_prev, 1);

        // Normalise w to obtain next Lanczos vector (if any)
        if (j + 1 < m) {
            if (bj != 0.0) {
                double inv_bj = 1.0 / bj;
                // Copy w into next column and scale
                cblas_dcopy((int)N, w, 1, Q + (j + 1), (int)m);
                cblas_dscal((int)N, inv_bj, Q + (j + 1), (int)m);
            } else {
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

