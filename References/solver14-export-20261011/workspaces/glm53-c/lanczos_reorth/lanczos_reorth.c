#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* CBLAS (LP64) declaration - values are ABI-stable. */
enum { KB_ROWMAJOR = 101, KB_COLMAJOR = 102, KB_NOTRANS = 111, KB_TRANS = 112 };
void cblas_dgemv(int layout, int trans, int m, int n, double alpha,
                 const double *a, int lda, const double *x, int incx,
                 double beta, double *y, int incy);

void lanczos_reorth_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    double *restrict Q,
    double *restrict alpha,
    const double *restrict b,
    double *restrict beta,
    const int64_t NX, const int64_t NY, const int64_t NZ,
    const int64_t m,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    const int64_t N = NX * NY * NZ;
    if (N <= 0) return;
    if (m <= 0) return;

    /* scratch: QT (m x N, row-major = column-major N x m, lda N), w (N), dots (m) */
    const size_t need_doubles = (size_t)N * (size_t)m + (size_t)N + (size_t)m;
    double *QT, *w, *dots;
    uint8_t *heap = NULL;
    if (workspace != NULL &&
        workspace_size >= (int64_t)(need_doubles * sizeof(double)) &&
        ((((uintptr_t)workspace) & 63u) == 0u)) {
        QT = (double *)(void *)workspace;
    } else {
        heap = (uint8_t *)malloc(need_doubles * sizeof(double));
        if (heap == NULL) return;
        QT = (double *)(void *)heap;
    }
    w = QT + (size_t)N * (size_t)m;
    dots = w + N;

    /* q_0 = b / ||b|| */
    double nrm = 0.0;
    #pragma omp parallel for reduction(+:nrm) schedule(static)
    for (int64_t i = 0; i < N; i++) nrm += b[i] * b[i];
    nrm = sqrt(nrm);
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) QT[i] = b[i] / nrm;

    double beta_prev = 0.0;
    for (int64_t j = 0; j < m; j++) {
        const double *restrict qj = QT + (size_t)j * (size_t)N;
        const double *restrict qprev = QT + (size_t)(j - 1) * (size_t)N;
        const double bp = beta_prev;

        /* w = A q_j  (minus beta_prev q_{j-1} when j > 0) */
        if (j > 0) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                double acc = 0.0;
                const int64_t k0 = A_indptr[i], k1 = A_indptr[i + 1];
                for (int64_t k = k0; k < k1; k++)
                    acc += A_data[k] * qj[A_indices[k]];
                w[i] = acc - bp * qprev[i];
            }
        } else {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                double acc = 0.0;
                const int64_t k0 = A_indptr[i], k1 = A_indptr[i + 1];
                for (int64_t k = k0; k < k1; k++)
                    acc += A_data[k] * qj[A_indices[k]];
                w[i] = acc;
            }
        }

        /* alpha_j = q_j . w ; w -= alpha_j q_j */
        double a = 0.0;
        #pragma omp parallel for reduction(+:a) schedule(static)
        for (int64_t i = 0; i < N; i++) a += qj[i] * w[i];
        alpha[j] = a;
        const double aneg = -a;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; i++) w[i] += aneg * qj[i];

        /* two passes of full reorthogonalization (blocked classical GS) */
        for (int pass = 0; pass < 2; pass++) {
            const int64_t kk = j + 1;
            cblas_dgemv(KB_COLMAJOR, KB_TRANS, (int)N, (int)kk, 1.0,
                        QT, (int)N, w, 1, 0.0, dots, 1);
            cblas_dgemv(KB_COLMAJOR, KB_NOTRANS, (int)N, (int)kk, -1.0,
                        QT, (int)N, dots, 1, 1.0, w, 1);
        }

        double bj = 0.0;
        #pragma omp parallel for reduction(+:bj) schedule(static)
        for (int64_t i = 0; i < N; i++) bj += w[i] * w[i];
        bj = sqrt(bj);
        beta[j] = bj;

        if (j + 1 < m) {
            double *restrict qn = QT + (size_t)(j + 1) * (size_t)N;
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++) qn[i] = w[i] / bj;
        }
        beta_prev = bj;
    }

    /* blocked transpose QT (m x N) -> Q (N x m) */
    {
        const int TJ = 8;
        const int64_t TI = 64;
        #pragma omp parallel for schedule(static)
        for (int64_t i0 = 0; i0 < N; i0 += TI) {
            const int64_t i1 = (i0 + TI < N) ? i0 + TI : N;
            for (int64_t j0 = 0; j0 < m; j0 += TJ) {
                const int64_t j1 = (j0 + TJ < m) ? j0 + TJ : m;
                for (int64_t i = i0; i < i1; i++) {
                    double *restrict qrow = Q + (size_t)i * (size_t)m;
                    for (int64_t j = j0; j < j1; j++)
                        qrow[j] = QT[(size_t)j * (size_t)N + i];
                }
            }
        }
    }

    free(heap);
}
