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

#define LOCALLIMIT 128

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

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)nnz; (void)workspace_size;
    if (N <= 0) return;

    int64_t maxrl = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t rl = A_indptr[i + 1] - A_indptr[i];
        if (rl > maxrl) maxrl = rl;
    }

    double *restrict inv_diag = (double *)workspace;
    int64_t *lcols = (int64_t *)(workspace + 8 * (size_t)N);
    double *restrict lvals = (double *)(workspace + 8 * (size_t)N + maxrl * 8);

    if (maxrl > LOCALLIMIT) {
        /* dense serial fallback */
        double *restrict row_val = (double *)workspace;
        uint32_t *restrict mark = (uint32_t *)(workspace + 8 * (size_t)N);
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
        return;
    }

    for (int64_t i = 0; i < N; i++) {
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
}
