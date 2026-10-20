/*
 * Optimized implementation of the Householder QR factorization and least-squares solve.
 *
 * The reference implementation (see householder_qr_numpy.py) follows the textbook
 * algorithm: it builds the matrix of Householder vectors V and the scalars beta,
 * updates the trailing sub‑matrix, extracts R, constructs the thin Q matrix, and
 * finally solves R·x = Qᵀ·b.  This version is written for the HPCAgent benchmark.
 * It uses the OpenBLAS BLAS API (cblas_*) for the heavy linear‑algebra kernels –
 * matrix‑vector products and rank‑1 updates – which gives us highly tuned,
 * multi‑threaded code without explicit OpenMP pragmas.  The algorithmic
 * structure mirrors the reference implementation so that numerical results are
 * identical within the required tolerance (rtol=1e‑09, atol=1e‑11).
 *
 * Memory layout:
 *   * All matrices are stored in row‑major order (the layout used by the Python
 *     reference and by the benchmark harness).
 *   * A is overwritten in‑place; after the factorisation the upper‑triangular
 *     part of A holds the rows of R (copied to the dedicated R buffer).
 *   * The Householder vectors are kept in a temporary workspace.  The benchmark
 *     provides a `workspace` buffer; if it is large enough we store V (size
 *     M×N) and the beta array there, otherwise we fall back to heap allocation.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cblas.h>

void householder_qr_fp64(double *restrict A,
                         double *restrict Q,
                         double *restrict R,
                         const double *restrict b,
                         double *restrict x,
                         const int64_t M,
                         const int64_t N,
                         uint8_t *restrict workspace,
                         const int64_t workspace_size) {
    /* -------------------------------------------------------------------- */
    /* Determine the amount of temporary storage required for V (M×N) and   */
    /* the beta array (N).  If the supplied workspace is large enough we      */
    /* reuse it; otherwise we allocate on the heap.                         */
    /* -------------------------------------------------------------------- */
    size_t needed = (size_t)M * (size_t)N * sizeof(double) + (size_t)N * sizeof(double);
    double *V = NULL;   /* Householder vectors, stored column‑wise (row‑major layout) */
    double *beta = NULL;
    int allocated = 0;
    if (workspace != NULL && (int64_t)needed <= workspace_size) {
        V = (double *)workspace;
        beta = (double *)(workspace + (size_t)M * (size_t)N * sizeof(double));
    } else {
        V = (double *)malloc((size_t)M * (size_t)N * sizeof(double));
        beta = (double *)malloc((size_t)N * sizeof(double));
        allocated = 1;
    }

    /* -------------------------------------------------------------------- */
    /* Main Householder reduction of A (M×N).  All operations are performed */
    /* on the trailing sub‑matrix A[k:M, k:N].  The BLAS routines are used  */
    /* for the inner products and the rank‑1 update.                        */
    /* -------------------------------------------------------------------- */
    for (int64_t k = 0; k < N; ++k) {
        int rows = (int)(M - k);       // number of rows in the active panel
        int cols = (int)(N - k);       // number of columns in the active panel
        double *Acol_k = &A[k * N + k]; // start of column k at row k (row‑major)
        double *Vcol_k = &V[k * N + k]; // same stride for V

        /* Copy the current column of A into V (the Householder vector). */
        cblas_dcopy(rows, Acol_k, N, Vcol_k, N);

        /* Compute the Euclidean norm of the vector (Acol_k). */
        double normx = cblas_ddot(rows, Acol_k, N, Acol_k, N);
        normx = sqrt(normx);

        /* Determine the sign to avoid cancellation. */
        double a_kk = Acol_k[0]; // A[k,k]
        double sign = (a_kk >= 0.0) ? 1.0 : -1.0;
        Vcol_k[0] = a_kk + sign * normx; // store the modified first element

        /* Compute beta = 2 / (vᵀv). */
        double vv = cblas_ddot(rows, Vcol_k, N, Vcol_k, N);
        beta[k] = (vv > 0.0) ? 2.0 / vv : 0.0;

        if (beta[k] != 0.0) {
            /* w = beta * (Vᵀ * A_sub)  => size = cols */
            double w[256]; // N ≤ 256, safe on stack
            cblas_dgemv(CblasRowMajor, CblasTrans, rows, cols,
                        beta[k], &A[k * N + k], N,
                        Vcol_k, N, 0.0, w, 1);

            /* A_sub = A_sub - V * wᵀ  (rank‑1 update) */
            cblas_dger(CblasRowMajor, rows, cols,
                        -1.0, Vcol_k, N, w, 1, &A[k * N + k], N);
        }

        /* Extract the k‑th row of the (now upper‑triangular) A into R. */
        for (int64_t j = k; j < N; ++j) {
            R[k * N + j] = A[k * N + j];
        }
    }

    /* -------------------------------------------------------------------- */
    /* Build the thin Q matrix (M×N).  Start from the identity in the first   */
    /* N rows and apply the reflectors in reverse order.                    */
    /* -------------------------------------------------------------------- */
    // Zero Q and set the leading N×N identity block.
    memset(Q, 0, (size_t)M * (size_t)N * sizeof(double));
    for (int64_t i = 0; i < N; ++i) {
        Q[i * N + i] = 1.0;
    }

    // Apply reflectors H_k from k = N‑1 down to 0.
    for (int64_t k = N - 1; k >= 0; --k) {
        int rows = (int)(M - k); // rows of the current trailing block of Q
        // Compute w = beta[k] * (Vᵀ * Q_sub)  => size N
        double wQ[256];
        cblas_dgemv(CblasRowMajor, CblasTrans, rows, (int)N,
                    beta[k], &Q[k * N], N,
                    &V[k * N + k], N, 0.0, wQ, 1);
        // Q_sub = Q_sub - V * wᵀ
        cblas_dger(CblasRowMajor, rows, (int)N,
                    -1.0, &V[k * N + k], N, wQ, 1, &Q[k * N], N);
    }

    /* -------------------------------------------------------------------- */
    /* Compute g = Qᵀ * b (size N).                                         */
    /* -------------------------------------------------------------------- */
    double g[256];
    cblas_dgemv(CblasRowMajor, CblasTrans, (int)M, (int)N,
                1.0, Q, N, b, 1, 0.0, g, 1);

    /* -------------------------------------------------------------------- */
    /* Back‑substitution to solve R·x = g.  R is stored upper‑triangular in  */
    /* the buffer R (row‑major).                                            */
    /* -------------------------------------------------------------------- */
    for (int64_t i = N - 1; i >= 0; --i) {
        double sum = 0.0;
        for (int64_t j = i + 1; j < N; ++j) {
            sum += R[i * N + j] * x[j];
        }
        x[i] = (g[i] - sum) / R[i * N + i];
    }

    if (allocated) {
        free(V);
        free(beta);
    }
}

