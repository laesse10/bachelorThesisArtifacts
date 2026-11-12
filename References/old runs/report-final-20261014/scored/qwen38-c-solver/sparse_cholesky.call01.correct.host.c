#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * Sparse Cholesky of the 7-point 3-D Poisson operator (RCB-ordered; the symbolic
 * pattern L/Lc is precomputed and handed in): numeric factorization + triangular
 * solve.
 *
 *  - FACTOR: up-looking by columns over the exact pattern.  Per-pattern work
 *    reorganizes each Lc column into row-sorted (row, q) pairs; 98.5 % of a
 *    column's rows form runs of consecutive indices, so the update
 *        w[row] -= Lc(row,k) * L(j,k)   for row >= j
 *    runs as a few short contiguous (vectorized) windows per predecessor, with
 *    values staged into a sorted per-call buffer.  Every w[row] is accumulated
 *    in ascending-k order, exactly like the reference.
 *  - The forward solve is fused into the same column pass (same dependency set).
 *  - BACKWARD SOLVE: plain descending-index pass (a valid topological order for
 *    the y[r] -> y[col] dependencies), each column's dot computed over the
 *    sorted-row runs.  Deliberately serial: the backward DAG is a near-chain,
 *    so a parallel queue only adds synchronization.
 *  - Factor columns are pulled from a lock-free FIFO as soon as all row
 *    predecessors are final (depcount up-counters); all per-pattern tables
 *    (successor edges, dep counts, sorted layout, runs) are memoized.
 */

static int64_t c_N = -1, c_nnz = -1;
static int64_t c_key[8];
static int32_t *dep0 = NULL;
static int32_t *succoff = NULL, *succ = NULL;
static int64_t succ_total = 0;
static int32_t *depA = NULL, *rq = NULL;
/* per-pattern, row-sorted Lc layout */
typedef struct { int32_t rs, rl, r0; } runt;
static int32_t *s_row = NULL, *s_q = NULL, *cnt = NULL;
static int32_t *runoff = NULL, *pos2run = NULL;
static runt *rt = NULL;
static int64_t runtot = 0;
/* per-call / per-thread */
static double *rinv = NULL, *workT = NULL, *s_val = NULL;
static int64_t capN = 0, capT = 0, capS = 0;

#define QFLAG 0x80000000

typedef struct { int32_t row, q; } pair_t;
static int cmp_pair(const void *a, const void *b)
{
    const pair_t *A = (const pair_t *)a, *B = (const pair_t *)b;
    return A->row > B->row ? 1 : (A->row < B->row ? -1 : 0);
}

static pair_t *sortbuf = NULL;
static size_t sortbufn = 0;

