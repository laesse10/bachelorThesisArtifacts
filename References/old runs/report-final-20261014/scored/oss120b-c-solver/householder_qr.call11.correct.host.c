/* High‑performance Householder QR factorisation and least‑squares solver.
 * This implementation follows the reference algorithm’s sign convention:
 * the diagonal of R is opposite the sign of the original matrix diagonal,
 * matching the reference NumPy implementation.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cblas.h>
#include <lapacke.h>
#include <omp.h>

void householder_qr_fp64(double *restrict A,
                         double *restrict Q,
                         double *restrict R,
                         const double *restrict b,
                         double *restrict x,
                         const int64_t M,
                         const int64_t N,
                         uint8_t *restrict workspace,
                         const int64_t workspace_size) {
    /* Allocate tau (length N) – use workspace if large enough. */
    double *tau = NULL;
    int64_t tau_bytes = N * (int64_t)sizeof(double);
    int allocated = 0;
    if (workspace && tau_bytes <= workspace_size) {
        tau = (double *)workspace;
    } else {
        tau = (double *)malloc((size_t)N * sizeof(double));
        allocated = 1;
    }

    /* 1. QR factorisation (LAPACK overwrites A with elementary reflectors). */
    LAPACKE_dgeqrf(LAPACK_ROW_MAJOR, (lapack_int)M, (lapack_int)N,
                   A, (lapack_int)N, tau);

    /* 2. Extract the upper‑triangular part of A as R. */
    for (int64_t i = 0; i < N; ++i) {
        for (int64_t j = 0; j < N; ++j) {
            R[i * N + j] = (j >= i) ? A[i * N + j] : 0.0;
        }
    }

    /* 3. Build the thin Q matrix from the reflectors stored in A. */
    memcpy(Q, A, (size_t)M * (size_t)N * sizeof(double));
    LAPACKE_dorgqr(LAPACK_ROW_MAJOR, (lapack_int)M, (lapack_int)N, (lapack_int)N,
                   Q, (lapack_int)N, tau);

    /* 4. Zero the strictly lower part of A to match the reference output format. */
    for (int64_t i = 0; i < M; ++i) {
        for (int64_t j = 0; j < N; ++j) {
            if (i > j) {
                A[i * N + j] = 0.0;
            }
        }
    }

    /* 5. Compute g = Qᵀ b. */
    double g[256];                 // N ≤ 256 by problem statement
    cblas_dgemv(CblasRowMajor, CblasTrans, (int)M, (int)N,
                1.0, Q, (int)N, b, 1, 0.0, g, 1);

    /* 6. Back‑substitution to solve R x = g. */
    for (int64_t i = N - 1; i >= 0; --i) {
        double sum = 0.0;
        for (int64_t j = i + 1; j < N; ++j) {
            sum += R[i * N + j] * x[j];
        }
        x[i] = (g[i] - sum) / R[i * N + i];
    }

    if (allocated) {
        free(tau);
    }
}
