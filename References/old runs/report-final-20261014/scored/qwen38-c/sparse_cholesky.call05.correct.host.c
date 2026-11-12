#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define SP_FUTEX_WAIT 0
#define SP_FUTEX_WAKE 1
static long sp_futex(void *addr, int op, unsigned int val)
{
    long ret;
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"(202L), "D"(addr), "S"((unsigned long)op),
                        "d"((unsigned long)val)
                      : "rcx", "r11", "memory");
    return ret;
}

typedef struct {
    const double *A_data; const int32_t *A_idx; const int32_t *A_ind;
    const int32_t *L_idx; const int32_t *L_ind; const int32_t *Lto32;
    const int32_t *Li32; const int32_t *Li_ind;
    const int64_t *A_idx64; const int64_t *A_ind64;
    const int64_t *L_idx64; const int64_t *L_ind64; const int64_t *Lto64;
    const int64_t *Li64; const int64_t *Li_ind64;
    double *Lc_data;
    int32_t *rowS; int32_t *csc_ind; int32_t *to_sorted; int32_t *csc_to_lc;
    double *sdata;
    const double *b; double *y; double *z;
    uint32_t N;
    double **work;
} K;

/* v5: level-batched waves + one-shot done flags + claim CAS + eager
   successor chaining.
   - level(j) = 1 + max(level of predecessors); predecessors: k<j in L row j
     (factor/fwd, computed ascending) or k>j in Lc row j (bwd, descending).
   - Columns counting-sorted by level; wave w = WAVE consecutive levels;
     one barrier per wave (wave depends only on earlier waves).
   - g_claim[j]: CAS 0->1 ownership (exactly one thread ever processes j).
   - g_done[j]: release-stored after j's values are written; ready(j) =
     all predecessors' g_done set (acquire loads).
   - Eager chaining: after finishing j, check j's successors (CSC rows for
     factor/fwd; Lc row for bwd); claim+process any that are ready. This
     keeps long dependency chains inside one thread (no cross-thread
     handoff latency) while wide parts flow through the shared pool.
   - Pops are frontier-limited (g_front + POPK levels) so parked retry
     lists stay small. */

#define MAXT 256
#define WAVE 32
#define POPK 8

static uint32_t *g_done;      /* per-column done flag */
static uint32_t *g_claim;     /* per-column claim flag */
static uint32_t *g_level;     /* per-column level */
static uint32_t *g_pool;      /* columns sorted by level (stable within level) */
static uint32_t *g_lb;        /* LB[l] = #columns with level < l (length nl+1) */
static uint32_t *g_cur;       /* fill cursor (scratch) */
static uint32_t *g_wstart;    /* wave start positions (nwaves+1) */
static uint32_t *g_ptr;       /* per-wave current pool position */
static uint32_t *g_ret;       /* per-thread retry lists (T*N) */
static int g_T, g_N, g_nw;
static uint32_t g_front, g_lmax;

static inline int ready_f(K *k, uint32_t j) {
    uint32_t p0 = (uint32_t)k->L_ind[j], p1 = (uint32_t)k->L_ind[j + 1] - 1u;
    for (uint32_t p = p0; p < p1; p++)
        if (!__atomic_load_n(&g_done[k->L_idx[p]], __ATOMIC_ACQUIRE)) return 0;
    return 1;
}
static inline int ready_b(K *k, uint32_t j) {
    uint32_t p0 = (uint32_t)k->Li_ind[j] + 1u, p1 = (uint32_t)k->Li_ind[j + 1];
    for (uint32_t p = p0; p < p1; p++)
        if (!__atomic_load_n(&g_done[k->Li32[p]], __ATOMIC_ACQUIRE)) return 0;
    return 1;
}

