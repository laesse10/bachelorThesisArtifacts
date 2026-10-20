#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void lanczos_reorth_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    double *restrict Q,
    double *restrict alpha,
    const double *restrict b,
    double *restrict beta,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t m,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const int64_t N = NX * NY * NZ;
    if (N <= 0 || m <= 0) return;

    const int nthr = omp_get_max_threads() < 1 ? 1 : omp_get_max_threads();

    double *w    = malloc((size_t)N * sizeof(double));
    double *cA   = malloc((size_t)N * sizeof(double));
    double *cB   = malloc((size_t)N * sizeof(double));
    double *Hbuf = malloc((size_t)nthr * (size_t)m * sizeof(double));
    double *Hs   = malloc((size_t)m * sizeof(double));
    if (!w || !cA || !cB || !Hbuf || !Hs) { free(w); free(cA); free(cB); free(Hbuf); free(Hs); exit(1); }

    /* q0 = b / ||b|| (faithful to reference: plain sum then sqrt) */
    double nrm2 = 0.0;
    #pragma omp parallel for reduction(+:nrm2) schedule(static)
    for (int64_t i = 0; i < N; i++) nrm2 += b[i] * b[i];
    const double nrm = sqrt(nrm2);

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        const double v = b[i] / nrm;
        cA[i] = v;
        Q[i * m] = v;
    }

    double *ccur  = cA;   /* contiguous copy of column j   */
    double *cprev = NULL; /* contiguous copy of column j-1 */
    double beta_prev = 0.0;

    for (int64_t j = 0; j < m; j++) {
        const int64_t K = j + 1;

        /* w = A * ccur */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; i++) {
            double acc = 0.0;
            const int64_t k0 = A_indptr[i], k1 = A_indptr[i + 1];
            for (int64_t k = k0; k < k1; k++)
                acc += A_data[k] * ccur[A_indices[k]];
            w[i] = acc;
        }

        if (j > 0) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++)
                w[i] -= beta_prev * cprev[i];
        }

        double a = 0.0;
        #pragma omp parallel for reduction(+:a) schedule(static)
        for (int64_t i = 0; i < N; i++) a += ccur[i] * w[i];
        alpha[j] = a;

        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N; i++)
            w[i] -= a * ccur[i];

        /* Full reorthogonalization, twice, against columns 0..j (K of them).
           Batched per pass: H[p] = q_p . w for all p, then w -= Q_H.  With Q
           orthonormal to ~eps^2 the batched pass equals the sequential one to
           O(eps^2). */
        for (int pass = 0; pass < 2; pass++) {
            memset(Hbuf, 0, (size_t)nthr * (size_t)K * sizeof(double));
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                double *Hacc = Hbuf + (size_t)omp_get_thread_num() * (size_t)K;
                const double *qr = Q + i * m;
                const double wr = w[i];
                for (int64_t p = 0; p < K; p++)
                    Hacc[p] += qr[p] * wr;
            }
            for (int64_t p = 0; p < K; p++) {
                double s = 0.0;
                for (int t = 0; t < nthr; t++)
                    s += Hbuf[(size_t)t * (size_t)K + (size_t)p];
                Hs[p] = s;
            }
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                const double *qr = Q + i * m;
                double s = 0.0;
                for (int64_t p = 0; p < K; p++)
                    s += Hs[p] * qr[p];
                w[i] -= s;
            }
        }

        double nrm2w = 0.0;
        #pragma omp parallel for reduction(+:nrm2w) schedule(static)
        for (int64_t i = 0; i < N; i++) nrm2w += w[i] * w[i];
        const double b_j = sqrt(nrm2w);
        beta[j] = b_j;

        if (j + 1 < m) {
            /* Reuse the buffer holding column j-1: it was only needed for the
               beta_prev subtraction at the top of this step. */
            double *newcol = (j == 0) ? cB : cprev;
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                const double v = w[i] / b_j;
                newcol[i] = v;
                Q[i * m + (j + 1)] = v;
            }
            cprev = ccur;
            ccur = newcol;
        }
        beta_prev = b_j;
    }

    free(w); free(cA); free(cB); free(Hbuf); free(Hs);
}
