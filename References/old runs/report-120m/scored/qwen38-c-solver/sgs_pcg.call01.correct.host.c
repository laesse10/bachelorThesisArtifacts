#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* SGS-preconditioned CG, one full port of the NumPy reference.
 * v1: faithful serial implementation. */

static void sgs_apply(const double *restrict A_data,
                      const int64_t *restrict A_indices,
                      const int64_t *restrict A_indptr,
                      const double *restrict diag,
                      const double *restrict r,
                      double *restrict y,
                      double *restrict z,
                      int64_t N) {
    /* Forward: (D+L) y = r */
    for (int64_t i = 0; i < N; i++) {
        double s = r[i];
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
            int64_t j = A_indices[k];
            if (j < i)
                s = s - A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    /* Backward: (D+U) z = D y */
    for (int64_t ii = 0; ii < N; ii++) {
        int64_t row = N - 1 - ii;
        double s = diag[row] * y[row];
        for (int64_t k = A_indptr[row]; k < A_indptr[row + 1]; k++) {
            int64_t j = A_indices[k];
            if (j > row)
                s = s - A_data[k] * z[j];
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
    const int64_t workspace_size) {

    const int64_t N = NX * NY * NZ;
    (void)workspace;
    (void)workspace_size;

    double *r  = (double *)malloc((size_t)N * sizeof(double));
    double *z  = (double *)malloc((size_t)N * sizeof(double));
    double *y  = (double *)malloc((size_t)N * sizeof(double));
    double *p  = (double *)malloc((size_t)N * sizeof(double));
    double *q  = (double *)malloc((size_t)N * sizeof(double));
    double *diag = (double *)calloc((size_t)N, sizeof(double));
    if (!r || !z || !y || !p || !q || !diag) { free(r); free(z); free(y); free(p); free(q); free(diag); abort(); }

    /* x0 = 0, initial residual is b. */
    for (int64_t i = 0; i < N; i++) {
        x[i] = 0.0;
        r[i] = b[i];
    }
    for (int64_t i = 0; i < N; i++) {
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
            if (A_indices[k] == i)
                diag[i] = A_data[k];
        }
    }

    sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);
    for (int64_t i = 0; i < N; i++)
        p[i] = z[i];
    double rz = 0.0;
    for (int64_t i = 0; i < N; i++)
        rz = rz + r[i] * z[i];

    for (int64_t _it = 0; _it < niter; _it++) {
        for (int64_t i = 0; i < N; i++) {
            double acc = 0.0;
            for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
                acc = acc + A_data[k] * p[A_indices[k]];
            }
            q[i] = acc;
        }
        double pq = 0.0;
        for (int64_t i = 0; i < N; i++)
            pq = pq + p[i] * q[i];
        double alpha = rz / pq;
        for (int64_t i = 0; i < N; i++) {
            x[i] = x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }
        sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);
        double rznew = 0.0;
        for (int64_t i = 0; i < N; i++)
            rznew = rznew + r[i] * z[i];
        double beta = rznew / rz;
        for (int64_t i = 0; i < N; i++)
            p[i] = z[i] + beta * p[i];
        rz = rznew;
    }

    free(r); free(z); free(y); free(p); free(q); free(diag);
}
