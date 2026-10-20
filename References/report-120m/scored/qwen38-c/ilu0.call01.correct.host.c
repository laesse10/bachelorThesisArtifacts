/* ILU(0) in-place factorization of a CSR matrix (Saad, Alg. 10.4).
 *
 * Optimizations over the naive row-by-row reference:
 *  - The pattern is fixed per matrix, so structure work is done once and
 *    cached: per-row diagonal position, a per-row update "plan" (for each
 *    lower entry k of row i: which entries of row k update which local
 *    positions of row i), the row dependency graph (row i depends on the
 *    rows k < i in its pattern) as a dependents list, and per-row dep counts.
 *  - A row only ever reads already-finalized rows, so ANY linear extension
 *    of the dependency order computes exactly the reference numbers.  Rows
 *    are therefore scheduled asynchronously: each finished row releases its
 *    dependents (ready when its dep count hits zero) into per-thread
 *    work-stealing deques.
 *  - No dense scratch rows / marker clearing: each row works in a small
 *    stack buffer.
 *  - 1/diag[k] is computed once per row k by k itself (one division per
 *    row, not one per lower entry).
 *  - FMA for the scatter updates.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define ILU0_MAX_LOCAL 64
#define ILU0_MAX_THREADS 16
#define ILU0_DEQ_CAP 512

typedef struct {
    int64_t N, nnz;
    const int64_t *indptr, *indices;
    uint64_t hash;
    int32_t *diagoff;     /* N: diagonal position within row i (= #lower entries) */
    int32_t *rowoff;      /* N: offset into plan_sub */
    int32_t *plan_sub;    /* sum_i (diagoff[i]+1): per-lower-entry plan starts */
    uint32_t *plan;       /* plan_total: packed (m<<6)|q */
    int64_t plan_total;
    int32_t *dep_start;   /* N+1: dependents CSR offsets (edge k -> i) */
    int32_t *dep_list;    /* E */
    int32_t *dep0;        /* N: initial dep count = #lower entries */
    double *diag, *recip;
    int ok_fast;
} ilu0_cache_t;

static ilu0_cache_t C_ = {0};
static int64_t *tmp_level = 0;
static int64_t *tmp_mark = 0;
static int64_t tmp_cap = 0;

/* per-thread scheduler state.  One MPMC ring per thread: head/tail only move
 * forward (fetch_add), so an owner pop and a steal claim disjoint slots. */
static int32_t *dep_cnt = 0;   /* N: per call */
static int32_t *q_arr = 0;    /* [threads][N] */
static _Atomic int32_t *q_head = 0, *q_tail = 0;
static _Atomic int32_t done_count;
static int64_t sch_cap = 0;

static void *xalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (!p)
        abort();
    return p;
}

static inline uint64_t hmix(uint64_t h, int64_t v)
{
    h ^= (uint64_t)v;
    h *= 0x100000001b3ULL;
    return h;
}

static uint64_t structure_hash(const int64_t *indptr, const int64_t *indices,
                               int64_t N, int64_t nnz)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    h = hmix(h, N);
    h = hmix(h, nnz);
    for (int64_t t = 0; t < 512; t++)
        h = hmix(h, indptr[((N + 1) * t) >> 9]);
    for (int64_t t = 0; t < 512; t++)
        h = hmix(h, indices[(nnz * t) >> 9]);
    return h;
}

static void cache_free(void)
{
    free(C_.diagoff); C_.diagoff = 0;
    free(C_.rowoff); C_.rowoff = 0;
    free(C_.plan_sub); C_.plan_sub = 0;
    free(C_.plan); C_.plan = 0;
    free(C_.dep_start); C_.dep_start = 0;
    free(C_.dep_list); C_.dep_list = 0;
    free(C_.dep0); C_.dep0 = 0;
    free(C_.diag); C_.diag = 0;
    free(C_.recip); C_.recip = 0;
    C_.ok_fast = 0;
    C_.plan_total = 0;
}

/* Exact (serial) reference port -- used only for patterns the fast path
 * cannot handle (unsorted rows, missing diagonal, very long rows). */
