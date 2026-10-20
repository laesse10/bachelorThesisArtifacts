#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <sched.h>

/* ===================================================================== *
 * Up-looking sparse Cholesky over a precomputed symbolic pattern, plus
 * the two triangular solves.  See sparse_cholesky_numpy.py for the
 * reference semantics.
 *
 *  * The column pattern is rebuilt in a row-sorted layout (O(nnz), from
 *    the CSR form of L).  An update of column j from an earlier column k
 *    then starts exactly at the position of row j in column k -- the
 *    reference instead walks all of column k and skips the rows below j
 *    (about half of the inner iterations).  Inside one column every
 *    accumulator still receives the same contributions in the same order,
 *    so the factor matches the reference bit for bit on the serial path.
 *  * Parallelism follows the elimination DAG: columns in independent
 *    subtrees run as tasks, wide columns (separators) are computed by
 *    the whole team with per-thread accumulators and one reduction.
 *  * The thread count adapts to the problem size and to the number of
 *    usable cores, so small inputs take the serial path.
 * ===================================================================== */

/* ---------------- persistent scratch, grown on demand ---------------- */
static uint8_t *g_buf = NULL;
static int64_t  g_buf_cap = 0;
static int      g_buf_T = 0;

typedef struct {
    /* problem */
    const double   *A_data;
    const int64_t  *A_indices;
    const int64_t  *A_indptr;
    const int64_t  *L_indices;
    const int64_t  *L_indptr;
    const int64_t  *L_to_Lc;
    double         *Lc_data;
    const int64_t  *Lc_indices;
    const int64_t  *Lc_indptr;
    const double   *b;
    double         *y;
    int64_t N, nnz, T;

    /* sorted pattern */
    double  *vals;      /* nnz, factor values in sorted layout          */
    int32_t *srow;      /* nnz, sorted row indices                      */
    int32_t *inv;       /* nnz, original Lc position -> sorted pos      */
    int32_t *parent;    /* N, elimination-tree parent (-1 at roots)    */
    int32_t *pending;   /* N, unfinished etree children                 */
    uint8_t *isfat;     /* N, team-parallel column flag                */
    int32_t *ready;     /* N, ready stack                              */
    int32_t *fstack;    /* N, deferred fat columns                     */
    int32_t *cnt;       /* T*N, preprocessing histograms / cursors      */
    double **workT;     /* T arrays of N doubles                        */
    int32_t **kbuf;     /* T arrays of maxrow int32                     */
    double  *dpart;    /* T per-thread diagonal contributions          */
    double  *dslot;    /* A diagonal of the current fat column         */
    int64_t  fat_thresh;

    /* scheduler state */
    _Atomic int64_t rhead;
    _Atomic int64_t fhead;
    _Atomic int64_t remaining;
    _Atomic int32_t fat_j;
    _Atomic int32_t qlock;
} Ctx;

static inline void q_lock(_Atomic int32_t *lk)
{
    for (;;) {
        if (!__sync_lock_test_and_set(lk, 1)) return;
        while (__atomic_load_n(lk, __ATOMIC_RELAXED)) { }
    }
}
static inline void q_unlock(_Atomic int32_t *lk) { __sync_lock_release(lk); }

static inline void push_stack(Ctx *c, int32_t *stack, _Atomic int64_t *head, int32_t v)
{
    q_lock(&c->qlock);
    stack[*head] = v;
    (*head)++;
    q_unlock(&c->qlock);
}

static inline int64_t pop_stack(Ctx *c, int32_t *stack, _Atomic int64_t *head)
{
    q_lock(&c->qlock);
    int64_t h = *head;
    int64_t v = -1;
    if (h > 0) { v = stack[h - 1]; (*head) = h - 1; }
    q_unlock(&c->qlock);
    return v;
}

