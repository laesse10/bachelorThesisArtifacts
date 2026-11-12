#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Householder QR factorization (A -> Q, R) + least-squares solve R x = Q^T b.
 * Matches the NumPy reference semantics.
 */

static double *g_V = NULL;      /* column-major scratch: V[col*M + row] */
static size_t  g_Vcap = 0;
static double *g_part = NULL;   /* per-thread partial accumulators [nthreads * N] */
static size_t  g_partcap = 0;

/* 64-byte aligned allocation (store original pointer for aligned free) */
static double *amalloc(size_t need) {
    size_t bytes = (need * sizeof(double) + 63);
    unsigned char *raw = malloc(bytes + 8);
    if (!raw) return NULL;
    unsigned char *p = (unsigned char*)(((uintptr_t)raw + 64) & ~(uintptr_t)63);
    ((unsigned char**)p)[-1] = raw;
    return (double*)p;
}
static void afree(double *p) { if (p) free(((unsigned char**)p)[-1]); }

static double *ensure(double **buf, size_t *cap, size_t need) {
    if (*cap < need) {
        if (*buf) afree(*buf);
        *buf = amalloc(need);
        *cap = *buf ? need : 0;
    }
    return *buf;
}

void householder_qr_fp64(
    double *restrict A, double *restrict Q, double *restrict R,
    const double *restrict b, double *restrict x,
    const int64_t M, const int64_t N,
    uint8_t *restrict workspace, const int64_t workspace_size) {

    (void)workspace; (void)workspace_size;

    const int nt = omp_get_max_threads();
    double *V = ensure(&g_V, &g_Vcap, (size_t)M * (size_t)N);
    double *part = ensure(&g_part, &g_partcap, (size_t)nt * (size_t)N);
    double *beta = malloc((size_t)N * sizeof(double));
    double *w = malloc((size_t)N * sizeof(double));
    double *g = malloc((size_t)N * sizeof(double));

    /* ================= Phase 1: factor A into R, store reflectors V, beta ================ */
    for (int64_t k = 0; k < N; k++) {
        const int64_t nk = N - k;          /* trailing columns k..N-1 */
        const double Akk = A[k * N + k];

        /* S[j] = sum_{i=k+1}^{M-1} A[i,k] * A[i, k+j]   (j in 0..nk-1), parallel over rows */
        for (int j = 0; j < nk; j++) w[j] = 0.0;
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            double *sp = part + (size_t)tid * (size_t)N;
            for (int j = 0; j < nk; j++) sp[j] = 0.0;
            const int64_t nrows = M - k - 1;              /* rows k+1..M-1 */
            const int64_t chunk = (nrows + nt - 1) / nt;
            const int64_t r0 = (int64_t)tid * chunk;
            int64_t r1 = r0 + chunk; if (r1 > nrows) r1 = nrows;
            for (int64_t ii = r0; ii < r1; ii++) {
                const int64_t row = k + 1 + ii;
                const double vc = A[row * N + k];
                V[k * M + row] = vc;
                const double *arow = A + row * N + k;
                for (int j = 0; j < nk; j++) sp[j] += vc * arow[j];
            }
        }
        for (int j = 0; j < nk; j++) {
            double s = 0.0;
            for (int t = 0; t < nt; t++) s += part[(size_t)t * (size_t)N + j];
            w[j] = s;
        }

        const double s2 = Akk * Akk + w[0];
        const double normx = sqrt(s2);
        const double sign = (Akk >= 0.0) ? 1.0 : -1.0;
        const double Vk = Akk + sign * normx;
        const double vv = Vk * Vk + w[0];
        const double betak = (vv > 0.0) ? 2.0 / vv : 0.0;
        beta[k] = betak;

        /* w[j] = betak * (Vk * A[k,k+j] + S[j]) */
        const double *Arowk = A + k * N + k;
        for (int j = 0; j < nk; j++) w[j] = betak * (Vk * Arowk[j] + w[j]);
        V[k * M + k] = Vk;

        /* update trailing: A[i, k+j] -= V[k*M+i] * w[j]  (i in k..M-1, j in 0..nk-1) */
        #pragma omp parallel for schedule(static)
        for (int64_t i = k; i < M; i++) {
            const double vci = V[k * M + i];
            double *arow = A + i * N + k;
            for (int j = 0; j < nk; j++) arow[j] -= vci * w[j];
        }
        /* R[k, k:N] = A[k, k:N] */
        const double *Arowk2 = A + k * N + k;
        for (int j = 0; j < nk; j++) R[k * N + k + j] = Arowk2[j];
    }

    /* ================= Phase 2: build Q = H_0 ... H_{N-1} from the identity ============= */
    for (int64_t i = 0; i < N; i++) Q[i * N + i] = 1.0;
    for (int64_t k = N - 1; k >= 0; k--) {
        const double betak = beta[k];
        if (betak == 0.0) continue;
        /* w[j] = betak * sum_{i=k}^{M-1} V[k*M+i] * Q[i, j]   (j in 0..N-1) */
        for (int j = 0; j < N; j++) w[j] = 0.0;
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            double *sp = part + (size_t)tid * (size_t)N;
            for (int j = 0; j < N; j++) sp[j] = 0.0;
            const int64_t nrows = M - k;
            const int64_t chunk = (nrows + nt - 1) / nt;
            const int64_t r0 = (int64_t)tid * chunk;
            int64_t r1 = r0 + chunk; if (r1 > nrows) r1 = nrows;
            for (int64_t ii = r0; ii < r1; ii++) {
                const int64_t row = k + ii;
                const double vc = V[k * M + row];
                const double *qrow = Q + row * N;
                for (int j = 0; j < N; j++) sp[j] += vc * qrow[j];
            }
        }
        for (int j = 0; j < N; j++) {
            double s = 0.0;
            for (int t = 0; t < nt; t++) s += part[(size_t)t * (size_t)N + j];
            w[j] = betak * s;
        }
        /* Q[i, j] -= V[k*M+i] * w[j]  (i in k..M-1, j in 0..N-1) */
        #pragma omp parallel for schedule(static)
        for (int64_t i = k; i < M; i++) {
            const double vci = V[k * M + i];
            double *qrow = Q + i * N;
            for (int j = 0; j < N; j++) qrow[j] -= vci * w[j];
        }
    }

    /* ================= g = Q^T b ; then back-substitute R x = g ======================== */
    for (int j = 0; j < N; j++) g[j] = 0.0;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *sp = part + (size_t)tid * (size_t)N;
        for (int j = 0; j < N; j++) sp[j] = 0.0;
        const int64_t chunk = (M + nt - 1) / nt;
        const int64_t r0 = (int64_t)tid * chunk;
        int64_t r1 = r0 + chunk; if (r1 > M) r1 = M;
        for (int64_t row = r0; row < r1; row++) {
            const double bb = b[row];
            const double *qrow = Q + row * N;
            for (int j = 0; j < N; j++) sp[j] += qrow[j] * bb;
        }
    }
    for (int j = 0; j < N; j++) {
        double s = 0.0;
        for (int t = 0; t < nt; t++) s += part[(size_t)t * (size_t)N + j];
        g[j] = s;
    }
    for (int64_t i = N - 1; i >= 0; i--) {
        double s = g[i];
        const double *rrow = R + i * N + i + 1;
        for (int64_t j = i + 1; j < N; j++) s -= rrow[j - i - 1] * x[j];
        x[i] = s / R[i * N + i];
    }

    free(beta); free(w); free(g);
}
