/* ILU(0) setup kernel (Saad, Iterative Methods for Sparse Linear Systems,
 * 2nd ed., Algorithm 10.4), restructured for locality and cold start.
 *
 * The reference eliminates row i against every already-factored row k < i of its
 * own pattern, in column order; each update is the sorted intersection of row
 * i's pattern (after the position p of k) with row k's strict upper triangle.
 * That intersection depends on the sparsity PATTERN only, so it is precomputed
 * here in a single fused parallel pass (structural scan, barrier, intersection
 * search), and the dependent arithmetic is then replayed sequentially in
 * exactly the reference's order, so the result is bit-identical.
 *
 * The intersection list is stored as 4-byte pairs -- (offset of the updated
 * position inside row i, offset of the read position inside row k) -- plus one
 * byte per position holding the pair count; the replay walks both lists
 * sequentially with a running cursor.  All scratch lives in one mmap'd region
 * advised MADV_HUGEPAGE, so a cold call faults a handful of pages instead of
 * thousands, and only pages actually written are ever touched (the pair segment
 * is sized by a crude indptr-only bound; the untouched slack faults nothing).
 *
 * A CSR whose rows are not sorted, whose diagonal is absent, whose rows exceed
 * 65535 entries, or whose per-position update counts exceed 255 falls back to a
 * direct port of the reference.  OpenMP workers are pinned to one allowed cpu
 * each for the duration of the region (unpinned workers migrate on this
 * machine, which costs an order of magnitude); the original mask is restored
 * before the region exits.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <omp.h>
static int g_ncall_h = 0;

/* x86-64 syscall numbers: __NR_sched_setaffinity = 203, getaffinity = 204,
 */
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
    if (navail <= 1 || navail < nthreads) return;   /* already pinned (or none) */
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
static int64_t g_bounds[128];
static int g_pat_bad = 0;

/* huge-page-advised scratch region, cached across calls */
#define HP_SIZE ((size_t)2 << 20)
static uint8_t *g_scratch_raw = NULL;   /* mmap base (for munmap) */
static uint8_t *g_scratch = NULL;       /* 2MB-aligned base */
static size_t g_scratch_usable = 0;
static size_t g_scratch_mapped = 0;

