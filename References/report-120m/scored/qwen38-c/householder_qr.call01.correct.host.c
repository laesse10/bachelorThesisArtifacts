/* Householder QR (tall-skinny) + least-squares solve.
 * Matches the NumPy reference semantics of householder_qr_numpy.py.
 *
 * Layouts: A, Q are M x N row-major; R is N x N row-major (only the upper
 * triangle is written, the lower triangle is zeroed); b is M; x is N.
 *
 * Reflector vectors are stored k-major and contiguous: V[k*M + i] holds
 * reflector k's i-th component (used for i >= k), so per-row scalar reads
 * and the vv dot are coalesced.
 *
 * Each rank-1 update C -= v (v^T C) needs two passes.  Both passes are split
 * over rows with the SAME strip assignment, so the lines a thread reads in
 * pass 1 are the lines it updates in pass 2 (and re-reads at the next
 * step): no cross-CCX / cross-NUMA line transfers.
 *
 *   pass 1: w = v^T C  -- each thread accumulates a partial w over its row
 *           strip with full-width vector accumulators; partials are summed
 *           after the region.
 *   pass 2: row update -- row strips, 512-bit FMA.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

#define MAXSTACK_N 512
#define MAXT 64

/* W x nt partial sums for pass 1; grown as needed, row stride W. */
static double *wpart_grow(int W)
{
    static double *p = NULL;
    static int cap = 0;
    if (W > cap) {
        int n = W < 512 ? 512 : W;
        double *q = (double *)aligned_alloc(64, (size_t)MAXT * n * 8);
        if (!q)
            q = (double *)malloc((size_t)MAXT * n * 8);
        if (!q)
            return NULL;
        free(p);
        p = q;
        cap = n;
    }
    return p;
}

/* Row-strip pass 1: w[j] = sum_i vc[i] * C[i*N + j], j < W, where C points
 * at row k (columns k..k+W-1).  Strips are assigned round-robin so the same
 * row set is processed by the same thread in pass 2. */
static void pass1(const double *C, int64_t N, int64_t L,
                  const double *vc, int64_t W, double *w)
{
    double *part = wpart_grow((int)W);
    if (!part)
        return;
    int nt_used = 1;
    #pragma omp parallel
    {
        int ntA = omp_get_num_threads(), tid = omp_get_thread_num();
        nt_used = ntA < MAXT ? ntA : MAXT;
        int64_t per = (L + nt_used - 1) / nt_used;
        for (int g = tid; g < nt_used; g += ntA) {
            int64_t i0 = (int64_t)g * per, i1 = i0 + per;
            if (i1 > L) i1 = L;
            double *pw = part + (size_t)g * W;
            for (int64_t j = 0; j < W; j++)
                pw[j] = 0.0;
            if (W >= 8 && W <= 256) {
                int64_t nv = W >> 3;
                __m512d acc[32];
                for (int64_t c = 0; c < nv; c++)
                    acc[c] = _mm512_setzero_pd();
                for (int64_t i = i0; i < i1; i++) {
                    __m512d vb = _mm512_set1_pd(vc[i]);
                    const double *row = C + (size_t)i * N;
                    for (int64_t c = 0; c < nv; c++)
                        acc[c] = _mm512_fmadd_pd(vb, _mm512_loadu_pd(row + (c << 3)), acc[c]);
                }
                for (int64_t c = 0; c < nv; c++)
                    _mm512_storeu_pd(pw + (c << 3), acc[c]);
                for (int64_t j = W & ~7; j < W; j++) {
                    double s = 0.0;
                    for (int64_t i = i0; i < i1; i++)
                        s += vc[i] * C[i * N + j];
                    pw[j] = s;
                }
            } else if (W < 8) {
                for (int64_t j = 0; j < W; j++) {
                    double s = 0.0;
                    for (int64_t i = i0; i < i1; i++)
                        s += vc[i] * C[i * N + j];
                    pw[j] = s;
                }
            } else {
                /* wide fallback: 16-column chunks, two live vectors */
                for (int64_t c = 0; c < W; c += 16) {
                    __m512d s0 = _mm512_setzero_pd(), s1 = _mm512_setzero_pd();
                    int w16 = c + 16 <= W ? 16 : W - c;
                    for (int64_t i = i0; i < i1; i++) {
                        const double *row = C + i * N + c;
                        __m512d vb = _mm512_set1_pd(vc[i]);
                        if (w16 >= 16) {
                            s0 = _mm512_fmadd_pd(vb, _mm512_loadu_pd(row), s0);
                            s1 = _mm512_fmadd_pd(vb, _mm512_loadu_pd(row + 8), s1);
                        } else if (w16 >= 8) {
                            s0 = _mm512_fmadd_pd(vb, _mm512_loadu_pd(row), s0);
                        } else {
                            for (int64_t j = 0; j < w16; j++)
                                pw[c + j] += vc[i] * row[j];
                            continue;
                        }
                    }
                    if (w16 >= 8) _mm512_storeu_pd(pw + c, s0);
                    if (w16 >= 16) _mm512_storeu_pd(pw + c + 8, s1);
                }
            }
        }
    }
    /* reduce partials (small: nt_used x W) */
    for (int64_t j = 0; j < W; j++) {
        double s = 0.0;
        for (int t = 0; t < nt_used; t++)
            s += part[(size_t)t * W + j];
        w[j] = s;
    }
}

