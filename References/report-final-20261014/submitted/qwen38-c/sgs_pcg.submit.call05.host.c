/* SGS-preconditioned Conjugate Gradient, 27-point 3D variable-coefficient
 * stencil (CSR, fp64). Port of sgs_pcg_numpy.py.
 *
 * The SGS sweeps stay strictly sequential in row order (that dependence is
 * the kernel).  Per row the reference computes  s = r[i] - sum_{j<i} a*y[j],
 *  y[i] = s / d_i  with the j = i-1 (z-neighbor) term last among j < i.  We
 *  pre-multiply the partial sum and that coupling by 1/d_i so the row update
 *  becomes   y[i] = fma(-(a_{i-1}/d_i), y[i - 1], (r[i] - sum_{j<i, j!=i-1}
 *                        a*y[j]) * (1/d_i))
 * which differs from the reference only by a couple of ulps per row while
 * removing the ~15-cycle division from the dependency chain.
 * The backward sweep needs j > i, i.e. row order reversed; several
 * microarchitectures stream reverse-order access far worse than forward
 * (no effective prefetching), so we run it as a forward sweep on reversed
 * copies:  i' = N-1-i,  the "upper" entries of original row i become the
 * "lower" entries of reversed row i', the chain neighbor i+1 becomes i'-1,
 * and z is stored in reversed space.  The only added per-iteration cost is
 * a parallel copy y' = reverse(y).
 * CSR indices are copied to int32 (falling back to int64 for huge inputs).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define FWD_SWEEP(ITYPE)                                                      \
static void sweep_fwd_##ITYPE(const double *restrict Ad, const ITYPE *restrict Ai, \
                              const ITYPE *restrict Ap, const ITYPE *restrict Dp, \
                              const double *restrict ZC, const double *restrict invd, \
                              const double *restrict r, double *restrict y, int64_t N) \
{                                                                             \
    for (int64_t i = 0; i < N; i++) {                                         \
        ITYPE ks = Ap[i], ke = Dp[i];                                         \
        double s = r[i];                                                      \
        ITYPE k;                                                              \
        for (k = ks; k < ke - 1; k++) s = s - Ad[k] * y[Ai[k]];               \
        if (ke > ks && Ai[ke - 1] != i - 1)                                   \
            s = s - Ad[ke - 1] * y[Ai[ke - 1]];                               \
        s = s * invd[i];                                                      \
        y[i] = (ZC[i] != 0.0) ? fma(-ZC[i], y[i - 1], s) : s;                 \
    }                                                                         \
}

FWD_SWEEP(int32_t)
FWD_SWEEP(int64_t)

/* Backward triangular sweep executed as a FORWARD sweep on reversed space:
   i' = N-1-i.  z'[i'] = z[N-1-i] solves
       z'[i'] = y'[i'] - (sum_{j'<i'} a'[i'][j'] * z'[j']) / D'[i']
   where the diagonal chain neighbor of i' is i'-1 (coefficient ZC[i']), and
   D' = original diagonal, so y' is NOT scaled by invd (unlike the true
   forward solve).  The last ascending entry of a row is i'-1 iff the original
   row had a (row+1) entry. */
#define REV_SWEEP(ITYPE)                                                          \
static void sweep_rev_##ITYPE(const double *restrict Ad, const ITYPE *restrict Ai, \
                              const ITYPE *restrict Ap, const ITYPE *restrict Dp,  \
                              const double *restrict ZC, const double *restrict invd, \
                              const double *restrict yR, double *restrict z,       \
                              int64_t N)                                          \
{                                                                                 \
    for (int64_t i = 0; i < N; i++) {                                             \
        ITYPE ks = Ap[i], ke = Ap[i] + Dp[i];                                     \
        double t = 0.0;                                                           \
        ITYPE k;                                                                  \
        for (k = ks; k < ke - 1; k++) t = t - Ad[k] * z[Ai[k]];                   \
        if (ke > ks && Ai[ke - 1] != i - 1)                                       \
            t = t - Ad[ke - 1] * z[Ai[ke - 1]];                                   \
        /* t = -sum(a*z) over non-chain entries: s = y' + invd*t */               \
        double s = fma(invd[i], t, yR[i]);                                        \
        z[i] = (ZC[i] != 0.0) ? fma(-ZC[i], z[i - 1], s) : s;                     \
    }                                                                             \
}

