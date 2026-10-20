#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void lanczos_reorth_csr_fp64(const double *restrict A_data,
    const int64_t *restrict A_indices, const int64_t *restrict A_indptr,
    double *restrict Q, double *restrict alpha, const double *restrict b,
    double *restrict beta, const int64_t NX, const int64_t NY, const int64_t NZ,
    const int64_t m, uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    int64_t N = NX*NY*NZ;
    double *q_prev = calloc((size_t)N, sizeof(double));
    double *w = calloc((size_t)N, sizeof(double));
    double nrm = 0.0;
    for (int64_t i = 0; i < N; i++) nrm = nrm + b[i]*b[i];
    nrm = sqrt(nrm);
    for (int64_t i = 0; i < N; i++) Q[i*(size_t)m + 0] = b[i]/nrm;
    double beta_prev = 0.0;
    for (int64_t j = 0; j < m; j++) {
        for (int64_t i = 0; i < N; i++) {
            double acc = 0.0;
            for (int64_t k = A_indptr[i]; k < A_indptr[i+1]; k++)
                acc = acc + A_data[k]*Q[A_indices[k]*(size_t)m + j];
            w[i] = acc;
        }
        if (j > 0) for (int64_t i = 0; i < N; i++) w[i] = w[i] - beta_prev*q_prev[i];
        double a = 0.0;
        for (int64_t i = 0; i < N; i++) a = a + Q[i*(size_t)m + j]*w[i];
        alpha[j] = a;
        for (int64_t i = 0; i < N; i++) w[i] = w[i] - a*Q[i*(size_t)m + j];
        for (int pass = 0; pass < 2; pass++) {
            for (int64_t p = 0; p <= j; p++) {
                double dot = 0.0;
                for (int64_t i = 0; i < N; i++) dot = dot + Q[i*(size_t)m + p]*w[i];
                for (int64_t i = 0; i < N; i++) w[i] = w[i] - dot*Q[i*(size_t)m + p];
            }
        }
        double b_j = 0.0;
        for (int64_t i = 0; i < N; i++) b_j = b_j + w[i]*w[i];
        b_j = sqrt(b_j);
        beta[j] = b_j;
        for (int64_t i = 0; i < N; i++) q_prev[i] = Q[i*(size_t)m + j];
        if (j + 1 < m)
            for (int64_t i = 0; i < N; i++) Q[i*(size_t)m + (j+1)] = w[i]/b_j;
        beta_prev = b_j;
    }
    free(q_prev); free(w);
}
