/* v4: stencil wavefront SGS-PCG, custom spin barrier, order-agnostic, bitwise-exact */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <omp.h>

#define SGS_SPBLK 512

#if defined(__x86_64__) || defined(__i386__)
#define SGS_RELAX() __builtin_ia32_pause()
#else
#define SGS_RELAX() do {} while (0)
#endif

/* sense-reversing spin barrier */
typedef struct {
    volatile int64_t cnt __attribute__((aligned(64)));
    volatile int64_t sense __attribute__((aligned(64)));
} sgs_bar_t;

static inline void sgs_bar(sgs_bar_t *bar, int T)
{
    int64_t s = __atomic_load_n(&bar->sense, __ATOMIC_RELAXED);
    int64_t c = __atomic_add_fetch(&bar->cnt, 1, __ATOMIC_ACQ_REL);
    if (c == T) {
        __atomic_store_n(&bar->cnt, 0, __ATOMIC_RELAXED);
        __atomic_store_n(&bar->sense, 1 - s, __ATOMIC_RELEASE);
    } else {
        while (__atomic_load_n(&bar->sense, __ATOMIC_ACQUIRE) == s)
            SGS_RELAX();
    }
}

/* ================= general-CSR fallback ================= */

static void sgs_apply_seq(const double *restrict A_data, const int *restrict A_idx,
                          const int *restrict A_ptr, const double *restrict dg,
                          const double *restrict r, double *restrict y,
                          double *restrict z, int N)
{
    for (int i = 0; i < N; i++) {
        double s = r[i];
        for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++) {
            int j = A_idx[k];
            if (j < i) s = s - A_data[k] * y[j];
        }
        y[i] = s / dg[i];
    }
    for (int ii = 0; ii < N; ii++) {
        int row = N - 1 - ii;
        double s = dg[row] * y[row];
        for (int k = A_ptr[row]; k < A_ptr[row + 1]; k++) {
            int j = A_idx[k];
            if (j > row) s = s - A_data[k] * z[j];
        }
        z[row] = s / dg[row];
    }
}

static void pcg_csr(const double *restrict A_data, const int64_t *restrict A_indices,
                   const int64_t *restrict A_indptr, const double *restrict b,
                   double *restrict x, int N, long niter)
{
    double *r  = (double *)malloc(sizeof(double) * (size_t)N);
    double *y  = (double *)malloc(sizeof(double) * (size_t)N);
    double *z  = (double *)malloc(sizeof(double) * (size_t)N);
    double *p  = (double *)malloc(sizeof(double) * (size_t)N);
    double *q  = (double *)malloc(sizeof(double) * (size_t)N);
    double *dg = (double *)malloc(sizeof(double) * (size_t)N);
    int *A_idx = (int *)malloc(sizeof(int) * (size_t)A_indptr[N]);
    int *A_ptr = (int *)malloc(sizeof(int) * (size_t)(N + 1));
    if (!r || !y || !z || !p || !q || !dg || !A_idx || !A_ptr) {
        free(r); free(y); free(z); free(p); free(q); free(dg); free(A_idx); free(A_ptr);
        return;
    }
    {
        int64_t nnz = A_indptr[N];
        #pragma omp parallel for schedule(static)
        for (int64_t k = 0; k < nnz; k++) A_idx[k] = (int)A_indices[k];
        #pragma omp parallel for schedule(static)
        for (int i = 0; i <= N; i++) A_ptr[i] = (int)A_indptr[i];
    }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) { x[i] = 0.0; r[i] = b[i]; }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        double d = 0.0;
        for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++)
            if (A_idx[k] == i) d = A_data[k];
        dg[i] = d;
    }
    sgs_apply_seq(A_data, A_idx, A_ptr, dg, r, y, z, N);
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) p[i] = z[i];
    double rz = 0.0;
    for (int i = 0; i < N; i++) rz += r[i] * z[i];
    for (long it = 0; it < niter; it++) {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            double acc = 0.0;
            for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++)
                acc = acc + A_data[k] * p[A_idx[k]];
            q[i] = acc;
        }
        double pq = 0.0;
        for (int i = 0; i < N; i++) pq += p[i] * q[i];
        double alpha = rz / pq;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            x[i] = x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }
        sgs_apply_seq(A_data, A_idx, A_ptr, dg, r, y, z, N);
        double rznew = 0.0;
        for (int i = 0; i < N; i++) rznew += r[i] * z[i];
        double beta = rznew / rz;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) p[i] = z[i] + beta * p[i];
        rz = rznew;
    }
    free(r); free(y); free(z); free(p); free(q); free(dg); free(A_idx); free(A_ptr);
}