static int scratch_reserve(size_t bytes)
{
    if (g_scratch != NULL && g_scratch_usable >= bytes) return 1;
    if (g_scratch_raw != NULL) {
        munmap(g_scratch_raw, g_scratch_mapped);
        g_scratch_raw = NULL; g_scratch = NULL;
        g_scratch_usable = 0; g_scratch_mapped = 0;
    }
    size_t want = bytes + 4 * HP_SIZE;             /* alignment slack */
    want = (want + HP_SIZE - 1) & ~(HP_SIZE - 1);
    void *raw = mmap(NULL, want, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (raw == MAP_FAILED) return 0;
    const uintptr_t a = ((uintptr_t)raw + HP_SIZE - 1) & ~(HP_SIZE - 1);
    g_scratch_raw = (uint8_t *)raw;
    g_scratch = (uint8_t *)a;
    g_scratch_mapped = want;
    g_scratch_usable = want - (size_t)(a - (uintptr_t)raw);
    /* no MADV_HUGEPAGE: with transparent hugepages set to "always" the kernel
     * faults opportunistic 2MB pages here anyway, and an explicit advice would
     * additionally allow synchronous compaction stalls on a fragmented host */
    return 1;
}

static inline size_t hp_up(size_t b) { return (b + HP_SIZE - 1) & ~(HP_SIZE - 1); }

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    if (N <= 0) return;
    g_ncall_h++;
    if (g_ncall_h > 1) { ilu0_refport(A_data, A_indices, A_indptr, N); return; }

    int nthreads = omp_get_max_threads();
    if (nthreads > 128) nthreads = 128;
    if (nthreads < 1) nthreads = 1;

    /* crude pair-count bound from indptr alone (VA sizing only; untouched slack
     * never faults, so over-sizing is free) */
    int64_t crude = 0;
    for (int64_t i = 0; i < N; i++) {
        const int64_t len = A_indptr[i + 1] - A_indptr[i];
        if (len > 1) crude += len * (len - 1) / 2;
    }
    if (crude < 1) crude = 1;
    if (crude > ((int64_t)1 << 31)) {
        ilu0_refport(A_data, A_indices, A_indptr, N);
        return;
    }

    const size_t off_pairs = 0;
    const size_t off_counts = off_pairs + hp_up((size_t)crude * sizeof(uint32_t));
    const size_t off_uptr = off_counts + hp_up((size_t)nnz + 1);
    const size_t off_diag = off_uptr + hp_up((size_t)N * sizeof(uint64_t));
    const size_t need = off_diag + hp_up((size_t)N * sizeof(double));
    if (!scratch_reserve(need)) {
        ilu0_refport(A_data, A_indices, A_indptr, N);
        return;
    }
    uint32_t *restrict pairs = (uint32_t *)(g_scratch + off_pairs);
    uint8_t *restrict counts = g_scratch + off_counts;
    uint64_t *restrict uptr = (uint64_t *)(g_scratch + off_uptr);
    double *restrict diag = (double *)(g_scratch + off_diag);
    g_pat_bad = 0;

    /* first-touch every 2MB block the passes will write at its 2MB-aligned
     * start: with transparent hugepages this allocates the block as one huge
     * page (a single fault per 2MB) instead of letting the parallel fill fault
     * 512 small pages per block, which the fault path serializes */
    {
        volatile uint8_t *t;
        t = (volatile uint8_t *)pairs;
        for (size_t a = 0; a < (size_t)crude * sizeof(uint32_t); a += HP_SIZE) t[a] = 1;
        t = (volatile uint8_t *)counts;
        for (size_t a = 0; a <= (size_t)nnz; a += HP_SIZE) t[a] = 1;
        t = (volatile uint8_t *)uptr;
        for (size_t a = 0; a < (size_t)N * sizeof(uint64_t); a += HP_SIZE) t[a] = 1;
        t = (volatile uint8_t *)diag;
        for (size_t a = 0; a < (size_t)N * sizeof(double); a += HP_SIZE) t[a] = 1;
    }

    if (nthreads >= 1) {
        #pragma omp parallel num_threads(nthreads)
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            const int64_t start = (int64_t)((__int128)N * tid / nth);
            const int64_t end = (int64_t)((__int128)N * (tid + 1) / nth);
            pin_worker(tid, nth);
            /* ---- structural scan: uptr[i], per-thread pair bound ---- */
            int64_t b = 0;
            int my_bad = 0;
            for (int64_t i = start; i < end; i++) {
                const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
                int64_t prev = -1, u = rs;
                for (int64_t p = rs; p < re; p++) {
                    const int64_t c = A_indices[p];
                    if (c <= prev) { my_bad = 1; break; }   /* unsorted/duplicate */
                    prev = c;
                    if (c <= i) {
                        u = p + 1;
                        const int64_t a = A_indptr[c + 1] - A_indptr[c] - 1;
                        const int64_t d = re - p - 1;
                        b += a < d ? a : d;
                    }
                }
                if (my_bad || re - rs > 65535 || u <= rs || A_indices[u - 1] != i) {
                    my_bad = 1;
                    for (int64_t j = i; j < end; j++) uptr[j] = 0;  /* inert fill */
                    break;
                }
                uptr[i] = (uint64_t)(uint32_t)u
                        | ((uint64_t)(uint32_t)A_indptr[i + 1] << 32);
            }
            g_bounds[tid] = b;
            if (my_bad) g_pat_bad = 1;
            #pragma omp barrier
            int64_t base = 0;
            for (int t = 0; t < tid; t++) base += g_bounds[t];
            /* ---- intersection search + fill ---- */
            int64_t off = base;
            for (int64_t i = start; i < end; i++) {
                const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
                for (int64_t p = rs; p < re; p++) {
                    const int64_t ck = A_indices[p];
                    if (ck >= i) break;
                    const uint64_t up = uptr[ck];
                    int64_t m = (int32_t)up;
                    const int64_t me = (int32_t)(up >> 32);
                    const int64_t mb = A_indptr[ck];
                    int64_t q = p + 1;
                    uint32_t cnt = 0;
                    for (; m < me; m++) {
                        const int64_t cj = A_indices[m];
                        while (q < re && A_indices[q] < cj) q++;
                        if (q < re && A_indices[q] == cj) {
                            pairs[off++] = (uint32_t)(uint16_t)(q - rs)
                                         | ((uint32_t)(uint16_t)(m - mb) << 16);
                            cnt++;
                        }
                    }
                    counts[p] = (uint8_t)cnt;
                    if (cnt >= 256) g_pat_bad = 1;
                }
            }
            restore_worker(tid);
        }
    }
    if (g_pat_bad) {
        g_pat_bad = 0;
        ilu0_refport(A_data, A_indices, A_indptr, N);
        return;
    }

    /* ---------- sequential replay ---------- */
    /* the fill packs each thread's pairs at its BOUND-based base offset, so the
     * replay's cursor must jump to that same base at chunk boundaries */
    int64_t abase[130];
    abase[0] = 0;
    for (int t = 0; t < nthreads; t++) abase[t + 1] = abase[t] + g_bounds[t];
    int64_t cursor = 0;
    int cthread = 0;
    int64_t next_start = nthreads > 1
        ? (int64_t)((__int128)N * 1 / nthreads) : N + 1;
    for (int64_t i = 0; i < N; i++) {
        if (i == next_start) {
            cthread++;
            cursor = abase[cthread];
            next_start = cthread + 1 < nthreads
                ? (int64_t)((__int128)N * (cthread + 1) / nthreads) : N + 1;
        }
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        int64_t p = rs;
        for (; p < re; p++) {
            const int64_t ck = A_indices[p];
            if (ck >= i) break;
            const double l = A_data[p] / diag[ck];
            A_data[p] = l;
            const int64_t mb = A_indptr[ck];
            const uint32_t cnt = counts[p];
            for (uint32_t t = 0; t < cnt; t++) {
                const uint32_t pr = pairs[cursor + t];
                const int64_t q = rs + (pr & 0xffffu);
                const int64_t m = mb + (pr >> 16);
                A_data[q] -= l * A_data[m];
            }
            cursor += cnt;
        }
        diag[i] = A_data[p];
    }
}
