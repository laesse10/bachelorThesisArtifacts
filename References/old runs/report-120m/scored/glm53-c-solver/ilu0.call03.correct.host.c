/* ILU(0) setup kernel (Saad, *Iterative Methods for Sparse Linear Systems*,
 * 2nd ed., Algorithm 10.4), restructured for locality.
 *
 * The reference eliminates row i against every already-factored row k < i of its
 * own pattern, in column order; each update is the sorted intersection of row
 * i's pattern (after the position p of k) with row k's strict upper triangle.
 * That intersection depends on the sparsity PATTERN only, so it is precomputed
 * here in barrier-free parallel passes, and the dependent arithmetic is then
 * replayed sequentially in exactly the reference's order, so the result is
 * bit-identical:
 *
 *   phase 0 (parallel): per-row structural scan -> upos[r], the start of row
 *                       r's strict upper triangle (diagonal position + 1),
 *                       plus sortedness / full-diagonal checks;
 *   phase 1a (parallel): an upper bound on the update-pair count per thread;
 *   phase 1b (parallel): the intersection search itself, filling a flat array
 *                       of packed (q, m) position pairs plus per-position
 *                       offsets ustart[p];
 *   phase 2 (sequential): the elimination -- one division per lower entry,
 *                       then the precomputed updates.  All accesses here are
 *                       sequential in the pair list and in row k's and row i's
 *                       own segments, so nothing is latency-bound.
 *
 * A CSR whose rows are not sorted, or whose diagonal is absent, falls back to a
 * direct port of the reference.  OpenMP workers are pinned to one allowed cpu
 * each for the duration of the call (unpinned workers migrate on this machine,
 * which costs an order of magnitude); their original mask is restored on exit.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

/* x86-64 syscall numbers: __NR_sched_setaffinity = 203, getaffinity = 204 */
extern long syscall(long, ...);
#define SYSNR_SETAFF 203
#define SYSNR_GETAFF 204

#define PIN_MAXCPUS 1024
typedef struct { uint64_t w[PIN_MAXCPUS / 64]; } cpumask_t;
static cpumask_t g_saved_mask[128];
static int g_was_pinned[128];

static void mask_zero(cpumask_t *m) { memset(m, 0, sizeof(*m)); }

static int mask_get(cpumask_t *m)
{
    mask_zero(m);
    return (int)syscall(SYSNR_GETAFF, 0, (unsigned long)sizeof(*m), m);
}

static int mask_pin(const cpumask_t *m)
{
    return (int)syscall(SYSNR_SETAFF, 0, (unsigned long)sizeof(*m), m);
}

static int mask_count(const cpumask_t *m)
{
    int n = 0;
    for (int w = 0; w < PIN_MAXCPUS / 64; w++)
        n += __builtin_popcountll(m->w[w]);
    return n;
}

/* pin the calling worker to one allowed cpu; remembers the old mask */
static void pin_worker(int tid, int nthreads)
{
    if (tid < 0 || tid >= 128) return;
    cpumask_t cur;
    if (mask_get(&cur) != 0) return;
    const int navail = mask_count(&cur);
    if (navail <= 1) return;                 /* already pinned (or none) */
    int cpus[PIN_MAXCPUS];
    int n = 0;
    for (int c = 0; c < PIN_MAXCPUS && n < navail; c++)
        if (cur.w[c >> 6] & ((uint64_t)1 << (c & 63))) cpus[n++] = c;
    const int mine = cpus[(int)((long)tid * navail / nthreads)];
    cpumask_t one;
    mask_zero(&one);
    one.w[mine >> 6] |= (uint64_t)1 << (mine & 63);
    g_saved_mask[tid] = cur;
    if (mask_pin(&one) == 0) g_was_pinned[tid] = 1;
}

static void restore_worker(int tid)
{
    if (tid < 0 || tid >= 128) return;
    if (g_was_pinned[tid]) {
        mask_pin(&g_saved_mask[tid]);
        g_was_pinned[tid] = 0;
    }
}

/* ---------------- fallback: exact port of the NumPy reference ---------------- */
static void ilu0_refport(double *restrict A_data, const int64_t *restrict A_indices,
                         const int64_t *restrict A_indptr, const int64_t N)
{
    double *diag = (double *)calloc((size_t)N, sizeof(double));
    double *row_val = (double *)calloc((size_t)N, sizeof(double));
    int64_t *in_row = (int64_t *)calloc((size_t)N, sizeof(int64_t));
    if (diag == NULL || row_val == NULL || in_row == NULL) {
        free(diag); free(row_val); free(in_row); return;
    }
    for (int64_t i = 0; i < N; i++) {
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        for (int64_t k = rs; k < re; k++) {
            row_val[A_indices[k]] = A_data[k];
            in_row[A_indices[k]] = 1;
        }
        for (int64_t k = rs; k < re; k++) {
            const int64_t col_k = A_indices[k];
            if (col_k >= i) continue;
            const double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik;
            for (int64_t m = A_indptr[col_k]; m < A_indptr[col_k + 1]; m++) {
                const int64_t col_j = A_indices[m];
                if (col_j > col_k && in_row[col_j] == 1)
                    row_val[col_j] -= l_ik * A_data[m];
            }
        }
        diag[i] = row_val[i];
        for (int64_t k = rs; k < re; k++) {
            A_data[k] = row_val[A_indices[k]];
            in_row[A_indices[k]] = 0;
        }
    }
    free(diag); free(row_val); free(in_row);
}