/* ================= stencil path ================= */

/* m forward lines interleaved. Line L's 27 planes are contiguous at fv + L*27a.
   Per-row op order unchanged -> bitwise identical. Consecutive k at one level:
   ib += 2, ic -= 1, so L(t) = L0 + t*lstride with lstride = 2 - b. */
static void fwd_lines_m(const double *restrict fv, const double *restrict r,
                        double *restrict y, double *restrict far,
                        int L0, int lstride, int m, int a,
                        const int *restrict off)
{
    int L[4];
    double *f[4];
    const size_t A = (size_t)a;
    const size_t S27 = 27 * A;
    for (int t = 0; t < m; t++) {
        L[t] = L0 + t * lstride;
        f[t] = far + (size_t)t * a;
    }
    /* far parts: full line each, one linear stream per line */
    for (int t = 0; t < m; t++) {
        const double *src = fv + (size_t)L[t] * S27;
        double *ft = f[t];
        const int bt = L[t] * a;
        for (int j = 0; j < a; j++) ft[j] = r[bt + j];
        for (int p = 0; p < 12; p++) {
            const double *vp = src + (size_t)p * A;
            const double *ys = y + bt + off[p];
            for (int j = 0; j < a; j++)
                ft[j] = ft[j] - vp[j] * ys[j];
        }
    }
    /* chains, interleaved across lines */
    for (int j = 0; j < a; j++) {
        for (int t = 0; t < m; t++) {
            int i = L[t] * a + j;
            const double *src = fv + (size_t)L[t] * S27;
            double yv = (f[t][j] - src[(size_t)12 * A + j] * y[i - 1]) / src[(size_t)13 * A + j];
            y[i] = yv;
        }
    }
}

/* m backward lines interleaved (bitwise identical per-row order) */
static void bwd_lines_m(const double *restrict fv, const double *restrict y,
                        double *restrict z, int L0, int lstride, int m, int a,
                        const int *restrict off)
{
    int L[4];
    const size_t A = (size_t)a;
    const size_t S27 = 27 * A;
    for (int t = 0; t < m; t++)
        L[t] = L0 + t * lstride;
    if (a > 256) {
        /* u[j] = diag[i]*y[i] precomputed as separate rounded muls (reference rounding) */
        for (int t = 0; t < m; t++) {
            const double *src = fv + (size_t)L[t] * S27;
            const int bt = L[t] * a;
            for (int j = 0; j < a; j++)
                z[bt + j] = src[(size_t)13 * A + j] * y[bt + j];
        }
    }
    if (a <= 256) {
        /* stage fv planes 13..26 of the m lines into a local buffer with
           vectorized copies (pure data movement: loads do not round). The
           chain then reads L1/L2 with the exact same ops, order and fusions. */
        double fb[4][14 * 256];
        for (int t = 0; t < m; t++) {
            const double *s0 = fv + (size_t)L[t] * S27 + (size_t)13 * A;
            double *d = fb[t];
            const int cnt = 14 * a;
            for (int jj = 0; jj < cnt; jj++)
                d[jj] = s0[jj];
        }
        /* u pass (separate rounded muls), then interleaved descending chains */
        for (int t = 0; t < m; t++) {
            const int bt = L[t] * a;
            const double *d = fb[t];
            for (int j = 0; j < a; j++)
                z[bt + j] = d[j] * y[bt + j];
        }
        for (int j = a - 1; j >= 0; j--) {
            for (int t = 0; t < m; t++) {
                int i = L[t] * a + j;
                const double *d = fb[t];
                double s = z[i];
                s = s - d[a + j] * z[i + 1];
                for (int p = 0; p < 12; p++)
                    s = s - d[(2 + p) * a + j] * z[i + off[p + 15]];
                z[i] = s / d[j];
            }
        }
        return;
    }
    /* interleaved descending chains */
    for (int j = a - 1; j >= 0; j--) {
        for (int t = 0; t < m; t++) {
            int i = L[t] * a + j;
            const double *src = fv + (size_t)L[t] * S27 + (size_t)14 * A;
            double s = z[i];
            s = s - src[j] * z[i + 1];
            for (int p = 0; p < 12; p++)
                s = s - src[(size_t)(p + 1) * A + j] * z[i + off[p + 15]];
            z[i] = s / fv[(size_t)L[t] * S27 + (size_t)13 * A + j];
        }
    }
}

