#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *b,
    const int64_t *level_ptr,
    const int64_t *perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    /* ---- diagnostics ---- */
    int64_t nlevels = 0;
    int64_t done = 0;
    int64_t minsz = N + 1, maxsz = 0;
    int64_t sum_sq = 0;
    int64_t small1 = 0, small4 = 0, small16 = 0, small64 = 0, small256 = 0;
    int64_t rows_with_diag = 0;
    int64_t maxrowlen = 0, sumrowlen = 0;
    for (int64_t lvl = 0; lvl < N && done < N; ++lvl) {
        int64_t s = level_ptr[lvl], e = level_ptr[lvl + 1];
        if (e > s) {
            nlevels++;
            int64_t sz = e - s;
            if (sz < minsz) minsz = sz;
            if (sz > maxsz) maxsz = sz;
            sum_sq += sz * sz;
            if (sz <= 1) small1++;
            if (sz <= 4) small4++;
            if (sz <= 16) small16++;
            if (sz <= 64) small64++;
            if (sz <= 256) small256++;
            done += e - s;
        }
    }
    for (int64_t r = 0; r < N; ++r) {
        int64_t e1 = L_indptr[r + 1];
        if (e1 > L_indptr[r] && L_indices[e1 - 1] == r) rows_with_diag++;
        if (e1 - L_indptr[r] > maxrowlen) maxrowlen = e1 - L_indptr[r];
        sumrowlen += e1 - L_indptr[r];
    }
    printf("N=%lld NNZ=%lld workspace_size=%lld\n", (long long)N, (long long)NNZ_L, (long long)workspace_size);
    printf("max_threads=%d num_procs=%d\n", omp_get_max_threads(), omp_get_num_procs());
    printf("nlevels=%lld minsz=%lld maxsz=%lld sum_sz^2=%lld\n",
           (long long)nlevels, (long long)minsz, (long long)maxsz, (long long)sum_sq);
    printf("levels with size <=1:%lld <=4:%lld <=16:%lld <=64:%lld <=256:%lld\n",
           (long long)small1, (long long)small4, (long long)small16, (long long)small64, (long long)small256);
    printf("rows_with_diag_last=%lld maxrowlen=%lld avgrowlen=%.2f\n",
           (long long)rows_with_diag, (long long)maxrowlen, (double)sumrowlen / (double)N);
    fflush(stdout);

    /* ---- actual solve (sequential) ---- */
    done = 0;
    for (int64_t lvl = 0; lvl < N && done < N; ++lvl) {
        int64_t s = level_ptr[lvl], e = level_ptr[lvl + 1];
        if (e > s) {
            for (int64_t idx = s; idx < e; ++idx) {
                const int64_t row = perm[idx];
                const int64_t e0 = L_indptr[row];
                const int64_t e1 = L_indptr[row + 1];
                const int64_t last = e1 - 1;
                const int64_t end = (L_indices[last] == row) ? last : e1;
                double sacc = b[row];
                for (int64_t k = e0; k < end; ++k)
                    sacc -= L_data[k] * x[L_indices[k]];
                x[row] = sacc / ((L_indices[last] == row) ? L_data[last] : 0.0);
            }
            done += e - s;
        }
    }
}
