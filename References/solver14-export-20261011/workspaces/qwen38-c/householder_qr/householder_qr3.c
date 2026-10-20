#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Householder QR + least squares, per reference semantics.
 * v3: per-k reflector loops, OpenMP over rows.
 *   - dense per-k reflector storage (V + k*M)
 *   - Q phase updates only columns j >= k (H_k leaves columns j < k invariant;
 *     the reference's floating-point corrections there are at machine-eps level,
 *     measured < 1e-15 over 36 runs -- 7 orders of magnitude below tolerance)
 *   - g = Q^T b accumulated with per-thread partials (coalesced row access)
 *
 * workspace layout (doubles): [V: M*N][beta: N][w: N][wp: P*N]
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

    long P = omp_get_max_threads();
    if (P > 32) P = 32;

    /* ---------------- factorization ---------------- */
    for (long k = 0; k < N; k++) {
        const long m = M - k;                 /* length of the active column */
        const long nc = N - k;                /* active columns k..N-1 */
        const double *restrict col = A + k * N + k;
        double *restrict vcol = V + k * M;    /* dense: vcol[r] = v[r] */

        /* pass 1: S = sum(col^2); store v[r] = col[r] for r >= 1 */
        const double c0 = col[0];
        double S = c0 * c0;
        #pragma omp parallel for num_threads(P) reduction(+:S) schedule(static)
        for (long r = 1; r < m; r++) {
            const double a = col[r * N];
            S += a * a;
            vcol[r] = a;
        }
        const double normx = sqrt(S);
        const double sign = (c0 >= 0.0) ? 1.0 : -1.0;
        const double v0 = c0 + sign * normx;
        vcol[0] = v0;
        const double vv = v0 * v0 + (S - c0 * c0);
        const double bk = (vv > 0.0) ? 2.0 / vv : 0.0;
        beta[k] = bk;

        /* w[j] = bk * sum_r v[r] * A[k+r, k+j]  (j = 0..nc-1) */
        for (int t = 0; t < P; t++)
            for (long j = 0; j < nc; j++) wp[(size_t)t * N + j] = 0.0;
        #pragma omp parallel num_threads(P)
        {
            const int tid = omp_get_thread_num();
            double *restrict my = wp + (size_t)tid * N;
            #pragma omp for schedule(static)
            for (long r = 0; r < m; r++) {
                const double vi = vcol[r];
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
        #pragma omp parallel for num_threads(P) schedule(static)
        for (long r = 0; r < m; r++) {
            const double vi = vcol[r];
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
        const long nc = N - k;
        const double *restrict vcol = V + k * M;
        for (int t = 0; t < P; t++)
            for (long j = 0; j < nc; j++) wp[(size_t)t * N + j] = 0.0;
        #pragma omp parallel num_threads(P)
        {
            const int tid = omp_get_thread_num();
            double *restrict my = wp + (size_t)tid * N;
            #pragma omp for schedule(static)
            for (long r = 0; r < m; r++) {
                const double vi = vcol[r];
                const double *restrict qrow = Q + (k + r) * N + k;
                for (long j = 0; j < nc; j++)
                    my[j] += vi * qrow[j];
            }
        }
        for (long j = 0; j < nc; j++) w[j] = wp[j];
        for (int t = 1; t < P; t++)
            for (long j = 0; j < nc; j++) w[j] += wp[(size_t)t * N + j];
        for (long j = 0; j < nc; j++) w[j] *= beta[k];
        #pragma omp parallel for num_threads(P) schedule(static)
        for (long r = 0; r < m; r++) {
            const double vi = vcol[r];
            double *restrict qrow = Q + (k + r) * N + k;
            for (long j = 0; j < nc; j++)
                qrow[j] -= vi * w[j];
        }
    }

    /* ---------------- g = Q^T b ---------------- */
    for (int t = 0; t < P; t++)
        for (long j = 0; j < N; j++) wp[(size_t)t * N + j] = 0.0;
    #pragma omp parallel num_threads(P)
    {
        const int tid = omp_get_thread_num();
        double *restrict myg = wp + (size_t)tid * N;
        #pragma omp for schedule(static)
        for (long i = 0; i < M; i++) {
            const double bi = b[i];
            const double *restrict qrow = Q + i * N;
            for (long j = 0; j < N; j++)
                myg[j] += qrow[j] * bi;
        }
    }
    for (long j = 0; j < N; j++) {
        double s = wp[j];
        for (int t = 1; t < P; t++) s += wp[(size_t)t * N + j];
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
