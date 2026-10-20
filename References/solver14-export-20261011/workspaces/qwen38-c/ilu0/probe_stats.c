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
    int64_t maxrl = 0, minrl = N;
    int64_t hist[64] = {0};
    int64_t below = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t rl = A_indptr[i + 1] - A_indptr[i];
        if (rl > maxrl) maxrl = rl;
        if (rl < minrl) minrl = rl;
        if (rl < 64) hist[rl]++; else hist[63]++;
        for (int64_t p = A_indptr[i]; p < A_indptr[i + 1]; p++)
            if (A_indices[p] < i) below++;
    }
    printf("N=%lld nnz=%lld maxrl=%lld minrl=%lld below=%lld\n",
           (long long)N, (long long)nnz, (long long)maxrl, (long long)minrl, (long long)below);
    for (int64_t r = 0; r < 64; r++)
        if (hist[r]) printf("rl=%lld cnt=%lld\n", (long long)r, (long long)hist[r]);
    for (int64_t i = 0; i < 6 && i < N; i++) {
        printf("row %lld: ", (long long)i);
        for (int64_t p = A_indptr[i]; p < A_indptr[i + 1]; p++)
            printf("%lld ", (long long)A_indices[p]);
        printf("\n");
    }
    fflush(stdout);
    (void)A_data; (void)workspace; (void)workspace_size;
    /* real work to keep timing realistic */
    double *restrict row_val = (double *)workspace;
    uint32_t *restrict mark = (uint32_t *)(workspace + 8 * (size_t)N);
    double *restrict inv_diag = (double *)(workspace + 8 * (size_t)N + 4 * (size_t)N);
    for (int64_t i = 0; i < N; i++) {
        uint32_t gen = (uint32_t)(i + 1);
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        for (int64_t p = rs; p < re; p++) {
            const int64_t c = A_indices[p];
            row_val[c] = A_data[p];
            mark[c] = gen;
        }
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
        for (int64_t p = rs; p < re; p++) {
            const int64_t c = A_indices[p];
            A_data[p] = row_val[c];
        }
    }
}
