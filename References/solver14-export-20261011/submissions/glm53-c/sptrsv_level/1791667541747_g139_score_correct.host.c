/* Level-scheduled SpTRSV, sync-free execution.
 *
 * The level loop is sequential, but instead of a global OpenMP barrier per level
 * (971 levels here), every thread walks the level list itself and takes a fixed
 * slice of every level.  Cross-thread dependences are resolved with a per-row
 * "done" flag carrying a per-call epoch: a row may only be evaluated once every
 * one of its off-diagonal parents has published x[col].  That removes all global
 * synchronisation, which otherwise dominates this kernel (median level holds
 * only ~30 rows over 24 threads).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdatomic.h>
#include <omp.h>

static uint32_t *g_flag = NULL;
static int64_t   g_flag_cap = 0;
static uint64_t  g_calls = 0;

static inline void sptrsv_pause(void)
{
#if defined(__x86_64__) || defined(__i386__)
    __builtin_ia32_pause();
#endif
}

__attribute__((always_inline))
static inline void solve_row(const int64_t row,
                            const double *restrict L_data,
                            const int64_t *restrict L_indices,
                            const int64_t *restrict L_indptr,
                            const double *restrict b,
                            double *restrict x,
                            _Atomic uint32_t *restrict flag,
                            const uint32_t epoch)
{
    const int64_t e0 = L_indptr[row];
    const int64_t e1 = L_indptr[row + 1];
    if (e1 == e0) {                 /* empty row: reference divides by diag == 0 */
        x[row] = b[row] / 0.0;
        atomic_store_explicit(&flag[row], epoch, memory_order_release);
        return;
    }
    const int64_t last = e1 - 1;
    const int64_t hasdiag = (L_indices[last] == row);
    const int64_t end = hasdiag ? last : e1;
    double acc = b[row];
    for (int64_t k = e0; k < end; ++k) {
        const int64_t col = L_indices[k];
        while (atomic_load_explicit(&flag[col], epoch) != epoch)
            sptrsv_pause();
        acc -= L_data[k] * x[col];
    }
    x[row] = acc / (hasdiag ? L_data[last] : 0.0);
    atomic_store_explicit(&flag[row], epoch, memory_order_release);
}

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *restrict b,
    const int64_t *level_ptr,
    const int64_t *perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    (void)NNZ_L;
    if (N <= 0)
        return;

    /* per-row done-flags with an epoch, so no clearing is needed between calls */
    if (g_flag_cap < N) {
        uint32_t *nf = (uint32_t *)realloc((void *)g_flag,
                                           (size_t)N * sizeof(uint32_t));
        if (nf == NULL)
            return;               /* keep flags: buffer must hold for this call */
        g_flag = nf;
        g_flag_cap = N;
        memset(g_flag, 0, (size_t)N * sizeof(uint32_t));
        g_calls = 0;
    }
    g_calls += 1;
    uint32_t epoch = (uint32_t)g_calls;
    if (epoch == 0) {              /* wrapped: clear once and restart the clock */
        memset(g_flag, 0, (size_t)N * sizeof(uint32_t));
        g_calls = 1;
        epoch = 1;
    }

    _Atomic uint32_t *flag = (_Atomic uint32_t *)g_flag;

#pragma omp parallel
    {
        const int    t  = omp_get_thread_num();
        const int    T  = omp_get_num_threads();
        int64_t done = 0;
        for (int64_t lvl = 0; lvl < N && done < N; ++lvl) {
            const int64_t s  = level_ptr[lvl];
            const int64_t e  = level_ptr[lvl + 1];
            const int64_t sz = e - s;
            if (sz > 0) {
                const int64_t lo = s + (int64_t)t * sz / T;
                const int64_t hi = s + (int64_t)(t + 1) * sz / T;
                for (int64_t idx = lo; idx < hi; ++idx)
                    solve_row(perm[idx], L_data, L_indices, L_indptr, b, x,
                              flag, epoch);
                done += sz;
            }
        }
    }
}
