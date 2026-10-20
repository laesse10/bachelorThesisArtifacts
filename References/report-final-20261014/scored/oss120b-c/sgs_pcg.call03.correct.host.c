// SPDX-License-Identifier: GPL-3.0-or-later
// Implementation of SGS-preconditioned CG (symmetric Gauss-Seidel preconditioner) in C.
// Mirrors the reference Python implementation (sgs_pcg_numpy.py). OpenMP is used for
// parallelism where the algorithm permits it.

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// Apply one symmetric Gauss-Seidel sweep: solve M z = r with M = (D+L) D^{-1} (D+U).
// The forward and backward solves must stay sequential because each row depends
// on previously computed values.
static inline void sgs_apply(const double *restrict A_data,
                              const int64_t *restrict A_indices,
                              const int64_t *restrict A_indptr,
                              const double *restrict diag,
                              const double *restrict r,
                              double *restrict y,
                              double *restrict z,
                              const int64_t N) {
    // Forward substitution: (D+L) y = r.
    // A_indices are sorted ascending within each row, so we can stop when the column index
    // reaches the diagonal (i) because all subsequent entries are >= i.
    for (int64_t i = 0; i < N; ++i) {
        double s = r[i];
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];
        for (int64_t k = row_start; k < row_end; ++k) {
            const int64_t j = A_indices[k];
            if (j >= i) break;               // only entries with column < i affect the forward sweep
            s -= A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    // Backward substitution: (D+U) z = D y.
    // Scan each row backwards; A_indices are ascending, so we walk from the end and stop
    // once we encounter a column index <= row.
    for (int64_t ii = 0; ii < N; ++ii) {
        const int64_t row = N - 1 - ii;
        double s = diag[row] * y[row];
        const int64_t row_start = A_indptr[row];
        const int64_t row_end   = A_indptr[row + 1];
        for (int64_t k = row_end - 1; k >= row_start; --k) {
            const int64_t j = A_indices[k];
            if (j <= row) break;               // only entries with column > row affect the backward sweep
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
    // We need six temporary vectors of length N: r, z, y, p, q, diag.
    const int64_t needed_bytes = (int64_t)6 * N * (int64_t)sizeof(double);
    double *buf = NULL;
    bool use_ws = (workspace_size >= needed_bytes);
    if (use_ws) {
        buf = (double *)workspace;
    } else {
        buf = (double *)malloc(needed_bytes);
        if (buf == NULL) {
            return; // allocation failure, abort
        }
    }
    double *r   = buf;
    double *z   = r + N;
    double *y   = z + N;
    double *p   = y + N;
    double *q   = p + N;
    double *diag = q + N;

    // Initialise x = 0, residual r = b (since x0 = 0).
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        x[i] = 0.0;
        r[i] = b[i];
    }

    // Extract diagonal entries from the CSR matrix. Each row is independent.
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        double d = 0.0;
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];
        for (int64_t k = row_start; k < row_end; ++k) {
            if (A_indices[k] == i) {
                d = A_data[k];
                break;
            }
        }
        diag[i] = d;
    }

    // First preconditioner application.
    sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);

    // Initialise search direction p = z.
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        p[i] = z[i];
    }

    // Compute initial inner product r^T z.
    double rz = 0.0;
    #pragma omp parallel for reduction(+:rz) schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        rz += r[i] * z[i];
    }

    // Main CG loop.
    for (int64_t it = 0; it < niter; ++it) {
        // q = A * p (sparse matrix-vector multiplication).
        #pragma omp parallel for schedule(static)
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
        double pq = 0.0;
        #pragma omp parallel for reduction(+:pq) schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            pq += p[i] * q[i];
        }
        double alpha = (pq != 0.0) ? (rz / pq) : 0.0;

        // Update solution x and residual r.
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * q[i];
        }

        // Apply SGS preconditioner to the new residual.
        sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);

        // Compute new inner product r^T z.
        double rznew = 0.0;
        #pragma omp parallel for reduction(+:rznew) schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            rznew += r[i] * z[i];
        }
        double beta = (rz != 0.0) ? (rznew / rz) : 0.0;

        // Update search direction p = z + beta * p.
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            p[i] = z[i] + beta * p[i];
        }

        // Prepare for next iteration.
        rz = rznew;
    }

    // Free temporary buffer if we allocated it.
    if (!use_ws) {
        free(buf);
    }
    // x now contains the solution.
}