static inline int claim(uint32_t j) {
    uint32_t z = 0;
    return __atomic_compare_exchange_n(&g_claim[j], &z, 1u, 0,
                                       __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}
static uint32_t g_waiters = 0;
static uint32_t g_fut = 0;

static inline void unpark(void)
{
    if (__atomic_load_n(&g_waiters, __ATOMIC_RELAXED) > 0u) {
        __atomic_add_fetch(&g_fut, 1u, __ATOMIC_ACQ_REL);
        sp_futex((void *)&g_fut, SP_FUTEX_WAKE, 3u);
    }
}

static inline void markdone(uint32_t j) {
    __atomic_store_n(&g_done[j], 1u, __ATOMIC_RELEASE);
    uint32_t lv = g_level[j];
    uint32_t f = __atomic_load_n(&g_front, __ATOMIC_RELAXED);
    while (lv > f)
        if (__atomic_compare_exchange_n(&g_front, &f, lv, 0,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)) break;
    unpark();
}

static void sort_by_level(uint32_t N) {
    uint32_t lmax = 0;
    for (uint32_t j = 0; j < N; j++) if (g_level[j] > lmax) lmax = g_level[j];
    uint32_t nl = lmax + 1;
    memset(g_cur, 0, (size_t)nl * 4);
    for (uint32_t j = 0; j < N; j++) g_cur[g_level[j]]++;
    g_lb[0] = 0;
    for (uint32_t l = 0; l < nl; l++) g_lb[l + 1] = g_lb[l] + g_cur[l];
    for (uint32_t l = 0; l < nl; l++) g_cur[l] = g_lb[l];
    for (uint32_t j = 0; j < N; j++) g_pool[g_cur[g_level[j]]++] = j;
    g_lmax = lmax;
    g_nw = (int)((nl + WAVE - 1) / WAVE);
    for (int w = 0; w <= g_nw; w++) {
        uint32_t l = (uint32_t)w * WAVE;
        if (l > nl) l = nl;
        g_wstart[w] = g_lb[l];
    }
}

static void build_level_f(K *k) {
    uint32_t N = k->N;
    for (uint32_t j = 0; j < N; j++) {
        uint32_t lv = 0;
        uint32_t p0 = (uint32_t)k->L_ind[j], p1 = (uint32_t)k->L_ind[j + 1] - 1u;
        for (uint32_t p = p0; p < p1; p++) {
            uint32_t l = g_level[k->L_idx[p]] + 1u;
            if (l > lv) lv = l;
        }
        g_level[j] = lv;
    }
    sort_by_level(N);
}

static void build_level_b(K *k) {
    uint32_t N = k->N;
    for (uint32_t j = N; j-- > 0; ) {
        uint32_t lv = 0;
        uint32_t p0 = (uint32_t)k->Li_ind[j] + 1u, p1 = (uint32_t)k->Li_ind[j + 1];
        for (uint32_t p = p0; p < p1; p++) {
            uint32_t l = g_level[k->Li32[p]] + 1u;
            if (l > lv) lv = l;
        }
        g_level[j] = lv;
    }
    sort_by_level(N);
}

/* ---- shared wave-pop scaffolding (used by all three phases) ---- */
#define POP_WAVE(wv, hold, hh) do { \
        for (;;) { \
            uint32_t h = __atomic_load_n(&g_ptr[wv], __ATOMIC_RELAXED); \
            uint32_t end = g_wstart[wv + 1]; \
            if (h >= end) break; \
            uint32_t f = __atomic_load_n(&g_front, __ATOMIC_RELAXED); \
            uint32_t lim = f + POPK; \
            if (lim > g_lmax) lim = g_lmax; \
            uint32_t limpos = g_lb[lim + 1]; \
            if (limpos > end) limpos = end; \
            if (limpos <= h) break; \
            uint32_t take = limpos - h; \
            if (take > 8) take = 8; \
            if (__atomic_compare_exchange_n(&g_ptr[wv], &h, h + take, 0, \
                                            __ATOMIC_RELAXED, __ATOMIC_RELAXED)) { \
                hh = take; \
                for (uint32_t i = 0; i < take; i++) hold[i] = g_pool[h + i]; \
                break; \
            } \
        } \
    } while (0)

