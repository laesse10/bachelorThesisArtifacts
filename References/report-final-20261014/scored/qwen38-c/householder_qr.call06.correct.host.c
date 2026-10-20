/* Householder QR (tall-skinny) + least-squares solve.
 * Matches the NumPy reference semantics of householder_qr_numpy.py.
 *
 * Layouts: A, Q are M x N row-major; R is N x N row-major (upper triangle
 * written, lower triangle zeroed); b is M; x is N.
 *
 * Reflector vectors are stored k-major and contiguous: V[k*M + i] holds
 * reflector k's i-th component (used for i >= k).
 *
 * Each rank-1 update C -= vc (vc^T C) scale needs two passes over the
 * trailing matrix, split over rows with the SAME strip assignment, so the
 * lines a thread reads in pass A are the lines it updates in pass B.
 *
 *   pass A: w = v^T C  -- partial w per row strip, 256-bit accumulators.
 *   pass B: row update -- same strips, 512-bit FMA.
 *
 * Reflector k+1 is precomputed at the tail of step k: column k+1 after
 * H_k is col(k+1) - vc_k * w[1] (w[1] available before the row update).
 * Its sum of squares (needed for beta[k+1]) is reduced lazily in the
 * NEXT step's single block, which removes one barrier+single per step.
 *
 * The whole A factorization runs in ONE persistent parallel region, and
 * the Q construction (H_0..H_{N-1} applied to I, right to left) plus the
 * final g = Q^T b dot run in a second persistent region: no per-step
 * region creation/teardown.
 *
 * A step k (in the A region):
 *   phase A (partials) -> barrier -> single { beta[k+1] from step k's
 *   tail sums (except k=0, precomputed); R row k-1 copy; w = beta[k]*(v^T C) }
 *   -> barrier -> tail (vnext partials + strip sums) -> phase B.
 * beta[k] is read only in the single (scale), never before; a zero
 * reflector (beta == 0) is handled uniformly: w = 0, the update is an
 * exact no-op, and the tail copies the untouched column.
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

/* Partial-sum scratch, grown once.  Slots: MAXT threads x W doubles. */
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