/* ---------------- one column, single thread ---------------- */
static void column_serial(Ctx *c, int64_t j, double *restrict w)
{
    const int64_t *restrict Lind = c->L_indices;
    const int64_t *restrict Lp = c->L_indptr;
    const int64_t *restrict L2 = c->L_to_Lc;
    const int64_t *restrict Cp = c->Lc_indptr;
    int32_t *restrict sr = c->srow;
    int32_t *restrict inv = c->inv;
    double *restrict v = c->vals;

    for (int64_t p = c->A_indptr[j]; p < c->A_indptr[j + 1]; p++) {
        int64_t row = c->A_indices[p];
        if (row >= j) w[row] = c->A_data[p];
    }
    int64_t pe = Lp[j + 1] - 1;
    for (int64_t p = Lp[j]; p < pe; p++) {
        int64_t k = Lind[p];
        int32_t q0 = inv[L2[p]];
        double ljk = v[q0];
        int32_t hi = (int32_t)Cp[k + 1];
        int32_t q = q0;
        for (; q + 4 <= hi; q += 4) {
            int64_t r0 = sr[q], r1 = sr[q+1], r2 = sr[q+2], r3 = sr[q+3];
            w[r0] -= v[q]   * ljk;
            w[r1] -= v[q+1] * ljk;
            w[r2] -= v[q+2] * ljk;
            w[r3] -= v[q+3] * ljk;
        }
        for (; q < hi; q++)
            w[sr[q]] -= v[q] * ljk;
    }
    double diag = w[j];
    w[j] = 0.0;
    double ljj = sqrt(diag);
    v[Cp[j]] = ljj;
    int32_t ce = (int32_t)Cp[j + 1];
    for (int32_t q = (int32_t)Cp[j] + 1; q < ce; q++) {
        v[q] = w[sr[q]] / ljj;
        w[sr[q]] = 0.0;
    }
}

/* ---------------- column completion bookkeeping ---------------- */
static void finish_column(Ctx *c, int64_t j)
{
    __atomic_fetch_sub(&c->remaining, 1, __ATOMIC_SEQ_CST);
    int64_t par = c->parent[j];
    if (par >= 0) {
        if (__atomic_fetch_sub(&c->pending[par], 1, __ATOMIC_SEQ_CST) == 1) {
            if (c->isfat[par]) {
                int32_t cur = -1;
                if (__sync_bool_compare_and_swap(&c->fat_j, cur, (int32_t)par))
                    return;
                push_stack(c, c->fstack, &c->fhead, (int32_t)par);
            } else {
                push_stack(c, c->ready, &c->rhead, (int32_t)par);
            }
        }
    }
}

/* ---------------- build the sorted column structure (serial) -------- */
static void build_sorted_serial(Ctx *c)
{
    const int64_t N = c->N;
    int32_t *restrict sr = c->srow;
    int32_t *restrict inv = c->inv;
    for (int64_t k = 0; k < N; k++) {
        int32_t q0 = (int32_t)c->Lc_indptr[k];
        sr[q0] = (int32_t)k;
        inv[q0] = q0;
    }
    int64_t *cur = (int64_t *)malloc((size_t)N * sizeof(int64_t));
    if (!cur) return;
    for (int64_t k = 0; k < N; k++) cur[k] = c->Lc_indptr[k] + 1;
    for (int64_t i = 0; i < N; i++) {
        int64_t pe = c->L_indptr[i + 1] - 1;
        for (int64_t p = c->L_indptr[i]; p < pe; p++) {
            int64_t k = c->L_indices[p];
            int32_t q = (int32_t)(cur[k]++);
            sr[q] = (int32_t)i;
            int64_t o = c->L_to_Lc[p];
            inv[o] = q;
        }
    }
    free(cur);
}