typedef int (*sp_ready_fn)(K *, uint32_t);

static void sp_park(K *k, int wv, uint32_t *hold, uint32_t *hh,
                    uint32_t *ret, uint32_t *rb, uint32_t *jout,
                    int *haveout, int *pexit, sp_ready_fn rf)
{
    for (;;) {
        if (*rb == 0 &&
            __atomic_load_n(&g_ptr[wv], __ATOMIC_ACQUIRE) >= g_wstart[wv + 1]) {
            *pexit = 1; return;
        }
        __atomic_add_fetch(&g_waiters, 1u, __ATOMIC_RELAXED);
        POP_WAVE(wv, hold, (*hh));
        if (*hh > 0) { __atomic_sub_fetch(&g_waiters, 1u, __ATOMIC_RELEASE); return; }
        int pw = 0;
        for (uint32_t i = 0; i < *rb; i++)
            if (rf(k, ret[i])) {
                *jout = ret[i]; ret[i] = ret[--(*rb)];
                *haveout = 1; pw = 1; break;
            }
        if (pw) { __atomic_sub_fetch(&g_waiters, 1u, __ATOMIC_RELEASE); return; }
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        uint32_t v = __atomic_load_n(&g_fut, __ATOMIC_ACQUIRE);
        for (uint32_t i = 0; i < *rb; i++)
            if (rf(k, ret[i])) {
                *jout = ret[i]; ret[i] = ret[--(*rb)];
                *haveout = 1; pw = 1; break;
            }
        if (pw) { __atomic_sub_fetch(&g_waiters, 1u, __ATOMIC_RELEASE); return; }
        if (__atomic_load_n(&g_waiters, __ATOMIC_RELAXED) > 1u)
            sp_futex((void *)&g_fut, SP_FUTEX_WAKE, 1u);
        sp_futex((void *)&g_fut, SP_FUTEX_WAIT, v);
        __atomic_sub_fetch(&g_waiters, 1u, __ATOMIC_RELEASE);
    }
}


