#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

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

    /* Allocate temporary buffers. Prefer supplied workspace if large enough. */
    const size_t needed = 2 * N * sizeof(double);
    double *w = NULL;
    double *q_prev = NULL;
    bool owned = false;
    if (workspace != NULL && (size_t)workspace_size >= needed) {
        w = (double *)workspace;
        q_prev = (double *)(workspace + N * sizeof(double));
    } else {
        w = (double *)malloc(N * sizeof(double));
        q_prev = (double *)malloc(N * sizeof(double));
        owned = true;
        if (!w || !q_prev) {
            if (w) free(w);
            if (q_prev) free(q_prev);
            return;
        }
    }

    /* Normalise starting vector b and store as first Lanczos vector */
    double nrm = 0.0;
    #pragma omp parallel for reduction(+:nrm)
    for (size_t i = 0; i < N; ++i) {
        nrm += b[i] * b[i];
    }
    nrm = sqrt(nrm);
    if (nrm == 0.0) {
        if (owned) { free(w); free(q_prev); }
        return;
    }
    const double inv_nrm = 1.0 / nrm;
    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        Q[i * (size_t)m + 0] = b[i] * inv_nrm;
    }

    double beta_prev = 0.0;

    for (int64_t j = 0; j < m; ++j) {
        /* Sparse matrix-vector multiplication w = A * Q[:,j] */
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
            /* w = w - beta_prev * q_prev */
            #pragma omp parallel for
            for (size_t i = 0; i < N; ++i) {
                w[i] -= beta_prev * q_prev[i];
            }
        }

        /* alpha_j = Q[:,j]^T * w */
        double a = 0.0;
        #pragma omp parallel for reduction(+:a)
        for (size_t i = 0; i < N; ++i) {
            a += Q[i * (size_t)m + j] * w[i];
        }
        alpha[j] = a;

        /* w = w - a * Q[:,j] */
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) {
            w[i] -= a * Q[i * (size_t)m + j];
        }

        /* Full reorthogonalisation (two passes) */
        for (int pass = 0; pass < 2; ++pass) {
            for (int64_t p = 0; p <= j; ++p) {
                double dot = 0.0;
                #pragma omp parallel for reduction(+:dot)
                for (size_t i = 0; i < N; ++i) {
                    dot += Q[i * (size_t)m + p] * w[i];
                }
                #pragma omp parallel for
                for (size_t i = 0; i < N; ++i) {
                    w[i] -= dot * Q[i * (size_t)m + p];
                }
            }
        }

        /* beta_j = ||w|| */
        double bj = 0.0;
        #pragma omp parallel for reduction(+:bj)
        for (size_t i = 0; i < N; ++i) {
            bj += w[i] * w[i];
        }
        bj = sqrt(bj);
        beta[j] = bj;

        /* Store current Lanczos vector for next iteration */
        #pragma omp parallel for
        for (size_t i = 0; i < N; ++i) {
            q_prev[i] = Q[i * (size_t)m + j];
        }

        /* Normalise w to obtain next vector, if any */
        if (j + 1 < m) {
            if (bj != 0.0) {
                double inv_bj = 1.0 / bj;
                #pragma omp parallel for
                for (size_t i = 0; i < N; ++i) {
                    Q[i * (size_t)m + (j + 1)] = w[i] * inv_bj;
                }
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
    }
}