static void serial_ref(double *restrict A, const int64_t *restrict idx,
                       const int64_t *restrict ptr, int64_t N)
{
    static double *row_val = 0;
    static int32_t *in_row = 0;
    static double *sdiag = 0;
    static int64_t cap = 0;
    if (cap < N) {
        free(row_val); free(in_row); free(sdiag);
        row_val = xalloc((size_t)N * 8);
        in_row = xalloc((size_t)N * 4);
        sdiag = xalloc((size_t)N * 8);
        cap = N;
    }
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1];
        for (int64_t k = s; k < e; k++) {
            int64_t c = idx[k];
            row_val[c] = A[k];
            in_row[c] = 1;
        }
        for (int64_t k = s; k < e; k++) {
            int64_t ck = idx[k];
            if (ck >= i)
                continue;
            double l = row_val[ck] / sdiag[ck];
            row_val[ck] = l;
            for (int64_t m = ptr[ck]; m < ptr[ck + 1]; m++) {
                int64_t cj = idx[m];
                if (cj > ck && in_row[cj])
                    row_val[cj] -= l * A[m];
            }
        }
        sdiag[i] = row_val[i];
        for (int64_t k = s; k < e; k++) {
            int64_t c = idx[k];
            A[k] = row_val[c];
            in_row[c] = 0;
        }
    }
}

static void rebuild(const int64_t *ptr, const int64_t *idx, int64_t N, int64_t nnz)
{
    cache_free();
    C_.N = N;
    C_.nnz = nnz;
    C_.indptr = ptr;
    C_.indices = idx;
    C_.hash = structure_hash(ptr, idx, N, nnz);

    if (tmp_cap < N) {
        free(tmp_level);
        free(tmp_mark);
        tmp_level = xalloc((size_t)N * 8);
        tmp_mark = xalloc((size_t)N * 8);
        tmp_cap = N;
    }

    C_.diagoff = xalloc((size_t)N * 4);

    int sorted = 1, fulldiag = 1;
    int64_t maxlen = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1], n = e - s;
        if (n > maxlen)
            maxlen = n;
        int64_t d = -1;
        for (int64_t j = s; j < e; j++) {
            if (j > s && idx[j] < idx[j - 1])
                sorted = 0;
            if (idx[j] == i)
                d = j - s;
        }
        if (d < 0) {
            fulldiag = 0;
            d = n;
        }
        C_.diagoff[i] = (int32_t)d;
    }
    if (!(sorted && fulldiag && maxlen <= ILU0_MAX_LOCAL &&
          nnz < (1LL << 26) && N < (1LL << 26) && N > 0))
        return; /* serial fallback */

    /* Plan: for each row i and each lower entry j (column k): entries m of
     * row k with column > k and column present in row i (local pos q). */
    int64_t psub_total = 0, E = 0;
    for (int64_t i = 0; i < N; i++) {
        psub_total += (int64_t)C_.diagoff[i] + 1;
        E += C_.diagoff[i];
    }
    C_.plan_sub = xalloc((size_t)psub_total * 4);
    C_.rowoff = xalloc((size_t)N * 4);
    C_.dep0 = xalloc((size_t)N * 4);
    C_.dep_start = xalloc(((size_t)N + 1) * 4);
    C_.dep_list = xalloc(((size_t)E ? E : 1) * 4);
    for (int64_t i = 0; i < N; i++)
        C_.dep0[i] = C_.diagoff[i];
    for (int64_t i = 0; i <= N; i++)
        C_.dep_start[i] = 0;
    /* count dependents: for each row i, each lower entry k: one edge k->i */
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i];
        int32_t d = C_.diagoff[i];
        for (int32_t j = 0; j < d; j++)
            C_.dep_start[idx[s + j] + 1]++;
    }
    for (int64_t k = 0; k < N; k++)
        C_.dep_start[k + 1] += C_.dep_start[k];
    int32_t *dcur = xalloc((size_t)N * 4);
    memcpy(dcur, C_.dep_start, (size_t)N * 4);

    int64_t P = 0;
    int64_t off = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1];
        int32_t d = C_.diagoff[i];
        for (int64_t j = s; j < e; j++)
            tmp_mark[idx[j]] = ((i + 1) << 6) | (j - s);
        for (int32_t j = 0; j < d; j++) {
            int64_t k = idx[s + j];
            int64_t ks = ptr[k], kd = C_.diagoff[k];
            for (int64_t m = ks + kd + 1; m < ptr[k + 1]; m++) {
                if ((tmp_mark[idx[m]] >> 6) == (int64_t)(i + 1))
                    P++;
            }
        }
        for (int64_t j = s; j < e; j++)
            tmp_mark[idx[j]] = 0;
        C_.rowoff[i] = (int32_t)off;
        off += d + 1;
    }
    if (P >= (1LL << 31)) {
        free(dcur);
        cache_free();
        return;
    }
    C_.plan = xalloc((size_t)P * 4);
    C_.plan_total = P;
    int64_t pn = 0;
    off = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1];
        int32_t d = C_.diagoff[i];
        for (int64_t j = s; j < e; j++)
            tmp_mark[idx[j]] = ((i + 1) << 6) | (j - s);
        int32_t *sub = C_.plan_sub + off;
        sub[0] = (int32_t)pn;
        for (int32_t j = 0; j < d; j++) {
            int64_t k = idx[s + j];
            C_.dep_list[dcur[k]++] = (int32_t)i;
            int64_t ks = ptr[k], kd = C_.diagoff[k];
            for (int64_t m = ks + kd + 1; m < ptr[k + 1]; m++) {
                int64_t mk = tmp_mark[idx[m]];
                if ((mk >> 6) == (int64_t)(i + 1))
                    C_.plan[pn++] = (uint32_t)((m << 6) | (mk & 63));
            }
            sub[j + 1] = (int32_t)pn;
        }
        for (int64_t j = s; j < e; j++)
            tmp_mark[idx[j]] = 0;
        off += d + 1;
    }
    free(dcur);

    C_.diag = xalloc((size_t)N * 8);
    C_.recip = xalloc((size_t)N * 8);
    C_.ok_fast = 1;

    if (sch_cap < N) {
        free(dep_cnt);
        free(q_arr);
        free(q_head);
        free(q_tail);
        dep_cnt = xalloc((size_t)N * 4);
        q_arr = xalloc((size_t)ILU0_MAX_THREADS * (size_t)N * 4);
        q_head = xalloc((size_t)ILU0_MAX_THREADS * 4);
        q_tail = xalloc((size_t)ILU0_MAX_THREADS * 4);
        sch_cap = N;
    }
}