/* factorization phase */
static void phase_factor(K *k) {
    uint32_t N = k->N;
    memset(g_done, 0, (size_t)N * 4);
    memset(g_claim, 0, (size_t)N * 4);
    g_front = 0;
    g_nw = 1;
    g_wstart[0] = 0;
    g_wstart[1] = N;
    g_ptr[0] = 0;
    #pragma omp parallel num_threads(2)
    {
        uint32_t t = (uint32_t)omp_get_thread_num();
        double *wbuf = k->work[t];
        uint32_t *ret = &g_ret[(size_t)t * N];
        for (int wv = 0; wv < g_nw; wv++) {
            uint32_t rb = 0, hh = 0, hi = 0, sp = 0;
            uint32_t hold[8];
            int pexit = 0;
            for (;;) {
                uint32_t j = 0;
                int have = 0;
                if (hh > 0) { j = hold[hi++]; hh--; have = 1; }
                else {
                    hi = 0;
                    POP_WAVE(wv, hold, hh);
                    if (hh > 0) { j = hold[hi++]; hh--; have = 1; }
                    else if (rb > 0) {
                        for (uint32_t i = 0; i < rb; i++)
                            if (ready_f(k, ret[i])) {
                                j = ret[i]; ret[i] = ret[--rb];
                                have = 1;
                                break;
                            }
                    }
                }
                if (!have) {
                    if (hh == 0 && rb == 0) {
                        uint32_t h2 = __atomic_load_n(&g_ptr[wv], __ATOMIC_ACQUIRE);
                        if (h2 >= g_wstart[wv + 1]) break;
                    }
                    if (sp < 2048u) { sp++; __builtin_ia32_pause(); continue; }
                    sp = 0;
                    sp_park(k, wv, hold, &hh, ret, &rb, &j, &have, &pexit,
                            (sp_ready_fn)ready_f);
                    if (pexit) break;
                    continue;
                }
                sp = 0;
                if (!ready_f(k, j)) { ret[rb++] = j; continue; }
                if (!claim(j)) continue;
            process:
                {
                    uint32_t s0 = (uint32_t)k->csc_ind[j], s1 = (uint32_t)k->csc_ind[j + 1];
                    for (uint32_t s = s0; s < s1; s++) wbuf[k->rowS[s]] = 0.0;
                }
                {
                    uint32_t p0 = (uint32_t)k->A_ind[j], p1 = (uint32_t)k->A_ind[j + 1];
                    for (uint32_t p = p0; p < p1; p++) {
                        uint32_t r = (uint32_t)k->A_idx[p];
                        if (r >= j) wbuf[r] = k->A_data[p];
                    }
                }
                {
                    uint32_t p0 = (uint32_t)k->L_ind[j], p1 = (uint32_t)k->L_ind[j + 1] - 1u;
                    for (uint32_t p = p0; p < p1; p++) {
                        uint32_t kk = (uint32_t)k->L_idx[p];
                        double ljk = k->sdata[k->to_sorted[p]];
                        uint32_t end = (uint32_t)k->csc_ind[kk + 1];
                        for (uint32_t s = (uint32_t)k->to_sorted[p]; s < end; s++) {
                            uint32_t r = (uint32_t)k->rowS[s];
                            wbuf[r] = fma(-ljk, k->sdata[s], wbuf[r]);
                        }
                    }
                }
                double ljj = sqrt(wbuf[j]);
                k->Lc_data[k->Li_ind[j]] = ljj;
                k->sdata[k->csc_ind[j]] = ljj;
                {
                    uint32_t s0 = (uint32_t)k->csc_ind[j] + 1u, s1 = (uint32_t)k->csc_ind[j + 1];
                    for (uint32_t s = s0; s < s1; s++) {
                        uint32_t r = (uint32_t)k->rowS[s];
                        double v = wbuf[r] / ljj;
                        k->sdata[s] = v;
                        k->Lc_data[k->csc_to_lc[s]] = v;
                    }
                }
                markdone(j);
                /* eager successors */
                {
                    uint32_t s0 = (uint32_t)k->csc_ind[j] + 1u, s1 = (uint32_t)k->csc_ind[j + 1];
                    for (uint32_t s = s0; s < s1; s++) {
                        uint32_t r = (uint32_t)k->rowS[s];
                        if (ready_f(k, r) && claim(r)) { j = r; goto process; }
                    }
                }
            }
            #pragma omp barrier
        }
    }
}

