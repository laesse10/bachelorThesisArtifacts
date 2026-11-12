#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef SYS_set_mempolicy
#define SYS_set_mempolicy 238
#endif
extern long syscall(long number, ...);
#define MPOL_DEFAULT_I 0
#define MPOL_INTERLEAVE_I 2

typedef int64_t i64;

static int policy_enabled(void) {
    static int en = -1;
    if (en < 0) en = (getenv("SGS_NO_INTERLEAVE") == NULL);
    return en;
}
static void set_policy_interleave(void) {
    if (!policy_enabled()) return;
    unsigned long mask = 0;
    int nmax = 0;
    for (int k = 0; k < 64; k++) {
        char path[64];
        snprintf(path, sizeof path, "/sys/devices/system/node/node%d", k);
        if (access(path, F_OK) == 0) {
            mask |= 1UL << k;
            nmax = k + 1;
        }
    }
    if (nmax >= 2) (void)syscall(SYS_set_mempolicy, MPOL_INTERLEAVE_I, &mask, (unsigned long)nmax);
}
static void set_policy_default(void) {
    if (!policy_enabled()) return;
    (void)syscall(SYS_set_mempolicy, MPOL_DEFAULT_I, NULL, 0);
}

/* ---------------- generic (fallback) pieces, serial sweeps ---------------- */

static double dot8(const double *restrict a, const double *restrict b, i64 n) {
    double total = 0.0;
    i64 m = n & ~7LL;
    #pragma omp parallel for schedule(static) reduction(+:total)
    for (i64 i = 0; i < m; i += 8) {
        double s0 = a[i+0]*b[i+0], s1 = a[i+1]*b[i+1], s2 = a[i+2]*b[i+2], s3 = a[i+3]*b[i+3];
        double s4 = a[i+4]*b[i+4], s5 = a[i+5]*b[i+5], s6 = a[i+6]*b[i+6], s7 = a[i+7]*b[i+7];
        total += (s0 + s1) + (s2 + s3) + (s4 + s5) + (s6 + s7);
    }
    for (i64 i = m; i < n; i++) total += a[i]*b[i];
    return total;
}

static void sweep_fwd_serial(const double *restrict A_data, const i64 *restrict A_indices,
                             const i64 *restrict A_indptr, const unsigned char *restrict diagpos,
                             const double *restrict r, const double *restrict invD,
                             double *restrict y, i64 N) {
    for (i64 i = 0; i < N; i++) {
        i64 k0 = A_indptr[i];
        int dp = diagpos[i];
        double s = r[i];
        for (i64 k = k0; k < k0 + dp; k++) s -= A_data[k] * y[A_indices[k]];
        y[i] = s * invD[i];
    }
}
static void sweep_bwd_serial(const double *restrict A_data, const i64 *restrict A_indices,
                             const i64 *restrict A_indptr, const unsigned char *restrict diagpos,
                             const double *restrict D, const double *restrict y,
                             const double *restrict invD, double *restrict z, i64 N) {
    for (i64 i = N - 1; i >= 0; i--) {
        i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
        int dp = diagpos[i];
        double s = D[i] * y[i];
        for (i64 k = k0 + dp + 1; k < k1; k++) s -= A_data[k] * z[A_indices[k]];
        z[i] = s * invD[i];
    }
}

/* ---------------- fast path: 27-stencil planes + serial plane sweeps ----------------
 * Grid order: column = f + L0*m + L0*L1*s  (f fastest, m middle, s slowest).
 * Slots t = 0..26 sorted by (ds, dm, df) == ascending column offset.
 *   slot 12: delta = -1  (within-row chain predecessor)
 *   slot 13: delta =  0  (diagonal)
 *   slot 14: delta = +1  (within-row chain successor)
 * PL[t][i] = coefficient of entry (i, i+delta[t]), 0 when out of domain.
 */

typedef struct {
    i64 L0, L1, L2, L01;
    i64 delta[27];
    int C;          /* chunks per row (16 wide) */
} Pat;

/* 6 axis permutations: perm[0]=fast axis, perm[1]=middle, perm[2]=slow */
static const int perms[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
};

