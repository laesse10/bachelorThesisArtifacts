#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* file-scope scheduler state (kernel is not re-entrant) */
static int32_t *s_front = NULL;
static int32_t *s_pend = NULL;      /* padded: 16 int32 (64 B) per row */
static uint32_t s_head = 0, s_tail = 0;
static volatile uint32_t *s_done = NULL;   /* per-thread done counters */
static volatile uint32_t s_stop = 0;       /* set by the last-retired watcher */
static volatile uint32_t s_retire = 0;     /* number of retired (idle) threads */

static inline int32_t *pend(int32_t r) { return s_pend + (size_t)r * 16; }

/* queue: claim slot via CAS first, store value after claim; consumer spins
   (acquire) until a published (>= 0) value appears. s_front holds the -1
   sentinel in unused slots, reset at the start of each phase. */
static inline void s_push(int32_t x) {
    for (;;) {
        uint32_t t = __atomic_load_n(&s_tail, __ATOMIC_RELAXED);
        if (__atomic_compare_exchange_n(&s_tail, &t, t + 1, 1,
                                        __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {
            s_front[t] = x;
            return;
        }
    }
}
static inline int32_t s_pop(void) {
    for (;;) {
        uint32_t h = __atomic_load_n(&s_head, __ATOMIC_ACQUIRE);
        uint32_t t = __atomic_load_n(&s_tail, __ATOMIC_ACQUIRE);
        if (h >= t) return -1;
        if (__atomic_compare_exchange_n(&s_head, &h, h + 1, 1,
                                        __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
            int32_t v;
            do {
                v = __atomic_load_n(&s_front[h], __ATOMIC_ACQUIRE);
                if (v < 0) __builtin_ia32_pause();
            } while (v < 0);
            return v;
        }
    }
}
static inline int s_all_done(int nt, uint32_t N) {
    uint32_t s = 0;
    for (int i = 0; i < nt; i++) s += s_done[i];
    return s == N;
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
    const int64_t N = EDGE * EDGE * EDGE;
    const int64_t nnzL = Lc_indptr[N];

    /* ---------------- sequential path for small N (reference order) ------------ */
    if (N <= 1024) {
        double *work;
        if (workspace && workspace_size >= N * (int64_t)sizeof(double))
            work = (double *)workspace;
        else
            work = (double *)malloc((size_t)N * sizeof(double));
        memset(work, 0, (size_t)N * sizeof(double));

        for (int64_t j = 0; j < N; j++) {
            for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
                int64_t row = A_indices[p];
                if (row >= j) work[row] = A_data[p];
            }
            for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
                int64_t k = L_indices[p];
                double ljk = Lc_data[L_to_Lc[p]];
                int64_t q = Lc_indptr[k] + 1;
                int64_t q1 = Lc_indptr[k + 1];
                for (; q < q1 && Lc_indices[q] < j; q++) {}
                for (; q < q1; q++) work[Lc_indices[q]] -= Lc_data[q] * ljk;
            }
            double diag = work[j];
            work[j] = 0.0;
            double ljj = sqrt(diag);
            Lc_data[Lc_indptr[j]] = ljj;
            for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++) {
                int64_t row = Lc_indices[p];
                Lc_data[p] = work[row] / ljj;
                work[row] = 0.0;
            }
        }
        for (int64_t j = 0; j < N; j++) y[j] = b[j];
        for (int64_t j = 0; j < N; j++) {
            double ljj = Lc_data[Lc_indptr[j]];
            double yj = y[j] / ljj;
            y[j] = yj;
            for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++)
                y[Lc_indices[p]] -= Lc_data[p] * yj;
        }
        for (int64_t jj = 0; jj < N; jj++) {
            int64_t col = N - 1 - jj;
            double s = y[col];
            for (int64_t p = Lc_indptr[col] + 1; p < Lc_indptr[col + 1]; p++)
                s -= Lc_data[p] * y[Lc_indices[p]];
            y[col] = s / Lc_data[Lc_indptr[col]];
        }
        if (workspace == NULL || workspace_size < N * (int64_t)sizeof(double)) free(work);
        return;
    }

    /* ---------------- parallel path -------------------------------------------- */
    const int T = omp_get_max_threads();

    int64_t maxc = 1;
    for (int64_t j = 0; j < N; j++) {
        int64_t c = Lc_indptr[j + 1] - Lc_indptr[j];
        if (c > maxc) maxc = c;
    }

    int64_t off = 0;
    int64_t o_rrow  = off; off += nnzL * 4;
    int64_t o_orig  = off; off += nnzL * 4;
    int64_t o_sval  = off; off += nnzL * 8;
    int64_t o_ip32  = off; off += (N + 1) * 4;
    int64_t o_kord  = off; off += nnzL * 4;
    int64_t o_ltord = off; off += nnzL * 4;
    int64_t o_pend  = off; off += (N + 1) * 64;
    int64_t o_front = off; off += N * 4;
    int64_t o_done  = off; off += T * 4;
    int64_t o_tmp   = off; off += (int64_t)T * maxc * 16;
    int64_t o_work  = off; off += (int64_t)T * N * 8;

    uint8_t *base;
    if (workspace && workspace_size >= off) base = workspace;
    else base = (uint8_t *)malloc((size_t)off);

    int32_t *rrow   = (int32_t *)(base + o_rrow);
    int32_t *sorig  = (int32_t *)(base + o_orig);
    double  *sval   = (double  *)(base + o_sval);
    int32_t *ip32   = (int32_t *)(base + o_ip32);
    int32_t *kord   = (int32_t *)(base + o_kord);
    int32_t *ltord  = (int32_t *)(base + o_ltord);
    s_pend          = (int32_t *)(base + o_pend);
    s_front         = (int32_t *)(base + o_front);
    s_done          = (volatile uint32_t *)(base + o_done);
    int32_t *tmp    = (int32_t *)(base + o_tmp);
    double  *work0  = (double  *)(base + o_work);
    memset(s_pend, 0, (size_t)(N + 1) * 64);

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        memset(work0 + (int64_t)tid * N, 0, (size_t)N * sizeof(double));

        /* phase 0: int32 column offsets; per-column row-sorted layout via
           multi-pass 6-bit radix; per-row k ordering. */
        #pragma omp for schedule(static)
        for (int64_t j = 0; j <= N; j++) ip32[j] = (int32_t)Lc_indptr[j];

        #pragma omp for schedule(static)
        for (int64_t j = 0; j < N; j++) {
            int32_t c0 = ip32[j], c1 = ip32[j + 1], c = c1 - c0;
            if (c <= 2) {
                for (int32_t t = 0; t < c; t++) {
                    rrow[c0 + t] = (int32_t)Lc_indices[c0 + t];
                    sorig[c0 + t] = c0 + t;
                }
                continue;
            }
            int32_t *base4 = tmp + (size_t)tid * 4 * maxc;
            int32_t *as = base4,        *ar = base4 + maxc;
            int32_t *bs = base4 + 2*maxc, *br = base4 + 3*maxc;
            for (int32_t t = 0; t < c; t++) {
                as[t] = c0 + t;
                ar[t] = (int32_t)Lc_indices[c0 + t];
            }
            int32_t *cs = as, *cr = ar, *ds = bs, *dr = br;
            int shift = 0;
            for (;;) {
                int32_t cnt[64];
                memset(cnt, 0, sizeof(cnt));
                for (int32_t t = 0; t < c; t++)
                    cnt[(cr[t] >> shift) & 63]++;
                int32_t pos[64];
                pos[0] = 0;
                for (int b = 1; b < 64; b++) pos[b] = pos[b - 1] + cnt[b - 1];
                for (int32_t t = 0; t < c; t++) {
                    int32_t d = (cr[t] >> shift) & 63;
                    int32_t s = pos[d]++;
                    ds[s] = cs[t];
                    dr[s] = cr[t];
                }
                if ((int64_t)N <= (1LL << (shift + 6))) {
                    for (int32_t t = 0; t < c; t++) {
                        rrow[c0 + t] = dr[t];
                        sorig[c0 + t] = ds[t];
                    }
                    break;
                }
                int32_t *x = cs; cs = ds; ds = x;
                x = cr; cr = dr; dr = x;
                shift += 6;
            }
        }

        #pragma omp for schedule(static)
        for (int64_t j = 0; j < N; j++) {
            int64_t p0 = L_indptr[j], p1 = L_indptr[j + 1] - 1;
            int64_t m = p1 - p0;
            if (m <= 256) {
                int32_t kk[256], ll[256];
                for (int64_t t = 0; t < m; t++) {
                    kk[t] = (int32_t)L_indices[p0 + t];
                    ll[t] = (int32_t)L_to_Lc[p0 + t];
                }
                for (int64_t t = 1; t < m; t++) {
                    int32_t ka = kk[t], lb = ll[t];
                    int64_t s = t;
                    while (s > 0 && ip32[kk[s - 1]] > ip32[ka]) {
                        kk[s] = kk[s - 1]; ll[s] = ll[s - 1]; s--;
                    }
                    kk[s] = ka; ll[s] = lb;
                }
                for (int64_t t = 0; t < m; t++) {
                    kord[p0 + t] = kk[t];
                    ltord[p0 + t] = ll[t];
                }
            } else {
                for (int64_t t = 0; t < m; t++) {
                    kord[p0 + t] = (int32_t)L_indices[p0 + t];
                    ltord[p0 + t] = (int32_t)L_to_Lc[p0 + t];
                }
            }
        }
        #pragma omp for schedule(static)
        for (int64_t j = 0; j < N; j++)
            *pend((int32_t)j) = (int32_t)(L_indptr[j + 1] - L_indptr[j]) - 1;

        /* ---- phase 1: fused factorization + forward solve, with eager chaining ---- */
        #pragma omp single
        {
            for (int i = 0; i < T; i++) s_done[i] = 0;
            s_stop = 0;
            s_retire = 0;
            memset(s_front, 0xFF, (size_t)N * 4);
            s_tail = 0;
            for (int64_t j = 0; j < N; j++) if (*pend((int32_t)j) == 0) s_front[s_tail++] = (int32_t)j;
            s_head = 0;
        }
        {
            double *w = work0 + (int64_t)tid * N;
            uint32_t local = 0;
            int ret = 0;
            int32_t stack[2048];
            int sp = 0;
            for (;;) {
                /* process the eager worklist */
                while (sp > 0) {
                    sp--;
                    int32_t j = stack[sp];
                    for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
                        int64_t row = A_indices[p];
                        if (row >= j) w[row] = A_data[p];
                    }
                    for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
                        int32_t k = kord[p];
                        double ljk = Lc_data[ltord[p]];
                        int32_t qe = ip32[k + 1], q0 = ip32[k];
                        for (int32_t q = qe - 1; q > q0; q--) {
                            int32_t row = rrow[q];
                            if (row < j) break;
                            w[row] -= sval[q] * ljk;
                        }
                    }
                    {
                        double diag = w[j];
                        w[j] = 0.0;
                        double ljj = sqrt(diag);
                        sval[ip32[j]] = ljj;
                        Lc_data[sorig[ip32[j]]] = ljj;
                        for (int32_t q = ip32[j] + 1; q < ip32[j + 1]; q++) {
                            int32_t row = rrow[q];
                            double v = w[row] / ljj;
                            sval[q] = v;
                            Lc_data[sorig[q]] = v;
                            w[row] = 0.0;
                        }
                        double s = b[j];
                        for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++)
                            s -= Lc_data[ltord[p]] * y[kord[p]];
                        y[j] = s / ljj;
                    }
                    for (int32_t q = ip32[j] + 1; q < ip32[j + 1]; q++) {
                        int32_t row = rrow[q];
                        if (__atomic_sub_fetch(pend(row), 1, __ATOMIC_RELEASE) == 0)
                            stack[sp++] = row;
                    }
                    local++;
                }
                /* queue */
                int32_t j = s_pop();
                if (j >= 0) {
                    if (ret) { ret = 0; __atomic_sub_fetch(&s_retire, 1, __ATOMIC_RELAXED); }
                    stack[sp++] = j;
                    continue;
                }
                if (s_stop) break;
                if (!ret) {
                    ret = 1;
                    s_done[tid] = local;
                    uint32_t r = __atomic_add_fetch(&s_retire, 1, __ATOMIC_RELEASE);
                    if (r == (uint32_t)T) {
                        for (;;) {
                            if (s_all_done(T, (uint32_t)N)) {
                                __atomic_store_n(&s_stop, 1, __ATOMIC_RELEASE);
                                break;
                            }
                            for (int i = 0; i < 256; i++) __builtin_ia32_pause();
                        }
                    }
                }
                for (int i = 0; i < 64; i++) __builtin_ia32_pause();
            }
            s_done[tid] = local;
        }

        /* ---- phase 2: back substitution (dependencies reversed) ---- */
        #pragma omp for schedule(static)
        for (int64_t j = 0; j < N; j++)
            *pend((int32_t)j) = (int32_t)(Lc_indptr[j + 1] - Lc_indptr[j]) - 1;
        #pragma omp single
        {
            for (int i = 0; i < T; i++) s_done[i] = 0;
            s_stop = 0;
            s_retire = 0;
            memset(s_front, 0xFF, (size_t)N * 4);
            s_tail = 0;
            for (int64_t j = 0; j < N; j++) if (*pend((int32_t)j) == 0) s_front[s_tail++] = (int32_t)j;
            s_head = 0;
        }
        {
            uint32_t local = 0;
            int ret = 0;
            int32_t stack[2048];
            int sp = 0;
            for (;;) {
                while (sp > 0) {
                    sp--;
                    int32_t j = stack[sp];
                    {
                        double s = y[j];
                        for (int32_t q = ip32[j] + 1; q < ip32[j + 1]; q++)
                            s -= sval[q] * y[rrow[q]];
                        y[j] = s / sval[ip32[j]];
                    }
                    for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
                        int32_t k = (int32_t)L_indices[p];
                        if (__atomic_sub_fetch(pend(k), 1, __ATOMIC_RELEASE) == 0)
                            stack[sp++] = k;
                    }
                    local++;
                }
                int32_t j = s_pop();
                if (j >= 0) {
                    if (ret) { ret = 0; __atomic_sub_fetch(&s_retire, 1, __ATOMIC_RELAXED); }
                    stack[sp++] = j;
                    continue;
                }
                if (s_stop) break;
                if (!ret) {
                    ret = 1;
                    s_done[tid] = local;
                    uint32_t r = __atomic_add_fetch(&s_retire, 1, __ATOMIC_RELEASE);
                    if (r == (uint32_t)T) {
                        for (;;) {
                            if (s_all_done(T, (uint32_t)N)) {
                                __atomic_store_n(&s_stop, 1, __ATOMIC_RELEASE);
                                break;
                            }
                            for (int i = 0; i < 256; i++) __builtin_ia32_pause();
                        }
                    }
                }
                for (int i = 0; i < 64; i++) __builtin_ia32_pause();
            }
            s_done[tid] = local;
        }
    }

    if (base != workspace) free(base);
}
