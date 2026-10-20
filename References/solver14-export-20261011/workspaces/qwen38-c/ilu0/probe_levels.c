#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size) {
    (void)nnz; (void)workspace_size;
    int32_t *level = (int32_t *)malloc((size_t)N * 4);
    int32_t maxlev = 0;
    long long sumparents = 0, sumwork = 0, crit = 0;
    int badself = 0, unsorted = 0;
    for (int64_t i = 0; i < N; i++) {
        int32_t lv = 0;
        int64_t rs = A_indptr[i], re = A_indptr[i+1];
        int hasdiag = 0;
        for (int64_t p = rs; p < re; p++) {
            int64_t c = A_indices[p];
            if (c == i) hasdiag = 1;
            if (p > rs && c < A_indices[p-1]) unsorted++;
        }
        if (!hasdiag) badself++;
        for (int64_t p = rs; p < re; p++) {
            int64_t k = A_indices[p];
            if (k < i) { sumparents++; int32_t cand = level[k] + 1; if (cand > lv) lv = cand; }
        }
        level[i] = lv;
        if (lv > maxlev) maxlev = lv;
    }
    int32_t *width = (int32_t *)malloc((size_t)(maxlev+1) * 4);
    memset(width, 0, (size_t)(maxlev+1) * 4);
    for (int64_t i = 0; i < N; i++) width[level[i]]++;
    int32_t maxw = 0, minw = 0x7fffffff; long long nzlevels = 0;
    for (int32_t L = 0; L <= maxlev; L++) if (width[L]) { nzlevels++; if (width[L] > maxw) maxw = width[L]; if (width[L] < minw) minw = width[L]; }
    long long *maxwork = (long long *)calloc((size_t)(maxlev+1) > 1 ? (size_t)(maxlev+1) : 1, 8);
    for (int64_t i = 0; i < N; i++) {
        long long w = 0;
        for (int64_t p = A_indptr[i]; p < A_indptr[i+1]; p++) { int64_t k = A_indices[p]; if (k < i) w += (A_indptr[k+1]-A_indptr[k]); }
        sumwork += w;
        if (w > maxwork[level[i]]) maxwork[level[i]] = w;
    }
    for (int32_t L = 0; L <= maxlev; L++) crit += maxwork[L];
    long long bw[8] = {0};
    for (int32_t L = 0; L <= maxlev; L++) {
        int32_t w = width[L];
        if (!w) continue;
        if (w==1) bw[0]++; else if (w<=4) bw[1]++; else if (w<=16) bw[2]++; else if (w<=64) bw[3]++;
        else if (w<=256) bw[4]++; else if (w<=1024) bw[5]++; else if (w<=4096) bw[6]++; else bw[7]++;
    }
    printf("maxlev=%d nzlevels=%lld maxw=%d minw=%d sumparents=%lld sumwork=%lld critpath=%lld badself=%d unsorted=%d\n",
           maxlev, nzlevels, maxw, minw, sumparents, sumwork, crit, badself, unsorted);
    printf("wbuckets=%lld %lld %lld %lld %lld %lld %lld %lld\n", bw[0],bw[1],bw[2],bw[3],bw[4],bw[5],bw[6],bw[7]);
    fflush(stdout);
    free(level); free(width); free(maxwork);
    /* real factorization (v1 algorithm) to keep output correct */
    double *restrict row_val = (double *)workspace;
    uint32_t *restrict mark = (uint32_t *)(workspace + 8 * (size_t)N);
    double *restrict inv_diag = (double *)(workspace + 8 * (size_t)N + 4 * (size_t)N);
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
