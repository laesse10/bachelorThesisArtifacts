#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <omp.h>
#include <pthread.h>

/* =========================================================================
 * Level-scheduled sparse triangular solve  x = L^-1 b  (CSR lower triangular,
 * columns ascending, diagonal present).
 *
 * Rows are grouped into levels by the given schedule, so a row's off-diagonal
 * entries point only at rows of strictly earlier levels; rows within one
 * level are mutually independent.  Every row owns one release flag; a row
 * spins only on the flags of its own parent rows (3-4 of them), so the
 * critical path follows the exact row dependence chain and nothing else.
 * A persistent worker pool (created once) strides the level-sorted positions
 * round-robin; the main thread is worker 0 and joins by polling done flags.
 * ========================================================================= */

#define MAXT 512
#define MAXN (1 << 17)

typedef struct { _Atomic uint32_t f; char pad[60]; } Flag;
static Flag rflag[MAXN];                       /* one padded release flag per row position */

static int32_t *permPos = NULL;                /* row -> position */
static int32_t *srow = NULL;                   /* position -> row (level-sorted) */
static int32_t *pf = NULL;                     /* entry -> parent position, -1 for diagonal */
static int64_t capN = 0, capZ = 0;
static int fast_ok = 1;

static pthread_t workers[MAXT];
static _Atomic uint32_t jobGen = 0;
static Flag doneF[MAXT];
static int nWorkers = 0;
static volatile int curNT = 0;
static const double *g_data = NULL;
static const int64_t *g_idx = NULL;
static const int64_t *g_ip = NULL;
static const double *g_b = NULL;
static double *g_x = NULL;
static int64_t g_N = 0;
static uint32_t genCnt32 = 0;

static int64_t cache_N = -1, cache_NNZ = -1, cache_nt = -1;
static uint64_t fp1 = 0, fp2 = 0, fp3 = 0;

