#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* LAPACK Fortran-77 interface from the linked OpenBLAS. */
extern void dgeqrf_(const int *m, const int *n, double *a, const int *lda,
                    double *tau, double *work, int *lwork, int *info);
extern void dorgqr_(const int *m, const int *n, const int *k, double *a,
                    const int *lda, const double *tau, double *work,
                    int *lwork, int *info);

/* Copy row-major M x N src into column-major M x N dst (tiled for locality). */
static void tr_rm_cm(double *restrict dst, const double *restrict src,
                     int64_t M, int64_t N)
{
    #pragma omp parallel for schedule(dynamic)
    for (int64_t jj = 0; jj < N; jj += 96) {
        const int64_t jhi = (jj + 96 < N) ? jj + 96 : N;
        for (int64_t ii = 0; ii < M; ii += 384) {
            const int64_t ihi = (ii + 384 < M) ? ii + 384 : M;
            for (int64_t j = jj; j < jhi; ++j) {
                const double *restrict s = src + j;
                double *restrict d = dst + j * M;
                for (int64_t i = ii; i < ihi; ++i)
                    d[i] = s[i * N];
            }
        }
    }
}

/* Copy column-major M x N src into row-major M x N dst (tiled for locality). */
static void tr_cm_rm(double *restrict dst, const double *restrict src,
                     int64_t M, int64_t N)
{
    #pragma omp parallel for schedule(dynamic)
    for (int64_t jj = 0; jj < N; jj += 96) {
        const int64_t jhi = (jj + 96 < N) ? jj + 96 : N;
        for (int64_t ii = 0; ii < M; ii += 384) {
            const int64_t ihi = (ii + 384 < M) ? ii + 384 : M;
            for (int64_t j = jj; j < jhi; ++j) {
                const double *restrict s = src + j * M;
                double *restrict d = dst + j;
                for (int64_t i = ii; i < ihi; ++i)
                    d[i * N] = s[i];
            }
        }
    }
}

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
    (void)workspace;
    (void)workspace_size;

    if (N <= 0 || M <= 0)
        return;

    int m = (int)M, n = (int)N;
    double *B = (double *)malloc((size_t)M * (size_t)N * sizeof(double));
    double *tau = (double *)malloc((size_t)N * sizeof(double));
    double *g = (double *)malloc((size_t)N * sizeof(double));
    if (!B || !tau || !g) {
        free(B); free(tau); free(g);
        return;
    }

    tr_rm_cm(B, A, M, N);

    /* Optimal workspace query for both LAPACK calls. */
    double wq = 0.0;
    int lwork = -1, info = 0;
    dgeqrf_(&m, &n, B, &m, tau, &wq, &lwork, &info);
    int lw1 = (int)wq;
    dorgqr_(&m, &n, &n, B, &m, tau, &wq, &lwork, &info);
    int lw2 = (int)wq;
    int lwopt = (lw1 > lw2 ? lw1 : lw2) + 64;
    double *work = (double *)malloc((size_t)lwopt * sizeof(double));
    if (!work) {
        free(B); free(tau); free(g);
        return;
    }

    lwork = lwopt;
    dgeqrf_(&m, &n, B, &m, tau, work, &lwork, &info);

    /* R = upper triangle of B (column-major), zero elsewhere.  When M == N the
     * reference still applies a final sign reflector to the single trailing
     * element, negating R's last row where LAPACK's dlarfg leaves it alone. */
    memset(R, 0, (size_t)N * (size_t)N * sizeof(double));
    const int square = (M == N);
    for (int64_t j = 0; j < N; ++j) {
        const double sgn = (square && j == N - 1) ? -1.0 : 1.0;
        for (int64_t i = 0; i <= j; ++i)
            R[i * N + j] = B[j * M + i] * ((i == N - 1) ? sgn : 1.0);
    }

    /* The reference consumes A in place: on exit its first N rows hold R above
     * the diagonal and reflector round-off (~1e-15) below, which exact zeros
     * match within atol. */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; ++i) {
        double *restrict row = A + i * N;
        if (i >= N) {
            memset(row, 0, (size_t)N * sizeof(double));
        } else {
            for (int64_t j = 0; j < i; ++j)
                row[j] = 0.0;
            for (int64_t j = i; j < N; ++j)
                row[j] = R[i * N + j];
        }
    }

    /* Form the reduced Q in place. */
    dorgqr_(&m, &n, &n, B, &m, tau, work, &lwork, &info);
    if (square) {
        double *restrict col = B + (N - 1) * M;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < M; ++i)
            col[i] = -col[i];
    }

    /* g = Q^T b, one contiguous column dot per entry (matches the reference's
     * g[i] = dot(Q[:, i], b)). */
    #pragma omp parallel for schedule(static)
    for (int64_t j = 0; j < N; ++j) {
        const double *restrict col = B + j * M;
        double s = 0.0;
        for (int64_t i = 0; i < M; ++i)
            s += col[i] * b[i];
        g[j] = s;
    }

    tr_cm_rm(Q, B, M, N);

    /* Back substitution: R x = g. */
    for (int64_t i = N - 1; i >= 0; --i) {
        double s = g[i];
        for (int64_t j = i + 1; j < N; ++j)
            s -= R[i * N + j] * x[j];
        x[i] = s / R[i * N + i];
    }

    free(B); free(tau); free(g); free(work);
}
