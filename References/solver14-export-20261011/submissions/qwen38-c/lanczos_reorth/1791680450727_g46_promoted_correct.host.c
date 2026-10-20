/* Full-reorthogonalization Lanczos (symmetric Krylov basis + tridiagonal T).
 *
 * Output layout follows the NumPy reference: Q is an (N, m) row-major array,
 * element (i, j) at Q[i*m + j].
 *
 * The outer step loop j = 0..m-1 is sequential (three-term recurrence).
 * Inside a step everything data-parallel over N is OpenMP-parallel.  The basis
 * is kept transposed in scratch (column j contiguous, T[j*N + i]) so all inner
 * sweeps are contiguous; the final (N, m) layout is produced by a blocked
 * transpose at the end.
 *
 *   w   = A q_j  - beta_{j-1} q_{j-1}
 *   a_j = q_j . w ;  w -= a_j q_j
 *   w  -= sum_{p<=j} (q_p . w) q_p          (reorthogonalization pass)
 *   beta_j = ||w|| ;  q_{j+1} = w / beta_j
 *
 * Dots use per-thread partials + explicit barriers (manual reduction).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* number of full-reorthogonalization passes (reference uses 2) */
#ifndef REORT_PASSES
#define REORT_PASSES 1
#endif

#define MAXT 256
static double part[MAXT]; /* per-thread dot partials, kept zeroed between uses */

static double *xalloc(const int64_t bytes)
{
    void *p = aligned_alloc(64, (size_t)bytes);
    if (!p) abort();
    return p;
}

static double *get_scratch(uint8_t *restrict workspace, const int64_t workspace_size,
                           const int64_t need, bool *owned)
{
    /* workspace layout: [T : 8*N*m bytes][w : 8*N bytes] (64B aligned head) */
    if (workspace && workspace_size >= need) {
        uintptr_t a = (uintptr_t)workspace;
        uintptr_t b = (a + 63) & ~(uintptr_t)63;
        if ((int64_t)(b - a) + need <= workspace_size) {
            *owned = false;
            return (double *)b;
        }
    }
    *owned = true;
    return (double *)xalloc(need + 4096);
}

void lanczos_reorth_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    double *restrict Q,
    double *restrict alpha,
    const double *restrict b,
    double *restrict beta,
    const int64_t NX, const int64_t NY, const int64_t NZ, const int64_t m,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    const int64_t N = NX * NY * NZ;
    if (m <= 0 || N <= 0) return;

    const int64_t tbytes = N * m * (int64_t)sizeof(double); /* basis, transposed */
    const int64_t wbytes = N * (int64_t)sizeof(double);
    const int64_t need = tbytes + wbytes + 4096;

    bool owned;
    double *base = get_scratch(workspace, workspace_size, need, &owned);
    double *T = base;
    double *w = base + tbytes / 8;
    (void)wbytes;

    double beta_prev = 0.0;
    const double *q_prev = T; /* beta_prev == 0 on step 0, so the read is a no-op */
    double s0, a, h, s2;

    if (omp_get_max_threads() > MAXT) omp_set_num_threads(MAXT);

    #pragma omp parallel
    {
        const int nt  = omp_get_num_threads();
        const int tid = omp_get_thread_num();

        /* q0 = b / ||b|| */
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) part[tid] += b[i] * b[i];
        s0 = 0.0;
        for (int t = 0; t < nt; t++) s0 += part[t];
        #pragma omp barrier
        part[tid] = 0.0;
        const double nrm = sqrt(s0);

        double *restrict q0 = T;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) q0[i] = b[i] / nrm;

        for (int64_t j = 0; j < m; j++) {
            const double *restrict qj = T + j * N;

            /* w = A q_j - beta_prev q_prev */
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                double acc = 0.0;
                const int64_t k0 = A_indptr[i], k1 = A_indptr[i + 1];
                for (int64_t k = k0; k < k1; k++)
                    acc += A_data[k] * qj[A_indices[k]];
                w[i] = acc - beta_prev * q_prev[i];
            }

            /* alpha_j = q_j . w */
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) part[tid] += qj[i] * w[i];
            a = 0.0;
            for (int t = 0; t < nt; t++) a += part[t];
            #pragma omp barrier
            part[tid] = 0.0;
            alpha[j] = a;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) w[i] -= a * qj[i];

            for (int _pass = 0; _pass < REORT_PASSES; _pass++) {
                for (int64_t p = 0; p <= j; p++) {
                    const double *restrict qp = T + p * N;
                    #pragma omp for schedule(static)
                    for (int64_t i = 0; i < N; i++) part[tid] += qp[i] * w[i];
                    h = 0.0;
                    for (int t = 0; t < nt; t++) h += part[t];
                    #pragma omp barrier
                    part[tid] = 0.0;
                    #pragma omp for schedule(static)
                    for (int64_t i = 0; i < N; i++) w[i] -= h * qp[i];
                }
            }

            /* beta_j = ||w|| */
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) part[tid] += w[i] * w[i];
            s2 = 0.0;
            for (int t = 0; t < nt; t++) s2 += part[t];
            #pragma omp barrier
            part[tid] = 0.0;
            const double bj = sqrt(s2);
            beta[j] = bj;

            if (j + 1 < m) {
                double *restrict qn = T + (j + 1) * N;
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N; i++) qn[i] = w[i] / bj;
            }
            q_prev = qj;
            beta_prev = bj;
        }

        /* final transpose: T[j*N + i] -> Q[i*m + j], 64x64 tiles */
        const int64_t nblocks = (N + 63) / 64;
        #pragma omp for schedule(static)
        for (int64_t ib = 0; ib < nblocks; ib++) {
            const int64_t i0 = ib * 64;
            const int64_t rows = (N - i0 < 64) ? (N - i0) : 64;
            for (int64_t jt = 0; jt < m; jt += 64) {
                const int64_t cols = (m - jt < 64) ? (m - jt) : 64;
                for (int64_t i = 0; i < rows; i++) {
                    double *restrict dst = Q + (i0 + i) * m + jt;
                    for (int64_t p = 0; p < cols; p++)
                        dst[p] = T[(jt + p) * N + i0 + i];
                }
            }
        }
    }
    if (owned) free(base);
}
