// SPDX-License-Identifier: GPL-3.0-or-later
// Optimized implementation of SGS-preconditioned Conjugate Gradient (CG) kernel.
// Mirrors the reference Python version (sgs_pcg_numpy.py). Uses a persistent
// OpenMP parallel region to keep threads alive across the many small loops.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// One symmetric Gauss-Seidel sweep: forward and backward substitution.
static inline void sgs_apply(const double *restrict A_data,
                              const int64_t *restrict A_indices,
                              const int64_t *restrict A_indptr,
                              const double *restrict diag,
                              const double *restrict r,
                              double *restrict y,
                              double *restrict z,
                              const int64_t N) {
    // Forward substitution (D+L) y = r.
    for (int64_t i = 0; i < N; ++i) {
        double s = r[i];
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];
        for (int64_t k = row_start; k < row_end; ++k) {
            const int64_t j = A_indices[k];
            if (j >= i) break; // entries sorted, stop at diagonal and above
            s -= A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    // Backward substitution (D+U) z = D y.
    for (int64_t ii = 0; ii < N; ++ii) {
        const int64_t row = N - 1 - ii;
        double s = diag[row] * y[row];
        const int64_t row_start = A_indptr[row];
        const int64_t row_end   = A_indptr[row + 1];
        for (int64_t k = row_end - 1; k >= row_start; --k) {
            const int64_t j = A_indices[k];
            if (j <= row) break; // entries sorted, stop when reaching diagonal
            s -= A_data[k] * z[j];
        }
        z[row] = s / diag[row];
    }
}

void sgs_pcg_csr_fp64(const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const double *restrict b,
    double *restrict x,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t niter,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {

    const int64_t N = NX * NY * NZ;

    // Allocate six temporary vectors: r, z, y, p, q, diag.
    const int64_t needed_bytes = (int64_t)6 * N * (int64_t)sizeof(double);
    double *buf = NULL;
    bool use_ws = (workspace_size >= needed_bytes);
    if (use_ws) {
        buf = (double *)workspace;
    } else {
        buf = (double *)malloc(needed_bytes);
        if (!buf) return; // allocation failure
    }
    double *r   = buf;
    double *z   = r + N;
    double *y   = z + N;
    double *p   = y + N;
    double *q   = p + N;
    double *diag = q + N;

    // Shared scalars for the CG algorithm.
    double rz = 0.0;   // inner product <r, z>
    double alpha = 0.0;
    double beta = 0.0;
    double pq = 0.0;
    double rznew = 0.0;

    // Persistent parallel region.
    #pragma omp parallel shared(A_data, A_indices, A_indptr, diag, b, x, r, z, y, p, q, N, niter, rz, alpha, beta, pq, rznew)
    {
        // Initialise x = 0 and residual r = b.
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            x[i] = 0.0;
            r[i] = b[i];
        }

        // Extract diagonal entries.
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            double d = 0.0;
            const int64_t row_start = A_indptr[i];
            const int64_t row_end   = A_indptr[i + 1];
            for (int64_t k = row_start; k < row_end; ++k) {
                if (A_indices[k] == i) { d = A_data[k]; break; }
            }
            diag[i] = d;
        }

        // First SGS preconditioner application.
        #pragma omp single
        sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);

        // Initialise search direction p = z.
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            p[i] = z[i];
        }

        // Compute initial inner product <r, z>.
        #pragma omp single
        rz = 0.0; // reset before reduction
        #pragma omp for reduction(+:rz) schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            rz += r[i] * z[i];
        }

        // Main CG iterations.
        for (int64_t it = 0; it < niter; ++it) {
            // q = A * p (SpMV).
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                double acc = 0.0;
                const int64_t row_start = A_indptr[i];
                const int64_t row_end   = A_indptr[i + 1];
                for (int64_t k = row_start; k < row_end; ++k) {
                    acc += A_data[k] * p[A_indices[k]];
                }
                q[i] = acc;
            }

            // Compute p^T q.
            #pragma omp single
            pq = 0.0; // reset before reduction
            #pragma omp for reduction(+:pq) schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                pq += p[i] * q[i];
            }
            #pragma omp single
            { alpha = (pq != 0.0) ? (rz / pq) : 0.0; }

            // Update x and r.
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                x[i] += alpha * p[i];
                r[i] -= alpha * q[i];
            }

            // Apply SGS preconditioner to the new residual.
            #pragma omp single
            sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);

            // Compute new inner product <r, z>.
            #pragma omp single
            rznew = 0.0; // reset before reduction
            #pragma omp for reduction(+:rznew) schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                rznew += r[i] * z[i];
            }
            #pragma omp single
            {
                beta = (rz != 0.0) ? (rznew / rz) : 0.0;
                rz = rznew; // prepare for next iteration
            }

            // Update search direction p = z + beta * p.
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; ++i) {
                p[i] = z[i] + beta * p[i];
            }
        }
    }

    if (!use_ws) {
        free(buf);
    }
    // Solution stored in x.
}