/* Row-strip pass 2: C[i*N + k + j] -= vc[i] * w[j], i = 0..L-1, j = 0..W-1. */
static void pass2(double *C, int64_t N, int64_t L,
                  const double *vc, const double *w, int64_t W)
{
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < L; i++) {
        double vi = vc[i];
        double *row = C + i * N;
        int64_t j = 0;
        for (; j + 8 <= W; j += 8) {
            __m512d a = _mm512_loadu_pd(row + j);
            a = _mm512_fnmadd_pd(_mm512_set1_pd(vi), _mm512_loadu_pd(w + j), a);
            _mm512_storeu_pd(row + j, a);
        }
        for (; j < W; j++)
            row[j] -= vi * w[j];
    }
}

static double *vbuf_alloc(int64_t need_bytes, uint8_t *workspace, int64_t ws_size)
{
    if (workspace && ws_size >= need_bytes && ((uintptr_t)workspace & 63) == 0)
        return (double *)workspace;
    static double *sv = NULL;
    static int64_t sc = 0;
    if (sc < need_bytes) {
        free(sv);
        sv = (double *)aligned_alloc(64, (size_t)need_bytes);
        if (!sv)
            sv = (double *)malloc((size_t)need_bytes);
        if (!sv)
            return NULL;
        sc = need_bytes;
    }
    return sv;
}

void householder_qr_fp64(double *restrict A,
                         double *restrict Q,
                         double *restrict R,
                         const double *restrict b,
                         double *restrict x,
                         const int64_t M0,
                         const int64_t N0,
                         uint8_t *restrict workspace,
                         const int64_t workspace_size)
{
    extern int setenv(const char *, const char *, int);
    setenv("OMP_PROC_BIND", "spread", 0);

    int64_t M = M0, N = N0;
    if (M <= 0 || N <= 0)
        return;
    if (N > M)
        N = M;

    for (int64_t i = 0; i < N; i++)
        for (int64_t j = 0; j < i; j++)
            R[i * N + j] = 0.0;

    int64_t vneed = M * N * (int64_t)sizeof(double);
    double *V = vbuf_alloc(vneed, workspace, workspace_size);
    if (!V)
        return;

    double w_stack[MAXSTACK_N], beta_stack[MAXSTACK_N];
    double *w = (N <= MAXSTACK_N) ? w_stack : (double *)malloc((size_t)N * 8);
    double *beta = (N <= MAXSTACK_N) ? beta_stack : (double *)malloc((size_t)N * 8);
    if (N > MAXSTACK_N && (!w || !beta))
        return;

    /* ---------------- Phase 1: factor A, store reflectors ---------------- */
    for (int64_t k = 0; k < N; k++) {
        int64_t L = M - k;
        double *vc = V + k * M + k;
        const double *Acol = A + k * N + k;
        int64_t W = N - k;

        double x0 = Acol[0];
        double a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0;
        int64_t i = 1;
        for (; i + 8 <= L; i += 8) {
            double v0 = Acol[i*N],     v1 = Acol[(i+1)*N],
                   v2 = Acol[(i+2)*N], v3 = Acol[(i+3)*N],
                   v4 = Acol[(i+4)*N], v5 = Acol[(i+5)*N],
                   v6 = Acol[(i+6)*N], v7 = Acol[(i+7)*N];
            vc[i] = v0;     vc[i+1] = v1; vc[i+2] = v2; vc[i+3] = v3;
            vc[i+4] = v4;   vc[i+5] = v5; vc[i+6] = v6; vc[i+7] = v7;
            a0 += v0*v0; a1 += v1*v1; a2 += v2*v2; a3 += v3*v3;
            a4 += v4*v4; a5 += v5*v5; a6 += v6*v6; a7 += v7*v7;
        }
        for (; i < L; i++) {
            double v = Acol[i * N];
            vc[i] = v;
            a0 += v * v;
        }
        double sum1 = (a0 + a1) + (a2 + a3) + (a4 + a5) + (a6 + a7);
        double norm = sqrt(x0 * x0 + sum1);
        double sgn = (x0 >= 0.0) ? 1.0 : -1.0;
        double v0 = x0 + sgn * norm;
        vc[0] = v0;
        double vv = sum1 + v0 * v0;
        double bk = vv > 0.0 ? 2.0 / vv : 0.0;
        beta[k] = bk;

        pass1(A + k * N + k, N, L, vc, W, w);
        for (int64_t j = 0; j < W; j++)
            w[j] *= bk;
        pass2(A + k * N + k, N, L, vc, w, W);

        memcpy(R + k * N + k, A + k * N + k, (size_t)W * sizeof(double));
    }

    /* ---------------- Phase 2: Q = H_{N-1} ... H_0 applied to I --------- */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; i++)
        memset(Q + i * N, 0, (size_t)N * sizeof(double));
    for (int64_t i = 0; i < N; i++)
        Q[i * N + i] = 1.0;

    for (int64_t k = N - 1; k >= 0; k--) {
        int64_t L = M - k;
        double *vc = V + k * M + k;
        double bk = beta[k];

        pass1(Q + k * N, N, L, vc, N, w);
        for (int64_t j = 0; j < N; j++)
            w[j] *= bk;
        pass2(Q + k * N, N, L, vc, w, N);
    }

    /* ---------------- Phase 3: g = Q^T b, then back-substitute ---------- */
    pass1(Q, N, M, b, N, w);
    {
        double *g = w;
        for (int64_t i = N - 1; i >= 0; i--) {
            double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
            int64_t j = i + 1;
            for (; j + 3 < N; j += 4) {
                s0 += R[i*N + j]   * x[j];
                s1 += R[i*N + j+1] * x[j+1];
                s2 += R[i*N + j+2] * x[j+2];
                s3 += R[i*N + j+3] * x[j+3];
            }
            double s = (s0 + s1) + (s2 + s3);
            for (; j < N; j++)
                s += R[i*N + j] * x[j];
            x[i] = (g[i] - s) / R[i*N + i];
        }
    }

    if (N > MAXSTACK_N) { free(w); free(beta); }
}