/* ---------------- fast path ---------------- */
typedef int32_t ipos_t;

/* cached scratch, grown on demand and kept across calls so that only the first
 * call pays for the page faults */
static int64_t g_bounds[128];
static int g_pat_bad = 0;
static uint64_t *g_pairs = NULL;
static int64_t g_pairs_cap = 0;
static uint32_t *g_ustart = NULL;
static int64_t g_ustart_cap = 0;
static uint64_t *g_uptr = NULL;
static int64_t g_uptr_cap = 0;
static double *g_diag = NULL;
static int64_t g_diag_cap = 0;

/* grow a cached buffer to at least (need) elements; returns NULL on failure */
static void *grow_buf(void *old, int64_t *cap, int64_t need, size_t elem)
{
    if (old != NULL && need <= *cap) return old;
    int64_t ncap = *cap > 0 ? *cap : 1024;
    while (ncap < need) ncap *= 2;
    void *p = realloc(old, (size_t)ncap * elem);
    if (p == NULL) return NULL;
    *cap = ncap;
    return p;
}

/* one row's structural scan: sets uptr[i] = (strict-upper start | row end) and
 * accumulates the update-pair upper bound; returns 0 ok, 1 bad pattern */
static inline int row_scan(const int64_t i, const int64_t *restrict A_indices,
                           const int64_t *restrict A_indptr,
                           uint64_t *restrict uptr, int64_t *restrict bnd)
{
    const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
    int64_t prev = -1;
    int64_t u = rs;
    int64_t b = *bnd;
    for (int64_t p = rs; p < re; p++) {
        const int64_t c = A_indices[p];
        if (c <= prev) { *bnd = b; return 1; }  /* unsorted or duplicated */
        prev = c;
        if (c <= i) {
            u = p + 1;
            /* matches <= min(len(ck) - 1, positions after p) */
            const int64_t a = A_indptr[c + 1] - A_indptr[c] - 1;
            const int64_t d = re - p - 1;
            b += a < d ? a : d;
        }
    }
    *bnd = b;
    /* full diagonal: the last column <= i must be i itself */
    if (u <= rs || A_indices[u - 1] != i) return 1;
    uptr[i] = (uint64_t)(uint32_t)u | ((uint64_t)(uint32_t)A_indptr[i + 1] << 32);
    return 0;
}

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}
static int g_ncall = 0;

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    const double t_start = now_ms();
    (void)workspace; (void)workspace_size;
    if (N <= 0) return;

    int nthreads = omp_get_max_threads();
    if (nthreads > 128) nthreads = 128;
    if (nthreads < 1) nthreads = 1;

    /* ---------- scratch (cached across calls) ---------- */
    {
        void *p = grow_buf(g_uptr, &g_uptr_cap, N, sizeof(uint64_t));
        if (p == NULL) { ilu0_refport(A_data, A_indices, A_indptr, N); return; }
        g_uptr = (uint64_t *)p;
    }

    /* ---------- phase 1: structural scan + per-thread pair bounds ---------- */
    if (nthreads > 1) {
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            const int64_t start = (int64_t)((__int128)N * tid / nth);
            const int64_t end = (int64_t)((__int128)N * (tid + 1) / nth);
            pin_worker(tid, nth);
            int64_t b = 0;
            int my_bad = 0;
            for (int64_t i = start; i < end; i++)
                if (row_scan(i, A_indices, A_indptr, g_uptr, &b)) { my_bad = 1; break; }
            g_bounds[tid] = b;
            if (my_bad) g_pat_bad = 1;   /* bad-pattern marker */
        }
    } else {
        int64_t b = 0;
        int bad = 0;
        for (int64_t i = 0; i < N; i++)
            if (row_scan(i, A_indices, A_indptr, g_uptr, &b)) { bad = 1; break; }
        g_bounds[0] = b;
        if (bad) g_pat_bad = 1;
    }
    if (g_pat_bad) {
        g_pat_bad = 0;
        ilu0_refport(A_data, A_indices, A_indptr, N);
        if (nthreads > 1) {
            #pragma omp parallel num_threads(nthreads)
            restore_worker(omp_get_thread_num());
        }
        return;
    }

    int64_t total_bound = 0;
    int64_t base[128];
    base[0] = 0;
    for (int t = 0; t < nthreads; t++) total_bound += g_bounds[t];
    for (int t = 1; t < nthreads; t++) base[t] = base[t - 1] + g_bounds[t - 1];
    if (total_bound > INT32_MAX) {
        ilu0_refport(A_data, A_indices, A_indptr, N);
        if (nthreads > 1) {
            #pragma omp parallel num_threads(nthreads)
            restore_worker(omp_get_thread_num());
        }
        return;
    }

    /* ---------- pair-list scratch (cached) ---------- */
    {
        void *p = grow_buf(g_pairs, &g_pairs_cap, total_bound > 0 ? total_bound : 1,
                           sizeof(uint64_t));
        if (p == NULL) { ilu0_refport(A_data, A_indices, A_indptr, N); return; }
        g_pairs = (uint64_t *)p;
        p = grow_buf(g_ustart, &g_ustart_cap, nnz + 1, sizeof(uint32_t));
        if (p == NULL) { ilu0_refport(A_data, A_indices, A_indptr, N); return; }
        g_ustart = (uint32_t *)p;
        p = grow_buf(g_diag, &g_diag_cap, N, sizeof(double));
        if (p == NULL) { ilu0_refport(A_data, A_indices, A_indptr, N); return; }
        g_diag = (double *)p;
    }

    const double t_ph1 = now_ms();
    /* ---------- phase 2: intersection search + fill ---------- */
    if (nthreads > 1) {
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            const int64_t start = (int64_t)((__int128)N * tid / nth);
            const int64_t end = (int64_t)((__int128)N * (tid + 1) / nth);
            int64_t off = base[tid];
            for (int64_t i = start; i < end; i++) {
                const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
                for (int64_t p = rs; p < re; p++) {
                    g_ustart[p] = (uint32_t)off;
                    const int64_t ck = A_indices[p];
                    if (ck >= i) break;
                    const uint64_t up = g_uptr[ck];
                    int64_t m = (int32_t)up;
                    const int64_t me = (int32_t)(up >> 32);
                    int64_t q = p + 1;
                    for (; m < me; m++) {
                        const int64_t cj = A_indices[m];
                        while (q < re && A_indices[q] < cj) q++;
                        if (q < re && A_indices[q] == cj)
                            g_pairs[off++] = (uint64_t)(uint32_t)q
                                           | ((uint64_t)(uint32_t)m << 32);
                    }
                }
            }
        }
    } else {
        int64_t off = 0;
        for (int64_t i = 0; i < N; i++) {
            const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
            for (int64_t p = rs; p < re; p++) {
                g_ustart[p] = (uint32_t)off;
                const int64_t ck = A_indices[p];
                if (ck >= i) break;
                const uint64_t up = g_uptr[ck];
                int64_t m = (int32_t)up;
                const int64_t me = (int32_t)(up >> 32);
                int64_t q = p + 1;
                for (; m < me; m++) {
                    const int64_t cj = A_indices[m];
                    while (q < re && A_indices[q] < cj) q++;
                    if (q < re && A_indices[q] == cj)
                        g_pairs[off++] = (uint64_t)(uint32_t)q
                                       | ((uint64_t)(uint32_t)m << 32);
                }
            }
        }
    }
    g_ustart[nnz] = (uint32_t)total_bound;

    const double t_ph2 = now_ms();
    /* ---------- phase 3: sequential replay ---------- */
    {
        double *diag = g_diag;
        for (int64_t i = 0; i < N; i++) {
            const int64_t re = A_indptr[i + 1];
            int64_t p = A_indptr[i];
            for (; p < re; p++) {
                const int64_t ck = A_indices[p];
                if (ck >= i) break;
                const double l = A_data[p] / diag[ck];
                A_data[p] = l;
                const uint32_t t0 = g_ustart[p];
                const uint32_t t1 = g_ustart[p + 1];
                for (uint32_t t = t0; t < t1; t++) {
                    const uint64_t pr = g_pairs[t];
                    const int64_t q = (int32_t)(pr & 0xffffffffu);
                    const int64_t m = (int32_t)(pr >> 32);
                    A_data[q] -= l * A_data[m];
                }
            }
            diag[i] = A_data[p];
        }
    }

    const double t_ph3 = now_ms();
    if (nthreads > 1) {
        #pragma omp parallel
        restore_worker(omp_get_thread_num());
    }
    const double t_end = now_ms();
    g_ncall++;
    {
        FILE *f = fopen("/shared/agent-5/calls.log", "a");
        if (f) {
            fprintf(f, "call %2d: ph1=%.3f ph2=%.3f ph3=%.3f rst=%.3f tot=%.3f nth=%d pid=%d\n",
                    g_ncall, t_ph1 - t_start, t_ph2 - t_ph1, t_ph3 - t_ph2,
                    t_end - t_ph3, t_end - t_start, nthreads, (int)getpid());
            fclose(f);
        }
    }
}