/* forward solve: z = L^-1 b (same levels/pool as factor) */
static void phase_fwd(K *k) {
    uint32_t N = k->N;
    memset(g_done, 0, (size_t)N * 4);
    memset(g_claim, 0, (size_t)N * 4);
    g_front = 0;
    g_nw = 1;
    g_wstart[0] = 0;
    g_wstart[1] = N;
    g_ptr[0] = 0;
    #pragma omp parallel num_threads(2)
    {
        uint32_t t = (uint32_t)omp_get_thread_num();
        uint32_t *ret = &g_ret[(size_t)t * N];
        for (int wv = 0; wv < g_nw; wv++) {
            uint32_t rb = 0, hh = 0, hi = 0, sp = 0;
            uint32_t hold[8];
            int pexit = 0;
            for (;;) {
                uint32_t j = 0;
                int have = 0;
                if (hh > 0) { j = hold[hi++]; hh--; have = 1; }
                else {
                    hi = 0;
                    POP_WAVE(wv, hold, hh);
                    if (hh > 0) { j = hold[hi++]; hh--; have = 1; }
                    else if (rb > 0) {
                        for (uint32_t i = 0; i < rb; i++)
                            if (ready_f(k, ret[i])) {
                                j = ret[i]; ret[i] = ret[--rb];
                                have = 1;
                                break;
                            }
                    }
                }
                if (!have) {
                    if (hh == 0 && rb == 0) {
                        uint32_t h2 = __atomic_load_n(&g_ptr[wv], __ATOMIC_ACQUIRE);
                        if (h2 >= g_wstart[wv + 1]) break;
                    }
                    if (sp < 2048u) { sp++; __builtin_ia32_pause(); continue; }
                    sp = 0;
                    sp_park(k, wv, hold, &hh, ret, &rb, &j, &have, &pexit,
                            (sp_ready_fn)ready_f);
                    if (pexit) break;
                    continue;
                }
                sp = 0;
                if (!ready_f(k, j)) { ret[rb++] = j; continue; }
                if (!claim(j)) continue;
            process:
                double s = k->b[j];
                uint32_t p0 = (uint32_t)k->L_ind[j], p1 = (uint32_t)k->L_ind[j + 1] - 1u;
                for (uint32_t p = p0; p < p1; p++)
                    s = fma(-k->Lc_data[k->Lto32[p]], k->z[k->L_idx[p]], s);
                k->z[j] = s / k->Lc_data[k->Li_ind[j]];
                markdone(j);
                {
                    uint32_t s0 = (uint32_t)k->csc_ind[j] + 1u, s1 = (uint32_t)k->csc_ind[j + 1];
                    for (uint32_t s = s0; s < s1; s++) {
                        uint32_t r = (uint32_t)k->rowS[s];
                        if (ready_f(k, r) && claim(r)) { j = r; goto process; }
                    }
                }
            }
            #pragma omp barrier
        }
    }
}

/* backward solve: y = L^-T z (in Lc original entry order, like the reference) */
static void phase_bwd(K *k) {
    uint32_t N = k->N;
    memset(g_done, 0, (size_t)N * 4);
    memset(g_claim, 0, (size_t)N * 4);
    g_front = 0;
    g_nw = 1;
    g_wstart[0] = 0;
    g_wstart[1] = N;
    g_ptr[0] = 0;
    #pragma omp parallel num_threads(2)
    {
        uint32_t t = (uint32_t)omp_get_thread_num();
        uint32_t *ret = &g_ret[(size_t)t * N];
        for (int wv = 0; wv < g_nw; wv++) {
            uint32_t rb = 0, hh = 0, hi = 0, sp = 0;
            uint32_t hold[8];
            int pexit = 0;
            for (;;) {
                uint32_t j = 0;
                int have = 0;
                if (hh > 0) { j = hold[hi++]; hh--; have = 1; }
                else {
                    hi = 0;
                    POP_WAVE(wv, hold, hh);
                    if (hh > 0) { j = hold[hi++]; hh--; have = 1; }
                    else if (rb > 0) {
                        for (uint32_t i = 0; i < rb; i++)
                            if (ready_b(k, ret[i])) {
                                j = ret[i]; ret[i] = ret[--rb];
                                have = 1;
                                break;
                            }
                    }
                }
                if (!have) {
                    if (hh == 0 && rb == 0) {
                        uint32_t h2 = __atomic_load_n(&g_ptr[wv], __ATOMIC_ACQUIRE);
                        if (h2 >= g_wstart[wv + 1]) break;
                    }
                    if (sp < 2048u) { sp++; __builtin_ia32_pause(); continue; }
                    sp = 0;
                    sp_park(k, wv, hold, &hh, ret, &rb, &j, &have, &pexit,
                            (sp_ready_fn)ready_b);
                    if (pexit) break;
                    continue;
                }
                sp = 0;
                if (!ready_b(k, j)) { ret[rb++] = j; continue; }
                if (!claim(j)) continue;
            process:
                double s = k->z[j];
                uint32_t p0 = (uint32_t)k->Li_ind[j] + 1u, p1 = (uint32_t)k->Li_ind[j + 1];
                for (uint32_t p = p0; p < p1; p++)
                    s = fma(-k->Lc_data[p], k->y[k->Li32[p]], s);
                k->y[j] = s / k->Lc_data[k->Li_ind[j]];
                markdone(j);
                {
                    uint32_t p0 = (uint32_t)k->L_ind[j], p1 = (uint32_t)k->L_ind[j + 1] - 1u;
                    for (uint32_t p = p0; p < p1; p++) {
                        uint32_t r = (uint32_t)k->L_idx[p];
                        if (ready_b(k, r) && claim(r)) { j = r; goto process; }
                    }
                }
            }
            #pragma omp barrier
        }
    }
}