/* Per-thread tail sum-of-squares (one double per thread slot). */
static double *ssum_grow(void)
{
    static double *p = NULL;
    if (!p)
        p = (double *)aligned_alloc(64, (size_t)MAXT * 8);
    return p;
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
    if (N <= MAXSTACK_N)
        memset(beta_stack, 0, (size_t)N * sizeof(double));
    double *w = (N <= MAXSTACK_N) ? w_stack : (double *)malloc((size_t)N * 8);
    double *beta = (N <= MAXSTACK_N) ? beta_stack : (double *)malloc((size_t)N * 8);
    if (N > MAXSTACK_N && (!w || !beta))
        return;

    double *part = wpart_grow((int)N);
    double *sps = ssum_grow();
    if (!part || !sps)
        return;

    /* ---------------- Reflector 0: norm + copy, in parallel ------------ */
    {
        int64_t L = M;
        double *vc = V;
        const double *Acol = A;
        double x0 = Acol[0];
        double sum1 = 0.0;
        {
            double *ss = wpart_grow(1);
            int nchunk = 0;
            #pragma omp parallel
            {
                int nt = omp_get_num_threads(), tid = omp_get_thread_num();
                nchunk = nt;
                int64_t per = (L - 1 + nt - 1) / nt;
                int64_t i0 = 1 + (int64_t)tid * per, i1 = i0 + per;
                if (i1 > L) i1 = L;
                double a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0;
                int64_t i = i0;
                for (; i + 8 <= i1; i += 8) {
                    double v0 = Acol[i*   N], v1 = Acol[(i+1)*N],
                           v2 = Acol[(i+2)*N], v3 = Acol[(i+3)*N],
                           v4 = Acol[(i+4)*N], v5 = Acol[(i+5)*N],
                           v6 = Acol[(i+6)*N], v7 = Acol[(i+7)*N];
                    vc[i]=v0; vc[i+1]=v1; vc[i+2]=v2; vc[i+3]=v3;
                    vc[i+4]=v4; vc[i+5]=v5; vc[i+6]=v6; vc[i+7]=v7;
                    a0+=v0*v0; a1+=v1*v1; a2+=v2*v2; a3+=v3*v3;
                    a4+=v4*v4; a5+=v5*v5; a6+=v6*v6; a7+=v7*v7;
                }
                for (; i < i1; i++) {
                    double v = Acol[i * N];
                    vc[i] = v;
                    a0 += v * v;
                }
                ss[tid] = (a0+a1)+(a2+a3)+(a4+a5)+(a6+a7);
            }
            for (int t = 0; t < nchunk; t++)
                sum1 += ss[t];
        }
        double norm = sqrt(x0 * x0 + sum1);
        double sgn = (x0 >= 0.0) ? 1.0 : -1.0;
        double v0 = x0 + sgn * norm;
        vc[0] = v0;
        double vv = sum1 + v0 * v0;
        beta[0] = vv > 0.0 ? 2.0 / vv : 0.0;
    }

    /* ---------------- Q = I -------------------------------------------- */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; i++)
        memset(Q + i * N, 0, (size_t)N * sizeof(double));
    for (int64_t i = 0; i < N; i++)
        Q[i * N + i] = 1.0;

    /* ---------------- A factorization, one persistent region ----------- */
    {
        #pragma omp parallel
        {
            int ntA = omp_get_num_threads(), tid = omp_get_thread_num();
            int nt_used = ntA < MAXT ? ntA : MAXT;
            for (int64_t k = 0; k < N; k++) {
                int64_t L = M - k;
                double *vc = V + k * M + k;
                int64_t W = N - k;
                double *C0A = A + k * N + k;
                int64_t per = (L + nt_used - 1) / nt_used;
                /* ---- pass A: partial column dots over row strips ----
                 * Row 0 is left out of the partials: for k >= 1 its
                 * coefficient vc[0] is only known after the tail sums of
                 * the previous step are reduced (the reflector head is
                 * normalized inside the single below), so the leader adds
                 * vc[0] * row0 itself.  Uniform for all k (k = 0 already
                 * has vc[0] normalized). */
                for (int g = tid; g < nt_used; g += ntA) {
                    int64_t i0 = (int64_t)g * per, i1 = i0 + per;
                    if (i1 > L) i1 = L;
                    if (i0 == 0)
                        i0 = 1;
                    double *pw = part + (size_t)g * W;
                    for (int64_t j = 0; j < W; j++)
                        pw[j] = 0.0;
                    for (int64_t jbase = 0; jbase < W; jbase += 64) {
                        int64_t cw = W - jbase;
                        if (cw > 64)
                            cw = 64;
                        if (cw >= 8) {
                            __m256d acc[16];
                            int nc = (int)(cw >> 2);
                            for (int c = 0; c < 16; c++)
                                acc[c] = _mm256_setzero_pd();
                            for (int64_t i = i0; i < i1; i++) {
                                __m256d vb = _mm256_set1_pd(vc[i]);
                                const double *row = C0A + (size_t)i * N + jbase;
                                for (int c = 0; c < nc; c++)
                                    acc[c] = _mm256_fmadd_pd(vb, _mm256_loadu_pd(row + (c << 2)), acc[c]);
                            }
                            for (int c = 0; c < nc; c++)
                                _mm256_storeu_pd(pw + jbase + (c << 2), acc[c]);
                        }
                        for (int64_t j = (cw & ~7); j < cw; j++) {
                            double s = 0.0;
                            for (int64_t i = i0; i < i1; i++)
                                s += vc[i] * (C0A + i * N)[jbase + j];
                            pw[jbase + j] = s;
                        }
                    }
                }
                #pragma omp barrier
                #pragma omp single
                {
                    /* deferred bookkeeping from step k-1 */
                    if (k >= 1) {
                        int64_t Wp = N - (k - 1);
                        memcpy(R + (k - 1) * N + (k - 1), A + (k - 1) * N + (k - 1),
                               (size_t)Wp * sizeof(double));
                        double s = 0.0;
                        for (int t = 0; t < nt_used; t++)
                            s += sps[t];
                        /* vnext of step k-1 is exactly vc of step k */
                        double x0n = vc[0];
                        double sgn = (x0n >= 0.0) ? 1.0 : -1.0;
                        double v0n = x0n + sgn * sqrt(s);
                        vc[0] = v0n;
                        double s1 = s - x0n * x0n;
                        if (s1 < 0.0)
                            s1 = 0.0;
                        double vv = v0n * v0n + s1;
                        beta[k] = vv > 0.0 ? 2.0 / vv : 0.0;
                    }
                    /* w = beta[k] * (vc^T C); row 0's term is added here
                     * because pass A skipped it (see above). */
                    double bk = beta[k];
                    double v0 = vc[0];
                    for (int64_t j = 0; j < W; j++) {
                        double s = 0.0;
                        for (int t = 0; t < nt_used; t++)
                            s += part[(size_t)t * W + j];
                        w[j] = (s + v0 * C0A[j]) * bk;
                    }
                }
                double *vnext = (k + 1 < N) ? V + (k + 1) * M + (k + 1) : NULL;
                /* ---- tail: precompute reflector k+1 (before the update) ---- */
                if (vnext) {
                    double w1 = w[1];
                    for (int g = tid; g < nt_used; g += ntA) {
                        int64_t i0 = (int64_t)g * per, i1 = i0 + per;
                        if (i1 > L) i1 = L;
                        if (i0 == 0)
                            i0 = 1;
                        double s = 0.0;
                        for (int64_t i = i0; i < i1; i++) {
                            double vN = (C0A + i * N)[1] - vc[i] * w1;
                            vnext[i - 1] = vN;
                            s += vN * vN;
                        }
                        sps[g] = s;
                    }
                }
                /* ---- pass B: row update over the same strips ---- */
                for (int g = tid; g < nt_used; g += ntA) {
                    int64_t i0 = (int64_t)g * per, i1 = i0 + per;
                    if (i1 > L) i1 = L;
                    for (int64_t i = i0; i < i1; i++) {
                        double vi = vc[i];
                        double *row = C0A + i * N;
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
                /* tail/pass-B writes (vnext, A) must land before the next
                 * step's pass A reads them */
                #pragma omp barrier
            }
            /* last R row (nothing follows it inside the region) */
            #pragma omp single
            memcpy(R + (N - 1) * N + (N - 1), A + (N - 1) * N + (N - 1), sizeof(double));
        }
    }

    /* ---- Q construction (right to left) + g = Q^T b, one region ---- */
    {
        #pragma omp parallel
        {
            int ntA = omp_get_num_threads(), tid = omp_get_thread_num();
            int nt_used = ntA < MAXT ? ntA : MAXT;
            for (int64_t k = N - 1; k >= 0; k--) {
                int64_t L = M - k;
                double *vc = V + k * M + k;
                int64_t per = (L + nt_used - 1) / nt_used;
                for (int g = tid; g < nt_used; g += ntA) {
                    int64_t i0 = (int64_t)g * per, i1 = i0 + per;
                    if (i1 > L) i1 = L;
                    double *pw = part + (size_t)g * N;
                    for (int64_t j = 0; j < N; j++)
                        pw[j] = 0.0;
                    for (int64_t jbase = 0; jbase < N; jbase += 64) {
                        int64_t cw = N - jbase;
                        if (cw > 64)
                            cw = 64;
                        if (cw >= 8) {
                            __m256d acc[16];
                            int nc = (int)(cw >> 2);
                            for (int c = 0; c < 16; c++)
                                acc[c] = _mm256_setzero_pd();
                            for (int64_t i = i0; i < i1; i++) {
                                __m256d vb = _mm256_set1_pd(vc[i]);
                                const double *row = (Q + k * N) + (size_t)i * N + jbase;
                                for (int c = 0; c < nc; c++)
                                    acc[c] = _mm256_fmadd_pd(vb, _mm256_loadu_pd(row + (c << 2)), acc[c]);
                            }
                            for (int c = 0; c < nc; c++)
                                _mm256_storeu_pd(pw + jbase + (c << 2), acc[c]);
                        }
                        for (int64_t j = (cw & ~7); j < cw; j++) {
                            double s = 0.0;
                            for (int64_t i = i0; i < i1; i++)
                                s += vc[i] * ((Q + k * N) + i * N)[jbase + j];
                            pw[jbase + j] = s;
                        }
                    }
                }
                #pragma omp barrier
                #pragma omp single
                for (int64_t j = 0; j < N; j++) {
                    double s = 0.0;
                    for (int t = 0; t < nt_used; t++)
                        s += part[(size_t)t * N + j];
                    w[j] = s * beta[k];
                }
                for (int g = tid; g < nt_used; g += ntA) {
                    int64_t i0 = (int64_t)g * per, i1 = i0 + per;
                    if (i1 > L) i1 = L;
                    for (int64_t i = i0; i < i1; i++) {
                        double vi = vc[i];
                        double *row = (Q + k * N) + i * N;
                        int64_t j = 0;
                        for (; j + 8 <= N; j += 8) {
                            __m512d a = _mm512_loadu_pd(row + j);
                            a = _mm512_fnmadd_pd(_mm512_set1_pd(vi), _mm512_loadu_pd(w + j), a);
                            _mm512_storeu_pd(row + j, a);
                        }
                        for (; j < N; j++)
                            row[j] -= vi * w[j];
                    }
                }
                /* step-k updates must land before step k-1's pass A */
                #pragma omp barrier
            }
            /* phase 3: g = Q^T b (dot only) */
            {
                int64_t L = M;
                const double *vc = b;
                int64_t per = (L + nt_used - 1) / nt_used;
                for (int g = tid; g < nt_used; g += ntA) {
                    int64_t i0 = (int64_t)g * per, i1 = i0 + per;
                    if (i1 > L)
                        i1 = L;
                    double *pw = part + (size_t)g * N;
                    for (int64_t j = 0; j < N; j++)
                        pw[j] = 0.0;
                    for (int64_t jbase = 0; jbase < N; jbase += 64) {
                        int64_t cw = N - jbase;
                        if (cw > 64)
                            cw = 64;
                        if (cw >= 8) {
                            __m256d acc[16];
                            int nc = (int)(cw >> 2);
                            for (int c = 0; c < 16; c++)
                                acc[c] = _mm256_setzero_pd();
                            for (int64_t i = i0; i < i1; i++) {
                                __m256d vb = _mm256_set1_pd(vc[i]);
                                const double *row = Q + (size_t)i * N + jbase;
                                for (int c = 0; c < nc; c++)
                                    acc[c] = _mm256_fmadd_pd(vb, _mm256_loadu_pd(row + (c << 2)), acc[c]);
                            }
                            for (int c = 0; c < nc; c++)
                                _mm256_storeu_pd(pw + jbase + (c << 2), acc[c]);
                        }
                        for (int64_t j = (cw & ~7); j < cw; j++) {
                            double s = 0.0;
                            for (int64_t i = i0; i < i1; i++)
                                s += vc[i] * (Q + i * N)[jbase + j];
                            pw[jbase + j] = s;
                        }
                    }
                }
                #pragma omp barrier
                #pragma omp single
                for (int64_t j = 0; j < N; j++) {
                    double s = 0.0;
                    for (int t = 0; t < nt_used; t++)
                        s += part[(size_t)t * N + j];
                    w[j] = s;
                }
            }
        }
    }

    /* ---------------- back substitution -------------------------------- */
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
