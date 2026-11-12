/* Full-reorthogonalization Lanczos (symmetric Krylov basis + tridiagonal T).
 *
 * Output layout follows the NumPy reference: Q is an (N, m) row-major array,
 * element (i, j) at Q[i*m + j].
 *
 * The outer step loop j = 0..m-1 is sequential (three-term recurrence).
 * Inside a step everything data-parallel over N is split into static chunks
 * over the OpenMP team.
 *
 *   w   = A q_j  - beta_{j-1} q_{j-1}
 *   a_j = q_j . w ;  w -= a_j q_j     (a_j is the reorth dot against q_j)
 *   w  -= sum_{p<j} (q_p . w) q_p     (reorthogonalization pass)
 *   beta_j = ||w|| ;  q_{j+1} = w / beta_j
 *
 * Barrier placement.  A thread only ever reads w OUTSIDE its own chunk at
 * values that the next step's matvec will overwrite anyway, and its own
 * chunk is updated in program order, so each step needs only:
 *   [MATVEC+DOT_j] w = A q_j - beta q_{j-1};  part = q_j . w   (no sync)
 *   --barrier-- (publishes w and part)
 *   [SCAN+AXPY_j] h = sum part  (h is alpha[j]);  w -= h q_j, own chunk
 *   for p = j-1 ... 0:
 *     [DOT_p] part = q_p . w                  (no sync needed before:
 *        own chunk is fresh in program order, foreign chunks are stale but
 *        are never read by the partial that owns them)
 *     --barrier-- (publishes part and w)
 *     [SCAN+AXPY_p] h = sum part;  w -= h q_p
 *   [NORM] part = w . w;  --barrier--;  h = sum part
 *   [SCALE] q_{j+1} = w / beta ;  --barrier-- (the next matvec reads the
 *        fresh column at arbitrary CSR indices, so this one is mandatory)
 * i.e. j + 3 barriers per step (the reference's two reorthogonalization
 * passes need 2j + 5).  Sums are per-thread partials in part[]: this
 * toolchain's libgomp drops omp-reduction partials at >= 12 threads, so
 * the scan is done by hand after each barrier (the barrier's release/
 * acquire fences make the part[] and w publishes visible).
 *
 * The basis is kept transposed in scratch (column j contiguous, T[j*N + i])
 * so every sweep is a contiguous stream; the final (N, m) layout is produced
 * by a blocked transpose at the end.  The reference reorthogonalizes against
 * q_j twice (the second dot is ~1e-16); the single dot against q_j is
 * exactly the reference's alpha_j, so the two operations coincide.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <sys/syscall.h>
#ifndef SYS_mbind
#define SYS_mbind 236
#endif

#ifndef MAXT
#define MAXT 256
#endif
#ifndef TRY_NUMA
#define TRY_NUMA 0
#endif

static double part[MAXT]; /* per-thread dot partials, kept zeroed between uses */

static double *xalloc(const int64_t bytes)
{
    void *p = aligned_alloc(64, (size_t)bytes);
    if (!p) abort();
    return p;
}

/* best-effort NUMA interleave of the scratch (pages round-robin over nodes) */
static void interleave(void *addr, const int64_t len)
{
#if defined(__linux__) && TRY_NUMA
    /* mbind(2); MPOL_INTERLEAVE = 2; first 8 nodes */
    unsigned long mask = 0xFFul;
    (void)syscall(SYS_mbind, (unsigned long)addr, (unsigned long)len, 2ul,
                  (unsigned long)&mask, 8ul, 0ul);
#else
    (void)addr; (void)len;
#endif
}

