#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <cblas.h>
#include <stdio.h>

void householder_qr_fp64(
    double *restrict A,
    double *restrict Q,
    double *restrict R,
    const double *restrict b,
    double *restrict x,
    const int64_t M,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    // Determine required workspace for V, beta, w, g in double units
    size_t rows = (size_t)M;
    size_t cols = (size_t)N;
    size_t needed_entries = rows * cols + 3 * cols; // V (MxN) + beta (N) + w (N) + g (N)
    size_t needed_bytes = needed_entries * sizeof(double);
    double *mem = (double *)malloc(needed_bytes);
    int allocated = 1;
    double *V = mem;                     // size M*N
    double *beta = V + rows * cols;      // size N
    double *w = beta + cols;             // size N (temporary vector)
    double *g = w + cols;                // size N (temporary vector)

    // ---------- Householder reduction (compute R and store reflectors) ----------
    for (int64_t k = 0; k < N; ++k) {
        // Compute the 2‑norm of column k (rows k..M-1)
        double norm_sq = 0.0;
        #pragma omp parallel for reduction(+:norm_sq)
        for (int64_t i = k; i < M; ++i) {
            double a = A[i * N + k];
            norm_sq += a * a;
        }
        double normx = sqrt(norm_sq);
        double sign = (A[k * N + k] >= 0.0) ? 1.0 : -1.0;

        // Store the Householder vector in V (copy column, then modify the leading entry)
        for (int64_t i = k; i < M; ++i) {
            V[i * N + k] = A[i * N + k];
        }
        V[k * N + k] = A[k * N + k] + sign * normx;

        // Compute beta = 2 / (vᵀv) unless the vector is zero
        double vv = 0.0;
        #pragma omp parallel for reduction(+:vv)
        for (int64_t i = k; i < M; ++i) {
            double v = V[i * N + k];
            vv += v * v;
        }
        beta[k] = (vv > 0.0) ? (2.0 / vv) : 0.0;

        // Apply the reflector to the trailing sub‑matrix A[k:M, k:N]
        int rows_sub = (int)(M - k);
        int cols_sub = (int)(N - k);
        const double *Vcol = V + k * N + k;
        double *A_sub = A + k * N + k;
        // Compute w = beta * Vcolᵀ * A_sub  (size cols_sub)
        for (int j = 0; j < cols_sub; ++j) {
                const double *Ap = A_sub + j; // column j of the sub‑matrix
                double sum = cblas_ddot(rows_sub, Vcol, (int)N, Ap, (int)N);
                w[j] = beta[k] * sum;
        }
                // Rank‑1 update: A_sub -= Vcol * wᵀ
        for (int i = 0; i < rows_sub; ++i) {
            double vi = Vcol[i * N];
            double *Ap = A_sub + i * N;
            for (int j = 0; j < cols_sub; ++j) {
                Ap[j] -= vi * w[j];
            }
        }

        // Extract the current row of A as the k‑th row of R (upper triangular part)
        for (int64_t j = 0; j < k; ++j) {
            R[k * N + j] = 0.0;
        }
        for (int64_t j = k; j < N; ++j) {
            R[k * N + j] = A[k * N + j];
        }
            }

    // ---------- Build orthogonal matrix Q = H₀ H₁ … H_{N‑1} ----------
    // Initialise Q to the identity M×N (zero then diagonal 1 for rows < N)
    #pragma omp parallel for collapse(2)
    for (int64_t i = 0; i < M; ++i) {
        for (int64_t j = 0; j < N; ++j) {
            Q[i * N + j] = 0.0;
        }
    }
    int64_t diag = (M < N) ? M : N; // M ≥ N in our use case, so diag = N
    for (int64_t i = 0; i < diag; ++i) {
        Q[i * N + i] = 1.0;
    }
    // Apply each reflector in reverse order
    for (int64_t kk = N - 1; kk >= 0; --kk) {
        int rows_sub = (int)(M - kk);
        const double *Vcol = V + kk * N + kk;
        double *Q_sub = Q + kk * N; // start of row kk
        // Compute w = beta[kk] * Vcolᵀ * Q_sub   (size N)
        for (int j = 0; j < N; ++j) {
            const double *Qp = Q_sub + j; // column j offset within sub‑matrix
            double sum = cblas_ddot(rows_sub, Vcol, (int)N, Qp, (int)N);
            w[j] = beta[kk] * sum;
        }
        // Rank‑1 update: Q_sub -= Vcol * wᵀ
        for (int i = 0; i < rows_sub; ++i) {
            double vi = Vcol[i * N];
            double *Qp = Q_sub + i * N;
            for (int j = 0; j < N; ++j) {
                Qp[j] -= vi * w[j];
            }
        }
    }

    // ---------- Solve least‑squares problem: compute g = Qᵀ b and back‑substitution ----------
    // g_i = dot( column i of Q, b )
    for (int64_t i = 0; i < N; ++i) {
        double sum = 0.0;
        const double *Qcol = Q + i; // column i start
        for (int64_t r = 0; r < M; ++r) {
            sum += Qcol[r * N] * b[r];
        }
        g[i] = sum;
    }
    // Back substitution to solve R x = g (R is upper triangular)
    for (int64_t i = N - 1; i >= 0; --i) {
        double sum = 0.0;
        for (int64_t j = i + 1; j < N; ++j) {
            sum += R[i * N + j] * x[j];
        }
        x[i] = (g[i] - sum) / R[i * N + i];
    }

    if (allocated) {
        free(mem);
    }
}