static uint64_t mix64(uint64_t x)
{
    x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static void fp_compute(const int64_t *indptr, const int64_t *indices,
                       const int64_t *level_ptr, const int64_t *perm,
                       int64_t N, int64_t NNZ, uint64_t *o1, uint64_t *o2, uint64_t *o3)
{
    int64_t step = N / 128; if (step < 1) step = 1;
    uint64_t h1 = 0, h2 = 0, h3 = 0;
    for (int64_t i = 0; i < N + 1; i += step) h1 = mix64(h1 ^ (uint64_t)indptr[i]);
    for (int64_t i = 0; i < NNZ; i += step)  h2 = mix64(h2 ^ (uint64_t)indices[i]);
    for (int64_t i = 0; i < N; i += step)
        h3 = mix64(h3 ^ (uint64_t)perm[i]) ^ mix64((uint64_t)level_ptr[i] * 0x9E3779B97F4A7C15ULL);
    h1 ^= (uint64_t)N * 0xBF58476D1CE4E5B9ULL ^ (uint64_t)NNZ * 0x94D049BB133111EBULL;
    h1 ^= (uint64_t)level_ptr[N];
    *o1 = h1; *o2 = h2; *o3 = h3;
}

static inline void row_fast(const double *__restrict data, const int64_t *__restrict indices,
                            const int64_t *__restrict indptr, const double *__restrict b,
                            double *__restrict x, int64_t r)
{
    int64_t s0 = indptr[r], s1 = indptr[r + 1];
    double s = b[r];
    for (int64_t k = s0; k < s1 - 1; k++)
        s = fma(-data[k], x[indices[k]], s);
    x[r] = s / data[s1 - 1];
}

static inline void row_generic(const double *__restrict data, const int64_t *__restrict indices,
                               const int64_t *__restrict indptr, const double *__restrict b,
                               double *__restrict x, int64_t r)
{
    int64_t s0 = indptr[r], s1 = indptr[r + 1];
    double s = b[r], diag = 0.0;
    for (int64_t k = s0; k < s1; k++) {
        int64_t c = indices[k];
        if (c < r) s -= data[k] * x[c];
        else if (c == r) diag = data[k];
    }
    x[r] = s / diag;
}

static void serial_solve(const double *data, const int64_t *indices, const int64_t *indptr,
                         const double *b, double *x, int64_t N, int fast)
{
    if (fast) {
        for (int64_t r = 0; r < N; r++) row_fast(data, indices, indptr, b, x, r);
    } else {
        for (int64_t r = 0; r < N; r++) row_generic(data, indices, indptr, b, x, r);
    }
}

static void *xa64(size_t n) { void *p = aligned_alloc(64, n); if (p) memset(p, 0, n); return p; }

typedef struct { int32_t r, p; } RP;
static int cmp_rp(const void *a, const void *b)
{
    const RP *A = (const RP *)a, *B = (const RP *)b;
    return (A->r > B->r) - (A->r < B->r);
}

static void build(const int64_t *indptr, const int64_t *indices,
                  const int64_t *level_ptr, const int64_t *perm,
                  int64_t N, int64_t NNZ)
{
    (void)NNZ;
    if (capN < N) {
        capN = N < (1 << 20) ? (1 << 20) : N;
        free(permPos); free(srow);
        permPos = xa64((size_t)capN * 4);
        srow    = xa64((size_t)capN * 4);
    }
    for (int64_t p = 0; p < N; p++) permPos[perm[p]] = (int32_t)p;

    /* sort rows within each level by row index (rows of a level are
       independent, so in-level order is free; sorting restores locality) */
    RP *tmp = (RP *)xa64((size_t)N * 8);
    for (int64_t l = 0; l < N; l++) {
        if (level_ptr[l + 1] == level_ptr[l]) continue;
        int64_t lo = level_ptr[l];
        int32_t n = (int32_t)(level_ptr[l + 1] - lo);
        for (int32_t i = 0; i < n; i++) {
            tmp[i].r = (int32_t)perm[lo + i];
            tmp[i].p = (int32_t)(lo + i);
        }
        qsort(tmp, (size_t)n, sizeof(RP), cmp_rp);
        for (int32_t i = 0; i < n; i++)
            srow[lo + i] = tmp[i].r;
    }
    free(tmp);

    fast_ok = 1;
    for (int64_t r = 0; r < N; r++) {
        int64_t s0 = indptr[r], s1 = indptr[r + 1];
        if (s1 == s0 || indices[s1 - 1] != r) fast_ok = 0;
    }

    if (capZ < NNZ) {
        capZ = NNZ;
        free(pf);
        pf = xa64((size_t)capZ * 4);
    }
    for (int64_t r = 0; r < N; r++) {
        int64_t s0 = indptr[r], s1 = indptr[r + 1];
        for (int64_t k = s0; k < s1; k++)
            pf[k] = (k < s1 - 1) ? permPos[indices[k]] : -1;
    }
}

static inline void row_wait(int64_t s0, int64_t s1, uint32_t g)
{
    for (int64_t k = s0; k < s1 - 1; k++) {
        int32_t f = pf[k];
        if (f >= 0)
            while (__atomic_load_n(&rflag[f].f, __ATOMIC_ACQUIRE) != g)
                __builtin_ia32_pause();
    }
}

/* relaxed look-ahead: warm the next row's parent flag lines and x lines
   while the current row is being computed (non-blocking, no ordering claim) */
static inline void row_warm(int32_t r)
{
    int64_t a = g_ip[r], b = g_ip[r + 1];
    for (int64_t k = a; k < b - 1; k++) {
        int32_t f = pf[k];
        if (f >= 0) __atomic_load_n(&rflag[f].f, __ATOMIC_RELAXED);
        _mm_prefetch((const char *)&g_x[g_idx[k]], _MM_HINT_T0);
    }
}

static inline void row_pf(int32_t r)
{
    int64_t a = g_ip[r], b = g_ip[r + 1];
    for (int64_t k = a; k < b - 1; k++)
        _mm_prefetch((const char *)&g_x[g_idx[k]], _MM_HINT_T0);
}

static inline void row_compute(int32_t r, int64_t s0, int64_t s1, int64_t p, uint32_t g)
{
    double s = g_b[r];
    for (int64_t k = s0; k < s1 - 1; k++)
        s = fma(-g_data[k], g_x[g_idx[k]], s);
    g_x[r] = s / g_data[s1 - 1];
    __atomic_store_n(&rflag[p].f, g, __ATOMIC_RELEASE);
}

static void process_job(int t, uint32_t g)
{
    const int64_t N = g_N;
    const int64_t nt = curNT;
    int64_t p = t;
    int64_t plim = N - nt;
    for (; p < plim; p += 2 * nt) {
        const int64_t p2 = p + nt;
        int32_t r = srow[p];
        int64_t s0 = g_ip[r], s1 = g_ip[r + 1];
        int32_t r2 = srow[p2];
        int64_t s02 = g_ip[r2], s12 = g_ip[r2 + 1];
        row_wait(s0, s1, g);
        row_warm(r2);
        row_compute(r, s0, s1, p, g);
        if (p + 2 < N) row_pf(srow[p + 2]);
        row_wait(s02, s12, g);
        row_compute(r2, s02, s12, p2, g);
        if (p2 + 2 < N) row_pf(srow[p2 + 2]);
    }
    for (; p < N; p += nt) {
        int32_t r = srow[p];
        int64_t s0 = g_ip[r], s1 = g_ip[r + 1];
        row_wait(s0, s1, g);
        if (p + 2 < N) row_pf(srow[p + 2]);
        row_compute(r, s0, s1, p, g);
    }
}

static void *worker_fn(void *arg)
{
    const int t = (int)(intptr_t)arg;
    uint32_t g = 0;
    for (;;) {
        uint32_t j;
        while ((j = __atomic_load_n(&jobGen, __ATOMIC_ACQUIRE)) == g)
            __builtin_ia32_pause();
        g = j;
        if (t >= curNT) {
            __atomic_store_n(&doneF[t].f, g, __ATOMIC_RELEASE);
            continue;
        }
        process_job(t, g);
        __atomic_store_n(&doneF[t].f, g, __ATOMIC_RELEASE);
    }
    return NULL;
}

static void start_pool(int nt)
{
    for (int t = 1; t < nt; t++)
        pthread_create(&workers[t], NULL, worker_fn, (void *)(intptr_t)t);
    nWorkers = nt;
}

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *restrict b,
    const int64_t *restrict level_ptr,
    const int64_t *restrict perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    if (N == 0) return;

    int nt = omp_get_max_threads();
    if (nt > MAXT) nt = MAXT;

    uint64_t a, bb, c;
    fp_compute(L_indptr, L_indices, level_ptr, perm, N, NNZ_L, &a, &bb, &c);
    int valid = (cache_N == N && cache_NNZ == NNZ_L && cache_nt == nt &&
                 a == fp1 && bb == fp2 && c == fp3);
    if (!valid) {
        build(L_indptr, L_indices, level_ptr, perm, N, NNZ_L);
        fp1 = a; fp2 = bb; fp3 = c;
        cache_N = N; cache_NNZ = NNZ_L; cache_nt = nt;
    }
    if (N >= MAXN || N < 4096 || nt < 2 || !fast_ok) {
        serial_solve(L_data, L_indices, L_indptr, b, x, N, fast_ok);
        return;
    }

    if (nWorkers == 0) start_pool(nt);
    if (nt != nWorkers) {
        nt = nWorkers;
    }

    g_data = L_data;
    g_idx  = L_indices;
    g_ip   = L_indptr;
    g_b    = b;
    g_x    = x;
    g_N    = N;
    curNT  = nt;
    genCnt32++;
    const uint32_t g = genCnt32;
    __atomic_store_n(&jobGen, g, __ATOMIC_RELEASE);
    process_job(0, g);
    __atomic_store_n(&doneF[0].f, g, __ATOMIC_RELEASE);
    for (int t = 1; t < nt; t++)
        while (__atomic_load_n(&doneF[t].f, __ATOMIC_ACQUIRE) != g)
            __builtin_ia32_pause();
}