/* ===================================================================== */
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
    const int64_t N = EDGE * EDGE * EDGE;
    (void)workspace; (void)workspace_size;
    if (N <= 0) return;
    const int64_t nnz = Lc_indptr[N];
    if (nnz <= 0) { for (int64_t j = 0; j < N; j++) y[j] = b[j]; return; }

    Ctx c;
    c.A_data = A_data; c.A_indices = A_indices; c.A_indptr = A_indptr;
    c.L_indices = L_indices; c.L_indptr = L_indptr; c.L_to_Lc = L_to_Lc;
    c.Lc_data = Lc_data; c.Lc_indices = Lc_indices; c.Lc_indptr = Lc_indptr;
    c.b = b; c.y = y; c.N = N; c.nnz = nnz; c.T = 1;

    /* ---- how much parallelism does this problem justify? ---- */
    {
        int tmax = omp_get_max_threads();
        int nproc = omp_get_num_procs();
        if (nproc < 1) nproc = 1;
        if (tmax < 1) tmax = 1;
        if (tmax > nproc) tmax = nproc;
        /* useful inner iterations (upper bound via column counts) */
        int64_t work = 0;
        for (int64_t k = 0; k < N; k++) {
            int64_t cc = Lc_indptr[k + 1] - Lc_indptr[k];
            work += cc * cc;
        }
        /* each thread needs a large block of work to pay for the
         * cross-thread synchronisation; ~64M inner iterations is safe */
        int64_t want = work / (int64_t)(128 * 1024 * 1024);
        if (want < 1) want = 1;
        if ((int64_t)tmax > want) tmax = (int)want;
        if ((int64_t)tmax > N) tmax = (int)N;
        c.T = tmax;
    }
    const int T = (int)c.T;

    /* max row length of L (for kbuf) */
    int64_t maxrow = 1;
    for (int64_t i = 0; i < N; i++) {
        int64_t l = L_indptr[i + 1] - L_indptr[i];
        if (l > maxrow) maxrow = l;
    }

    /* ---------------- scratch layout ---------------- */
    int64_t need = 0;
    need += (nnz * 8 + 63) & ~(int64_t)63;              /* vals   */
    int64_t off_srow = need;  need += (nnz * 4 + 63) & ~(int64_t)63;
    int64_t off_inv  = need;  need += (nnz * 4 + 63) & ~(int64_t)63;
    int64_t off_par  = need;  need += (N * 4 + 63) & ~(int64_t)63;
    int64_t off_pend = need;  need += (N * 4 + 63) & ~(int64_t)63;
    int64_t off_fat  = need;  need += (N + 63) & ~(int64_t)63;
    int64_t off_rdy  = need;  need += (N * 4 + 63) & ~(int64_t)63;
    int64_t off_fst  = need;  need += (N * 4 + 63) & ~(int64_t)63;
    int64_t off_cnt  = need;  need += ((int64_t)T * N * 4 + 63) & ~(int64_t)63;
    int64_t off_wT   = need;  need += ((int64_t)T * N * 8 + 63) & ~(int64_t)63;
    int64_t off_kbuf = need;  need += ((int64_t)T * maxrow * 4 + 63) & ~(int64_t)63;
    int64_t off_dpart = need;  need += ((int64_t)T * 8 + 16 + 63) & ~(int64_t)63;
    need += 256;

    if (g_buf_cap < need || g_buf_T != T) {
        if (g_buf) { free(g_buf); g_buf = NULL; g_buf_cap = 0; g_buf_T = 0; }
        g_buf = (uint8_t *)malloc((size_t)need);
        if (!g_buf) return;
        g_buf_cap = need;
        g_buf_T = T;
    }

    c.vals  = (double *)(g_buf);
    c.srow  = (int32_t *)(g_buf + off_srow);
    c.inv   = (int32_t *)(g_buf + off_inv);
    c.parent= (int32_t *)(g_buf + off_par);
    c.pending = (int32_t *)(g_buf + off_pend);
    c.isfat = g_buf + off_fat;
    c.ready = (int32_t *)(g_buf + off_rdy);
    c.fstack= (int32_t *)(g_buf + off_fst);
    c.cnt   = (int32_t *)(g_buf + off_cnt);
    c.dpart = (double *)(g_buf + off_dpart);
    c.dslot = (double *)(g_buf + off_dpart) + T;
    c.fat_thresh = 2048 * (int64_t)T * (int64_t)T;
    static double *wptr[256];
    static int32_t *kptr[256];
    for (int t = 0; t < T; t++) {
        wptr[t] = (double *)(g_buf + off_wT) + (size_t)t * (size_t)N;
        kptr[t] = (int32_t *)(g_buf + off_kbuf) + (size_t)t * (size_t)maxrow;
    }
    c.workT = wptr; c.kbuf = kptr;

    __atomic_store_n(&c.rhead, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&c.fhead, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&c.remaining, N, __ATOMIC_SEQ_CST);
    __atomic_store_n(&c.fat_j, -1, __ATOMIC_SEQ_CST);
    __atomic_store_n(&c.qlock, 0, __ATOMIC_SEQ_CST);

    if (T <= 1) {
        /* =================== serial fast path =================== */
        double *restrict w = c.workT[0];
        build_sorted_serial(&c);
        for (int64_t i = 0; i < N; i++) w[i] = 0.0;
        for (int64_t j = 0; j < N; j++) column_serial(&c, j, w);
    } else {
        /* =================== parallel path ====================== */
        #pragma omp parallel num_threads(T)
        {
            const int tid = omp_get_thread_num();
            const int nthr = omp_get_num_threads();
            double *restrict w = c.workT[tid];
            const int64_t *restrict Lind = L_indices;
            const int64_t *restrict Lp = L_indptr;
            const int64_t *restrict L2 = L_to_Lc;
            const int64_t *restrict Cp = Lc_indptr;
            int32_t *restrict sr = c.srow;
            int32_t *restrict inv = c.inv;
            double *restrict vals = c.vals;

            /* ---- pass A: per-thread column histograms ---- */
            {
                int32_t *restrict my = c.cnt + (size_t)tid * (size_t)N;
                for (int64_t k = 0; k < N; k++) my[k] = 0;
                int64_t lo = N * tid / nthr, hi = N * (tid + 1) / nthr;
                for (int64_t i = lo; i < hi; i++) {
                    int64_t pe = Lp[i + 1] - 1;
                    for (int64_t p = Lp[i]; p < pe; p++) my[Lind[p]]++;
                }
            }
            #pragma omp barrier
            #pragma omp for
            for (int64_t k = 0; k < N; k++) {
                int32_t acc = (int32_t)Cp[k] + 1;
                for (int t = 0; t < nthr; t++) {
                    int32_t v = c.cnt[(size_t)t * (size_t)N + (size_t)k];
                    c.cnt[(size_t)t * (size_t)N + (size_t)k] = acc;
                    acc += v;
                }
            }
            #pragma omp barrier
            /* ---- pass B: scatter the sorted structure ---- */
            {
                int32_t *restrict my = c.cnt + (size_t)tid * (size_t)N;
                int64_t lo = N * tid / nthr, hi = N * (tid + 1) / nthr;
                for (int64_t i = lo; i < hi; i++) {
                    int64_t pe = Lp[i + 1] - 1;
                    int64_t wk = 0;
                    for (int64_t p = Lp[i]; p < pe; p++) {
                        int64_t k = Lind[p];
                        int32_t q = my[k]++;
                        sr[q] = (int32_t)i;
                        inv[L2[p]] = q;
                        wk += Cp[k + 1] - q;
                    }
                    c.isfat[i] = (uint8_t)(wk > c.fat_thresh);
                }
            }
            #pragma omp barrier
            /* ---- etree parents / children ---- */
            #pragma omp for schedule(static)
            for (int64_t k = 0; k < N; k++) {
                int32_t q0 = (int32_t)Cp[k];
                sr[q0] = (int32_t)k;
                inv[q0] = q0;
                c.parent[k] = (Cp[k + 1] - Cp[k] > 1) ? sr[q0 + 1] : -1;
                c.pending[k] = 0;
            }
            #pragma omp barrier
            #pragma omp for schedule(static)
            for (int64_t k = 0; k < N; k++) {
                int64_t par = c.parent[k];
                if (par >= 0) __atomic_fetch_add(&c.pending[par], 1, __ATOMIC_RELAXED);
            }
            #pragma omp barrier
            /* ---- each thread zeroes its OWN accumulator over the full range ---- */
            for (int64_t i = 0; i < N; i++) w[i] = 0.0;
            #pragma omp barrier
            if (tid == 0) {
                for (int64_t j = 0; j < N; j++)
                    if (c.pending[j] == 0) push_stack(&c, c.ready, &c.rhead, (int32_t)j);
            }
            #pragma omp barrier

            /* ================= task scheduler ================= */
            int yield_ctr = 0;
            for (;;) {
                if (__atomic_load_n(&c.remaining, __ATOMIC_SEQ_CST) <= 0) break;
                int64_t jf = __atomic_load_n(&c.fat_j, __ATOMIC_SEQ_CST);
                if (jf >= 0) {
                    /* ---------- team column: rank-1 updates ---------- */
                    int64_t p0 = Lp[jf], p1 = Lp[jf + 1] - 1;
                    int64_t nk = p1 - p0;
                    /* A scatter: rows > jf only.  The diagonal is kept in a
                     * shared slot; the workT[jf] accumulators are never
                     * touched, so no thread can zero another's input. */
                    if (tid == 0) {
                        double *restrict w0 = c.workT[0];
                        for (int64_t p = A_indptr[jf]; p < A_indptr[jf + 1]; p++) {
                            int64_t row = A_indices[p];
                            if (row == jf) c.dslot[0] = A_data[p];
                            else if (row > jf) w0[row] = A_data[p];
                        }
                    }
                    /* work-balanced split of the k's (each thread, redundantly) */
                    int32_t *restrict lens = c.kbuf[tid];
                    int64_t total = 0;
                    for (int64_t i = 0; i < nk; i++) {
                        int64_t p = p0 + i;
                        int32_t l = (int32_t)Cp[Lind[p] + 1] - inv[L2[p]];
                        lens[i] = l;
                        total += l;
                    }
                    int64_t start = total * tid / nthr;
                    int64_t stop  = total * (tid + 1) / nthr;
                    double dpart = 0.0;   /* this thread's diagonal contribution */
                    {
                        /* thread t owns exactly the k's whose cumulative
                         * end falls in (start, stop] -- a true partition of
                         * (0, total], so every k is applied once.  The first
                         * tail entry is always row jf itself; it goes into
                         * the register accumulator, not the workT array. */
                        int64_t a = 0, acc = 0;
                        while (a < nk && acc + lens[a] <= start) { acc += lens[a]; a++; }
                        int64_t b = a, accb = acc;
                        while (b < nk && accb + lens[b] <= stop) { accb += lens[b]; b++; }
                        for (int64_t i = a; i < b; i++) {
                            int64_t p = p0 + i;
                            int64_t k = Lind[p];
                            int32_t q0 = inv[L2[p]];
                            double ljk = vals[q0];
                            int32_t hi = (int32_t)Cp[k + 1];
                            dpart -= vals[q0] * ljk;
                            int32_t q = q0 + 1;
                            for (; q + 4 <= hi; q += 4) {
                                int64_t r0 = sr[q], r1 = sr[q+1], r2 = sr[q+2], r3 = sr[q+3];
                                w[r0] -= vals[q]   * ljk;
                                w[r1] -= vals[q+1] * ljk;
                                w[r2] -= vals[q+2] * ljk;
                                w[r3] -= vals[q+3] * ljk;
                            }
                            for (; q < hi; q++)
                                w[sr[q]] -= vals[q] * ljk;
                        }
                        c.dpart[tid] = dpart;
                    }
                    #pragma omp barrier
                    /* ---------- reduction over the accumulators ---------- */
                    {
                        int32_t q0 = (int32_t)Cp[jf];
                        int32_t q1 = (int32_t)Cp[jf + 1];
                        int64_t cc = q1 - q0 - 1;   /* off-diagonal entries */
                        double d = c.dslot[0];
                        for (int t = 0; t < nthr; t++) d += c.dpart[t];
                        double ljj = sqrt(d);
                        int32_t a = (int32_t)(q0 + 1 + cc * tid / nthr);
                        int32_t z = (int32_t)(q0 + 1 + cc * (tid + 1) / nthr);
                        if (tid == 0) vals[q0] = ljj;
                        for (int32_t q = a; q < z; q++) {
                            int64_t row = sr[q];
                            double dr = 0.0;
                            for (int t = 0; t < nthr; t++) dr += c.workT[t][row];
                            vals[q] = dr / ljj;
                            for (int t = 0; t < nthr; t++) c.workT[t][row] = 0.0;
                        }
                    }
                    #pragma omp barrier
                    if (tid == 0) {
                        __atomic_fetch_sub(&c.remaining, 1, __ATOMIC_SEQ_CST);
                        int32_t next = -1;
                        int64_t par = c.parent[jf];
                        if (par >= 0 &&
                            __atomic_fetch_sub(&c.pending[par], 1, __ATOMIC_SEQ_CST) == 1) {
                            if (c.isfat[par]) next = (int32_t)par;
                            else push_stack(&c, c.ready, &c.rhead, (int32_t)par);
                        }
                        if (next < 0) {
                            int64_t pf = pop_stack(&c, c.fstack, &c.fhead);
                            if (pf >= 0) next = (int32_t)pf;
                        }
                        __atomic_store_n(&c.fat_j, next, __ATOMIC_SEQ_CST);
                    }
                    #pragma omp barrier
                    continue;
                }
                /* pop a batch of independent ready columns columns (one stack
                 * head is checked read-only first so idle threads never
                 * touch the lock) */
                if (__atomic_load_n(&c.rhead, __ATOMIC_ACQUIRE) > 0) {
                    int64_t j = pop_stack(&c, c.ready, &c.rhead);
                    if (j >= 0) {
                        column_serial(&c, j, w);
                        finish_column(&c, j);
                        continue;
                    }
                }
                /* nothing ready: try to install a deferred fat column */
                if (__atomic_load_n(&c.fhead, __ATOMIC_ACQUIRE) > 0) {
                    int64_t pf = pop_stack(&c, c.fstack, &c.fhead);
                    if (pf >= 0) {
                        int32_t cur = -1;
                        if (__sync_bool_compare_and_swap(&c.fat_j, cur, (int32_t)pf))
                            continue;
                        push_stack(&c, c.fstack, &c.fhead, (int32_t)pf);
                    }
                }
                /* wait: spin a while, then yield (safe when oversubscribed) */
                if (++yield_ctr > 2048) {
                    yield_ctr = 0;
                    sched_yield();
                }
            }
            /* ================= end scheduler ================= */
        }
    }

    /* ---------------- copy values back to the original layout --------- */
    {
        const int32_t *restrict inv = c.inv;
        const double *restrict vals = c.vals;
        if (T > 1) {
            #pragma omp parallel for num_threads(T) schedule(static)
            for (int64_t p = 0; p < nnz; p++)
                Lc_data[p] = vals[inv[p]];
        } else {
            for (int64_t p = 0; p < nnz; p++)
                Lc_data[p] = vals[inv[p]];
        }
    }

    /* ---------------- triangular solves (reference order) -------------- */
    for (int64_t j = 0; j < N; j++) y[j] = b[j];
    for (int64_t j = 0; j < N; j++) {
        double ljj = Lc_data[Lc_indptr[j]];
        y[j] = y[j] / ljj;
        int64_t pe = Lc_indptr[j + 1];
        for (int64_t p = Lc_indptr[j] + 1; p < pe; p++) {
            int64_t row = Lc_indices[p];
            y[row] -= Lc_data[p] * y[j];
        }
    }
    for (int64_t jj = 0; jj < N; jj++) {
        int64_t col = N - 1 - jj;
        double ljj = Lc_data[Lc_indptr[col]];
        double s = y[col];
        int64_t pe = Lc_indptr[col + 1];
        for (int64_t p = Lc_indptr[col] + 1; p < pe; p++) {
            int64_t row = Lc_indices[p];
            s -= Lc_data[p] * y[row];
        }
        y[col] = s / ljj;
    }
}