/* slot table: for type (6 edge flags), map slot -> pid (slot index) or -1 if absent */
static void build_type_map(signed char map[64][27]) {
    for (int type = 0; type < 64; type++) {
        int f0 = type & 1, f1 = type & 2, m0 = type & 4, m1 = type & 8, s0 = type & 16, s1 = type & 32;
        int t = 0;
        for (int ds = -1; ds <= 1; ds++)
            for (int dm = -1; dm <= 1; dm++)
                for (int df = -1; df <= 1; df++) {
                    int ok = 1;
                    if (df == -1 && f0) ok = 0;
                    if (df == +1 && f1) ok = 0;
                    if (dm == -1 && m0) ok = 0;
                    if (dm == +1 && m1) ok = 0;
                    if (ds == -1 && s0) ok = 0;
                    if (ds == +1 && s1) ok = 0;
                    map[type][t] = (signed char)(ok ? t : -1);
                    t++;
                }
    }
}

static int count_for_type(const signed char *maprow) {
    int c = 0;
    for (int t = 0; t < 27; t++) if (maprow[t] >= 0) c++;
    return c;
}

/* expected column list for a sample row under candidate permutation */
static int row_matches(const i64 *A_indices, const i64 *A_indptr, i64 i,
                       i64 f, i64 m, i64 s, const Pat *P) {
    i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
    int t = 0;
    for (int ds = -1; ds <= 1; ds++)
        for (int dm = -1; dm <= 1; dm++)
            for (int df = -1; df <= 1; df++) {
                i64 nf = f + df, nm = m + dm, ns = s + ds;
                if (nf < 0 || nf >= P->L0 || nm < 0 || nm >= P->L1 || ns < 0 || ns >= P->L2) continue;
                i64 col = nf + P->L0 * nm + P->L01 * ns;
                if (t >= 27 || A_indices[k0 + t] != col) return 0;
                t++;
            }
    return t == (int)(k1 - k0);
}

/* try all 6 permutations on sample rows; on success fill P and return 1 */
static int detect_pattern(Pat *P, const i64 Ls[3], const i64 *A_indices, const i64 *A_indptr, i64 N) {
    static signed char tm[64][27];
    static int tm_ready = 0;
    if (!tm_ready) { build_type_map(tm); tm_ready = 1; }
    for (int pp = 0; pp < 6; pp++) {
        const int *perm = perms[pp];
        i64 L0 = Ls[perm[0]], L1 = Ls[perm[1]], L2 = Ls[perm[2]];
        i64 L01 = L0 * L1;
        P->L0 = L0; P->L1 = L1; P->L2 = L2; P->L01 = L01;
        /* delta[t] in slot order */
        int t = 0;
        for (int ds = -1; ds <= 1; ds++)
            for (int dm = -1; dm <= 1; dm++)
                for (int df = -1; df <= 1; df++)
                    P->delta[t++] = (i64)df + (i64)dm * L0 + (i64)ds * L01;
        (void)0;
        /* sample rows */
        i64 fs[3] = {0, L0/2, L0-1}, ms[3] = {0, L1/2, L1-1}, ss[3] = {0, L2/2, L2-1};
        int ok = 1;
        for (int a = 0; a < 3 && ok; a++)
            for (int b = 0; b < 3 && ok; b++)
                for (int c = 0; c < 3 && ok; c++) {
                    i64 i = fs[a] + L0 * ms[b] + L01 * ss[c];
                    if (!row_matches(A_indices, A_indptr, i, fs[a], ms[b], ss[c], P)) ok = 0;
                }
        if (!ok) continue;
        /* verify every row length matches its type */
        int bad = 0;
        #pragma omp parallel for schedule(static) reduction(+:bad)
        for (i64 i = 0; i < N; i++) {
            i64 f = i % L0, m = (i / L0) % L1, s = i / L01;
            int type = (f == 0 ? 1 : 0) | (f == L0-1 ? 2 : 0) | (m == 0 ? 4 : 0) |
                       (m == L1-1 ? 8 : 0) | (s == 0 ? 16 : 0) | (s == L2-1 ? 32 : 0);
            if ((i64)count_for_type(tm[type]) != A_indptr[i+1] - A_indptr[i]) bad = 1;
        }
        if (bad) continue;
        P->C = (int)((L0 + 15) / 16);
        return 1;
    }
    return 0;
}