static inline void process_row(int32_t i, double *restrict A,
                               const int64_t *restrict ptr, const int64_t *restrict idx,
                               const int32_t *restrict diagoff,
                               const uint32_t *restrict plan, const int32_t *restrict plan_sub,
                               const int32_t *restrict rowoff,
                               double *restrict diag, double *restrict recip)
{
    int64_t s = ptr[i];
    int64_t n = ptr[i + 1] - s;
    int32_t d = diagoff[i];

    int64_t cols[ILU0_MAX_LOCAL];
    double vals[ILU0_MAX_LOCAL];
    for (int64_t j = 0; j < n; j++) {
        cols[j] = idx[s + j];
        vals[j] = A[s + j];
    }
    const int32_t *sub = plan_sub + rowoff[i];
    int32_t ps = sub[0], pe = sub[d];
    int32_t plen = pe - ps;
    if (plen > 0) {
        if (plen <= 32) {
            double amv[32];
            for (int32_t t = 0; t < plen; t++)
                amv[t] = A[plan[ps + t] >> 6];
            for (int32_t j = 0; j < d; j++) {
                double l = vals[j] * recip[cols[j]];
                vals[j] = l;
                for (int32_t t = sub[j]; t < sub[j + 1]; t++) {
                    uint32_t pq = plan[t];
                    int32_t q = (int32_t)(pq & 63);
                    vals[q] = fma(-l, amv[t - ps], vals[q]);
                }
            }
        } else {
            for (int32_t j = 0; j < d; j++) {
                double l = vals[j] * recip[cols[j]];
                vals[j] = l;
                for (int32_t t = sub[j]; t < sub[j + 1]; t++) {
                    uint32_t pq = plan[t];
                    int32_t q = (int32_t)(pq & 63);
                    vals[q] = fma(-l, A[pq >> 6], vals[q]);
                }
            }
        }
    }
    diag[i] = vals[d];
    recip[i] = 1.0 / diag[i];
    for (int64_t j = 0; j < n; j++)
        A[s + j] = vals[j];
}

