#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Householder QR + least squares, per reference semantics.
 * v1: straightforward per-k reflector loop, OpenMP over rows.
 *
 * workspace layout: [V: M*N doubles][beta: N][w: N]
 */
void householder_qr_fp64(
    double *restrict A,
    double *restrict Q,
    double *restrict R,
    const double *restrict b,
    double *restrict x,
    const int64_t M,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace_size;
    double *restrict V = (double *)workspace;
    double *restrict beta = V + M * N;
    double *restrict w = beta + N;
    double *restrict wp = w + N;          /* P * N partials */

    const long P = omp_get_max_threads();

    /* ---------------- factorization ---------------- */
    for (long k = 0; k < N; k++) {
        const long m = M - k;                 /* length of the active column */
        const long nc = N - k;                /* active columns k..N-1 */
        const double *restrict col = A + k * N + k;
        double *restrict vcol = V + k * N + k;

        /* pass 1: S = sum(col^2); store v[i] = col[i] for i >= 1 */
        double S = 0.0, c0 = col[0];
        #pragma omp parallel reduction(+:S)
        {
            long chunk = (m + P - 1) / P;
            #pragma omp for schedule(static)
            for (long i = 0; i < m; i += chunk) {
                long i1 = i + chunk; if (i1 > m) i1 = m;
                double s = 0.0;
                for (long r = i; r < i1; r++) {
                    double a = col[r * N];
                    s += a * a;
                    if (r != 0) vcol[r * N] = a;
                }
                S += s;
            }
        }
        const double normx = sqrt(S);
        const double sign = (c0 >= 0.0) ? 1.0 : -1.0;
        const double v0 = c0 + sign * normx;
        vcol[0] = v0;
        const double vv = v0 * v0 + (S - c0 * c0);
        const double bk = (vv > 0.0) ? 2.0 / vv : 0.0;
        beta[k] = bk;

        /* w[j] = bk * sum_i v[i] * A[k+i, k+j]  (j = 0..nc-1) */
        for (long j = 0; j < nc; j++) w[j] = 0.0;
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            double *restrict my = wp + (size_t)tid * N;
            for (long j = 0; j < nc; j++) my[j] = 0.0;
            #pragma omp for schedule(static)
            for (long r = 0; r < m; r++) {
                const double vi = vcol[r * N];
                const double *restrict arow = A + (k + r) * N + k;
                for (long j = 0; j < nc; j++)
                    my[j] += vi * arow[j];
            }
        }
        for (long j = 0; j < nc; j++) w[j] = wp[j];
        for (int t = 1; t < P; t++)
            for (long j = 0; j < nc; j++) w[j] += wp[(size_t)t * N + j];
        for (long j = 0; j < nc; j++) w[j] *= bk;

        /* A[k:M, k:N] -= v * w^T */
        #pragma omp parallel for schedule(static)
        for (long r = 0; r < m; r++) {
            const double vi = vcol[r * N];
            double *restrict arow = A + (k + r) * N + k;
            for (long j = 0; j < nc; j++)
                arow[j] -= vi * w[j];
        }
        /* R[k, k:N] = A[k, k:N] */
        memcpy(R + k * N + k, A + k * N + k, nc * sizeof(double));
    }

    /* ---------------- Q = H_0 H_1 ... H_{N-1} ---------------- */
    memset(Q, 0, (size_t)M * N * sizeof(double));
    for (long j = 0; j < N; j++) Q[j * N + j] = 1.0;

    for (long k = N - 1; k >= 0; k--) {
        const long m = M - k;
        const double *restrict vcol = V + k * N + k;
        for (long j = 0; j < N; j++) w[j] = 0.0;
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            double *restrict my = wp + (size_t)tid * N;
            for (long j = 0; j < N; j++) my[j] = 0.0;
            #pragma omp for schedule(static)
            for (long r = 0; r < m; r++) {
                const double vi = vcol[r * N];
                const double *restrict qrow = Q + (k + r) * N;
                for (long j = 0; j < N; j++)
                    my[j] += vi * qrow[j];
            }
        }
        for (long j = 0; j < N; j++) w[j] = wp[j];
        for (int t = 1; t < P; t++)
            for (long j = 0; j < N; j++) w[j] += wp[(size_t)t * N + j];
        for (long j = 0; j < N; j++) w[j] *= beta[k];
        #pragma omp parallel for schedule(static)
        for (long r = 0; r < m; r++) {
            const double vi = vcol[r * N];
            double *restrict qrow = Q + (k + r) * N;
            for (long j = 0; j < N; j++)
                qrow[j] -= vi * w[j];
        }
    }

    /* ---------------- g = Q^T b ---------------- */
    for (long j = 0; j < N; j++) {
        double s = 0.0;
        #pragma omp parallel for reduction(+:s) schedule(static)
        for (long i = 0; i < M; i++) s += Q[i * N + j] * b[i];
        w[j] = s;   /* reuse w as g */
    }

    /* ---------------- back substitution R x = g ---------------- */
    for (long i = N - 1; i >= 0; i--) {
        double s = w[i];
        const double *ri = R + i * N + i + 1;
        for (long j = i + 1; j < N; j++) s -= ri[j - i - 1] * x[j];
        x[i] = s / R[i * N + i];
    }
}
