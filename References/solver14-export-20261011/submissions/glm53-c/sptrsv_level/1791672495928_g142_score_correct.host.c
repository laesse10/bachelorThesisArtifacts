/* Level-scheduled SpTRSV, sync-free band decomposition.
 *
 * Every off-diagonal entry has col < row, so a thread that owns a contiguous
 * band of rows and walks it in ascending order sees every in-band parent
 * already solved: only parents below the band need a completion check.  Those
 * are a small minority of the edges (~20% for 24 bands on this matrix) and
 * they are resolved with per-row epoch flags instead of a global barrier per
 * level (971 levels here), which costs far more than the work between them.
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
    (void)level_ptr; (void)perm; (void)NNZ_L; (void)workspace; (void)workspace_size;
    if (N <= 0)
        return;

    if (g_flag_cap < N) {
        uint32_t *nf = (uint32_t *)realloc((void *)g_flag, (size_t)N * sizeof(uint32_t));
        if (nf == NULL)
            return;
        g_flag = nf;
        g_flag_cap = N;
        memset(g_flag, 0, (size_t)N * sizeof(uint32_t));
        g_calls = 0;
    }
    g_calls += 1;
    uint32_t epoch = (uint32_t)g_calls;
    if (epoch == 0) {
        memset(g_flag, 0, (size_t)N * sizeof(uint32_t));
        g_calls = 1;
        epoch = 1;
    }
    _Atomic uint32_t *flag = (_Atomic uint32_t *)g_flag;

#pragma omp parallel
    {
        const int    t  = omp_get_thread_num();
        const int    T  = omp_get_num_threads();
        const int64_t lo = (int64_t)t * N / T;
        const int64_t hi = (int64_t)(t + 1) * N / T;
        for (int64_t row = lo; row < hi; ++row) {
            const int64_t e0 = L_indptr[row];
            const int64_t e1 = L_indptr[row + 1];
            if (e1 == e0) {
                x[row] = b[row] / 0.0;
                atomic_store_explicit(&flag[row], epoch, memory_order_release);
                continue;
            }
            const int64_t last = e1 - 1;
            const int64_t hasdiag = (L_indices[last] == row);
            const int64_t end = hasdiag ? last : e1;
            double acc = b[row];
            for (int64_t k = e0; k < end; ++k) {
                const int64_t col = L_indices[k];
                if (__builtin_expect(col < lo, 0)) {
                    while (atomic_load_explicit(&flag[col], memory_order_acquire) != epoch)
                        sptrsv_pause();
                }
                acc -= L_data[k] * x[col];
            }
            x[row] = acc / (hasdiag ? L_data[last] : 0.0);
            atomic_store_explicit(&flag[row], epoch, memory_order_release);
        }
    }
}