/* ---- asynchronous wavefront sweeps: per-line epoch flags, no global barrier ----
   Line (ib,ic) has index L = ib + ic*b. Forward far part reads lines
   (ib-1,ic), (ib-1,ic-1), (ib,ic-1), (ib+1,ic-1); backward far part reads
   (ib+1,ic), (ib-1,ic+1), (ib,ic+1), (ib+1,ic+1) plus own line's forward y. */

static inline void spin_until(const volatile uint32_t *flag, uint32_t epoch)
{
    while (__atomic_load_n(flag, __ATOMIC_ACQUIRE) != epoch)
        SGS_RELAX();
}

static void fwd_sweep(const double *restrict fv, const double *restrict r,
                      double *restrict y, double *restrict far,
                      int a, int b, int c, const int *restrict off,
                      uint32_t epoch,
                      const int *restrict mylist, int mycnt,
                      volatile uint32_t *restrict fflag)
{
    (void)c;
    int idx = 0;
    while (idx < mycnt) {
        int L0 = mylist[idx];
        int m = 1;
        while (m < 4 && idx + m < mycnt) {
            int Ln = mylist[idx + m], Lp = mylist[idx + m - 1];
            if (Ln != Lp + 2) break;
            if (2 * (Ln / b) + Ln % b != 2 * (Lp / b) + Lp % b) break;
            m++;
        }
        for (int t = 0; t < m; t++) {
            int L = L0 + 2 * t;
            int ib = L % b, ic = L / b;
            if (ib > 0)            spin_until(&fflag[L - 1], epoch);
            if (ic > 0) {
                spin_until(&fflag[L - b], epoch);
                if (ib > 0)        spin_until(&fflag[L - b - 1], epoch);
                if (ib < b - 1)    spin_until(&fflag[L - b + 1], epoch);
            }
        }
        fwd_lines_m(fv, r, y, far, L0, 2, m, a, off);
        for (int t = 0; t < m; t++)
            __atomic_store_n(&fflag[L0 + 2 * t], epoch, __ATOMIC_RELEASE);
        idx += m;
    }
}

static void bwd_sweep(const double *restrict fv, const double *restrict y,
                      double *restrict z,
                      int a, int b, int c, const int *restrict off,
                      uint32_t fepoch, uint32_t bepoch,
                      const int *restrict mylist, int mycnt,
                      volatile uint32_t *restrict fflag,
                      volatile uint32_t *restrict bflag)
{
    (void)c;
    int idx = mycnt - 1;
    while (idx >= 0) {
        int L0 = mylist[idx];
        int m = 1;
        while (m < 4 && idx - m >= 0) {
            int Ln = mylist[idx - m], Lp = mylist[idx - m + 1];
            if (Ln != Lp - 2) break;
            if (2 * (Ln / b) + Ln % b != 2 * (Lp / b) + Lp % b) break;
            m++;
        }
        for (int t = 0; t < m; t++) {
            int L = L0 - 2 * t;
            int ib = L % b, ic = L / b;
            spin_until(&fflag[L], fepoch);            /* own line's y */
            if (ib < b - 1)       spin_until(&bflag[L + 1], bepoch);
            if (ic < c - 1) {
                spin_until(&bflag[L + b], bepoch);
                if (ib > 0)        spin_until(&bflag[L + b - 1], bepoch);
                if (ib < b - 1)    spin_until(&bflag[L + b + 1], bepoch);
            }
        }
        bwd_lines_m(fv, y, z, L0 - 2 * (m - 1), 2, m, a, off);
        for (int t = 0; t < m; t++)
            __atomic_store_n(&bflag[L0 - 2 * t], bepoch, __ATOMIC_RELEASE);
        idx -= m;
    }
}

