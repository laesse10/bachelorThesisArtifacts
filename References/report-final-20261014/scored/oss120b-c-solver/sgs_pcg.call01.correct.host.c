#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * Preconditioned Conjugate Gradient with Symmetric Gauss-Seidel (SGS) preconditioner.
 * The matrix A is stored in CSR format with double values (A_data), int64 column indices (A_indices),
 * and int64 row pointers (A_indptr). The routine follows the reference implementation in
 * sgs_pcg_numpy.py and is optimized for OpenMP parallelism where possible.
 */

static inline void sgs_apply(const double *restrict A_data,
                              const int64_t *restrict A_indices,
                              const int64_t *restrict A_indptr,
                              const double *restrict diag,
                              const double *restrict r,
                              double *restrict y,
                              double *restrict z,
                              const int64_t N) {
    // Forward sweep: solve (D+L) y = r  (row order 0 .. N-1)
    for (int64_t i = 0; i < N; ++i) {
        double s = r[i];
        // CSR column indices are sorted, so we can stop when we reach the diagonal or beyond.
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; ++k) {
            int64_t j = A_indices[k];
            if (j >= i) {
                // j == i is the diagonal; it is handled later by division.
                break;
            }
            s -= A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    // Backward sweep: solve (D+U) z = D y  (row order N-1 .. 0)
    for (int64_t ii = 0; ii < N; ++ii) {
        int64_t row = N - 1 - ii;
        double s = diag[row] * y[row];
        // Column indices are sorted; we skip entries <= row.
        for (int64_t k = A_indptr[row]; k < A_indptr[row + 1]; ++k) {
            int64_t j = A_indices[k];
            if (j <= row) {
                continue;
            }
            s -= A_data[k] * z[j];
        }
        z[row] = s / diag[row];
    }
}

void sgs_pcg_csr_fp64(const double *restrict A_data /* csr A: value of each entry */,
                      const int64_t *restrict A_indices /* csr A: column index of each entry */,
                      const int64_t *restrict A_indptr   /* csr A: row pointer */,
                      const double *restrict b,
                      double *restrict x,
                      const int64_t NX,
                      const int64_t NY,
                      const int64_t NZ,
                      const int64_t niter,
                      uint8_t *restrict workspace,
                      const int64_t workspace_size) {
    const int64_t N = NX * NY * NZ;
    const size_t needed = (size_t)6 * (size_t)N * sizeof(double);
    double *buf = NULL;
    int allocated = 0;
    if (workspace != NULL && workspace_size >= (int64_t)needed) {
        buf = (double *)workspace;
    } else {
        buf = (double *)malloc(needed);
        allocated = 1;
    }
    // Allocate workspace slices.
    double *r = buf;                 // residual
    double *z = r + N;               // preconditioned residual
    double *y = z + N;               // temporary for SGS forward sweep
    double *p = y + N;               // search direction
    double *q = p + N;               // A * p
    double *diag = q + N;            // diagonal entries of A

    // Initialise solution x to zero and copy RHS to residual r.
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        x[i] = 0.0;
        r[i] = b[i];
    }

    // Extract diagonal entries of A.
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        double d = 0.0;
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; ++k) {
            if (A_indices[k] == i) {
                d = A_data[k];
                break;
            }
        }
        diag[i] = d;
    }

    // Initial SGS preconditioning: solve M*z = r
    sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);

    // Initialise search direction p = z.
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        p[i] = z[i];
    }

    // Compute initial scalar product rz = r·z.
    double rz = 0.0;
    #pragma omp parallel for reduction(+:rz) schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        rz += r[i] * z[i];
    }

    // Main CG iteration.
    for (int64_t it = 0; it < niter; ++it) {
        // Compute q = A * p and accumulate p·q in one pass.
        double pq = 0.0;
        #pragma omp parallel for reduction(+:pq) schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            double acc = 0.0;
            for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; ++k) {
                acc += A_data[k] * p[A_indices[k]];
            }
            q[i] = acc;
            pq += p[i] * acc;
        }

        const double alpha = rz / pq;

        // Update solution x and residual r.
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * q[i];
        }

        // Apply SGS preconditioner to the new residual.
        sgs_apply(A_data, A_indices, A_indptr, diag, r, y, z, N);

        // Compute new rz = r·z.
        double rznew = 0.0;
        #pragma omp parallel for reduction(+:rznew) schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            rznew += r[i] * z[i];
        }

        const double beta = rznew / rz;

        // Update search direction p = z + beta * p.
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; ++i) {
            p[i] = z[i] + beta * p[i];
        }

        rz = rznew;
    }

    if (allocated) {
        free(buf);
    }
}

