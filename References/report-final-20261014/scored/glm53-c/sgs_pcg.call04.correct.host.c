/* v4: stencil wavefront SGS-PCG, custom spin barrier, order-agnostic, bitwise-exact */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

/* one forward line: rows base .. base+a-1 (bitwise-exact per-row order) */
static void fwd_line(const double *restrict vt, const double *restrict r,
                     double *restrict y, double *restrict far,
                     int base, int a, int N, const int *restrict off)
{
    for (int j = 0; j < a; j++) far[j] = r[base + j];
    for (int p = 0; p < 12; p++) {
        const double *vp = vt + (size_t)p * N + base;
        const double *ys = y + base + off[p];
        for (int j = 0; j < a; j++) far[j] = far[j] - vp[j] * ys[j];
    }
    const double *v12 = vt + (size_t)12 * N;
    const double *v13 = vt + (size_t)13 * N;
    double yprev = 0.0;
    for (int j = 0; j < a; j++) {
        int i = base + j;
        y[i] = (far[j] - v12[i] * yprev) / v13[i];
        yprev = y[i];
    }
}

/* one backward line: rows base+a-1 .. base (bitwise-exact per-row order) */
static void bwd_line(const double *restrict vt, const double *restrict y,
                     double *restrict z, int base, int a, int N,
                     const int *restrict off)
{
    const double *v13 = vt + (size_t)13 * N;
    const double *v14 = vt + (size_t)14 * N;
    /* u[j] = diag[i]*y[i] precomputed as separate rounded muls (reference rounding) */
    for (int j = 0; j < a; j++) z[base + j] = v13[base + j] * y[base + j];
    double znext = 0.0;
    for (int j = a - 1; j >= 0; j--) {
        int i = base + j;
        double s = z[i];
        s = s - v14[i] * znext;
        for (int p = 15; p <= 26; p++)
            s = s - vt[(size_t)p * N + i] * z[i + off[p]];
        z[i] = s / v13[i];
        znext = z[i];
    }
}

static void fwd_sweep(const double *restrict vt, const double *restrict r,
                      double *restrict y, double *restrict far,
                      int a, int b, int c, int N, const int *restrict off,
                      int tid, int T, sgs_bar_t *bar)
{
    const int L = 2 * (c - 1) + (b - 1);
    for (int l = 0; l <= L; l++) {
        int lo = l - 2 * (c - 1); if (lo < 0) lo = 0;
        int hi = l; if (hi > b - 1) hi = b - 1;
        if (((l - lo) & 1) != 0) lo++;
        int K = (hi >= lo) ? ((hi - lo) / 2 + 1) : 0;
        int k0 = (int)((int64_t)tid * K / T), k1 = (int)((int64_t)(tid + 1) * K / T);
        for (int k = k0; k < k1; k++) {
            int ib = lo + 2 * k;
            int ic = (l - ib) >> 1;
            fwd_line(vt, r, y, far, ib * a + ic * a * b, a, N, off);
        }
        sgs_bar(bar, T);
    }
}

static void bwd_sweep(const double *restrict vt, const double *restrict y,
                      double *restrict z,
                      int a, int b, int c, int N, const int *restrict off,
                      int tid, int T, sgs_bar_t *bar)
{
    const int L = 2 * (c - 1) + (b - 1);
    for (int l = L; l >= 0; l--) {
        int lo = l - 2 * (c - 1); if (lo < 0) lo = 0;
        int hi = l; if (hi > b - 1) hi = b - 1;
        if (((l - lo) & 1) != 0) lo++;
        int K = (hi >= lo) ? ((hi - lo) / 2 + 1) : 0;
        int k0 = (int)((int64_t)tid * K / T), k1 = (int)((int64_t)(tid + 1) * K / T);
        for (int k = k0; k < k1; k++) {
            int ib = lo + 2 * k;
            int ic = (l - ib) >> 1;
            bwd_line(vt, y, z, ib * a + ic * a * b, a, N, off);
        }
        sgs_bar(bar, T);
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
    double *vt = (double *)malloc(sizeof(double) * (size_t)27 * N);
    if (!vt) { free(tbl); pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter); return; }

    volatile int ok = 1;
    #pragma omp parallel
    {
        int lok = 1;
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) {
            int ia = i % adim, ib = (i / adim) % bdim, ic = i / (adim * bdim);
            for (int p = 0; p < 27; p++) vt[(size_t)p * N + i] = 0.0;
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
                vt[(size_t)p * N + i] = A_data[k];
            }
        }
        if (!lok) ok = 0;
    }
    free(tbl);

    if (!ok) {
        free(vt);
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
        free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(bar); free(vt);
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }
    memset(y_raw, 0, sizeof(double) * asz);
    memset(z_raw, 0, sizeof(double) * asz);
    memset(p_raw, 0, sizeof(double) * asz);
    memset(bar, 0, sizeof(sgs_bar_t));
    double *r = r_raw + g, *y = y_raw + g, *z = z_raw + g, *p = p_raw + g;

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int T = omp_get_num_threads();
        double *far = (double *)malloc(sizeof(double) * (size_t)adim + 64);
        const int64_t i0 = (int64_t)tid * N / T, i1 = (int64_t)(tid + 1) * N / T;
        const int M = (N + SGS_SPBLK - 1) / SGS_SPBLK;
        const int mb0 = (int)((int64_t)tid * M / T), mb1 = (int)((int64_t)(tid + 1) * M / T);

        /* x0 = 0, r0 = b */
        for (int64_t i = i0; i < i1; i++) { x[i] = 0.0; r[i] = b[i]; }
        sgs_bar(bar, T);

        fwd_sweep(vt, r, y, far, adim, bdim, cdim, N, off, tid, T, bar);
        bwd_sweep(vt, y, z, adim, bdim, cdim, N, off, tid, T, bar);

        for (int64_t i = i0; i < i1; i++) p[i] = z[i];
        sgs_bar(bar, T);

        double rz = 0.0;
        for (int i = 0; i < N; i++) rz += r[i] * z[i];

        for (long it = 0; it < niter; it++) {
            /* q = A p (block partition; mul then add, matching reference rounding) */
            for (int mb = mb0; mb < mb1; mb++) {
                int b0 = mb * SGS_SPBLK, b1 = b0 + SGS_SPBLK;
                if (b1 > N) b1 = N;
                double acc[SGS_SPBLK] __attribute__((aligned(64)));
                double tmp[SGS_SPBLK] __attribute__((aligned(64)));
                for (int j = b0; j < b1; j++) acc[j - b0] = 0.0;
                for (int pp = 0; pp < 27; pp++) {
                    const double *vp = vt + (size_t)pp * N;
                    const double *ps = p + off[pp];
                    for (int j = b0; j < b1; j++) tmp[j - b0] = vp[j] * ps[j];
                    for (int j = b0; j < b1; j++) acc[j - b0] = acc[j - b0] + tmp[j - b0];
                }
                for (int j = b0; j < b1; j++) q_arr[j] = acc[j - b0];
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

            fwd_sweep(vt, r, y, far, adim, bdim, cdim, N, off, tid, T, bar);
            bwd_sweep(vt, y, z, adim, bdim, cdim, N, off, tid, T, bar);

            double rznew = 0.0;
            for (int i = 0; i < N; i++) rznew += r[i] * z[i];
            double beta = rznew / rz;
            rz = rznew;

            for (int64_t i = i0; i < i1; i++) p[i] = z[i] + beta * p[i];
            sgs_bar(bar, T);
        }
        free(far);
    }

    free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(bar); free(vt);
}
