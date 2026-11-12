/* Level-scheduled sparse lower-triangular solve  x = L^-1 b.
 * Per-row "computed" flags stamped with a per-call generation let a consumer spin
 * only only on the few producers it actually reads, instead's relying on a global
 * barrier per level. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef MAXT
#define MAXT 24
#endif

static int32_t *g_flags = NULL;
static int64_t  g_flags_n = 0;
static int32_t  g_gen = 0;

static inline void x86_pause(void) { __builtin_ia32_pause(); }

static void solve_seq(const double *restrict L_data, const int64_t *restrict L_indices,
                      const int64_t *restrict L_indptr, const double *restrict b,
                      double *restrict x, int64_t N)
{
    for (int64_t row = 0; row < N; row++) {
        const int64_t p = L_indptr[row], e = L_indptr[row + 1];
        double s = b[row], diag = 0.0;
        for (int64_t k = p; k < e; k++) {
            const int64_t c = L_indices[k];
            if (c < row)      s -= L_data[k] * x[c];
            else if (c == row) diag = L_data[k];
        }
        x[row] = s / diag;
    }
}

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
    (void)NNZ_L; (void)workspace; (void)workspace_size;

    if (N <= 0) return;
    if (N < 8192 || level_ptr[0] != 0 || level_ptr[N] != N) {
        solve_seq(L_data, L_indices, L_indptr, b, x, N);
        return;
    }

    int64_t lo = 0, hi = N;
    while (lo < hi) {
        int64_t mid = lo + (hi - lo) / 2;
        if (level_ptr[mid] >= N) hi = mid; else lo = mid + 1;
    }
    const int64_t nlv = lo;

    int T = omp_get_max_threads();
    if (T > MAXT) T = MAXT;
    if (T < 1) T = 1;
    if (T == 1) { solve_seq(L_data, L_indices, L_indptr, b, x, N); return; }

    if (g_flags_n < N) {
        int32_t *nf = (int32_t *)calloc((size_t)N, sizeof(int32_t));
        if (!nf) { solve_seq(L_data, L_indices, L_indptr, b, x, N); return; }
        free(g_flags);
        g_flags = nf; g_flags_n = N; g_gen = 0;
    }
    const int32_t gen = ++g_gen;
    int32_t *restrict flags = g_flags;

    #pragma omp parallel num_threads(T)
    {
   const int tid = omp_get_thread_num();
        for (int64_t lvl = 0; lvl < nlv; lvl++) {
            const int64_t a = level_ptr[lvl];
            const int64_t z = level_ptr[lvl + 1];
            const int64_t n = z - a;
            if (n <= 0) continue;
            const int64_t base = n / T, extra = n % T;
            int64_t i0, cnt;
            if (tid < extra) { i0 = a + tid * (base + 1); cnt = base + 1; }
            else            { i0 = a + extra * (base + 1) + (tid - extra) * base; cnt = base; }
            int64_t i1 = i0 + cnt;
            if (i1 > z) i1 = z;
            for (int64_t i = i0; i < i1; i++) {
                const int64_t row = perm[i];
                const int64_t p = L_indptr[row];
                const int64_t e = L_indptr[row + 1];
                double s = b[row], diag = 0.0;
                for (int64_t k = p; k < e; k++) {
                    const int64_t c = L_indices[k];
                    if (c < row) {
                        if (__atomic_load_n(&flags[c], __ATOMIC_ACQUIRE) != gen) {
                            while (__atomic_load_n(&flags[c], __ATOMIC_ACQUIRE) != gen) x86_pause();
                        }
                        s -= L_data[k] * x[c];
                    } else if (c == row) diag = L_data[k];
                }
                x[row] = s / diag;
                __atomic_store_n(&flags[row], gen, __ATOMIC_RELEASE);
            }
        }
    }
}