static void phase_factor_serial(K *k)
{
    const uint32_t N = k->N;
    double *wbuf = k->work[0];
    const int32_t *csc_ind = k->csc_ind, *rowS = k->rowS, *to_sorted = k->to_sorted, *csc_to_lc = k->csc_to_lc;
    const double *A_data = k->A_data;
    double *sdata = k->sdata;
    const int64_t *A_ind64 = k->A_ind64, *A_idx64 = k->A_idx64;
    const int64_t *L_ind64 = k->L_ind64, *L_idx64 = k->L_idx64, *Li_ind64 = k->Li_ind64;
    double *Lc_data = k->Lc_data;
    /* 4-stride unroll wins when the factor working set is L2-resident (short windows,
       ~130KB); with L3-resident streams (~375KB) the plain loop is faster (measured) */
    const int fast4 = (uint64_t)csc_ind[N] * 8u <= 262144u;
    for (uint32_t j = 0; j < N; j++) {
        memset(wbuf, 0, (size_t)N * 8u);
        uint32_t p0 = (uint32_t)A_ind64[j], p1 = (uint32_t)A_ind64[j + 1];
        for (uint32_t p = p0; p < p1; p++) {
            uint32_t r = (uint32_t)A_idx64[p];
            if (r >= j) wbuf[r] = A_data[p];
        }
        uint32_t q0 = (uint32_t)L_ind64[j], q1 = (uint32_t)L_ind64[j + 1] - 1u;
        for (uint32_t p = q0; p < q1; p++) {
            double ljk = sdata[to_sorted[p]];
            uint32_t kk = (uint32_t)L_idx64[p];
            uint32_t s = (uint32_t)to_sorted[p];
            uint32_t end = (uint32_t)csc_ind[kk + 1];
            if (fast4) {
                while (s < end && (s & 3u)) {
                    uint32_t r = (uint32_t)rowS[s];
                    wbuf[r] = fma(-ljk, sdata[s], wbuf[r]);
                    s++;
                }
                for (; s + 3 < end; s += 4) {
                    uint64_t rs0, rs1;
                    __builtin_memcpy(&rs0, rowS + s, 8);
                    __builtin_memcpy(&rs1, rowS + s + 2, 8);
                    uint32_t r0 = (uint32_t)rs0, r1 = (uint32_t)(rs0 >> 32);
                    uint32_t r2 = (uint32_t)rs1, r3 = (uint32_t)(rs1 >> 32);
                    double v0 = fma(-ljk, sdata[s], wbuf[r0]);
                    double v1 = fma(-ljk, sdata[s + 1], wbuf[r1]);
                    double v2 = fma(-ljk, sdata[s + 2], wbuf[r2]);
                    double v3 = fma(-ljk, sdata[s + 3], wbuf[r3]);
                    wbuf[r0] = v0; wbuf[r1] = v1; wbuf[r2] = v2; wbuf[r3] = v3;
                }
            }
            for (; s < end; s++) {
                uint32_t r = (uint32_t)rowS[s];
                wbuf[r] = fma(-ljk, sdata[s], wbuf[r]);
            }
        }
        double ljj = sqrt(wbuf[j]);
        double rljj = 1.0 / ljj;
        Lc_data[Li_ind64[j]] = ljj;
        sdata[csc_ind[j]] = ljj;
        uint32_t e0 = (uint32_t)csc_ind[j] + 1u, e1 = (uint32_t)csc_ind[j + 1];
        for (uint32_t s = e0; s < e1; s++) {
            uint32_t r = (uint32_t)rowS[s];
            double v = wbuf[r] * rljj;
            sdata[s] = v;
            Lc_data[csc_to_lc[s]] = v;
        }
    }
}