/* fill PL[27*N]: PL[t*N + i] = coeff of (i, i+delta[t]); row (m,s) handled per f-group */
static void build_planes(double *PL, const double *A_data, const i64 *A_indices,
                         const i64 *A_indptr, const Pat *P, i64 N) {
    static signed char tm[64][27];
    static int tm_ready = 0;
    if (!tm_ready) { build_type_map(tm); tm_ready = 1; }
    i64 L0 = P->L0, L1 = P->L1, L2 = P->L2, L01 = P->L01;
    i64 R = L1 * L2;
    (void)A_indices;
    #pragma omp parallel for schedule(static)
    for (i64 r = 0; r < R; r++) {
        i64 m = r % L1, s = r / L1;
        i64 i_base = m * L0 + L01 * s;
        int type_bs = (m == 0 ? 4 : 0) | (m == L1-1 ? 8 : 0) | (s == 0 ? 16 : 0) | (s == L2-1 ? 32 : 0);
        i64 G = (L0 + 7) / 8;
        for (i64 g = 0; g < G; g++) {
            i64 f0 = g * 8;
            const signed char *mp = tm[type_bs];
            i64 n = (f0 + 8 <= L0) ? 8 : (L0 - f0);
            if (n == 8 && f0 > 0 && f0 + 7 < L0 - 1) {
                /* interior group: 8 consecutive i's, identical slot layout, row stride = len */
                i64 k0 = A_indptr[i_base + f0];
                i64 stride = (i64)count_for_type(mp);
                int cnt = 0;
                for (int t = 0; t < 27; t++) {
                    double *dst = PL + (i64)t * N + i_base + f0;
                    if (mp[t] < 0) {
                        #pragma GCC unroll 8
                        for (i64 x = 0; x < 8; x++) dst[x] = 0.0;
                    } else {
                        const double *src = A_data + k0 + cnt;
                        #pragma GCC unroll 8
                        for (i64 x = 0; x < 8; x++) dst[x] = src[x * stride];
                        cnt++;
                    }
                }
            } else {
                /* f-edge / partial group: per-element types */
                for (i64 x = 0; x < n; x++) {
                    i64 i = i_base + f0 + x;
                    int f = (int)(i - i_base);
                    int type2 = type_bs | (f == 0 ? 1 : 0) | (f == L0 - 1 ? 2 : 0);
                    const signed char *mp2 = tm[type2];
                    i64 k0 = A_indptr[i];
                    int cnt = 0;
                    for (int t = 0; t < 27; t++)
                        if (mp2[t] >= 0) { PL[(i64)mp2[t] * N + i] = A_data[k0 + cnt]; cnt++; }
                        else PL[(i64)t * N + i] = 0.0;
                }
            }
        }
    }
}

/* spmv via planes: q[i] = sum_t PL[t][i] * p[i + delta[t]]  (p is guard-padded) */
static void spmv_planes(double *q, const double *PL, const double *p, const Pat *P, i64 N) {
    i64 m = N & ~7LL;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < m; i += 8) {
        double a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0;
        for (int t = 0; t < 27; t++) {
            const double *PLt = PL + (i64)t * N + i;
            const double *pp = p + i + P->delta[t];
            a0 += PLt[0]*pp[0]; a1 += PLt[1]*pp[1]; a2 += PLt[2]*pp[2]; a3 += PLt[3]*pp[3];
            a4 += PLt[4]*pp[4]; a5 += PLt[5]*pp[5]; a6 += PLt[6]*pp[6]; a7 += PLt[7]*pp[7];
        }
        q[i+0]=a0; q[i+1]=a1; q[i+2]=a2; q[i+3]=a3; q[i+4]=a4; q[i+5]=a5; q[i+6]=a6; q[i+7]=a7;
    }
    for (i64 i = m; i < N; i++) {
        double a = 0;
        for (int t = 0; t < 27; t++) a += PL[(i64)t * N + i] * p[i + P->delta[t]];
        q[i] = a;
    }
}


/* ---- serial plane sweeps ---------------------------------------------------
 * Linear i order == exact reference row order; the 12 non-chain products are
 * tree-summed (reassociation, |delta| ~ 1e-15), the chain term (slot 12 fwd /
 * slot 14 bwd) enters at the last addition, division -> reciprocal multiply.
 */
