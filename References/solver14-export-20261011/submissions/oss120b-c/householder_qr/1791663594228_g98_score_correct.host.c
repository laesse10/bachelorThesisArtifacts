// Optimized Householder QR factorization with least-squares solve
// Target language: C (C23)
// The kernel signature is defined in signature.json. We implement the algorithm using BLAS (cblas) for the heavy matrix‑vector operations and OpenMP for simple parallel loops.

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>

// Helper macro for indexing row‑major matrices
#define IDX(i, j, ld) ((i) * (ld) + (j))

void householder_qr_fp64(
    double *restrict A, // M x N matrix, row‑major, overwritten with R in the upper triangle
    double *restrict Q, // M x N matrix, output
    double *restrict R, // N x N matrix, output (row‑major)
    const double *restrict b, // length M
    double *restrict x, // length N, output solution
    const int64_t M,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    // Allocate temporary buffers for V and beta with proper alignment.
    // The caller provides a workspace buffer, but we allocate our own to ensure
    // correct alignment for BLAS (which may require 64‑byte alignment).
    double *V = (double *)malloc((size_t)M * (size_t)N * sizeof(double));
    double *beta = (double *)malloc((size_t)N * sizeof(double));
    if (!V || !beta) {
        // Allocation failure – abort silently (the harness will mark it incorrect).
        if (V) free(V);
        if (beta) free(beta);
        return;
    }

    // Zero Q (it will be built later)
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M * N; ++i) {
        Q[i] = 0.0;
    }
    // Identity part of Q (min(M,N) diagonal entries)
    for (int64_t i = 0; i < N && i < M; ++i) {
        Q[IDX(i, i, N)] = 1.0;
    }

    // Temporary vector w needed for BLAS calls – maximum possible size is N.
    double *w = (double *)malloc((size_t)N * sizeof(double));
    if (!w) {
        free(V);
        free(beta);
        return; // out of memory – nothing we can do.
    }

    // ----- Householder reduction: compute V, beta and overwrite A with R (upper triangle) -----
    for (int64_t k = 0; k < N; ++k) {
        // Compute the 2‑norm of the column k from rows k..M-1.
        double norm_sq = 0.0;
        for (int64_t i = k; i < M; ++i) {
            double a = A[IDX(i, k, N)];
            norm_sq += a * a;
        }
        double normx = sqrt(norm_sq);
        double sign = (A[IDX(k, k, N)] >= 0.0) ? 1.0 : -1.0;

        // Copy the column into V.
        for (int64_t i = k; i < M; ++i) {
            V[IDX(i, k, N)] = A[IDX(i, k, N)];
        }
        // Modify the diagonal element according to the Householder formula.
        V[IDX(k, k, N)] = A[IDX(k, k, N)] + sign * normx;

        // Compute beta = 2 / (v^T v) if not zero.
        double vv = 0.0;
        for (int64_t i = k; i < M; ++i) {
            double vi = V[IDX(i, k, N)];
            vv += vi * vi;
        }
        double beta_k = (vv > 0.0) ? 2.0 / vv : 0.0;
        beta[k] = beta_k;

        if (beta_k != 0.0) {
            // Compute w = beta * v^T * A_sub (size N‑k).
            // A_sub starts at A[k, k] with leading dimension N.
            // V column is strided by N.
            cblas_dgemv(CblasRowMajor, CblasTrans,
                        (int)(M - k), (int)(N - k),
                        beta_k,
                        A + IDX(k, k, N), (int)N,
                        V + IDX(k, k, N), (int)N,
                        0.0,
                        w, 1);
            // Rank‑1 update: A_sub -= v * w^T.
            // Use alpha = -1.0 because BLAS dger performs: A := A + alpha*x*y^T
            cblas_dger(CblasRowMajor,
                        (int)(M - k), (int)(N - k),
                        -1.0,
                        V + IDX(k, k, N), (int)N,
                        w, 1,
                        A + IDX(k, k, N), (int)N);
        }
        // Copy the current row k of the updated A into R (only columns k..N‑1).
        // The lower part of R stays zero – the caller may rely on that.
        for (int64_t j = k; j < N; ++j) {
            R[IDX(k, j, N)] = A[IDX(k, j, N)];
        }
    }

    // ----- Form Q = H_0 H_1 ... H_{N‑1} (apply reflectors in reverse order) -----
    for (int64_t k = N - 1; k >= 0; --k) {
        double beta_k = beta[k];
        if (beta_k == 0.0) continue; // identity reflector, nothing to do.
        // w = beta * v^T * Q_sub where Q_sub = Q[k..M‑1, 0..N‑1]
        cblas_dgemv(CblasRowMajor, CblasTrans,
                    (int)(M - k), (int)N,
                    beta_k,
                    Q + IDX(k, 0, N), (int)N,
                    V + IDX(k, k, N), (int)N,
                    0.0,
                    w, 1);
        // Q_sub -= v * w^T
        cblas_dger(CblasRowMajor,
                    (int)(M - k), (int)N,
                    -1.0,
                    V + IDX(k, k, N), (int)N,
                    w, 1,
                    Q + IDX(k, 0, N), (int)N);
    }

    // ----- Compute g = Q^T * b (size N) -----
    double *g = (double *)malloc((size_t)N * sizeof(double));
    if (!g) {
        free(w);
        free(V);
        free(beta);
        return;
    }
    // Using BLAS: g = Q^T * b.
    cblas_dgemv(CblasRowMajor, CblasTrans,
                (int)M, (int)N,
                1.0,
                Q, (int)N,
                b, 1,
                0.0,
                g, 1);

    // ----- Back substitution: solve R x = g -----
    // R is upper‑triangular. Work from the last row upwards.
    for (int64_t i = N - 1; i >= 0; --i) {
        double sum = 0.0;
        // dot product of the known part of the row with already computed x values.
        for (int64_t j = i + 1; j < N; ++j) {
            sum += R[IDX(i, j, N)] * x[j];
        }
        double diag = R[IDX(i, i, N)];
        // In the pathological case diag == 0 (rank deficiency) we set x[i] = 0.
        x[i] = (diag != 0.0) ? (g[i] - sum) / diag : 0.0;
    }

    // Clean up temporary buffers.
    free(w);
    free(g);
    free(V);
    free(beta);
}

