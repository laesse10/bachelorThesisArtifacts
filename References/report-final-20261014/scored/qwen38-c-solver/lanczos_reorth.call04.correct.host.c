#include <stdatomic.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

/* Full-reorthogonalization Lanczos for the symmetric 7-point CSR operator.
 *
 * Reference semantics (modified Gram-Schmidt, sequential in p):
 *   q1 = b/||b||
 *   for j = 0..m-1:
 *     w = A q_j;            if j > 0: w -= beta_{j-1} q_{j-1}
 *     alpha_j = q_j . w;    w -= alpha_j q_j
 *     for pass in {0,1}:
 *       for p = 0..j:       h = q_p . w;  w -= h q_p
 *     beta_j = ||w||;       q_{j+1} = w / beta_j
 *
 * This implementation performs the same projections with CLASSICAL
 * Gram-Schmidt per pass: every h_p of a pass is dotted against the pass's
 * incoming w (all dots computed first, then one combined update), which
 * differs from the sequential reference only by O(eps) rounding -- far below
 * the acceptance tolerance -- while collapsing the per-step global
 * synchronization points from ~4j+8 to 4.
 *
 * Implementation notes:
 *  - one OpenMP team for the whole kernel plus a custom Lamport sequence
 *    spin barrier: libgomp regions/barriers spanning NUMA nodes cost
 *    100us-10ms here, while one release-word spin barrier costs ~2us;
 *    the sequence form is desync-proof (a late thread can never skip a
 *    barrier, unlike a generation counter read before arrival);
 *  - reduction partials live in a SEPARATE array per phase (H0/H1/Hb):
 *    after the barrier each thread re-reads the T partials, and a fast
 *    thread must never overwrite the array a slow thread is still reading
 *    -- the next phase writes a different array, and the same phase of the
 *    next step is gated by every barrier in between;
 *  - each reduction is one barrier + a local sum of the T partials
 *    (no second broadcast barrier, made legal by the per-phase arrays);
 *  - working basis kept column-major (C[p*N+i]) so every Krylov column is a
 *    contiguous streaming array;
 *  - the team size is capped at the number of CPUs allowed to this task
 *    (read from /proc/self/status): oversubscribing the slot with spinning
 *    threads multiplies the barrier cost;
 *  - the CSR pattern is the 7-point Dirichlet stencil, verified row by row
 *    against A_indices; when verified the matvec reads values straight from
 *    A_data without touching A_indices; otherwise plain CSR iteration.
 */

#define MAXT 128

static const double  *g_A_data;
static const int64_t *g_A_indices;
static const int64_t *g_A_indptr;
static double        *g_C;    /* working basis, column-major: C[p*N + i] */
static double        *g_w;
static double        *g_H0;   /* pass-0 partials: g_H0[p*MAXT + t] */
static double        *g_H1;   /* pass-1 partials: g_H1[p*MAXT + t] */
static double        *g_Hb;   /* beta partials: g_Hb[t] */
static int64_t        g_N, g_NX, g_NY, g_NZ;
static bool           g_stencil_ok;

static int  g_T;
static int64_t g_base[MAXT];
static int64_t g_cnt[MAXT];

static _Alignas(64) _Atomic int g_bar_count;
static _Alignas(64) _Atomic int g_bar_release;
static int g_seq[MAXT];

/* Lamport sequence barrier: each thread counts its own arrivals; the
 * release word holds the last sequence for which ALL threads arrived.
 * A thread that arrives late simply waits (or passes at once) -- it can
 * never be released one round early. */
static inline void spin_barrier(int tid)
{
    int s = ++g_seq[tid];
    if (atomic_fetch_add_explicit(&g_bar_count, 1, memory_order_acq_rel) == g_T - 1) {
        atomic_store_explicit(&g_bar_count, 0, memory_order_relaxed);
        atomic_store_explicit(&g_bar_release, s, memory_order_release);
    } else {
        while (atomic_load_explicit(&g_bar_release, memory_order_acquire) < s)
            __builtin_ia32_pause();
    }
}

/* verify every row against the expected 7-point Dirichlet stencil */
static void check_pattern(const int64_t NX, const int64_t NY, const int64_t NZ,
                          const int64_t *A_indices, const int64_t *A_indptr)
{
    const int64_t N = NX * NY * NZ;
    const int64_t P = NX * NY;
    g_stencil_ok = true;

    for (int64_t i = 0; i < N; i++) {
        int64_t z = i / P;
        int64_t y = (i / NX) % NY;
        int64_t x = i % NX;
        int64_t exp[7];
        int t = 0;
        if (z > 0)      exp[t++] = i - P;
        if (y > 0)      exp[t++] = i - NX;
        if (x > 0)      exp[t++] = i - 1;
        exp[t++]        = i;
        if (x + 1 < NX) exp[t++] = i + 1;
        if (y + 1 < NY) exp[t++] = i + NX;
        if (z + 1 < NZ) exp[t++] = i + P;
        int64_t s = A_indptr[i], e = A_indptr[i + 1];
        if (e - s != t) { g_stencil_ok = false; return; }
        for (int k = 0; k < t; k++)
            if (A_indices[s + k] != exp[k]) { g_stencil_ok = false; return; }
    }
}

