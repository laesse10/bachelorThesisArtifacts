/* v1: straightforward port - sequential SGS, OpenMP vector ops, int32 indices */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

static void sgs_apply_seq(const double *restrict A_data, const int *restrict A_idx,
                          const int *restrict A_ptr, const double *restrict diag,
                          const double *restrict r, double *restrict y,
                          double *restrict z, int N)
{
    for (int i = 0; i < N; i++) {
        double s = r[i];
        for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++) {
            int j = A_idx[k];
            if (j < i) s = s - A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    for (int ii = 0; ii < N; ii++) {
        int row = N - 1 - ii;
        double s = diag[row] * y[row];
        for (int k = A_ptr[row]; k < A_ptr[row + 1]; k++) {
            int j = A_idx[k];
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
    const int64_t NX_, const int64_t NY_, const int64_t NZ_,
    const int64_t niter_,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    const int NX = (int)NX_, NY = (int)NY_, NZ = (int)NZ_;
    const int N = NX * NY * NZ;
    const int64_t niter = niter_;
    (void)workspace; (void)workspace_size;

    double *r  = (double *)malloc(sizeof(double) * (size_t)N);
    double *y  = (double *)malloc(sizeof(double) * (size_t)N);
    double *z  = (double *)malloc(sizeof(double) * (size_t)N);
    double *p  = (double *)malloc(sizeof(double) * (size_t)N);
    double *q  = (double *)malloc(sizeof(double) * (size_t)N);
    double *dg = (double *)malloc(sizeof(double) * (size_t)N);
    int *A_idx = (int *)malloc(sizeof(int) * (size_t)A_indptr[N]);
    int *A_ptr = (int *)malloc(sizeof(int) * (size_t)(N + 1));
    if (!r || !y || !z || !p || !q || !dg || !A_idx || !A_ptr) { free(r);free(y);free(z);free(p);free(q);free(dg);free(A_idx);free(A_ptr); return; }

    {
        int64_t nnz = A_indptr[N];
        #pragma omp parallel for schedule(static)
        for (int64_t k = 0; k < nnz; k++) A_idx[k] = (int)A_indices[k];
        #pragma omp parallel for schedule(static)
        for (int i = 0; i <= N; i++) A_ptr[i] = (int)A_indptr[i];
    }

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) { x[i] = 0.0; r[i] = b[i]; }

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        double d = 0.0;
        for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++)
            if (A_idx[k] == i) d = A_data[k];
        dg[i] = d;
    }

    sgs_apply_seq(A_data, A_idx, A_ptr, dg, r, y, z, N);

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) p[i] = z[i];

    double rz = 0.0;
    #pragma omp parallel for schedule(static) reduction(+:rz)
    for (int i = 0; i < N; i++) rz += r[i] * z[i];

    for (int64_t it = 0; it < niter; it++) {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            double acc = 0.0;
            for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++)
                acc = acc + A_data[k] * p[A_idx[k]];
            q[i] = acc;
        }
        double pq = 0.0;
        #pragma omp parallel for schedule(static) reduction(+:pq)
        for (int i = 0; i < N; i++) pq += p[i] * q[i];

        double alpha = rz / pq;

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            x[i] = x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }

        sgs_apply_seq(A_data, A_idx, A_ptr, dg, r, y, z, N);

        double rznew = 0.0;
        #pragma omp parallel for schedule(static) reduction(+:rznew)
        for (int i = 0; i < N; i++) rznew += r[i] * z[i];

        double beta = rznew / rz;

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) p[i] = z[i] + beta * p[i];

        rz = rznew;
    }

    free(r); free(y); free(z); free(p); free(q); free(dg); free(A_idx); free(A_ptr);
}