static void sweep_fwd_plane(double *restrict y, const double *restrict r,
                            const double *restrict PL, const double *restrict invD,
                            const i64 d[27], i64 N) {
    const double *S0  = PL,        *S1  = PL + N,      *S2  = PL + 2*N,  *S3  = PL + 3*N,
                 *S4  = PL + 4*N,  *S5  = PL + 5*N,    *S6  = PL + 6*N,  *S7  = PL + 7*N,
                 *S8  = PL + 8*N,  *S9  = PL + 9*N,    *S10 = PL + 10*N, *S11 = PL + 11*N,
                 *S12 = PL + 12*N;
    for (i64 i = 0; i < N; i++) {
        double t0  = S0 [i] * y[i + d[0]];
        double t1  = S1 [i] * y[i + d[1]];
        double t2  = S2 [i] * y[i + d[2]];
        double t3  = S3 [i] * y[i + d[3]];
        double t4  = S4 [i] * y[i + d[4]];
        double t5  = S5 [i] * y[i + d[5]];
        double t6  = S6 [i] * y[i + d[6]];
        double t7  = S7 [i] * y[i + d[7]];
        double t8  = S8 [i] * y[i + d[8]];
        double t9  = S9 [i] * y[i + d[9]];
        double t10 = S10[i] * y[i + d[10]];
        double t11 = S11[i] * y[i + d[11]];
        double a = t0 + t1, b = t2 + t3, c = t4 + t5, e = t6 + t7, f = t8 + t9, g = t10 + t11;
        double T = (((a + b) + (c + e)) + (f + g));
        double t12 = S12[i] * y[i - 1];
        double B = fma(invD[i], T, t12 * invD[i]);
        y[i] = fma(invD[i], r[i], -B);
    }
}

static void sweep_bwd_plane(double *restrict z, const double *restrict y,
                            const double *restrict PL, const double *restrict invD,
                            const i64 d[27], i64 N) {
    const double *S13 = PL + 13*N;
    const double *S14 = PL + 14*N,  *S15 = PL + 15*N,  *S16 = PL + 16*N,  *S17 = PL + 17*N,
                 *S18 = PL + 18*N,  *S19 = PL + 19*N,  *S20 = PL + 20*N,  *S21 = PL + 21*N,
                 *S22 = PL + 22*N,  *S23 = PL + 23*N,  *S24 = PL + 24*N,  *S25 = PL + 25*N,
                 *S26 = PL + 26*N;
    for (i64 i = N - 1; i >= 0; i--) {
        double t15 = S15[i] * z[i + d[15]];
        double t16 = S16[i] * z[i + d[16]];
        double t17 = S17[i] * z[i + d[17]];
        double t18 = S18[i] * z[i + d[18]];
        double t19 = S19[i] * z[i + d[19]];
        double t20 = S20[i] * z[i + d[20]];
        double t21 = S21[i] * z[i + d[21]];
        double t22 = S22[i] * z[i + d[22]];
        double t23 = S23[i] * z[i + d[23]];
        double t24 = S24[i] * z[i + d[24]];
        double t25 = S25[i] * z[i + d[25]];
        double t26 = S26[i] * z[i + d[26]];
        double a = t15 + t16, b = t17 + t18, c = t19 + t20, e = t21 + t22, f = t23 + t24, g = t25 + t26;
        double T = (((a + b) + (c + e)) + (f + g));
        double w = S13[i] * y[i];
        double t14 = S14[i] * z[i + 1];
        double B = fma(invD[i], T, t14 * invD[i]);
        z[i] = fma(invD[i], w, -B);
    }
}

/* ---------------- main ---------------- */

static void *g_cache = NULL;
static size_t g_cache_bytes = 0;