static void phase_fwd_serial(K *k)
{
    const uint32_t N = k->N;
    for (uint32_t j = 0; j < N; j++) {
        double s = k->b[j];
        uint32_t p0 = (uint32_t)k->L_ind64[j], p1 = (uint32_t)k->L_ind64[j + 1] - 1u;
        for (uint32_t p = p0; p < p1; p++)
            s = fma(-k->Lc_data[k->Lto64[p]], k->z[k->L_idx64[p]], s);
        k->z[j] = s * (1.0 / k->Lc_data[k->Li_ind64[j]]);
    }
}

static void phase_bwd_serial(K *k)
{
    const uint32_t N = k->N;
    for (uint32_t j = N; j-- > 0; ) {
        double s = k->z[j];
        uint32_t p0 = (uint32_t)k->Li_ind64[j] + 1u, p1 = (uint32_t)k->Li_ind64[j + 1];
        for (uint32_t p = p0; p < p1; p++)
            s = fma(-k->Lc_data[p], k->y[k->Li64[p]], s);
        k->y[j] = s * (1.0 / k->Lc_data[k->Li_ind64[j]]);
    }
}

void sparse_cholesky_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const int64_t *restrict L_to_Lc,
    double *restrict Lc_data,
    const int64_t *restrict Lc_indices,
    const int64_t *restrict Lc_indptr,
    const double *restrict b,
    double *restrict y,
    const int64_t EDGE,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const uint32_t N = (uint32_t)(EDGE * EDGE * EDGE);
    const uint32_t nnzL = (uint32_t)Lc_indptr[N];

    int nt = 1;  /* serial: on this host class, the parallel pool never beats serial
                    (oversubscription + multi-socket placement + low-freq idle state);
                    the serial topological pass is ~6x faster than T>=2 at light load */
    int32_t *A_idx = NULL, *A_ind = NULL, *L_idx = NULL, *L_ind = NULL;
    int32_t *Lto32 = NULL, *Li32 = NULL, *Li_ind = NULL;
    if (nt > 1) {
        A_idx   = (int32_t *)malloc((uint64_t)A_indptr[N] * 4);
        A_ind   = (int32_t *)malloc((uint64_t)(N + 1) * 4);
        L_idx   = (int32_t *)malloc((uint64_t)nnzL * 4);
        L_ind   = (int32_t *)malloc((uint64_t)(N + 1) * 4);
        Lto32   = (int32_t *)malloc((uint64_t)nnzL * 4);
        Li32    = (int32_t *)malloc((uint64_t)nnzL * 4);
        Li_ind  = (int32_t *)malloc((uint64_t)(N + 1) * 4);
    }
    int32_t *rowS    = (int32_t *)malloc((uint64_t)nnzL * 4);
    int32_t *csc_ind = (int32_t *)malloc((uint64_t)(N + 1) * 4);
    int32_t *to_sorted = (int32_t *)malloc((uint64_t)nnzL * 4);
    int32_t *csc_to_lc = (int32_t *)malloc((uint64_t)nnzL * 4);
    int32_t *cursor  = (int32_t *)malloc((uint64_t)N * 4);
    double *sdata    = (double *)malloc((uint64_t)nnzL * 8);
    double *z        = (double *)malloc((uint64_t)N * 8);

    if (nt > 1) {
        for (uint32_t p = 0; p < A_indptr[N]; p++) A_idx[p] = (int32_t)A_indices[p];
        for (uint32_t i = 0; i <= N; i++) A_ind[i] = (int32_t)A_indptr[i];
        for (uint32_t p = 0; p < nnzL; p++) { L_idx[p] = (int32_t)L_indices[p]; Lto32[p] = (int32_t)L_to_Lc[p]; Li32[p] = (int32_t)Lc_indices[p]; }
        for (uint32_t i = 0; i <= N; i++) { L_ind[i] = (int32_t)L_indptr[i]; Li_ind[i] = (int32_t)Lc_indptr[i]; }
    }

    /* sorted CSC: rows of each column in ascending order; fill by walking L rows ascending */
    csc_ind[0] = 0;
    for (uint32_t j = 0; j < N; j++) csc_ind[j + 1] = csc_ind[j] + (uint32_t)(Lc_indptr[j + 1] - Lc_indptr[j]);
    for (uint32_t kk = 0; kk < N; kk++) cursor[kk] = csc_ind[kk];
    for (uint32_t r = 0; r < N; r++) {
        uint32_t p0 = (uint32_t)L_indptr[r], p1 = (uint32_t)L_indptr[r + 1];
        for (uint32_t p = p0; p < p1; p++) {
            uint32_t kcol = (uint32_t)L_indices[p];
            uint32_t pos = cursor[kcol]++;
            to_sorted[p] = (int32_t)pos;
            csc_to_lc[pos] = (int32_t)L_to_Lc[p];
            rowS[pos] = (int32_t)r;
        }
    }

    double **work = (double **)malloc(sizeof(double *) * (size_t)nt);
    for (int t = 0; t < nt; t++) work[t] = (double *)malloc(((uint64_t)N + 16) * 8);
    g_done   = (uint32_t *)malloc((uint64_t)N * 4);
    g_claim  = (uint32_t *)malloc((uint64_t)N * 4);
    g_level  = (uint32_t *)malloc((uint64_t)N * 4);
    g_pool   = (uint32_t *)malloc((uint64_t)N * 4);
    g_lb     = (uint32_t *)malloc((uint64_t)(N + 1) * 4);
    g_cur    = (uint32_t *)malloc((uint64_t)(N + 1) * 4);
    g_wstart = (uint32_t *)malloc((uint64_t)(N + 1) * 4);
    g_ptr    = (uint32_t *)malloc((uint64_t)(N + 1) * 4);
    g_ret    = (uint32_t *)malloc((uint64_t)nt * N * 4);

    K k = { .A_data=A_data, .A_idx=A_idx, .A_ind=A_ind, .L_idx=L_idx, .L_ind=L_ind, .Lto32=Lto32,
            .Li32=Li32, .Li_ind=Li_ind, .Lc_data=Lc_data, .rowS=rowS, .csc_ind=csc_ind,
            .to_sorted=to_sorted, .csc_to_lc=csc_to_lc, .sdata=sdata, .b=b, .y=y, .z=z,
            .A_idx64=A_indices, .A_ind64=A_indptr, .L_idx64=L_indices, .L_ind64=L_indptr,
            .Lto64=L_to_Lc, .Li64=Lc_indices, .Li_ind64=Lc_indptr,
            .N=N, .work=work };
    g_N = N; g_T = nt;
    if (nt == 1) {
        phase_factor_serial(&k);
        phase_fwd_serial(&k);
        phase_bwd_serial(&k);
    } else {
        build_level_f(&k);
        phase_factor(&k);
        phase_fwd(&k);
        build_level_b(&k);
        phase_bwd(&k);
    }

    free(g_done); free(g_claim); free(g_level); free(g_pool); free(g_lb); free(g_cur);
    free(g_wstart); free(g_ptr); free(g_ret); free(z); free(sdata); free(cursor);
    free(csc_to_lc); free(to_sorted); free(csc_ind); free(rowS); free(Li_ind); free(Li32);
    free(Lto32); free(L_ind); free(L_idx); free(A_ind); free(A_idx);
    for (int t = 0; t < nt; t++) free(work[t]);
    free(work);
}
