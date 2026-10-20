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

static inline int row_scan(const int64_t i, const int64_t *restrict A_indices,
                           const int64_t *restrict A_indptr, ipos_t *restrict upos)
{
    const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
    int64_t prev = -1;
    int64_t u = rs;
    for (int64_t p = rs; p < re; p++) {
        const int64_t c = A_indices[p];
        if (c <= prev) return 1;          /* unsorted or duplicated columns */
        prev = c;
        if (c <= i) u = p + 1;
    }
    /* full diagonal: the last column <= i must be i itself */
    if (u <= rs || A_indices[u - 1] != i) return 1;
    upos[i] = (ipos_t)u;
    return 0;
}

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    if (N <= 0) return;

    int nthreads = omp_get_max_threads();
    if (nthreads > 128) nthreads = 128;
    if (nthreads < 1) nthreads = 1;

    ipos_t *upos = (ipos_t *)malloc((size_t)N * sizeof(ipos_t));
    int *badflag = (int *)calloc((size_t)nthreads, sizeof(int));
    if (upos == NULL || badflag == NULL) {
        free(upos); free(badflag);
        ilu0_refport(A_data, A_indices, A_indptr, N);
        return;
    }

    /* ---------- phase 0: structural scan ---------- */
    if (nthreads > 1) {
        #pragma omp parallel num_threads(nthreads)
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            badflag[tid] = 0;
            pin_worker(tid, nth);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++)
                if (row_scan(i, A_indices, A_indptr, upos)) badflag[tid] = 1;
        }
    } else {
        for (int64_t i = 0; i < N; i++)
            if (row_scan(i, A_indices, A_indptr, upos)) { badflag[0] = 1; break; }
    }
    {
        int bad = 0;
        for (int t = 0; t < nthreads; t++)
            if (badflag[t]) bad = 1;
        if (bad) {
            free(upos); free(badflag);
            ilu0_refport(A_data, A_indices, A_indptr, N);
            if (nthreads > 1) {
                #pragma omp parallel num_threads(nthreads)
                restore_worker(omp_get_thread_num());
            }
            return;
        }
    }

    /* ---------- phase 1a: upper bound on the update-pair count ---------- */
    int64_t *bounds = (int64_t *)calloc((size_t)nthreads, sizeof(int64_t));
    if (bounds == NULL) {
        free(upos); free(badflag);
        ilu0_refport(A_data, A_indices, A_indptr, N);
        if (nthreads > 1) {
            #pragma omp parallel num_threads(nthreads)
            restore_worker(omp_get_thread_num());
        }
        return;
    }
    if (nthreads > 1) {
        #pragma omp parallel num_threads(nthreads)
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            const int64_t start = (int64_t)((__int128)N * tid / nth);
            const int64_t end = (int64_t)((__int128)N * (tid + 1) / nth);
            int64_t b = 0;
            for (int64_t i = start; i < end; i++) {
                const int64_t re = A_indptr[i + 1];
                for (int64_t p = A_indptr[i]; p < re; p++) {
                    const int64_t ck = A_indices[p];
                    if (ck >= i) break;
                    const int64_t a = A_indptr[ck + 1] - upos[ck];
                    const int64_t c = re - p - 1;
                    b += a < c ? a : c;
                }
            }
            bounds[tid] = b;
        }
    } else {
        int64_t b = 0;
        for (int64_t i = 0; i < N; i++) {
            const int64_t re = A_indptr[i + 1];
            for (int64_t p = A_indptr[i]; p < re; p++) {
                const int64_t ck = A_indices[p];
                if (ck >= i) break;
                const int64_t a = A_indptr[ck + 1] - upos[ck];
                const int64_t c = re - p - 1;
                b += a < c ? a : c;
            }
        }
        bounds[0] = b;
    }
    int64_t total_bound = 0;
    int64_t base[128];
    base[0] = 0;
    for (int t = 0; t < nthreads; t++) total_bound += bounds[t];
    for (int t = 1; t < nthreads; t++) base[t] = base[t - 1] + bounds[t - 1];

    /* ---------- memory for the pair list (judge workspace if it fits) ---------- */
    uint64_t *pairs = NULL;
    ipos_t *ustart = NULL;
    int pairs_owned = 0;
    const size_t need = (size_t)total_bound * sizeof(uint64_t)
                      + ((size_t)nnz + 1) * sizeof(ipos_t);
    if (workspace != NULL && (uint64_t)workspace_size >= (uint64_t)need) {
        pairs = (uint64_t *)(void *)workspace;
        ustart = (ipos_t *)(void *)(workspace + (size_t)total_bound * sizeof(uint64_t));
    } else {
        pairs = (uint64_t *)malloc((size_t)total_bound * sizeof(uint64_t));
        ustart = (ipos_t *)malloc(((size_t)nnz + 1) * sizeof(ipos_t));
        pairs_owned = 1;
    }
    if (pairs == NULL || ustart == NULL || total_bound > INT32_MAX) {
        if (pairs_owned) { free(pairs); free(ustart); }
        free(upos); free(badflag); free(bounds);
        ilu0_refport(A_data, A_indices, A_indptr, N);
        if (nthreads > 1) {
            #pragma omp parallel num_threads(nthreads)
            restore_worker(omp_get_thread_num());
        }
        return;
    }

    /* ---------- phase 1b: intersection search + fill ---------- */
    if (nthreads > 1) {
        #pragma omp parallel num_threads(nthreads)
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            const int64_t start = (int64_t)((__int128)N * tid / nth);
            const int64_t end = (int64_t)((__int128)N * (tid + 1) / nth);
            int64_t off = base[tid];
            for (int64_t i = start; i < end; i++) {
                const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
                for (int64_t p = rs; p < re; p++) {
                    ustart[p] = (ipos_t)off;
                    const int64_t ck = A_indices[p];
                    if (ck >= i) break;
                    int64_t m = upos[ck];
                    const int64_t me = A_indptr[ck + 1];
                    int64_t q = p + 1;
                    for (; m < me; m++) {
                        const int64_t cj = A_indices[m];
                        while (q < re && A_indices[q] < cj) q++;
                        if (q < re && A_indices[q] == cj)
                            pairs[off++] = (uint64_t)(uint32_t)q
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
                ustart[p] = (ipos_t)off;
                const int64_t ck = A_indices[p];
                if (ck >= i) break;
                int64_t m = upos[ck];
                const int64_t me = A_indptr[ck + 1];
                int64_t q = p + 1;
                for (; m < me; m++) {
                    const int64_t cj = A_indices[m];
                    while (q < re && A_indices[q] < cj) q++;
                    if (q < re && A_indices[q] == cj)
                        pairs[off++] = (uint64_t)(uint32_t)q
                                     | ((uint64_t)(uint32_t)m << 32);
                }
            }
        }
    }
    ustart[nnz] = (ipos_t)total_bound;

    /* ---------- phase 2: sequential replay ---------- */
    {
        double *diag = (double *)malloc((size_t)N * sizeof(double));
        if (diag == NULL) {
            if (pairs_owned) { free(pairs); free(ustart); }
            free(upos); free(badflag); free(bounds);
            ilu0_refport(A_data, A_indices, A_indptr, N);
            if (nthreads > 1) {
                #pragma omp parallel num_threads(nthreads)
                restore_worker(omp_get_thread_num());
            }
            return;
        }
        for (int64_t i = 0; i < N; i++) {
            const int64_t re = A_indptr[i + 1];
            int64_t p = A_indptr[i];
            for (; p < re; p++) {
                const int64_t ck = A_indices[p];
                if (ck >= i) break;
                const double l = A_data[p] / diag[ck];
                A_data[p] = l;
                const int32_t t0 = ustart[p];
                const int32_t t1 = ustart[p + 1];
                for (int32_t t = t0; t < t1; t++) {
                    const uint64_t pr = pairs[t];
                    const int64_t q = (int32_t)(pr & 0xffffffffu);
                    const int64_t m = (int32_t)(pr >> 32);
                    A_data[q] -= l * A_data[m];
                }
            }
            diag[i] = A_data[p];
        }
        free(diag);
    }

    if (pairs_owned) { free(pairs); free(ustart); }
    free(upos); free(badflag); free(bounds);
    if (nthreads > 1) {
        #pragma omp parallel num_threads(nthreads)
        restore_worker(omp_get_thread_num());
    }
}
