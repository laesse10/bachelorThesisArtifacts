#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#if defined(__AVX512F__)
#include <immintrin.h>
#endif

#define TMAX 48
#define LOCALLIMIT 64

/* find target in cols[0..n); return index or -1 */
static inline int find_col(const int64_t *cols, int n, int64_t target) {
#if defined(__AVX512F__)
    __m512i tv = _mm512_set1_epi64(target);
    int j = 0;
    for (; j + 8 <= n; j += 8) {
        __m512i v = _mm512_loadu_si512((const void *)(cols + j));
        __mmask8 m = _mm512_cmpeq_epi64_mask(v, tv);
        if (m) return j + __builtin_ctz((unsigned)m);
    }
    for (; j < n; j++) if (cols[j] == target) return j;
    return -1;
#else
    for (int j = 0; j < n; j++) if (cols[j] == target) return j;
    return -1;
#endif
}

static inline void row_local(
    int64_t i, double *restrict A_data, const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr, int64_t *restrict lcols, double *restrict lvals,
    double *restrict inv_diag)
{
    const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
    const int n = (int)(re - rs);
    for (int p = 0; p < n; p++) { lcols[p] = A_indices[rs + p]; lvals[p] = A_data[rs + p]; }
    for (int p = 0; p < n; p++) {
        const int64_t k = lcols[p];
        if (k >= i) continue;
        const double l = lvals[p] * inv_diag[k];
        lvals[p] = l;
        const int64_t qs = A_indptr[k], qe = A_indptr[k + 1];
        for (int64_t q = qs; q < qe; q++) {
            const int64_t cj = A_indices[q];
            if (cj > k) {
                const int pos = find_col(lcols, n, cj);
                if (pos >= 0) lvals[pos] -= l * A_data[q];
            }
        }
    }
    const int dp = find_col(lcols, n, i);
    inv_diag[i] = 1.0 / lvals[dp];
    for (int p = 0; p < n; p++) A_data[rs + p] = lvals[p];
}

/* serial dense fallback (always correct), used if a row is too long for local */
static void ilu0_dense(double *restrict A_data, const int64_t *restrict A_indices,
                       const int64_t *restrict A_indptr, const int64_t N,
                       uint8_t *restrict ws) {
    double *restrict row_val = (double *)ws;
    uint32_t *restrict mark = (uint32_t *)(ws + 8 * (size_t)N);
    double *restrict inv_diag = (double *)(ws + 8 * (size_t)N + 4 * (size_t)N);
    for (int64_t i = 0; i < N; i++) {
        uint32_t gen = (uint32_t)(i + 1);
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        for (int64_t p = rs; p < re; p++) { const int64_t c = A_indices[p]; row_val[c] = A_data[p]; mark[c] = gen; }
        for (int64_t p = rs; p < re; p++) {
            const int64_t k = A_indices[p];
            if (k >= i) continue;
            const double l = row_val[k] * inv_diag[k];
            row_val[k] = l;
            const int64_t qs = A_indptr[k], qe = A_indptr[k + 1];
            for (int64_t q = qs; q < qe; q++) {
                const int64_t cj = A_indices[q];
                if (cj > k && mark[cj] == gen) row_val[cj] -= l * A_data[q];
            }
        }
        inv_diag[i] = 1.0 / row_val[i];
        for (int64_t p = rs; p < re; p++) { const int64_t c = A_indices[p]; A_data[p] = row_val[c]; }
    }
}

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)nnz; (void)workspace_size;
    if (N <= 0) return;

    /* max row length + levels */
    int64_t maxrl = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t rl = A_indptr[i + 1] - A_indptr[i];
        if (rl > maxrl) maxrl = rl;
    }
    if (maxrl > LOCALLIMIT) {
        ilu0_dense(A_data, A_indices, A_indptr, N, workspace);
        return;
    }

    int nthreads = omp_get_max_threads();
    if (nthreads > TMAX) nthreads = TMAX;
    if (nthreads < 1) nthreads = 1;

    size_t off = 0;
    double *restrict inv_diag = (double *)(workspace + off); off += 8*(size_t)N;
    int32_t *level = (int32_t *)(workspace + off); off += 4*(size_t)N;
    int32_t *width = (int32_t *)(workspace + off); off += 4*(size_t)(N+1);
    int64_t *offset = (int64_t *)(workspace + off); off += 8*(size_t)(N+2);
    int64_t *rows_by_level = (int64_t *)(workspace + off); off += 8*(size_t)N;
    int32_t *fillcnt = (int32_t *)(workspace + off); off += 4*(size_t)(N+1);
    int64_t *lcols = (int64_t *)(workspace + off); off += (size_t)maxrl*8*(size_t)nthreads;
    double *lvals = (double *)(workspace + off); off += (size_t)maxrl*8*(size_t)nthreads;
    (void)off;

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
    memset(width, 0, 4*(size_t)(maxlev+1));
    for (int64_t i = 0; i < N; i++) width[level[i]]++;
    offset[0] = 0;
    for (int32_t L = 0; L <= maxlev; L++) offset[L+1] = offset[L] + width[L];
    memset(fillcnt, 0, 4*(size_t)(maxlev+1));
    for (int64_t i = 0; i < N; i++) {
        int32_t L = level[i];
        rows_by_level[offset[L] + fillcnt[L]] = i;
        fillcnt[L]++;
    }

    #pragma omp parallel num_threads(nthreads)
    {
        const int tid = omp_get_thread_num();
        int64_t *rc = lcols + (size_t)tid * maxrl;
        double *rv = lvals + (size_t)tid * maxrl;
        for (int32_t L = 0; L <= maxlev; L++) {
            #pragma omp for schedule(static)
            for (int64_t idx = offset[L]; idx < offset[L+1]; idx++) {
                int64_t i = rows_by_level[idx];
                row_local(i, A_data, A_indices, A_indptr, rc, rv, inv_diag);
            }
        }
    }
}