/* quick structural check of sampled rows for permutation (a,b,c) */
static int perm_check_rows(const int64_t *restrict A_indices, const int64_t *restrict A_indptr,
                           int N, int a, int b, int c, const int8_t *restrict tbl, int g)
{
    (void)b; (void)c;
    for (int s = 0; s < 40; s++) {
        int i = (int)(((int64_t)s * N) / 40);
        if (i >= N) i = N - 1;
        int ia = i % a, ib = (i / a) % b, ic = i / (a * b);
        int64_t jprev = -1;
        for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
            int64_t j = A_indices[k];
            if (j <= jprev) return 0;
            jprev = j;
            int64_t d = j - i;
            if (d < -g || d > g) return 0;
            int p = tbl[d + g];
            if (p < 0) return 0;
            int ja = j % a, jb = (j / a) % b, jc = j / (a * b);
            if (ia - ja > 1 || ja - ia > 1) return 0;
            if (ib - jb > 1 || jb - ib > 1) return 0;
            if (ic - jc > 1 || jc - ic > 1) return 0;
        }
    }
    return 1;
}

void sgs_pcg_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const double *restrict b,
    double *restrict x,
    const int64_t NX_, const int64_t NY_, const int64_t NZ_,
    const int64_t niter_,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const int dims[3] = { (int)NX_, (int)NY_, (int)NZ_ };
    const int N = dims[0] * dims[1] * dims[2];
    const long niter = (long)niter_;
    if (N <= 0) return;

    if (dims[0] < 3 || dims[1] < 3 || dims[2] < 3) {
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }

    int adim = 0, bdim = 0, cdim = 0;   /* chosen permutation: innermost..outermost */
    int off[27], dxa[27], dya[27], dza[27];
    int8_t *tbl = NULL;

    static const int perms[6][3] = { {0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0} };
    for (int pi = 0; pi < 6 && adim == 0; pi++) {
        int A = dims[perms[pi][0]], B = dims[perms[pi][1]], C = dims[perms[pi][2]];
        int g = A * B + A + 1;
        int8_t *t = (int8_t *)malloc((size_t)(2 * g + 1));
        if (!t) continue;
        memset(t, -1, (size_t)(2 * g + 1));
        int eo[27], ex[27], ey[27], ez[27];
        int cnt = 0;
        for (int dz = -1; dz <= 1; dz++)
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++) {
                    int o = dx + dy * A + dz * A * B;
                    if (o < -g || o > g) continue;
                    eo[cnt] = o; ex[cnt] = dx; ey[cnt] = dy; ez[cnt] = dz; cnt++;
                }
        for (int p = 1; p < cnt; p++) {  /* insertion sort ascending */
            int o = eo[p], ax = ex[p], ay = ey[p], az = ez[p], q = p - 1;
            while (q >= 0 && eo[q] > o) {
                eo[q + 1] = eo[q]; ex[q + 1] = ex[q]; ey[q + 1] = ey[q]; ez[q + 1] = ez[q];
                q--;
            }
            eo[q + 1] = o; ex[q + 1] = ax; ey[q + 1] = ay; ez[q + 1] = az;
        }
        for (int p = 0; p < cnt; p++) t[eo[p] + g] = (int8_t)p;
        if (perm_check_rows(A_indices, A_indptr, N, A, B, C, t, g)) {
            adim = A; bdim = B; cdim = C;
            for (int p = 0; p < cnt; p++) {
                off[p] = eo[p]; dxa[p] = ex[p]; dya[p] = ey[p]; dza[p] = ez[p];
            }
            tbl = t;
        } else {
            free(t);
        }
    }

    if (adim == 0) {  /* no stencil ordering found */
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }

    const int g = adim * bdim + adim + 1;
    double *fv = (double *)malloc(sizeof(double) * (size_t)27 * N);
    if (!fv) { free(tbl); pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter); return; }
    memset(fv, 0, sizeof(double) * (size_t)27 * N);

    volatile int ok = 1;
    #pragma omp parallel
    {
        int lok = 1;
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) {
            int ia = i % adim, ib = (i / adim) % bdim, ic = i / (adim * bdim);
            int L = ib + ic * bdim, j0 = ia;
            uint32_t mask = 0;
            int64_t jprev = -1;
            for (int64_t k = A_indptr[i]; k < A_indptr[i + 1]; k++) {
                int64_t j = A_indices[k];
                if (j <= jprev) { lok = 0; jprev = j; continue; }
                jprev = j;
                int64_t d = j - i;
                if (d < -g || d > g) { lok = 0; continue; }
                int p = tbl[d + g];
                if (p < 0) { lok = 0; continue; }
                if (ia + dxa[p] < 0 || ia + dxa[p] >= adim ||
                    ib + dya[p] < 0 || ib + dya[p] >= bdim ||
                    ic + dza[p] < 0 || ic + dza[p] >= cdim) { lok = 0; continue; }
                if (mask & (1u << p)) { lok = 0; continue; }
                mask |= (1u << p);
                fv[(size_t)L * (27 * adim) + (size_t)p * adim + j0] = A_data[k];
            }
        }
        if (!lok) ok = 0;
    }
    free(tbl);

    if (!ok) {
        free(fv);
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }

    /* guarded work arrays: valid indices [-g, N+g) */
    const size_t asz = (size_t)N + 2 * (size_t)g;
    double *r_raw = (double *)malloc(sizeof(double) * asz);
    double *y_raw = (double *)malloc(sizeof(double) * asz);
    double *z_raw = (double *)malloc(sizeof(double) * asz);
    double *p_raw = (double *)malloc(sizeof(double) * asz);
    double *q_arr = (double *)malloc(sizeof(double) * (size_t)N);
    sgs_bar_t *bar = (sgs_bar_t *)malloc(sizeof(sgs_bar_t));
    if (!r_raw || !y_raw || !z_raw || !p_raw || !q_arr || !bar) {
        free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(bar); free(fv);
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }
    memset(y_raw, 0, sizeof(double) * asz);
    memset(z_raw, 0, sizeof(double) * asz);
    memset(p_raw, 0, sizeof(double) * asz);
    memset(bar, 0, sizeof(sgs_bar_t));
    double *r = r_raw + g, *y = y_raw + g, *z = z_raw + g, *p = p_raw + g;

    /* per-thread line lists (wavefront order) + dependency flags */
    const int NL = bdim * cdim;
    int *lint = (int *)malloc(sizeof(int) * (size_t)NL);
    int *lofs = (int *)calloc(257, sizeof(int));
    volatile uint32_t *fflag = (volatile uint32_t *)calloc((size_t)NL, sizeof(uint32_t));
    volatile uint32_t *bflag = (volatile uint32_t *)calloc((size_t)NL, sizeof(uint32_t));
    if (!lint || !lofs || !fflag || !bflag) {
        free(lint); free(lofs); free((void *)fflag); free((void *)bflag);
        free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(bar); free(fv);
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }
    (void)NL;

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int T = omp_get_num_threads();
        double *far = (double *)malloc(sizeof(double) * (size_t)(4 * adim) + 64);
        const int64_t i0 = (int64_t)tid * N / T, i1 = (int64_t)(tid + 1) * N / T;
        const int NLINES = bdim * cdim;
        const int Ll0 = (int)((int64_t)tid * NLINES / T), Ll1 = (int)((int64_t)(tid + 1) * NLINES / T);
        const int *mylist = lint;
        int mycnt = 0;
        volatile uint32_t *myff = fflag, *mybf = bflag;

        /* thread 0 builds the per-thread line lists (wavefront order, level-major) */
        if (tid == 0) {
            int cnt[256];
            for (int t = 0; t < 256; t++) cnt[t] = 0;
            const int LMAX = 2 * (cdim - 1) + (bdim - 1);
            for (int l = 0; l <= LMAX; l++) {
                int lo = l - 2 * (cdim - 1); if (lo < 0) lo = 0;
                int hi = l; if (hi > bdim - 1) hi = bdim - 1;
                if (((l - lo) & 1) != 0) lo++;
                int K = (hi >= lo) ? ((hi - lo) / 2 + 1) : 0;
                for (int k = 0; k < K; k++) {
                    int th = (int)((int64_t)k * T / K);
                    if (th >= T) th = T - 1;
                    if (th < 0) th = 0;
                    cnt[th]++;
                }
            }
            int run = 0;
            for (int t = 0; t < 256; t++) { lofs[t] = run; run += cnt[t]; }
            lofs[256] = run;
            int cur[256];
            for (int t = 0; t < 256; t++) cur[t] = lofs[t];
            for (int l = 0; l <= LMAX; l++) {
                int lo = l - 2 * (cdim - 1); if (lo < 0) lo = 0;
                int hi = l; if (hi > bdim - 1) hi = bdim - 1;
                if (((l - lo) & 1) != 0) lo++;
                int K = (hi >= lo) ? ((hi - lo) / 2 + 1) : 0;
                for (int k = 0; k < K; k++) {
                    int ib = lo + 2 * k;
                    int ic = (l - ib) >> 1;
                    int th = (int)((int64_t)k * T / K);
                    if (th >= T) th = T - 1;
                    if (th < 0) th = 0;
                    lint[cur[th]++] = ib + ic * bdim;
                }
            }
        }

        /* x0 = 0, r0 = b */
        for (int64_t i = i0; i < i1; i++) { x[i] = 0.0; r[i] = b[i]; }
        sgs_bar(bar, T);

        int tt = tid < 256 ? tid : 255;
        mylist = lint + lofs[tt];
        mycnt = lofs[tt + 1] - lofs[tt];
        if (mycnt < 0) mycnt = 0;
        if (tid >= 256 || tt >= T) mycnt = 0;

        fwd_sweep(fv, r, y, far, adim, bdim, cdim, off, (uint32_t)1,
                  mylist, mycnt, myff);
        bwd_sweep(fv, y, z, adim, bdim, cdim, off, (uint32_t)1, (uint32_t)2,
                  mylist, mycnt, myff, mybf);
        sgs_bar(bar, T);   /* all async sweep lines complete */

        for (int64_t i = i0; i < i1; i++) p[i] = z[i];
        sgs_bar(bar, T);

        double rz = 0.0;
        for (int i = 0; i < N; i++) rz += r[i] * z[i];

        for (long it = 0; it < niter; it++) {
            /* q = A p (line-chunk partition; mul then add, matching reference rounding) */
            for (int L = Ll0; L < Ll1; L++) {
                const int b0 = L * adim;
                double acc[SGS_SPBLK] __attribute__((aligned(64)));
                double tmp[SGS_SPBLK] __attribute__((aligned(64)));
                for (int j0 = 0; j0 < adim; j0 += SGS_SPBLK) {
                    int je = j0 + SGS_SPBLK; if (je > adim) je = adim;
                    for (int j = j0; j < je; j++) acc[j - j0] = 0.0;
                    for (int pp = 0; pp < 27; pp++) {
                        const double *vp = fv + (size_t)L * (27 * adim) + (size_t)pp * adim;
                        const double *ps = p + b0 + off[pp];
                        for (int j = j0; j < je; j++) tmp[j - j0] = vp[j] * ps[j];
                        for (int j = j0; j < je; j++) acc[j - j0] = acc[j - j0] + tmp[j - j0];
                    }
                    for (int j = j0; j < je; j++) q_arr[b0 + j] = acc[j - j0];
                }
            }
            sgs_bar(bar, T);

            double pq = 0.0;
            for (int i = 0; i < N; i++) pq += p[i] * q_arr[i];
            double alpha = rz / pq;

            for (int64_t i = i0; i < i1; i++) {
                x[i] = x[i] + alpha * p[i];
                r[i] = r[i] - alpha * q_arr[i];
            }
            sgs_bar(bar, T);

            fwd_sweep(fv, r, y, far, adim, bdim, cdim, off,
                      (uint32_t)(2 * it + 3), mylist, mycnt, myff);
            bwd_sweep(fv, y, z, adim, bdim, cdim, off,
                      (uint32_t)(2 * it + 3), (uint32_t)(2 * it + 4),
                      mylist, mycnt, myff, mybf);
            sgs_bar(bar, T);   /* all async sweep lines complete */

            double rznew = 0.0;
            for (int i = 0; i < N; i++) rznew += r[i] * z[i];
            double beta = rznew / rz;
            rz = rznew;

            for (int64_t i = i0; i < i1; i++) p[i] = z[i] + beta * p[i];
            sgs_bar(bar, T);
        }
        free(far);
    }

    free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(bar); free(fv);
    free(lint); free(lofs); free((void *)fflag); free((void *)bflag);
}