REV_SWEEP(int32_t)
REV_SWEEP(int64_t)

static double dot_blk(const double *restrict a, const double *restrict c, int64_t N)
{
    const int64_t B = 16384;
    int64_t nb = (N + B - 1) / B;
    double *part = (double *)malloc((size_t)nb * sizeof(double));
#pragma omp parallel for schedule(static, 1)
    for (int64_t t = 0; t < nb; t++) {
        int64_t e = (t + 1) * B; if (e > N) e = N;
        double s = 0.0;
        for (int64_t i = t * B; i < e; i++) s = s + a[i] * c[i];
        part[t] = s;
    }
    double s = 0.0;
    for (int64_t t = 0; t < nb; t++) s = s + part[t];
    free(part);
    return s;
}

/* a . c with c stored in reversed space: sum_i a[i] * c[N-1-i] */
static double dot_rev(const double *restrict a, const double *restrict c, int64_t N)
{
    const int64_t B = 16384;
    int64_t nb = (N + B - 1) / B;
    double *part = (double *)malloc((size_t)nb * sizeof(double));
#pragma omp parallel for schedule(static, 1)
    for (int64_t t = 0; t < nb; t++) {
        int64_t e = (t + 1) * B; if (e > N) e = N;
        double s = 0.0;
        for (int64_t i = t * B; i < e; i++) s = s + a[i] * c[N - 1 - i];
        part[t] = s;
    }
    double s = 0.0;
    for (int64_t t = 0; t < nb; t++) s = s + part[t];
    free(part);
    return s;
}

/* build reversed CSR: row i' of Rev = "upper" part of original row N-1-i',
 * reindexed j' = N-1-j and ascending in j'. */
static void build_rev_int32_t(const double *restrict Ad,
                              const int64_t *restrict Aj,
                              const int64_t *restrict Ap,
                              const int64_t *restrict Dp, int64_t N,
                              double *restrict AdR, int32_t *restrict AiR,
                              int32_t *restrict ApR, int32_t *restrict DpR,
                              double *restrict diagR, double *restrict invdR,
                              double *restrict ZBR)
{
#pragma omp parallel for schedule(static, 256)
    for (int64_t i = 0; i < N; i++) {
        int64_t d = Dp[i], ke = Ap[i + 1];
        int64_t ip = N - 1 - i;
        ApR[ip + 1] = (int32_t)(ke - d - 1);
        diagR[ip] = Ad[d];
        double inv = 1.0 / Ad[d];
        invdR[ip] = inv;
        ZBR[ip] = (d + 1 < ke && Aj[d + 1] == i + 1) ? Ad[d + 1] * inv : 0.0;
    }
    ApR[0] = 0;
    for (int64_t i = 1; i <= N; i++) ApR[i] = ApR[i] + ApR[i - 1];
#pragma omp parallel for schedule(static, 256)
    for (int64_t i = 0; i < N; i++) {
        int64_t d = Dp[i], ke = Ap[i + 1];
        int64_t ip = N - 1 - i;
        int32_t m = 0;
        for (int64_t k = ke - 1; k > d; k--) {
            AiR[ApR[ip] + m] = (int32_t)(N - 1 - Aj[k]);
            AdR[ApR[ip] + m] = Ad[k];
            m++;
        }
        DpR[ip] = m;
    }
}

