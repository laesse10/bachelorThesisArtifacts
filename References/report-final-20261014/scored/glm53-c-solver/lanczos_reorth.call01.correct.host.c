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
    const int64_t N = NX * NY * NZ;
    (void)workspace; (void)workspace_size;
    if (N <= 0 || m <= 0) return;

    double *restrict qb[3];
    for (int t = 0; t < 3; t++)
        qb[t] = (double *)aligned_alloc(64, sizeof(double) * (size_t)N);
    double *restrict w = (double *)aligned_alloc(64, sizeof(double) * (size_t)N);
    int nthr = omp_get_max_threads();
    double *restrict cbuf = (double *)aligned_alloc(64, sizeof(double) * (size_t)nthr * (size_t)m);
    double *restrict c1   = (double *)aligned_alloc(64, sizeof(double) * (size_t)m);
    double *restrict ps   = (double *)aligned_alloc(64, sizeof(double) * (size_t)nthr);

    if (!qb[0] || !qb[1] || !qb[2] || !w || !cbuf || !c1 || !ps) {
        for (int t = 0; t < 3; t++) free(qb[t]);
        free(w); free(cbuf); free(c1); free(ps);
        return;
    }

    /* ---- initial basis vector ---- */
    double nrm = 0.0;
    #pragma omp parallel for reduction(+:nrm) schedule(static)
    for (int64_t i = 0; i < N; i++) nrm += b[i] * b[i];
    nrm = sqrt(nrm);
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        double v = b[i] / nrm;
        Q[(size_t)i * (size_t)m] = v;
        qb[0][i] = v;
    }

    double beta_prev = 0.0;
    double a_sh = 0.0, bsh = 0.0;
    int ci = 0;

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *restrict cloc = cbuf + (size_t)tid * (size_t)m;
        double *restrict myps = ps + tid;
        for (int64_t j = 0; j < m; j++) {
            const int64_t k = j + 1;
            const double *restrict qc = qb[ci];
            const double *restrict qp = qb[(ci + 2) % 3];
            double *restrict qn = qb[(ci + 1) % 3];

            /* ---- matvec w = A q_j ; w -= beta_prev q_prev ; a-partials ---- */
            const double bp = beta_prev;
            *myps = 0.0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                const int64_t s = A_indptr[i], e = A_indptr[i + 1];
                double acc = 0.0;
                for (int64_t kk = s; kk < e; kk++)
                    acc += A_data[kk] * qc[A_indices[kk]];
                if (j > 0) acc -= bp * qp[i];
                w[i] = acc;
                *myps += qc[i] * acc;
            }
            #pragma omp single
            {
                double a = 0.0;
                for (int t = 0; t < nthr; t++) a += ps[t];
                alpha[j] = a;
                a_sh = a;
            }
            const double a = a_sh;

            /* ---- sweep 1: w -= a q_j ; c1 = Q(:,0:k)^T w ---- */
            for (int64_t p = 0; p < k; p++) cloc[p] = 0.0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                double wi = w[i] - a * qc[i];
                w[i] = wi;
                const double *restrict qr = Q + (size_t)i * (size_t)m;
                for (int64_t p = 0; p < k; p++) cloc[p] += qr[p] * wi;
            }
            #pragma omp for schedule(static)
            for (int64_t p = 0; p < k; p++) {
                double s = 0.0;
                for (int t = 0; t < nthr; t++) s += cbuf[(size_t)t * (size_t)m + p];
                c1[p] = s;
            }

            /* ---- sweep 2: w -= Q(:,0:k) c1 ; norm partials ---- */
            *myps = 0.0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                const double *restrict qr = Q + (size_t)i * (size_t)m;
                double s = 0.0;
                for (int64_t p = 0; p < k; p++) s += qr[p] * c1[p];
                double v = w[i] - s;
                w[i] = v;
                *myps += v * v;
            }
            #pragma omp single
            {
                double b_j = 0.0;
                for (int t = 0; t < nthr; t++) b_j += ps[t];
                b_j = sqrt(b_j);
                beta[j] = b_j;
                bsh = b_j;
                beta_prev = b_j;
                ci = (ci + 1) % 3;
            }
            const double b_j = bsh;

            /* ---- pass C: Q(:,j+1) = w / b_j ---- */
            if (j + 1 < m) {
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N; i++) {
                    double v = w[i] / b_j;
                    Q[(size_t)i * (size_t)m + (j + 1)] = v;
                    qn[i] = v;
                }
            }
        }
    }

    for (int t = 0; t < 3; t++) free(qb[t]);
    free(w); free(cbuf); free(c1); free(ps);
}