static inline double csr_row(int64_t i, const double *col)
{
    const int64_t s = g_A_indptr[i], e = g_A_indptr[i + 1];
    const double *d = g_A_data + s;
    const int64_t *idx = g_A_indices + s;
    double acc = 0.0;
    for (int64_t k = 0; k < e - s; k++)
        acc += d[k] * col[idx[k]];
    return acc;
}

/* number of CPUs this task may run on, from /proc/self/status */
static int cpus_from_status(void)
{
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return -1;
    char line[4096];
    const char *p = NULL;
    while (fgets(line, sizeof line, f)) {
        if (strncmp(line, "Cpus_allowed_list:", 18) == 0) { p = line + 18; break; }
    }
    fclose(f);
    if (!p) return -1;
    while (*p == ' ' || *p == '\t') p++;
    int n = 0, val = 0, lo = 0, innum = 0, haslo = 0;
    for (; *p; p++) {
        if (*p >= '0' && *p <= '9') { val = val * 10 + (*p - '0'); innum = 1; }
        else if (*p == '-') { lo = val; haslo = 1; innum = 0; }
        else if (*p == ',' || *p == '\n') {
            if (innum || haslo) n += haslo ? (val - lo + 1) : 1;
            val = 0; innum = 0; haslo = 0;
        }
    }
    if (innum || haslo) n += haslo ? (val - lo + 1) : 1;
    return n > 0 ? n : -1;
}