static void build_cache(const int64_t *Lc_indptr, const int64_t *Lc_indices,
                        const int64_t *L_indptr, const int64_t *L_indices,
                        const int64_t *L_to_Lc,
                        int64_t N)
{
    const int64_t nnz = Lc_indptr[N];
    if (c_N == N && c_nnz == nnz) {
        int ok = 1;
        for (int i = 0; i < 8; i++)
            if (c_key[i] != Lc_indices[i]) ok = 0;
        if (ok) return;
    }
    if (N > capN) {
        free(dep0); free(succoff); free(succ); free(depA); free(rq);
        free(rinv); free(s_row); free(s_q); free(cnt); free(runoff); free(pos2run); free(rt);
        capN = N;
        dep0    = (int32_t *)malloc((size_t)N * 4);
        succoff = (int32_t *)malloc((size_t)(N + 1) * 4);
        depA    = (int32_t *)malloc((size_t)N * 4);
        rq      = (int32_t *)malloc((size_t)N * 4);
        rinv    = (double   *)malloc((size_t)N * 8);
        s_row   = (int32_t *)malloc((size_t)nnz * 4);
        s_q     = (int32_t *)malloc((size_t)nnz * 4);
        cnt     = (int32_t *)malloc((size_t)nnz * 4);
        pos2run = (int32_t *)malloc((size_t)nnz * 4);
        runoff  = (int32_t *)malloc((size_t)(N + 1) * 4);
        rt      = (runt *)malloc((size_t)(nnz ? nnz : 1) * sizeof(runt));
    }
    memset(depA, 0, (size_t)N * 4); /* reuse as per-column successor counts */
    for (int64_t j = 0; j < N; j++)
        dep0[j] = (int32_t)(L_indptr[j + 1] - L_indptr[j] - 1);
    for (int64_t r = 0; r < N; r++)
        for (int64_t p = L_indptr[r]; p < L_indptr[r + 1] - 1; p++)
            depA[L_indices[p]]++;
    succ_total = 0;
    for (int64_t j = 0; j < N; j++) {
        succoff[j] = (int32_t)succ_total;
        succ_total += depA[j];
    }
    succoff[N] = (int32_t)succ_total;
    free(succ);
    succ = (int32_t *)malloc((size_t)(succ_total ? succ_total : 1) * 4);
    /* successor edges: j -> r (r > j) for each k = j in L row r */
    {
        int32_t *cur = (int32_t *)malloc((size_t)(N + 1) * 4);
        for (int64_t j = 0; j < N; j++) cur[j] = succoff[j];
        for (int64_t r = 0; r < N; r++)
            for (int64_t p = L_indptr[r]; p < L_indptr[r + 1] - 1; p++) {
                const int64_t k = L_indices[p];
                succ[cur[k]++] = (int32_t)r;
            }
        free(cur);
    }
    /* ---- row-sorted per-column layout ---- */
    if (nnz > capS) {
        free(s_val);
        capS = nnz;
        s_val = (double *)malloc((size_t)capS * 8);
    }
    {
        int32_t *inv = (int32_t *)malloc((size_t)nnz * 4);
        for (int64_t p = 0; p < nnz; p++) inv[L_to_Lc[p]] = (int32_t)p;

        runtot = 0;
        for (int64_t k = 0; k < N; k++) {
            const int64_t m = Lc_indptr[k + 1] - Lc_indptr[k];
            const int64_t base = Lc_indptr[k];
            if (m > 0) {
                if ((size_t)m > sortbufn) {
                    free(sortbuf);
                    sortbufn = (size_t)m;
                    sortbuf = (pair_t *)malloc(sortbufn * sizeof(pair_t));
                }
                for (int64_t s = 0; s < m; s++) {
                    sortbuf[s].row = (int32_t)Lc_indices[base + s];
                    sortbuf[s].q   = (int32_t)(base + s);
                }
                if (m < 64) {
                    for (int64_t i = 1; i < m; i++) {
                        pair_t x = sortbuf[i];
                        int64_t t = i - 1;
                        while (t >= 0 && sortbuf[t].row > x.row) {
                            sortbuf[t + 1] = sortbuf[t];
                            t--;
                        }
                        sortbuf[t + 1] = x;
                    }
                } else {
                    qsort(sortbuf, (size_t)m, sizeof(pair_t), cmp_pair);
                }
                for (int64_t s = 0; s < m; s++) {
                    s_row[base + s] = sortbuf[s].row;
                    s_q[base + s]   = sortbuf[s].q;
                }
            }
            runoff[k] = (int32_t)runtot;
            {
                int64_t s = 0;
                while (s < m) {
                    const int32_t r0 = s_row[base + s];
                    int64_t e = s + 1;
                    while (e < m && s_row[base + e] == r0 + (int32_t)(e - s)) e++;
                    runt R;
                    R.rs = (int32_t)(base + s);
                    R.rl = (int32_t)(e - s);
                    R.r0 = r0;
                    rt[runtot] = R;
                    const int32_t ri = (int32_t)runtot;
                    runtot++;
                    for (int64_t u = s; u < e; u++)
                        pos2run[base + u] = ri;
                    s = e;
                }
            }
            for (int64_t s = 0; s < m; s++)
                cnt[inv[s_q[base + s]]] = (int32_t)(base + s);
        }
        runoff[N] = (int32_t)runtot;
        free(inv);
    }
    c_N = N;
    c_nnz = nnz;
    for (int i = 0; i < 8; i++) c_key[i] = Lc_indices[i];
}

/* claim the next ready column, or -1 when the whole queue is drained. */
static int32_t pop_ready(int32_t *head, int32_t *tail, int32_t *done, int32_t Ntot)
{
    for (;;) {
        const int32_t h = __atomic_load_n(head, __ATOMIC_RELAXED);
        const int32_t t = __atomic_load_n(tail, __ATOMIC_ACQUIRE);
        if (h >= t) {
            if (__atomic_load_n(done, __ATOMIC_ACQUIRE) >= Ntot) return -1;
            __builtin_ia32_pause();
            continue;
        }
        const int32_t i = __atomic_fetch_add(head, 1, __ATOMIC_RELAXED);
        if (i >= Ntot) {
            /* over-claim: head raced past the last real slot (0..Ntot-1 are all
             * claimed by now), so every real column is claimed and done will
             * reach Ntot; there is nothing for this thread to process. */
            for (;;) {
                if (__atomic_load_n(done, __ATOMIC_ACQUIRE) >= Ntot) return -1;
                __builtin_ia32_pause();
            }
        }
        int32_t v;
        do {
            v = __atomic_load_n(&rq[i], __ATOMIC_ACQUIRE);
            if (v & QFLAG) break;
            __builtin_ia32_pause();
        } while (1);
        return (int32_t)((uint32_t)v & (uint32_t)~QFLAG);
    }
}

static void push_ready(int32_t *tail, int32_t col)
{
    const int32_t slot = __atomic_fetch_add(tail, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&rq[slot], (int32_t)((uint32_t)col | QFLAG), __ATOMIC_RELEASE);
}