void sgs_pcg_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const double *restrict b,
    double *restrict x,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t niter,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const int64_t N = NX * NY * NZ;
    if (N <= 0) return;
    const int64_t nnz = A_indptr[N];
    const bool i32 = nnz < (1LL << 30);

    double *r    = (double *)malloc((size_t)N * sizeof(double));
    double *z    = (double *)malloc((size_t)N * sizeof(double)); /* reversed space */
    double *y    = (double *)malloc((size_t)N * sizeof(double));
    double *yR   = (double *)malloc((size_t)N * sizeof(double));
    double *p    = (double *)malloc((size_t)N * sizeof(double));
    double *q    = (double *)malloc((size_t)N * sizeof(double));
    double *invd = (double *)malloc((size_t)N * sizeof(double));
    double *ZF   = (double *)malloc((size_t)N * sizeof(double));
    double *invdR = (double *)malloc((size_t)N * sizeof(double));
    double *ZBR  = (double *)malloc((size_t)N * sizeof(double));
    double *diagR = (double *)malloc((size_t)N * sizeof(double));
    int64_t *dpos64 = (int64_t *)malloc((size_t)N * sizeof(int64_t));

    double *AdR = NULL;
    int32_t *Ai32 = NULL, *Ap32 = NULL, *Dp32 = NULL, *AiR32 = NULL, *ApR32 = NULL, *DpR32 = NULL;
    int64_t *AiR64 = NULL, *ApR64 = NULL, *DpR64 = NULL;
    if (i32) {
        Ai32  = (int32_t *)malloc((size_t)nnz * sizeof(int32_t));
        Ap32  = (int32_t *)malloc((size_t)(N + 1) * sizeof(int32_t));
        Dp32  = (int32_t *)malloc((size_t)N * sizeof(int32_t));
    } else {
        AiR64 = (int64_t *)malloc((size_t)nnz * sizeof(int64_t));
        ApR64 = (int64_t *)malloc((size_t)(N + 1) * sizeof(int64_t));
        DpR64 = (int64_t *)malloc((size_t)N * sizeof(int64_t));
    }
    AdR = (double *)malloc((size_t)nnz * sizeof(double));

    /* x0 = 0, so the initial residual is b itself. */
#pragma omp parallel for schedule(static, 2048)
    for (int64_t i = 0; i < N; i++) {
        x[i] = 0.0;
        r[i] = b[i];
    }

    /* per-row constants: diagonal position/value, 1/diag, forward chain coupling */
#pragma omp parallel for schedule(static, 256)
    for (int64_t i = 0; i < N; i++) {
        int64_t ks = A_indptr[i], ke = A_indptr[i + 1];
        int64_t d = ks;
        double dv = 0.0;
        for (int64_t k = ks; k < ke; k++)
            if (A_indices[k] == i) { d = k; dv = A_data[k]; }
        dpos64[i] = d;
        invd[i] = 1.0 / dv;
        ZF[i] = (d > ks && A_indices[d - 1] == i - 1) ? A_data[d - 1] * invd[i] : 0.0;
    }
    if (i32) {
#pragma omp parallel for schedule(static, 1024)
        for (int64_t k = 0; k < nnz; k++) Ai32[k] = (int32_t)A_indices[k];
#pragma omp parallel for schedule(static, 1024)
        for (int64_t i = 0; i <= N; i++) Ap32[i] = (int32_t)A_indptr[i];
#pragma omp parallel for schedule(static, 1024)
        for (int64_t i = 0; i < N; i++) Dp32[i] = (int32_t)dpos64[i];
    }

    if (i32) {
        AiR32 = (int32_t *)malloc((size_t)nnz * sizeof(int32_t));
        ApR32 = (int32_t *)calloc((size_t)(N + 1), sizeof(int32_t));
        DpR32 = (int32_t *)malloc((size_t)N * sizeof(int32_t));
        build_rev_int32_t(A_data, A_indices, A_indptr, dpos64, N, AdR, AiR32, ApR32, DpR32,
                          diagR, invdR, ZBR);
    } else {
        /* rare huge-input path: serial reversed build over int64 indices */
        int64_t *cnt = (int64_t *)malloc((size_t)N * sizeof(int64_t));
        for (int64_t i = 0; i < N; i++) cnt[N - 1 - i] = A_indptr[i + 1] - (dpos64[i] + 1);
        for (int64_t i = 0; i < N; i++) cnt[i + 1] += cnt[i];
        for (int64_t i = 0; i < N; i++) {
            int64_t d = dpos64[i], ke = A_indptr[i + 1];
            int64_t ip = N - 1 - i;
            diagR[ip] = A_data[d];
            double inv = 1.0 / A_data[d];
            invdR[ip] = inv;
            ZBR[ip] = (d + 1 < ke && A_indices[d + 1] == i + 1) ? A_data[d + 1] * inv : 0.0;
            int64_t m = 0;
            for (int64_t k = ke - 1; k > d; k--) {
                AiR64[cnt[ip] + m] = N - 1 - A_indices[k];
                AdR[cnt[ip] + m] = A_data[k];
                m++;
            }
            DpR64[ip] = m;
        }
        for (int64_t i = 0; i <= N; i++) ApR64[i] = cnt[i];
        free(cnt);
    }

    /* initial: y = (D+L)^-1 r, then z' = forward sweep on reversed data */
    if (i32)
        sweep_fwd_int32_t(A_data, Ai32, Ap32, Dp32, ZF, invd, r, y, N);
    else
        sweep_fwd_int64_t(A_data, A_indices, A_indptr, dpos64, ZF, invd, r, y, N);