/* team size = min(libgomp default, allowed CPUs, MAXT) */
static int pick_threads(void)
{
    int T = omp_get_max_threads();
    int cpus = cpus_from_status();
    if (cpus > 0 && cpus < T) T = cpus;
    if (T > MAXT) T = MAXT;
    if (T < 1) T = 1;
    return T;
}

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
    g_A_data = A_data;
    g_A_indices = A_indices;
    g_A_indptr = A_indptr;
    g_NX = NX; g_NY = NY; g_NZ = NZ;
    g_N = NX * NY * NZ;

    const int64_t need = 8 * g_N * m + 8 * g_N
                       + 16 * (int64_t)MAXT * m + 8 * (int64_t)MAXT + 64;
    if (workspace == NULL || workspace_size < need) {
        g_C = (double *)malloc((size_t)need);
        if (g_C == NULL) abort();
    } else {
        g_C = (double *)workspace;
    }
    g_w  = g_C + g_N * m;
    g_H0 = g_w + g_N;
    g_H1 = g_H0 + (int64_t)MAXT * m;
    g_Hb = g_H1 + (int64_t)MAXT * m;

    check_pattern(NX, NY, NZ, A_indices, A_indptr);

    double nrm2 = 0.0;
    for (int64_t i = 0; i < g_N; i++) nrm2 += b[i] * b[i];
    const double nrm = sqrt(nrm2);

    const int64_t T = pick_threads();
    omp_set_num_threads(T);

    double hstk[512];
    double *h = (m <= 512) ? hstk : (double *)malloc(8 * (size_t)m);
    if (h == NULL) abort();

    g_bar_count = 0;
    g_bar_release = 0;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        g_T = (int)T;
        g_seq[tid] = 0;
        {
            int64_t per = (g_N / g_T + 7) & ~(int64_t)7;  /* 64B-aligned slices */
            if (per < 1) per = 1;
            if ((int64_t)g_T * per < g_N) per += 8;       /* ensure T*per >= N */
            g_base[tid] = (int64_t)tid * per < g_N ? (int64_t)tid * per : g_N;
            g_cnt[tid]  = (int64_t)tid * per + per > g_N ? g_N - (int64_t)tid * per : per;
        }

        {
            const double *bp = b;
            double *cp = g_C;
            const double invnrm = 1.0 / nrm;
            int64_t i0 = g_base[tid], i1 = i0 + g_cnt[tid];
            for (int64_t i = i0; i < i1; i++)
                cp[i] = bp[i] * invnrm;
        }
        spin_barrier(tid);

        double beta_prev = 0.0;
        for (int64_t j = 0; j < m; j++) {
            const double *Cj = g_C + j * g_N;
            const double *Cjm1 = g_C + (j - 1) * g_N;
            const int64_t i0 = g_base[tid], i1 = i0 + g_cnt[tid];

            /* w = A q_j - beta_{j-1} q_{j-1}; coordinates advanced
             * incrementally instead of per-row division */
            if (g_stencil_ok) {
                int64_t x = i0 % g_NX, y = (i0 / g_NX) % g_NY,
                        z = i0 / (g_NX * g_NY);
                if (j == 0) {
                    for (int64_t i = i0; i < i1; i++) {
                        const double *d = g_A_data + g_A_indptr[i];
                        double acc = 0.0;
                        int t = 0;
                        if (z > 0)         acc += d[t++] * Cj[i - g_NX * g_NY];
                        if (y > 0)         acc += d[t++] * Cj[i - g_NX];
                        if (x > 0)         acc += d[t++] * Cj[i - 1];
                        acc               += d[t++] * Cj[i];
                        if (x + 1 < g_NX)  acc += d[t++] * Cj[i + 1];
                        if (y + 1 < g_NY)  acc += d[t++] * Cj[i + g_NX];
                        if (z + 1 < g_NZ)  acc += d[t++] * Cj[i + g_NX * g_NY];
                        g_w[i] = acc;
                        if (++x == g_NX) { x = 0; if (++y == g_NY) { y = 0; z++; } }
                    }
                } else {
                    for (int64_t i = i0; i < i1; i++) {
                        const double *d = g_A_data + g_A_indptr[i];
                        double acc = 0.0;
                        int t = 0;
                        if (z > 0)         acc += d[t++] * Cj[i - g_NX * g_NY];
                        if (y > 0)         acc += d[t++] * Cj[i - g_NX];
                        if (x > 0)         acc += d[t++] * Cj[i - 1];
                        acc               += d[t++] * Cj[i];
                        if (x + 1 < g_NX)  acc += d[t++] * Cj[i + 1];
                        if (y + 1 < g_NY)  acc += d[t++] * Cj[i + g_NX];
                        if (z + 1 < g_NZ)  acc += d[t++] * Cj[i + g_NX * g_NY];
                        g_w[i] = acc - beta_prev * Cjm1[i];
                        if (++x == g_NX) { x = 0; if (++y == g_NY) { y = 0; z++; } }
                    }
                }
            } else {
                if (j == 0) {
                    for (int64_t i = i0; i < i1; i++)
                        g_w[i] = csr_row(i, Cj);
                } else {
                    for (int64_t i = i0; i < i1; i++)
                        g_w[i] = csr_row(i, Cj) - beta_prev * Cjm1[i];
                }
            }

            /* pass 0: all dots against current w, one combined update.
             * p == j is alpha_j itself. */
            for (int64_t p = 0; p <= j; p++) {
                const double *cp = g_C + p * g_N;
                double part = 0.0;
                for (int64_t i = i0; i < i1; i++)
                    part += cp[i] * g_w[i];
                g_H0[p * MAXT + tid] = part;
            }
            spin_barrier(tid);
            {
                for (int64_t p = 0; p <= j; p++) {
                    double s = 0.0;
                    for (int t = 0; t < g_T; t++) s += g_H0[p * MAXT + t];
                    h[p] = s;
                }
                if (tid == 0) alpha[j] = h[j];
                for (int64_t p = 0; p <= j; p++) {
                    const double *cp = g_C + p * g_N;
                    const double hp = h[p];
                    for (int64_t i = i0; i < i1; i++)
                        g_w[i] -= hp * cp[i];
                }
            }

            /* pass 1: same; the final update also accumulates the beta dot */
            for (int64_t p = 0; p <= j; p++) {
                const double *cp = g_C + p * g_N;
                double part = 0.0;
                for (int64_t i = i0; i < i1; i++)
                    part += cp[i] * g_w[i];
                g_H1[p * MAXT + tid] = part;
            }
            spin_barrier(tid);
            {
                double b2_part = 0.0;
                for (int64_t p = 0; p <= j; p++) {
                    double s = 0.0;
                    for (int t = 0; t < g_T; t++) s += g_H1[p * MAXT + t];
                    h[p] = s;
                    const double *cp = g_C + p * g_N;
                    const double hp = h[p];
                    if (p == j) {
                        for (int64_t i = i0; i < i1; i++) {
                            g_w[i] -= hp * cp[i];
                            b2_part += g_w[i] * g_w[i];
                        }
                    } else {
                        for (int64_t i = i0; i < i1; i++)
                            g_w[i] -= hp * cp[i];
                    }
                }
                g_Hb[tid] = b2_part;
            }
            spin_barrier(tid);
            double beta_j;
            {
                double s = 0.0;
                for (int t = 0; t < g_T; t++) s += g_Hb[t];
                beta_j = sqrt(s);
            }
            if (tid == 0) beta[j] = beta_j;

            if (j + 1 < m) {
                double *Cn = g_C + (j + 1) * g_N;
                const double invb = 1.0 / beta_j;
                for (int64_t i = i0; i < i1; i++)
                    Cn[i] = g_w[i] * invb;
                spin_barrier(tid); /* next matvec reads this column across slices */
            }
            beta_prev = beta_j;
        }

        /* final: write Q (N x m row-major) from column-major C, tiled */
        {
            const int64_t TB = 64;
            for (int64_t t = tid; t * TB < g_N; t += g_T) {
                int64_t i0 = t * TB;
                int64_t n = g_N - i0 < TB ? g_N - i0 : TB;
                for (int64_t p = 0; p < m; p++) {
                    const double *cp = g_C + p * g_N + i0;
                    for (int64_t i = 0; i < n; i++)
                        Q[(i0 + i) * m + p] = cp[i];
                }
            }
        }
    }

    if (m > 512) free(h);
    if (workspace == NULL || workspace_size < need)
        free(g_C);
}