static void run_queue_ab(const double *restrict A_data,
                         const int64_t *restrict A_indices,
                         const int64_t *restrict A_indptr,
                         const int64_t *restrict L_indices,
                         const int64_t *restrict L_indptr,
                         const int64_t *restrict Lc_indices,
                         const int64_t *restrict Lc_indptr,
                         double *restrict Lc_data,
                         const double *restrict b,
                         double *restrict y,
                         int64_t N)
{
    memset(depA, 0, (size_t)N * 4);
    memset(rq, 0, (size_t)N * 4);
    int32_t head = 0, tail = 0, done = 0;
    for (int64_t j = 0; j < N; j++)
        if (dep0[j] == 0) push_ready(&tail, (int32_t)j);

#pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *restrict w = workT + (size_t)tid * (size_t)N;
        for (;;) {
            const int32_t jj = pop_ready(&head, &tail, &done, (int32_t)N);
            if (jj < 0) break;
            const int64_t j = jj;
            /* gather A's own structure */
            for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
                const int64_t r = A_indices[p];
                if (r >= j) w[r] = A_data[p];
            }
            /* up-looking: for each pred k, apply column k's rows >= j (sorted,
             * as contiguous runs) */
            for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
                const int32_t s0 = cnt[p];
                const int64_t se = Lc_indptr[L_indices[p] + 1];
                const double ljk = s_val[s0];
                int32_t ri = pos2run[s0];
                const runt R0 = rt[ri];
                if (s0 > R0.rs) {
                    const int64_t e = (int64_t)R0.rs + R0.rl;
                    const int64_t ee = e < se ? e : se;
                    for (int64_t s = s0; s < ee; s++)
                        w[s_row[s]] -= s_val[s] * ljk;
                    if (ee >= se) continue;
                    ri++;
                }
                const int32_t kend = runoff[L_indices[p] + 1];
                for (; ri < kend; ri++) {
                    const runt R = rt[ri];
                    if ((int64_t)R.rs >= se) break;
                    double *restrict wp = w + R.r0;
                    const double *restrict vp = s_val + R.rs;
                    const int64_t n = R.rl;
                    for (int64_t i = 0; i < n; i++)
                        wp[i] -= vp[i] * ljk;
                }
            }
            const double ljj = sqrt(w[j]);
            const double rj = 1.0 / ljj;
            Lc_data[Lc_indptr[j]] = ljj;
            rinv[j] = rj;
            for (int64_t q = Lc_indptr[j] + 1; q < Lc_indptr[j + 1]; q++) {
                const int64_t r = Lc_indices[q];
                Lc_data[q] = w[r] * rj;
                w[r] = 0.0;
            }
            /* stage the sorted values (this column is now final) */
            for (int64_t s = Lc_indptr[j]; s < Lc_indptr[j + 1]; s++)
                s_val[s] = Lc_data[s_q[s]];
            /* forward solve row j */
            double s = b[j];
            for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++)
                s -= s_val[cnt[p]] * y[L_indices[p]];
            y[j] = s * rj;
            /* release successors */
            for (int64_t i = succoff[j]; i < succoff[j + 1]; i++) {
                const int32_t r = succ[i];
                if (__atomic_fetch_add(&depA[r], 1, __ATOMIC_RELAXED) == dep0[r] - 1)
                    push_ready(&tail, r);
            }
            __atomic_fetch_add(&done, 1, __ATOMIC_RELEASE);
        }
    }
}

/* backward solve, serial descending-index pass over the sorted-row runs */
static void run_bwd(const int64_t *restrict Lc_indptr,
                    double *restrict y,
                    int64_t N)
{
    for (int64_t col = N - 1; col >= 0; col--) {
        double s = y[col];
        const int64_t base = Lc_indptr[col];
        int32_t ri = runoff[col];
        const int32_t kend = runoff[col + 1];
        for (; ri < kend; ri++) {
            const runt R = rt[ri];
            int64_t rs = R.rs;
            const int64_t e = (int64_t)R.rs + R.rl;
            if (rs == base) rs = base + 1; /* skip the diagonal */
            if (rs >= e) continue;
            double *restrict yy = y + R.r0 + (rs - R.rs);
            const double *restrict vp = s_val + rs;
            const int64_t n = e - rs;
            double d = 0.0;
            for (int64_t i = 0; i < n; i++)
                d += vp[i] * yy[i];
            s -= d;
        }
        y[col] = s * rinv[col];
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
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const int64_t N = EDGE * EDGE * EDGE;

    build_cache(Lc_indptr, Lc_indices, L_indptr, L_indices, L_to_Lc, N);

    const int T = omp_get_max_threads();
    if ((int64_t)T * N > capT) {
        free(workT);
        capT = (int64_t)T * N;
        workT = (double *)malloc((size_t)capT * 8);
    }

    /* zero the per-thread scratch once, in parallel */
#pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *restrict w = workT + (size_t)tid * (size_t)N;
        memset(w, 0, (size_t)N * 8);
    }

    run_queue_ab(A_data, A_indices, A_indptr, L_indices, L_indptr,
                 Lc_indices, Lc_indptr, Lc_data, b, y, N);
    run_bwd(Lc_indptr, y, N);
}