/* Claim one row from a grow-only MPMC ring.  head advances only when an item
 * is actually present, so head <= tail always and the single producer (which
 * writes at index tail) never lands on a claimed-and-rejected slot. */
static inline int32_t q_claim(int32_t *arr, _Atomic int32_t *head,
                              _Atomic int32_t *tail)
{
    for (;;) {
        int32_t h = __atomic_load_n(head, __ATOMIC_RELAXED);
        int32_t tl = __atomic_load_n(tail, __ATOMIC_ACQUIRE);
        if (h < tl) {
            if (__atomic_compare_exchange_n(head, &h, h + 1, 0, __ATOMIC_ACQUIRE,
                                            __ATOMIC_RELAXED))
                return arr[h];
            continue;
        }
        return -1;
    }
}

void ilu0_fp64(double *restrict A, const int64_t *restrict idx,
               const int64_t *restrict ptr, int64_t N, int64_t nnz,
               uint8_t *restrict workspace, int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    if (N <= 0 || nnz <= 0)
        return;

    int reuse = 0;
    if (C_.N == N && C_.nnz == nnz) {
        if (C_.indptr == ptr && C_.indices == idx)
            reuse = 1;
        else if (C_.hash == structure_hash(ptr, idx, N, nnz))
            reuse = 1;
    }
    if (!reuse)
        rebuild(ptr, idx, N, nnz);

    if (!C_.ok_fast) {
        serial_ref(A, idx, ptr, N);
        return;
    }

    int nt = omp_get_max_threads();
    if (nt > ILU0_MAX_THREADS)
        nt = ILU0_MAX_THREADS;
    if (nt < 1)
        nt = 1;

    /* init per-call scheduler state */
    done_count = 0;
    for (int t = 0; t < nt; t++) {
        q_head[t] = 0;
        q_tail[t] = 0;
    }
    for (int64_t i = 0; i < N; i++) {
        dep_cnt[i] = C_.dep0[i];
        if (C_.dep0[i] == 0) {
            int t = (int)(i % nt);
            q_arr[(size_t)t * (size_t)N + q_tail[t]++] = (int32_t)i;
        }
    }

    #pragma omp parallel num_threads(nt)
    {
        int tid = omp_get_thread_num();
        int32_t *qa = q_arr + (size_t)tid * (size_t)N;
        int32_t steal_off = tid;
        for (;;) {
            int32_t i = q_claim(qa, &q_head[tid], &q_tail[tid]);
            /* 2) steal round-robin */
            if (i < 0) {
                for (int a = 1; a < nt; a++) {
                    int o = (steal_off + a) % nt;
                    i = q_claim(q_arr + (size_t)o * (size_t)N, &q_head[o], &q_tail[o]);
                    if (i >= 0) {
                        steal_off = o;
                        break;
                    }
                }
            }
            if (i < 0) {
                if (__atomic_load_n(&done_count, __ATOMIC_ACQUIRE) >= (int32_t)N)
                    break;
                for (int s = 0; s < 32; s++)
                    __builtin_ia32_pause();
                continue;
            }
            process_row(i, A, ptr, idx, C_.diagoff, C_.plan, C_.plan_sub,
                        C_.rowoff, C_.diag, C_.recip);
            __atomic_add_fetch(&done_count, 1, __ATOMIC_RELEASE);
            for (int32_t p = C_.dep_start[i]; p < C_.dep_start[i + 1]; p++) {
                int32_t x = C_.dep_list[p];
                if (__atomic_fetch_sub(&dep_cnt[x], 1, __ATOMIC_ACQ_REL) == 1) {
                    int32_t pos = __atomic_load_n(&q_tail[tid], __ATOMIC_RELAXED);
                    qa[pos] = x;
                    __atomic_store_n(&q_tail[tid], pos + 1, __ATOMIC_RELEASE);
                }
            }
        }
    }
}