#pragma omp parallel for schedule(static, 2048)
    for (int64_t i = 0; i < N; i++) yR[i] = y[N - 1 - i];
    if (i32)
        sweep_rev_int32_t(AdR, AiR32, ApR32, DpR32, ZBR, invdR, yR, z, N);
    else
        sweep_rev_int64_t(AdR, AiR64, ApR64, DpR64, ZBR, invdR, yR, z, N);

    {
#pragma omp parallel for schedule(static, 2048)
        for (int64_t i = 0; i < N; i++) p[i] = z[N - 1 - i];
    }
    double rz = dot_rev(r, z, N);

    for (int64_t it = 0; it < niter; it++) {
        /* q = A p */
#pragma omp parallel for schedule(static, 128)
        for (int64_t i = 0; i < N; i++) {
            double acc = 0.0;
            if (i32) {
                for (int32_t k = Ap32[i], ke = Ap32[i + 1]; k < ke; k++)
                    acc = acc + A_data[k] * p[Ai32[k]];
            } else {
                for (int64_t k = A_indptr[i], ke = A_indptr[i + 1]; k < ke; k++)
                    acc = acc + A_data[k] * p[A_indices[k]];
            }
            q[i] = acc;
        }
        double pq = dot_blk(p, q, N);
        double alpha = rz / pq;
#pragma omp parallel for schedule(static, 2048)
        for (int64_t i = 0; i < N; i++) {
            x[i] = x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }
        /* forward sweep (original order) */
        if (i32)
            sweep_fwd_int32_t(A_data, Ai32, Ap32, Dp32, ZF, invd, r, y, N);
        else
            sweep_fwd_int64_t(A_data, A_indices, A_indptr, dpos64, ZF, invd, r, y, N);
#pragma omp parallel for schedule(static, 2048)
        for (int64_t i = 0; i < N; i++) yR[i] = y[N - 1 - i];
        /* backward sweep = forward sweep on reversed data; writes z' */
        if (i32)
            sweep_rev_int32_t(AdR, AiR32, ApR32, DpR32, ZBR, invdR, yR, z, N);
        else
            sweep_rev_int64_t(AdR, AiR64, ApR64, DpR64, ZBR, invdR, yR, z, N);
        double rznew = dot_rev(r, z, N);
        double beta = rznew / rz;
#pragma omp parallel for schedule(static, 2048)
        for (int64_t i = 0; i < N; i++) {
            p[i] = z[N - 1 - i] + beta * p[i];
        }
        rz = rznew;
    }

    free(r); free(z); free(y); free(yR); free(p); free(q); free(invd);
    free(ZF); free(invdR); free(ZBR); free(diagR);
    free(dpos64); free(AdR);
    if (i32) { free(Ai32); free(Ap32); free(Dp32); free(AiR32); free(ApR32); free(DpR32); }
    else { free(AiR64); free(ApR64); free(DpR64); }
}
