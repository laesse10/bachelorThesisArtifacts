/* SGS-preconditioned Conjugate Gradient for a 27-point variable-coefficient
 * 3D stencil operator (CSR, fp64).
 *
 * Port of sgs_pcg_numpy.py with the floating point evaluation order kept
 * identical to the reference:
 *   - SGS sweeps strictly sequential in row order (the dependence chain)
 *   - CSR row sums accumulated in CSR column order, exactly as the reference
 *   - dot products as plain left-to-right sums
 * The independent element-wise / row-parallel loops are OpenMP-parallel with
 * deterministic (chunked, order-preserving) reductions.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

static void sgs_sweep(const double *restrict A_data,
                      const int64_t *restrict A_indices,
                      const int64_t *restrict A_indptr,
                      const double *restrict diag,
                      const double *restrict r,
                      double *restrict y,
                      double *restrict z,
                      int64_t N)
{
    /* forward substitution: (D+L) y = r, increasing row order */
    for (int64_t i = 0; i < N; i++) {
        double s = r[i];
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
            int64_t j = A_indices[k];
            if (j < i) s = s - A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    /* backward substitution: (D+U) z = D y, decreasing row order */
    for (int64_t ii = 0; ii < N; ii++) {
        int64_t row = N - 1 - ii;
        double s = diag[row] * y[row];
        for (int64_t k = A_indptr[row]; k < A_indptr[row + 1]; k++) {
            int64_t j = A_indices[k];
            if (j > row) s = s - A_data[k] * z[j];
        }
        z[row] = s / diag[row];
    }
}

void sgs_pcg_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const double *restrict b,
    double *restrict x,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t niter,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const int64_t N = NX * NY * NZ;
    if (N <= 0) return;

    double *r    = (double *)malloc((size_t)N * sizeof(double));
    double *z    = (double *)malloc((size_t)N * sizeof(double));
    double *y    = (double *)malloc((size_t)N * sizeof(double));
    double *p    = (double *)malloc((size_t)N * sizeof(double));
    double *q    = (double *)malloc((size_t)N * sizeof(double));
    double *diag = (double *)malloc((size_t)N * sizeof(double));

    /* x0 = 0, so the initial residual is b itself. */
    #pragma omp parallel for schedule(static, 2048)
    for (int64_t i = 0; i < N; i++) {
        x[i] = 0.0;
        r[i] = b[i];
    }
    for (int64_t i = 0; i < N; i++) {
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
            if (A_indices[k] == i) {
                diag[i] = A_data[k];
            }
        }
    }

    sgs_sweep(A_data, A_indices, A_indptr, diag, r, y, z, N);
    #pragma omp parallel for schedule(static, 2048)
    for (int64_t i = 0; i < N; i++) {
        p[i] = z[i];
    }
    double rz = 0.0;
    for (int64_t i = 0; i < N; i++) {
        rz = rz + r[i] * z[i];
    }

    for (int64_t it = 0; it < niter; it++) {
        /* q = A p */
        #pragma omp parallel for schedule(static, 128)
        for (int64_t i = 0; i < N; i++) {
            double acc = 0.0;
            for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
                acc = acc + A_data[k] * p[A_indices[k]];
            }
            q[i] = acc;
        }
        double pq = 0.0;
        for (int64_t i = 0; i < N; i++) {
            pq = pq + p[i] * q[i];
        }
        double alpha = rz / pq;

        #pragma omp parallel for schedule(static, 2048)
        for (int64_t i = 0; i < N; i++) {
            x[i] = x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }

        sgs_sweep(A_data, A_indices, A_indptr, diag, r, y, z, N);
        double rznew = 0.0;
        for (int64_t i = 0; i < N; i++) {
            rznew = rznew + r[i] * z[i];
        }
        double beta = rznew / rz;
        #pragma omp parallel for schedule(static, 2048)
        for (int64_t i = 0; i < N; i++) {
            p[i] = z[i] + beta * p[i];
        }
        rz = rznew;
    }

    free(r);
    free(z);
    free(y);
    free(p);
    free(q);
    free(diag);
}