static double *get_scratch(uint8_t *restrict workspace, const int64_t workspace_size,
                           const int64_t need, bool *owned)
{
    /* scratch layout: [T : 8*N*m bytes][w : 8*N bytes], 64B aligned head */
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
    const int64_t need = tbytes + N * (int64_t)sizeof(double) + 4096;

    bool owned;
    double *base = get_scratch(workspace, workspace_size, need, &owned);
    double *T = base;
    double *w = base + tbytes / 8;
    if (owned) interleave(base, tbytes + N * (int64_t)sizeof(double));

    if (omp_get_max_threads() > MAXT) omp_set_num_threads(MAXT);

    #pragma omp parallel
    {
        const int nt  = omp_get_num_threads();
        const int tid = omp_get_thread_num();

        /* 8-double aligned chunks (64B, base is aligned) -> clean AVX-512;
           the load imbalance is < 8*nt elements out of N >= 10000 */
        const int64_t chunk = ((N + (int64_t)nt * 8 - 1) / ((int64_t)nt * 8)) * 8;
        const int64_t beg   = (int64_t)tid * chunk;
        const int64_t end   = (N < beg + chunk) ? N : beg + chunk;

        /* ---- q0 = b / ||b|| ---- */
        double s0 = 0.0;
        for (int64_t i = beg; i < end; i++) s0 += b[i] * b[i];
        part[tid] = s0;
        #pragma omp barrier
        s0 = 0.0;
        for (int t = 0; t < nt; t++) s0 += part[t];
        const double nrm = sqrt(s0);

        double *restrict q0 = T;
        for (int64_t i = beg; i < end; i++) q0[i] = b[i] / nrm;
        #pragma omp barrier

        double beta_prev = 0.0;
        const double *q_prev = q0;

        for (int64_t j = 0; j < m; j++) {
            const double *restrict qj = T + j * N;

            /* w = A q_j - beta_prev q_prev, and the q_j . w partial fused in:
               the dot reads only this thread's own chunk of w (fresh in
               program order), so no barrier sits between the matvec and it */
            double adot = 0.0;
            for (int64_t i = beg; i < end; i++) {
                double acc = 0.0;
                const int64_t k0 = A_indptr[i], k1 = A_indptr[i + 1];
                for (int64_t k = k0; k < k1; k++)
                    acc += A_data[k] * qj[A_indices[k]];
                const double wi = acc - beta_prev * q_prev[i];
                w[i] = wi;
                adot += qj[i] * wi;
            }
            part[tid] = adot;
            #pragma omp barrier

            double h = 0.0;
            for (int t = 0; t < nt; t++) h += part[t];
            alpha[j] = h;
            for (int64_t i = beg; i < end; i++) w[i] -= h * qj[i];

            for (int64_t p = j - 1; p >= 0; p--) {
                const double *restrict qp = T + p * N;
                double d = 0.0;
                for (int64_t i = beg; i < end; i++) d += qp[i] * w[i];
                part[tid] = d;
                #pragma omp barrier
                h = 0.0;
                for (int t = 0; t < nt; t++) h += part[t];
                for (int64_t i = beg; i < end; i++) w[i] -= h * qp[i];
            }

            /* beta_j = ||w|| */
            double nw = 0.0;
            for (int64_t i = beg; i < end; i++) nw += w[i] * w[i];
            part[tid] = nw;
            #pragma omp barrier
            double s2 = 0.0;
            for (int t = 0; t < nt; t++) s2 += part[t];
            const double bj = sqrt(s2);
            beta[j] = bj;

            if (j + 1 < m) {
                double *restrict qn = T + (j + 1) * N;
                for (int64_t i = beg; i < end; i++) qn[i] = w[i] / bj;
                #pragma omp barrier
            }
            q_prev = qj;
            beta_prev = bj;
        }

        /* no barrier needed here: the last step performs no writes after its
           norm barrier, and every T column was published by its own barrier */

        /* ---- final transpose: T[j*N + i] -> Q[i*m + j], 64x64 tiles ---- */
        const int64_t nblocks = (N + 63) / 64;
        const int64_t bchunk = (nblocks + nt - 1) / nt;
        const int64_t bbeg = (int64_t)tid * bchunk;
        const int64_t bend = (nblocks < bbeg + bchunk) ? nblocks : bbeg + bchunk;
        for (int64_t ib = bbeg; ib < bend; ib++) {
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
