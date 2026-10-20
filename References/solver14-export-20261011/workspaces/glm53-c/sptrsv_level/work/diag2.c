#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdatomic.h>
#include <stdio.h>
#include <omp.h>

static uint32_t *g_flag = NULL;
static int64_t   g_flag_cap = 0;
static uint64_t  g_calls = 0;

static inline void sptrsv_pause(void) { __builtin_ia32_pause(); }

__attribute__((always_inline))
static inline void solve_row(const int64_t row,
                             const double *restrict L_data,
                             const int64_t *restrict L_indices,
                             const int64_t *restrict L_indptr,
                             const double *restrict b,
                             double *restrict x,
                             _Atomic uint32_t *restrict flag,
                             const uint32_t epoch,
                             unsigned long long *spins)
{
    const int64_t e0 = L_indptr[row];
    const int64_t e1 = L_indptr[row + 1];
    const int64_t last = e1 - 1;
    const int64_t hasdiag = (L_indices[last] == row);
    const int64_t end = hasdiag ? last : e1;
    double acc = b[row];
    for (int64_t k = e0; k < end; ++k) {
        const int64_t col = L_indices[k];
        while (atomic_load_explicit(&flag[col], memory_order_acquire) != epoch) {
            sptrsv_pause();
            (*spins)++;
        }
        acc -= L_data[k] * x[col];
    }
    x[row] = acc / (hasdiag ? L_data[last] : 0.0);
    atomic_store_explicit(&flag[row], epoch, memory_order_release);
}

void sptrsv_level_csr_fp64(
    const double *restrict L_data, const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr, const double *restrict b,
    const int64_t *level_ptr, const int64_t *restrict perm,
    double *restrict x, const int64_t N, const int64_t NNZ_L,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size; (void)NNZ_L;
    if (N <= 0) return;
    if (g_flag_cap < N) {
        uint32_t *nf = (uint32_t *)realloc((void*)g_flag, (size_t)N * sizeof(uint32_t));
        if (!nf) return;
        g_flag = nf; g_flag_cap = N;
        memset(g_flag, 0, (size_t)N * sizeof(uint32_t));
        g_calls = 0;
    }
    g_calls += 1;
    uint32_t epoch = (uint32_t)g_calls;
    _Atomic uint32_t *flag = (_Atomic uint32_t *)g_flag;

    double t0 = omp_get_wtime();
    unsigned long long total_spins = 0;
    unsigned long long total_rows = 0;
    int Tseen = 0;
    #pragma omp parallel reduction(+:total_spins,total_rows)
    {
        const int t = omp_get_thread_num();
        const int T = omp_get_num_threads();
        unsigned long long spins = 0, rows = 0;
        int64_t done = 0;
        for (int64_t lvl = 0; lvl < N && done < N; ++lvl) {
            const int64_t s = level_ptr[lvl], e = level_ptr[lvl + 1];
            const int64_t sz = e - s;
            if (sz > 0) {
                const int64_t lo = s + (int64_t)t * sz / T;
                const int64_t hi = s + (int64_t)(t + 1) * sz / T;
                for (int64_t idx = lo; idx < hi; ++idx) {
                    solve_row(perm[idx], L_data, L_indices, L_indptr, b, x, flag, epoch, &spins);
                    rows++;
                }
                done += sz;
            }
        }
        total_spins += spins; total_rows += rows;
        #pragma omp single
        { Tseen = T; }
    }
    double t1 = omp_get_wtime();
    printf("threads=%d wall_ms=%.3f rows=%llu spins=%llu spins/row=%.1f\n",
           Tseen, (t1 - t0) * 1e3, (unsigned long long)total_rows,
           (unsigned long long)total_spins,
           total_rows ? (double)total_spins / (double)total_rows : 0.0);
    fflush(stdout);
}
