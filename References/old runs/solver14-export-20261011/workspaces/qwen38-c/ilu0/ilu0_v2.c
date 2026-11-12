#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define TMAX 48

static inline void spin_pause(void) {
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#endif
}

static inline void factorize_row(
    int64_t i, double *restrict A_data, const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr, double *restrict row_val,
    int32_t *restrict pos, const double *restrict inv_diag, double *diag_out)
{
    const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
    for (int64_t p = rs; p < re; p++) {
        const int64_t c = A_indices[p];
        row_val[c] = A_data[p];
        pos[c] = 1;
    }
    for (int64_t p = rs; p < re; p++) {
        const int64_t k = A_indices[p];
        if (k >= i) continue;
        const double l = row_val[k] * inv_diag[k];
        row_val[k] = l;
        const int64_t qs = A_indptr[k], qe = A_indptr[k + 1];
        for (int64_t q = qs; q < qe; q++) {
            const int64_t cj = A_indices[q];
            if (cj > k && pos[cj]) {
                row_val[cj] -= l * A_data[q];
            }
        }
    }
    diag_out[i] = 1.0 / row_val[i];
    for (int64_t p = rs; p < re; p++) {
        const int64_t c = A_indices[p];
        A_data[p] = row_val[c];
        pos[c] = 0;
    }
}

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)nnz; (void)workspace_size;
    if (N <= 0) return;

    int nthreads = omp_get_max_threads();
    if (nthreads > TMAX) nthreads = TMAX;
    if (nthreads < 1) nthreads = 1;

    size_t off = 0;
    double *restrict inv_diag = (double *)(workspace + off); off += 8*(size_t)N;
    int32_t *level = (int32_t *)(workspace + off); off += 4*(size_t)N;
    /* width, offset, rows_by_level, cursor, done, frontier sized by maxlev (<= N) */
    int32_t *width = (int32_t *)(workspace + off); off += 4*(size_t)(N+1);
    int64_t *offset = (int64_t *)(workspace + off); off += 8*(size_t)(N+2);
    int64_t *rows_by_level = (int64_t *)(workspace + off); off += 8*(size_t)N;
    int32_t *cursor = (int32_t *)(workspace + off); off += 4*(size_t)(N+1);
    int32_t *done = (int32_t *)(workspace + off); off += 4*(size_t)(N+1);
    int32_t *frontier = (int32_t *)(workspace + off); off += 4;
    double *row_val = (double *)(workspace + off); off += 8*(size_t)N*(size_t)nthreads;
    int32_t *pos = (int32_t *)(workspace + off); off += 4*(size_t)N*(size_t)nthreads;
    (void)off;

    /* --- serial precompute: levels --- */
    int32_t maxlev = 0;
    for (int64_t i = 0; i < N; i++) {
        int32_t lv = 0;
        for (int64_t p = A_indptr[i]; p < A_indptr[i+1]; p++) {
            int64_t k = A_indices[p];
            if (k < i) { int32_t c = level[k] + 1; if (c > lv) lv = c; }
        }
        level[i] = lv;
        if (lv > maxlev) maxlev = lv;
    }
    /* width */
    memset(width, 0, 4*(size_t)(maxlev+1));
    for (int64_t i = 0; i < N; i++) width[level[i]]++;
    /* offset */
    offset[0] = 0;
    for (int32_t L = 0; L <= maxlev; L++) offset[L+1] = offset[L] + width[L];
    /* rows_by_level */
    int32_t *cnt = cursor; /* reuse as fill counter */
    memset(cnt, 0, 4*(size_t)(maxlev+1));
    for (int64_t i = 0; i < N; i++) {
        int32_t L = level[i];
        rows_by_level[offset[L] + cnt[L]] = i;
        cnt[L]++;
    }
    /* init scheduler */
    memset(cursor, 0, 4*(size_t)(maxlev+1));
    memset(done, 0, 4*(size_t)(maxlev+1));
    *frontier = 0;

    /* --- parallel factorization, frontier work-queue --- */
    #pragma omp parallel num_threads(nthreads)
    {
        const int tid = omp_get_thread_num();
        double *restrict rv = row_val + (size_t)tid * N;
        int32_t *restrict ps = pos + (size_t)tid * N;
        for (;;) {
            int32_t L = __atomic_load_n(frontier, __ATOMIC_ACQUIRE);
            if (L > maxlev) break;
            int32_t idx = __atomic_fetch_add(&cursor[L], 1, __ATOMIC_RELAXED);
            if (idx < width[L]) {
                int64_t i = rows_by_level[offset[L] + idx];
                factorize_row(i, A_data, A_indices, A_indptr, rv, ps, inv_diag, inv_diag);
                if (__atomic_fetch_add(&done[L], 1, __ATOMIC_RELEASE) == width[L] - 1) {
                    __atomic_store_n(frontier, L + 1, __ATOMIC_RELEASE);
                }
            } else {
                if (__atomic_load_n(&done[L], __ATOMIC_ACQUIRE) == width[L]) {
                    int32_t exp = L;
                    __atomic_compare_exchange_n(frontier, &exp, L + 1, 0,
                                                __ATOMIC_RELEASE, __ATOMIC_RELAXED);
                }
                spin_pause();
            }
        }
    }
}