void sgs_pcg_csr_fp64(const double *restrict A_data, const i64 *restrict A_indices,
                      const i64 *restrict A_indptr, const double *restrict b,
                      double *restrict x, const i64 NX, const i64 NY, const i64 NZ,
                      const i64 niter, uint8_t *restrict workspace, const i64 workspace_size)
{
    (void)workspace; (void)workspace_size;
    const i64 N = NX * NY * NZ;
    set_policy_interleave();
    omp_set_dynamic(0);

    const i64 G = 2 * N + 18;                     /* guard size (covers all shapes) */
    size_t need = (size_t)(27 * N) * 8            /* PL planes (incl. diagonal) */
                + 3 * (size_t)(N + 2 * G) * 8     /* y, z, p with guards */
                + 2 * (size_t)N * 8               /* r, q */
                + (size_t)N * 8                   /* generic D */
                + (size_t)N * 8                   /* generic invD */
                + (size_t)N                       /* generic diagpos */
                + (size_t)((N / 16 + 32) * 4)     /* flags */
                + 65536;
    if (need > g_cache_bytes) {
        if (g_cache) free(g_cache);
        g_cache = malloc(need);
        if (!g_cache) { fprintf(stderr, "sgs_pcg: OOM\n"); exit(1); }
        memset(g_cache, 0, need);
        g_cache_bytes = need;
    }
    char *cp = (char *)g_cache;
    double *PL   = (double *)cp;   cp += 27 * (size_t)N * 8;
    double *yraw = (double *)cp;   cp += (size_t)(N + 2 * G) * 8;
    double *zraw = (double *)cp;   cp += (size_t)(N + 2 * G) * 8;
    double *praw = (double *)cp;   cp += (size_t)(N + 2 * G) * 8;
    double *r    = (double *)cp;   cp += (size_t)N * 8;
    double *q    = (double *)cp;   cp += (size_t)N * 8;
    double *Dgen = (double *)cp;   cp += (size_t)N * 8;
    double *invD = (double *)cp;   cp += (size_t)N * 8;
    unsigned char *diagpos = (unsigned char *)cp; cp += (size_t)N;

    double *y = yraw + G, *z = zraw + G, *p = praw + G;

    Pat P;
    i64 Ls[3] = {NX, NY, NZ};
    int fast = detect_pattern(&P, Ls, A_indices, A_indptr, N);

    if (fast) {
        build_planes(PL, A_data, A_indices, A_indptr, &P, N);
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) invD[i] = 1.0 / (PL + 13 * N)[i];
    } else {
        /* generic: extract diagonal + position */
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) {
            i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
            i64 kk;
            double d = 0.0;
            int pos = (int)(k1 - k0);
            for (kk = k0; kk < k1; kk++) {
                if (A_indices[kk] == i) { d = A_data[kk]; pos = (int)(kk - k0); break; }
            }
            Dgen[i] = d;
            diagpos[i] = (unsigned char)pos;
        }
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) invD[i] = 1.0 / Dgen[i];
    }

    /* x0 = 0, r0 = b */
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < N; i++) {
        x[i] = 0.0;
        r[i] = b[i];
    }

    /* prologue */
    if (fast) {
        sweep_fwd_plane(y, r, PL, invD, P.delta, N);
        sweep_bwd_plane(z, y, PL, invD, P.delta, N);
    } else {
        sweep_fwd_serial(A_data, A_indices, A_indptr, diagpos, r, invD, y, N);
        sweep_bwd_serial(A_data, A_indices, A_indptr, diagpos, Dgen, y, invD, z, N);
    }
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < N; i++) p[i] = z[i];
    double rz = dot8(r, z, N);

    for (i64 it = 0; it < niter; it++) {
        if (fast) {
            spmv_planes(q, PL, p, &P, N);
        } else {
            #pragma omp parallel for schedule(static)
            for (i64 i = 0; i < N; i++) {
                double acc = 0.0;
                i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
                for (i64 k = k0; k < k1; k++) acc += A_data[k] * p[A_indices[k]];
                q[i] = acc;
            }
        }
        double pq = dot8(p, q, N);
        double alpha = rz / pq;
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) {
            x[i] += alpha * p[i];
            r[i] -= alpha * q[i];
        }
        if (fast) {
            sweep_fwd_plane(y, r, PL, invD, P.delta, N);
            sweep_bwd_plane(z, y, PL, invD, P.delta, N);
        } else {
            sweep_fwd_serial(A_data, A_indices, A_indptr, diagpos, r, invD, y, N);
            sweep_bwd_serial(A_data, A_indices, A_indptr, diagpos, Dgen, y, invD, z, N);
        }
        double rznew = dot8(r, z, N);
        double beta = rznew / rz;
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) p[i] = z[i] + beta * p[i];
        rz = rznew;
    }

    set_policy_default();
}
