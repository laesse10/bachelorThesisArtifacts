#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <omp.h>

/* Preconditioned CG whose preconditioner is one symmetric Gauss-Seidel sweep
 * on a 27-point variable-coefficient 3-D stencil operator (CSR, fp64).
 *
 * Semantics preserved exactly:
 *  - the forward sweep solves (D+L) y = r in increasing row order, reading the
 *    UPDATED y of every coupled row j < i;
 *  - the backward sweep solves (D+U) z = D y in decreasing row order, reading
 *    the UPDATED z of every coupled row j > i.
 * Only reassociation of the (independent) intra-row terms, FMA contraction,
 * and reciprocal instead of the final division are used; the Krylov
 * recurrence, SpMV and dots are algebraically identical with reassociated
 * summations.
 *
 * Fast path (pattern-verified at entry): the coefficients are scattered once
 * into 26 dense per-direction arrays with a zero halo, so each sweep becomes
 * a straight 8-wide AVX-512 block loop (13 terms, no branches, no index
 * loads) processed in the exact row order, and SpMV / vector updates are
 * masked 8-wide dots run over OpenMP threads inside ONE persistent parallel
 * region (the whole iteration loop). Any CSR that is not exactly this stencil
 * falls back to a scalar port of the reference loops.
 */

typedef struct {
    const double *restrict A_data;
    const int64_t *restrict A_indices;
    const int64_t *restrict A_indptr;
    const double *restrict b;
    double *restrict x;
    int64_t NX, NY, NZ, niter;
} args_t;

/* ---- NUMA awareness ----------------------------------------------------
 * The dense coefficient block (26 arrays) is read by every phase on every
 * thread; on a multi-node machine that is most of the traffic. We replicate
 * it once per NUMA node that has at least one of our CPUs and let each
 * thread use the replica on its own node. Detection is best effort: any
 * failure degrades to the single-replica path. */

extern int sched_getcpu(void);

static int parse_cpu_list(const char *s, int *out, int max) {
    int n = 0;
    const char *p = s;
    while (*p && n < max) {
        char *end = NULL;
        long a = strtol(p, &end, 10);
        if (end == p) break;
        p = end;
        long b = a;
        if (*p == '-') { p++; b = strtol(p, &end, 10); if (end == p) break; p = end; }
        for (long c = a; c <= b && n < max; c++) out[n++] = (int)c;
        if (*p == ',') p++;
        else if (*p) break;
    }
    return n;
}

/* fill node_of_cpu[] (dense node ids 0..n-1) for the CPUs of our cpuset;
 * returns the number of distinct nodes, or 1 on any detection failure */
static int build_node_map(int *node_of_cpu, int maxcpu) {
    static const char *affpaths[] = {
        "/sys/fs/cgroup/cpuset.cpus.effective",
        "/sys/fs/cgroup/cpuset/cpuset.cpus",
    };
    int mine[1024], nm = 0;
    for (int pi = 0; pi < 2 && !nm; pi++) {
        FILE *f = fopen(affpaths[pi], "r");
        if (!f) continue;
        char buf[2048];
        if (fgets(buf, (int)sizeof buf, f)) nm = parse_cpu_list(buf, mine, 1024);
        fclose(f);
    }
    if (!nm) return 1;
    int sysnode[1024];
    for (int i = 0; i < 1024; i++) sysnode[i] = -1;
    int sysnodes = 0;
    int ncpus[1024];
    for (int nd = 0; nd < 64; nd++) {
        char path[128];
        snprintf(path, sizeof path, "/sys/devices/system/node/node%d/cpulist", nd);
        FILE *f = fopen(path, "r");
        if (!f) break;
        char buf[2048];
        int nc = 0;
        if (fgets(buf, (int)sizeof buf, f)) nc = parse_cpu_list(buf, ncpus, 1024);
        fclose(f);
        if (nc) {
            for (int i = 0; i < nc; i++) sysnode[ncpus[i]] = sysnodes;
            sysnodes++;
        }
    }
    if (!sysnodes) return 1;
    int map[64];
    for (int i = 0; i < 64; i++) map[i] = -1;
    int nnode = 0;
    for (int i = 0; i < maxcpu; i++) node_of_cpu[i] = 0;
    for (int i = 0; i < nm; i++) {
        int n = sysnode[mine[i]];
        if (n < 0) n = 0;
        if (map[n] < 0) map[n] = nnode++;
        node_of_cpu[mine[i]] = map[n];
    }
    return nnode > 0 ? nnode : 1;
}

#define NDIR 26

/* ============================ scalar reference port ======================= */

static void run_scalar(const args_t *a) {
    const int64_t N = a->NX * a->NY * a->NZ;
    double *r   = malloc((size_t)N * sizeof(double));
    double *z   = malloc((size_t)N * sizeof(double));
    double *y   = malloc((size_t)N * sizeof(double));
    double *p   = malloc((size_t)N * sizeof(double));
    double *q   = malloc((size_t)N * sizeof(double));
    double *diag= malloc((size_t)N * sizeof(double));

    for (int64_t i = 0; i < N; i++) { a->x[i] = 0.0; r[i] = a->b[i]; }
    for (int64_t i = 0; i < N; i++) {
        double d = 0.0;
        for (int64_t k = a->A_indptr[i]; k < a->A_indptr[i + 1]; k++)
            if (a->A_indices[k] == i) d = a->A_data[k];
        diag[i] = d;
    }

    for (int64_t i = 0; i < N; i++) {
        double s = r[i];
        for (int64_t k = a->A_indptr[i]; k < a->A_indptr[i + 1]; k++) {
            int64_t j = a->A_indices[k];
            if (j < i) s = s - a->A_data[k] * y[j];
        }
        y[i] = s / diag[i];
    }
    for (int64_t ii = 0; ii < N; ii++) {
        int64_t row = N - 1 - ii;
        double s = diag[row] * y[row];
        for (int64_t k = a->A_indptr[row]; k < a->A_indptr[row + 1]; k++) {
            int64_t j = a->A_indices[k];
            if (j > row) s = s - a->A_data[k] * z[j];
        }
        z[row] = s / diag[row];
    }
    for (int64_t i = 0; i < N; i++) p[i] = z[i];
    double rz = 0.0;
    for (int64_t i = 0; i < N; i++) rz = rz + r[i] * z[i];

    for (int64_t it = 0; it < a->niter; it++) {
        for (int64_t i = 0; i < N; i++) {
            double acc = 0.0;
            for (int64_t k = a->A_indptr[i]; k < a->A_indptr[i + 1]; k++)
                acc = acc + a->A_data[k] * p[a->A_indices[k]];
            q[i] = acc;
        }
        double pq = 0.0;
        for (int64_t i = 0; i < N; i++) pq = pq + p[i] * q[i];
        double alpha = rz / pq;
        for (int64_t i = 0; i < N; i++) {
            a->x[i] = a->x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }
        for (int64_t i = 0; i < N; i++) {
            double s = r[i];
            for (int64_t k = a->A_indptr[i]; k < a->A_indptr[i + 1]; k++) {
                int64_t j = a->A_indices[k];
                if (j < i) s = s - a->A_data[k] * y[j];
            }
            y[i] = s / diag[i];
        }
        for (int64_t ii = 0; ii < N; ii++) {
            int64_t row = N - 1 - ii;
            double s = diag[row] * y[row];
            for (int64_t k = a->A_indptr[row]; k < a->A_indptr[row + 1]; k++) {
                int64_t j = a->A_indices[k];
                if (j > row) s = s - a->A_data[k] * z[j];
            }
            z[row] = s / diag[row];
        }
        double rznew = 0.0;
        for (int64_t i = 0; i < N; i++) rznew = rznew + r[i] * z[i];
        double beta = rznew / rz;
        for (int64_t i = 0; i < N; i++) p[i] = z[i] + beta * p[i];
        rz = rznew;
    }
    free(r); free(z); free(y); free(p); free(q); free(diag);
}

/* ================================ fast path =============================== */

typedef struct {
    int64_t off[NDIR];
    unsigned char fwd[NDIR];
    unsigned char bwd[NDIR];
} dirtab_t;

static int build_dirtab(int64_t s0, int64_t s1, int64_t s2, dirtab_t *t) {
    int n = 0;
    for (int dz = -1; dz <= 1; dz++)
    for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        int64_t o = (int64_t)dx * s0 + (int64_t)dy * s1 + (int64_t)dz * s2;
        if (o == 0) return 0;
        for (int k = 0; k < n; k++)
            if (t->off[k] == o) return 0;
        t->off[n] = o; t->fwd[n] = (o < 0); t->bwd[n] = (o > 0); n++;
    }
    return n == NDIR;
}

/* row 0 is grid point (0,0,0) for any axis-wise index convention; its 7
 * off-diagonal columns are {s0, s1, s2, s0+s1, s0+s2, s1+s2, s0+s1+s2}
 * (columns sorted ascending; the diagonal is column 0). */
static int detect_strides(const args_t *a, int64_t *s0, int64_t *s1, int64_t *s2) {
    int64_t lo = a->A_indptr[0], hi = a->A_indptr[1];
    if (hi - lo != 8) return 0;
    int64_t c[7];
    int n = 0, sawdiag = 0;
    for (int k = 0; k < 8; k++) {
        int64_t v = a->A_indices[lo + k];
        if (v == 0) sawdiag = 1;
        else c[n++] = v;
    }
    if (n != 7 || !sawdiag) return 0;
    if (c[0] <= 0 || c[1] <= c[0] || c[6] <= c[5]) return 0;
    int64_t A = c[0], Bv = c[1], C = c[5] - c[1];
    if (C <= Bv) return 0;
    int64_t e[7] = { A, Bv, C, A + Bv, A + C, Bv + C, A + Bv + C };
    for (int i = 0; i < 7; i++)
        for (int j = i + 1; j < 7; j++)
            if (e[j] < e[i]) { int64_t tmp = e[i]; e[i] = e[j]; e[j] = tmp; }
    for (int k = 0; k < 7; k++)
        if (e[k] != c[k]) return 0;
    *s0 = A; *s1 = Bv; *s2 = C;
    return 1;
}

static int pick_threads(int64_t N) {
    int t = (int)(N / 12000);
    int mx = omp_get_max_threads();
    if (mx < 1) mx = 1;
    if (t > 24) t = 24;
    if (t > mx) t = mx;
    if (t < 1) t = 1;
    return t;
}

static inline double dot13(const double *restrict u, const double *restrict v,
                           int64_t N) {
#if defined(__AVX512F__)
    int64_t nfull = N & ~7;
    __m512d a0 = _mm512_setzero_pd(), a1 = _mm512_setzero_pd(),
            a2 = _mm512_setzero_pd(), a3 = _mm512_setzero_pd();
    for (int64_t i = 0; i < nfull; i += 32) {
        a0 = _mm512_fmadd_pd(_mm512_loadu_pd(u + i + 0),  _mm512_loadu_pd(v + i + 0),  a0);
        a1 = _mm512_fmadd_pd(_mm512_loadu_pd(u + i + 8),  _mm512_loadu_pd(v + i + 8),  a1);
        a2 = _mm512_fmadd_pd(_mm512_loadu_pd(u + i + 16), _mm512_loadu_pd(v + i + 16), a2);
        a3 = _mm512_fmadd_pd(_mm512_loadu_pd(u + i + 24), _mm512_loadu_pd(v + i + 24), a3);
    }
    double out = _mm512_reduce_add_pd(_mm512_add_pd(_mm512_add_pd(a0, a1),
                                                    _mm512_add_pd(a2, a3)));
    for (int64_t i = nfull; i < N; i++) out += u[i] * v[i];
    return out;
#else
    double out = 0.0;
    for (int64_t i = 0; i < N; i++) out += u[i] * v[i];
    return out;
#endif
}

#if defined(__AVX512F__)
#define DOT32(U, V, i, OUTL) { \
    int64_t rem = (N) - (i); \
    if (rem >= 32) { \
        __m512d a0 = _mm512_fmadd_pd(_mm512_loadu_pd((U) + (i)),      _mm512_loadu_pd((V) + (i)),      _mm512_setzero_pd()); \
        __m512d a1 = _mm512_fmadd_pd(_mm512_loadu_pd((U) + (i) + 8),  _mm512_loadu_pd((V) + (i) + 8),  _mm512_setzero_pd()); \
        __m512d a2 = _mm512_fmadd_pd(_mm512_loadu_pd((U) + (i) + 16), _mm512_loadu_pd((V) + (i) + 16), _mm512_setzero_pd()); \
        __m512d a3 = _mm512_fmadd_pd(_mm512_loadu_pd((U) + (i) + 24), _mm512_loadu_pd((V) + (i) + 24), _mm512_setzero_pd()); \
        (OUTL) += _mm512_reduce_add_pd(_mm512_add_pd(_mm512_add_pd(a0, a1), _mm512_add_pd(a2, a3))); \
    } else { \
        for (int64_t k = (i); k < (N); k++) (OUTL) += (U)[k] * (V)[k]; \
    } \
}
#endif
#define DOTTAIL(U, V, i, OUTL) { \
    int64_t rem = (N) - (i); int64_t lim = (rem >= 8 ? 8 : rem); \
    for (int64_t k = 0; k < lim; k++) (OUTL) += (U)[(i) + k] * (V)[(i) + k]; \
}


#if defined(__AVX512F__)
static inline __m512d hsum13(__m512d t0,  __m512d t1,  __m512d t2,  __m512d t3,
                             __m512d t4,  __m512d t5,  __m512d t6,  __m512d t7,
                             __m512d t8,  __m512d t9,  __m512d t10, __m512d t11,
                             __m512d t12) {
    __m512d p0 = _mm512_add_pd(t0,  t1);
    __m512d p1 = _mm512_add_pd(t2,  t3);
    __m512d p2 = _mm512_add_pd(t4,  t5);
    __m512d p3 = _mm512_add_pd(t6,  t7);
    __m512d p4 = _mm512_add_pd(t8,  t9);
    __m512d p5 = _mm512_add_pd(t10, t11);
    __m512d q0 = _mm512_add_pd(p0, p1);
    __m512d q1 = _mm512_add_pd(p2, p3);
    __m512d q2 = _mm512_add_pd(p4, p5);
    __m512d u0 = _mm512_add_pd(q0, q1);
    __m512d u1 = _mm512_add_pd(q2, t12);
    return _mm512_add_pd(u0, u1);
}
#endif

/* dst[i] = (init0[i] - sum_d aw[d][i]*dst[i+off[d]]) * invD[i]   (fwd)
 * dst[i] = (init0[i]*inity[i] - sum_d aw[d][i]*dst[i+off[d]]) * invD[i] (bwd)
 * buffers are padded; row i lives at [i]; N rows, halo already zeroed.
 * Strict row order is kept (i ascending / i descending).
 *
 * A direction with |off| < 8 reaches rows inside the current 8-row block:
 * at most one exists (the off = +-1 neighbour of a contiguous grid), and it
 * is a first-order recurrence y_k = a_k*y_{k-1} + b_k (fwd) resp.
 * z_k = c_k*z_{k+1} + d_k (bwd) that we evaluate over the 8 lanes with a
 * 3-stage prefix scan; its value before the block is the single carry.
 * All other directions read rows outside the block, which are settled. */
typedef struct {
    const double *const *buf;
    const int64_t *off;
    int dep; /* index of the in-block direction, -1 if none */
} sweep_desc_t;

static int sweep_dep(sweep_desc_t *d) {
    int n = 0, i, di = -1;
    for (i = 0; i < 13; i++)
        if (d->off[i] >= -7 && d->off[i] <= 7) {
            n++;
            di = i;
        }
    d->dep = (n == 1) ? di : -1;
    return n <= 1;
}

#if defined(__AVX512F__)
/* y_k = a_k*y_{k-1} + b_k, k = 0..7, input y_{-1}: y_k = A1_k*y_{-1} + B1_k */
static inline __m512d lane_r1(__m512d x) {
    return _mm512_permutex2var_pd(x, _mm512_set_epi64(6, 5, 4, 3, 2, 1, 0, 8), _mm512_setzero_pd());
}
static inline __m512d lane_l1(__m512d x) {
    return _mm512_permutex2var_pd(x, _mm512_set_epi64(8, 7, 6, 5, 4, 3, 2, 1), _mm512_setzero_pd());
}
static inline __m512d lane_r2(__m512d x) {
    return _mm512_permutex2var_pd(x, _mm512_set_epi64(5, 4, 3, 2, 1, 0, 8, 8), _mm512_setzero_pd());
}
static inline __m512d lane_l2(__m512d x) {
    return _mm512_permutex2var_pd(x, _mm512_set_epi64(8, 8, 7, 6, 5, 4, 3, 2), _mm512_setzero_pd());
}
static inline __m512d lane_r4(__m512d x) {
    return _mm512_permutex2var_pd(x, _mm512_set_epi64(3, 2, 1, 0, 8, 8, 8, 8), _mm512_setzero_pd());
}
static inline __m512d lane_l4(__m512d x) {
    return _mm512_permutex2var_pd(x, _mm512_set_epi64(8, 8, 8, 8, 7, 6, 5, 4), _mm512_setzero_pd());
}

/* y_k = a_k*y_{k-1} + b_k, k = 0..7, input y_{-1}: y_k = A1_k*y_{-1} + B1_k */

/* y_k = a_k*y_{k-1} + b_k, k = 0..7, input y_{-1}: y_k = A1_k*y_{-1} + B1_k */
static inline void scan8_fwd(__m512d A, __m512d B, __m512d *A1, __m512d *B1) {
    __m512d one = _mm512_set1_pd(1.0);
    __m512d sa = lane_r1(A), sb = lane_r1(B);
    sa = _mm512_mask_blend_pd(0x1u, sa, one);
    B = _mm512_fmadd_pd(A, sb, B);
    A = _mm512_mul_pd(A, sa);
    sa = lane_r2(A); sb = lane_r2(B);
    sa = _mm512_mask_blend_pd(0x3u, sa, one);
    B = _mm512_fmadd_pd(A, sb, B);
    A = _mm512_mul_pd(A, sa);
    sa = lane_r4(A); sb = lane_r4(B);
    sa = _mm512_mask_blend_pd(0xFu, sa, one);
    B = _mm512_fmadd_pd(A, sb, B);
    A = _mm512_mul_pd(A, sa);
    *A1 = A;
    *B1 = B;
}

/* z_k = c_k*z_{k+1} + d_k, k = 7..0, input z_8: z_k = A1_k*z_8 + B1_k */
static inline void scan8_bwd(__m512d A, __m512d B, __m512d *A1, __m512d *B1) {
    __m512d one = _mm512_set1_pd(1.0);
    __m512d sa = lane_l1(A), sb = lane_l1(B);
    sa = _mm512_mask_blend_pd(0x80u, sa, one);
    B = _mm512_fmadd_pd(A, sb, B);
    A = _mm512_mul_pd(A, sa);
    sa = lane_l2(A); sb = lane_l2(B);
    sa = _mm512_mask_blend_pd(0xC0u, sa, one);
    B = _mm512_fmadd_pd(A, sb, B);
    A = _mm512_mul_pd(A, sa);
    sa = lane_l4(A); sb = lane_l4(B);
    sa = _mm512_mask_blend_pd(0xF0u, sa, one);
    B = _mm512_fmadd_pd(A, sb, B);
    A = _mm512_mul_pd(A, sa);
    *A1 = A;
    *B1 = B;
}

/* 13 independent directions: full 13-term vector sum */
#define S13 \
    __m512d s0  = _mm512_mul_pd(_mm512_loadu_pd(buf[0] + i0), _mm512_loadu_pd(dst + off[0] + i0)); \
    __m512d s1  = _mm512_mul_pd(_mm512_loadu_pd(buf[1] + i0), _mm512_loadu_pd(dst + off[1] + i0)); \
    __m512d s2  = _mm512_mul_pd(_mm512_loadu_pd(buf[2] + i0), _mm512_loadu_pd(dst + off[2] + i0)); \
    __m512d s3  = _mm512_mul_pd(_mm512_loadu_pd(buf[3] + i0), _mm512_loadu_pd(dst + off[3] + i0)); \
    __m512d s4  = _mm512_mul_pd(_mm512_loadu_pd(buf[4] + i0), _mm512_loadu_pd(dst + off[4] + i0)); \
    __m512d s5  = _mm512_mul_pd(_mm512_loadu_pd(buf[5] + i0), _mm512_loadu_pd(dst + off[5] + i0)); \
    __m512d s6  = _mm512_mul_pd(_mm512_loadu_pd(buf[6] + i0), _mm512_loadu_pd(dst + off[6] + i0)); \
    __m512d s7  = _mm512_mul_pd(_mm512_loadu_pd(buf[7] + i0), _mm512_loadu_pd(dst + off[7] + i0)); \
    __m512d s8  = _mm512_mul_pd(_mm512_loadu_pd(buf[8] + i0), _mm512_loadu_pd(dst + off[8] + i0)); \
    __m512d s9  = _mm512_mul_pd(_mm512_loadu_pd(buf[9] + i0), _mm512_loadu_pd(dst + off[9] + i0)); \
    __m512d s10 = _mm512_mul_pd(_mm512_loadu_pd(buf[10] + i0), _mm512_loadu_pd(dst + off[10] + i0)); \
    __m512d s11 = _mm512_mul_pd(_mm512_loadu_pd(buf[11] + i0), _mm512_loadu_pd(dst + off[11] + i0)); \
    __m512d s12 = _mm512_mul_pd(_mm512_loadu_pd(buf[12] + i0), _mm512_loadu_pd(dst + off[12] + i0)); \
    __m512d ss = hsum13(s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12)

static void sweep_simd(double *restrict dst, const double *restrict init0,
                       const double *restrict inity, const double *restrict invD,
                       const sweep_desc_t *D, int64_t N, int fwd) {
    const double *const *buf = D->buf;
    const int64_t *off = D->off;
    const int dep = D->dep;
    int64_t nfull = N & ~7;
    int i;
    if (fwd) {
        if (dep < 0) {
            for (int64_t i0 = 0; i0 < nfull; i0 += 8) {
                S13;
                __m512d s = _mm512_sub_pd(_mm512_loadu_pd(init0 + i0), ss);
                _mm512_storeu_pd(dst + i0, _mm512_mul_pd(s, _mm512_loadu_pd(invD + i0)));
            }
        } else {
            int ind[12], n = 0;
            for (i = 0; i < 13; i++) if (i != dep) ind[n++] = i;
            const int64_t pfstep = (N >= 400000) ? (384 * 8) : (256 * 8);
            for (int64_t i0 = 0; i0 < nfull; i0 += 8) {
                __m512d A, B;
                { int64_t ip = i0 + pfstep;
                  if (ip < nfull) {
                      for (i = 0; i < 12; i++) __builtin_prefetch(buf[ind[i]] + ip, 0, 1);
                      __builtin_prefetch(buf[dep] + ip, 0, 1);
                      __builtin_prefetch(init0 + ip, 0, 1);
                      __builtin_prefetch(invD + ip, 0, 1);
                  } }
                __m512d ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[0]] + i0), _mm512_loadu_pd(dst + off[ind[0]] + i0), _mm512_setzero_pd());
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[1]] + i0), _mm512_loadu_pd(dst + off[ind[1]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[2]] + i0), _mm512_loadu_pd(dst + off[ind[2]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[3]] + i0), _mm512_loadu_pd(dst + off[ind[3]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[4]] + i0), _mm512_loadu_pd(dst + off[ind[4]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[5]] + i0), _mm512_loadu_pd(dst + off[ind[5]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[6]] + i0), _mm512_loadu_pd(dst + off[ind[6]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[7]] + i0), _mm512_loadu_pd(dst + off[ind[7]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[8]] + i0), _mm512_loadu_pd(dst + off[ind[8]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[9]] + i0), _mm512_loadu_pd(dst + off[ind[9]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[10]] + i0), _mm512_loadu_pd(dst + off[ind[10]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[11]] + i0), _mm512_loadu_pd(dst + off[ind[11]] + i0), ss);
                A = _mm512_sub_pd(_mm512_setzero_pd(),
                                  _mm512_mul_pd(_mm512_loadu_pd(buf[dep] + i0),
                                                _mm512_loadu_pd(invD + i0)));
                B = _mm512_mul_pd(_mm512_sub_pd(_mm512_loadu_pd(init0 + i0), ss),
                                  _mm512_loadu_pd(invD + i0));
                __m512d A1, B1;
                scan8_fwd(A, B, &A1, &B1);
                double carry = dst[i0 - 1]; /* i0 == 0: zeroed halo */
                _mm512_storeu_pd(dst + i0,
                                 _mm512_fmadd_pd(A1, _mm512_set1_pd(carry), B1));
            }
        }
        for (int64_t i64 = nfull; i64 < N; i64++) {
            double s = init0[i64];
            for (i = 0; i < 13; i++) s -= buf[i][i64] * dst[off[i] + i64];
            dst[i64] = s * invD[i64];
        }
    } else {
        int ntail = (int)(N & 7);
        for (int64_t i64 = N - 1; i64 >= N - ntail; i64--) {
            double s = init0[i64] * inity[i64];
            for (i = 0; i < 13; i++) s -= buf[i][i64] * dst[off[i] + i64];
            dst[i64] = s * invD[i64];
        }
        if (dep < 0) {
            for (int64_t i0 = N - ntail - 8; i0 >= 0; i0 -= 8) {
                S13;
                __m512d s = _mm512_fmsub_pd(_mm512_loadu_pd(init0 + i0),
                                            _mm512_loadu_pd(inity + i0), ss);
                _mm512_storeu_pd(dst + i0, _mm512_mul_pd(s, _mm512_loadu_pd(invD + i0)));
            }
        } else {
            int ind[12], n = 0;
            for (i = 0; i < 13; i++) if (i != dep) ind[n++] = i;
            for (int64_t i0 = N - ntail - 8; i0 >= 0; i0 -= 8) {
                __m512d A, B;
                { int64_t ip = i0 - (8 << 3);
                  if (ip >= 0) {
                      for (i = 0; i < 12; i++) __builtin_prefetch(buf[ind[i]] + ip, 0, 1);
                      __builtin_prefetch(buf[dep] + ip, 0, 1);
                      __builtin_prefetch(init0 + ip, 0, 1);
                      __builtin_prefetch(inity + ip, 0, 1);
                      __builtin_prefetch(invD + ip, 0, 1);
                  } }
                __m512d ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[0]] + i0), _mm512_loadu_pd(dst + off[ind[0]] + i0), _mm512_setzero_pd());
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[1]] + i0), _mm512_loadu_pd(dst + off[ind[1]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[2]] + i0), _mm512_loadu_pd(dst + off[ind[2]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[3]] + i0), _mm512_loadu_pd(dst + off[ind[3]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[4]] + i0), _mm512_loadu_pd(dst + off[ind[4]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[5]] + i0), _mm512_loadu_pd(dst + off[ind[5]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[6]] + i0), _mm512_loadu_pd(dst + off[ind[6]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[7]] + i0), _mm512_loadu_pd(dst + off[ind[7]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[8]] + i0), _mm512_loadu_pd(dst + off[ind[8]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[9]] + i0), _mm512_loadu_pd(dst + off[ind[9]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[10]] + i0), _mm512_loadu_pd(dst + off[ind[10]] + i0), ss);
                ss = _mm512_fmadd_pd(_mm512_loadu_pd(buf[ind[11]] + i0), _mm512_loadu_pd(dst + off[ind[11]] + i0), ss);
                __m512d s = _mm512_fmsub_pd(_mm512_loadu_pd(init0 + i0),
                                            _mm512_loadu_pd(inity + i0), ss);
                A = _mm512_sub_pd(_mm512_setzero_pd(),
                                  _mm512_mul_pd(_mm512_loadu_pd(buf[dep] + i0),
                                                _mm512_loadu_pd(invD + i0)));
                B = _mm512_mul_pd(s, _mm512_loadu_pd(invD + i0));
                __m512d A1, B1;
                scan8_bwd(A, B, &A1, &B1);
                double carry = dst[i0 + 8]; /* tail rows above are done */
                _mm512_storeu_pd(dst + i0,
                                 _mm512_fmadd_pd(A1, _mm512_set1_pd(carry), B1));
            }
        }
    }
}
#else
static void sweep_simd(double *restrict dst, const double *restrict init0,
                       const double *restrict inity, const double *restrict invD,
                       const sweep_desc_t *D, int64_t N, int fwd) {
    const double *const *buf = D->buf;
    const int64_t *off = D->off;
    int i;
    (void)inity;
    (void)D->dep;
    if (fwd) {
        for (int64_t i64 = 0; i64 < N; i64++) {
            double s = init0[i64];
            for (i = 0; i < 13; i++) s -= buf[i][i64] * dst[off[i] + i64];
            dst[i64] = s * invD[i64];
        }
    } else {
        for (int64_t i64 = N - 1; i64 >= 0; i64--) {
            double s = init0[i64] * inity[i64];
            for (i = 0; i < 13; i++) s -= buf[i][i64] * dst[off[i] + i64];
            dst[i64] = s * invD[i64];
        }
    }
}
#endif

/* parallel pieces; one block of 8 rows each, last block masked (halo
 * makes all loads safe, stores are masked to stay inside the compact part) */

static void blk_spmv(double *restrict q, const double *restrict p,
                     const double *restrict diagp, const double *dirbuf[NDIR],
                     const int64_t doff[NDIR], int64_t N, int64_t nb, int64_t b0) {
#if defined(__AVX512F__)
    for (int64_t k = 0; k < nb; k++) {
        int64_t i0 = (b0 + k) << 3;
        __mmask8 m = (i0 + 8 <= N) ? 0xFFu : (__mmask8)((1u << (N - i0)) - 1u);
        __m512d t0  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[1]  + i0), _mm512_loadu_pd(p + doff[1]  + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[0]  + i0), _mm512_loadu_pd(p + doff[0]  + i0)));
        __m512d t1  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[3]  + i0), _mm512_loadu_pd(p + doff[3]  + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[2]  + i0), _mm512_loadu_pd(p + doff[2]  + i0)));
        __m512d t2  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[5]  + i0), _mm512_loadu_pd(p + doff[5]  + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[4]  + i0), _mm512_loadu_pd(p + doff[4]  + i0)));
        __m512d t3  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[7]  + i0), _mm512_loadu_pd(p + doff[7]  + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[6]  + i0), _mm512_loadu_pd(p + doff[6]  + i0)));
        __m512d t4  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[9]  + i0), _mm512_loadu_pd(p + doff[9]  + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[8]  + i0), _mm512_loadu_pd(p + doff[8]  + i0)));
        __m512d t5  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[11] + i0), _mm512_loadu_pd(p + doff[11] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[10] + i0), _mm512_loadu_pd(p + doff[10] + i0)));
        __m512d t6  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[13] + i0), _mm512_loadu_pd(p + doff[13] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[12] + i0), _mm512_loadu_pd(p + doff[12] + i0)));
        __m512d t7  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[15] + i0), _mm512_loadu_pd(p + doff[15] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[14] + i0), _mm512_loadu_pd(p + doff[14] + i0)));
        __m512d t8  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[17] + i0), _mm512_loadu_pd(p + doff[17] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[16] + i0), _mm512_loadu_pd(p + doff[16] + i0)));
        __m512d t9  = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[19] + i0), _mm512_loadu_pd(p + doff[19] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[18] + i0), _mm512_loadu_pd(p + doff[18] + i0)));
        __m512d t10 = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[21] + i0), _mm512_loadu_pd(p + doff[21] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[20] + i0), _mm512_loadu_pd(p + doff[20] + i0)));
        __m512d t11 = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[23] + i0), _mm512_loadu_pd(p + doff[23] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[22] + i0), _mm512_loadu_pd(p + doff[22] + i0)));
        __m512d t12 = _mm512_fmadd_pd(_mm512_loadu_pd(dirbuf[25] + i0), _mm512_loadu_pd(p + doff[25] + i0),
                                      _mm512_mul_pd(_mm512_loadu_pd(dirbuf[24] + i0), _mm512_loadu_pd(p + doff[24] + i0)));
        __m512d ss = hsum13(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12);
        __m512d v = _mm512_fmadd_pd(_mm512_loadu_pd(diagp + i0), _mm512_loadu_pd(p + i0), ss);
        _mm512_mask_storeu_pd(q + i0, m, v);
    }
#else
    for (int64_t k = 0; k < nb; k++) {
        int64_t i0 = (b0 + k) << 3;
        for (int64_t i = i0; i < i0 + 8 && i < N; i++) {
            double s = 0.0;
            for (int d = 0; d < NDIR; d++) s += dirbuf[d][i] * p[doff[d] + i];
            q[i] = s + diagp[i] * p[i];
        }
    }
#endif
}

static void blk_xr(double *restrict x, double *restrict r, double *restrict q,
                   double *restrict pp, double alpha, int64_t N, int64_t nb, int64_t b0) {
#if defined(__AVX512F__)
    __m512d va = _mm512_set1_pd(alpha);
    for (int64_t k = 0; k < nb; k++) {
        int64_t i0 = (b0 + k) << 3;
        __mmask8 m = (i0 + 8 <= N) ? 0xFFu : (__mmask8)((1u << (N - i0)) - 1u);
        __m512d vx = _mm512_mask_loadu_pd(_mm512_setzero_pd(), m, x + i0);
        __m512d vr = _mm512_mask_loadu_pd(_mm512_setzero_pd(), m, r + i0);
        __m512d vp = _mm512_loadu_pd(pp + i0);
        __m512d vq = _mm512_mask_loadu_pd(_mm512_setzero_pd(), m, q + i0);
        _mm512_mask_storeu_pd(x + i0, m, _mm512_fmadd_pd(va, vp, vx));
        _mm512_mask_storeu_pd(r + i0, m, _mm512_fnmadd_pd(va, vq, vr));
    }
#else
    for (int64_t k = 0; k < nb; k++) {
        int64_t i0 = (b0 + k) << 3;
        for (int64_t i = i0; i < i0 + 8 && i < N; i++) {
            x[i] = x[i] + alpha * pp[i];
            r[i] = r[i] - alpha * q[i];
        }
    }
#endif
}

static void blk_pud(double *restrict pp, double *restrict zp, double beta,
                    int64_t N, int64_t nb, int64_t b0) {
    (void)N;
#if defined(__AVX512F__)
    __m512d vb = _mm512_set1_pd(beta);
    for (int64_t k = 0; k < nb; k++) {
        int64_t i0 = (b0 + k) << 3;
        __m512d vz = _mm512_loadu_pd(zp + i0);
        __m512d vp = _mm512_loadu_pd(pp + i0);
        _mm512_storeu_pd(pp + i0, _mm512_fmadd_pd(vb, vp, vz));
    }
#else
    for (int64_t k = 0; k < nb; k++) {
        int64_t i0 = (b0 + k) << 3;
        for (int64_t i = i0; i < i0 + 8; i++) pp[i] = zp[i] + beta * pp[i];
    }
#endif
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
    const int64_t workspace_size) {

    (void)workspace; (void)workspace_size;
    args_t a = { A_data, A_indices, A_indptr, b, x, NX, NY, NZ, niter };
    const int64_t N = NX * NY * NZ;
    if (N <= 0) return;
    if (niter < 0) return;

    int64_t s0, s1, s2;
    dirtab_t T;
    if (!detect_strides(&a, &s0, &s1, &s2) || !build_dirtab(s0, s1, s2, &T)) {
        run_scalar(&a);
        return;
    }

    const int64_t B  = s0 + s1 + s2;
    const int64_t B2 = (B + 7) & ~7;
    const int64_t total = N + 2 * B2 + 8;
    const int64_t nblk = (N + 7) >> 3;

    static int node_of_cpu[1024];
    int nnode = build_node_map(node_of_cpu, 1024); (void)nnode; nnode = 1;
    if (nnode > 8) nnode = 8;
    for (int c = 0; c < 1024; c++) if (node_of_cpu[c] >= nnode) node_of_cpu[c] = 0;
    const size_t awrep = (size_t)NDIR * total * sizeof(double);
    const size_t awtot = ((size_t)(nnode + 1) * NDIR + 4) * total * sizeof(double);
    double *Aw = malloc(awtot);
    if (!Aw) { run_scalar(&a); return; }
    memset(Aw, 0, awrep);
    memset(Aw + (size_t)(nnode + 1) * NDIR * total, 0, (size_t)4 * total * sizeof(double));
    double *yp = Aw + (size_t)(nnode + 1) * NDIR * total;
    double *zp = yp + total;
    double *pp = zp + total;
    double *diagp = pp + total;

    double *r    = malloc((size_t)N * sizeof(double));
    double *q    = malloc((size_t)N * sizeof(double));
    double *diag = malloc((size_t)N * sizeof(double));
    double *invD = malloc((size_t)N * sizeof(double));
    int32_t *omap = malloc((size_t)(2 * B + 1) * sizeof(int32_t));
    if (!r || !q || !diag || !invD || !omap) {
        free(Aw); free(r); free(q); free(diag); free(invD); free(omap);
        run_scalar(&a);
        return;
    }
    memset(omap, 0xFF, (size_t)(2 * B + 1) * sizeof(int32_t));
    for (int d = 0; d < NDIR; d++) omap[T.off[d] + B] = d;

    const double *fwdbuf[8][13];  int64_t foff[8][13];
    const double *bwdbuf[8][13];  int64_t boff[8][13];
    const double *repbuf[8][NDIR]; int64_t aoff[NDIR];
    sweep_desc_t fdesc[8], bdesc[8];
    int bad = 0;
    static int claim[8];
    memset(claim, 0, sizeof(claim));
    for (int k = 0; k < nnode; k++) {
        int nf = 0, nu = 0;
        for (int d = 0; d < NDIR; d++) {
            repbuf[k][d] = Aw + ((size_t)((nnode > 1) ? (1 + k) : k) * NDIR + d) * total + B2;
            aoff[d] = T.off[d];
            if (T.fwd[d]) { fwdbuf[k][nf] = repbuf[k][d]; foff[k][nf] = T.off[d]; nf++; }
            else          { bwdbuf[k][nu] = repbuf[k][d]; boff[k][nu] = T.off[d]; nu++; }
        }
        fdesc[k].buf = fwdbuf[k]; fdesc[k].off = foff[k];
        bdesc[k].buf = bwdbuf[k]; bdesc[k].off = boff[k];
    }
    for (int k = 0; k < nnode; k++)
        if (!sweep_dep(&fdesc[k]) || !sweep_dep(&bdesc[k])) {
            free(Aw); free(r); free(q); free(diag); free(invD); free(omap);
            run_scalar(&a);
            return;
        }

    const int nt = pick_threads(N);
    const int64_t nchunk = (nblk < 4 * (int64_t)nt) ? nblk : 4 * (int64_t)nt;
    const int64_t chunk  = (nblk + nchunk - 1) / nchunk;
    #define MNODE() ({ int _c = sched_getcpu(); \
                       int _n = (_c >= 0 && _c < 1024) ? node_of_cpu[_c] : 0; _n; })

    omp_set_num_threads(nt);
    double rz = 0.0, rznew = 0.0, alpha = 0.0, beta = 0.0, pq = 0.0;

    #pragma omp parallel
    {
        #pragma omp for schedule(static) reduction(|:bad)
        for (int64_t i = 0; i < N; i++) {
            unsigned mask = 0;
            int ok = 1;
            double dv = 0.0;
            for (int64_t k = a.A_indptr[i]; k < a.A_indptr[i + 1]; k++) {
                int64_t o = a.A_indices[k] - i;
                if (o == 0) {
                    dv = a.A_data[k];
                } else if (o < -B || o > B) {
                    ok = 0;
                } else {
                    int d = omap[o + B];
                    if (d < 0) ok = 0;
                    else {
                        unsigned m = (unsigned)1u << d;
                        if (mask & m) ok = 0;
                        mask |= m;
                        Aw[(size_t)d * total + B2 + i] = a.A_data[k];
                    }
                }
            }
            if (dv == 0.0) ok = 0;
            diag[i] = dv;
            diagp[B2 + i] = dv;
            for (int d = 0; d < NDIR; d++)
                if (!(mask & ((unsigned)1u << d))) Aw[(size_t)d * total + B2 + i] = 0.0;
            bad |= !ok;
        }

        /* replicate the coefficient block: every node gets its own copy of
         * the base replica; the node's own threads do the copy, so
         * first-touch NUMA placement puts the replica on that node */
        if (nnode > 1) {
            int cpu = sched_getcpu();
            int k = (cpu >= 0 && cpu < 1024) ? node_of_cpu[cpu] : 0;
            for (;;) {
                int i = __atomic_fetch_add(&claim[k], 1, __ATOMIC_RELAXED);
                if (i >= NDIR) break;
                memcpy(Aw + ((size_t)(1 + k) * NDIR + i) * total, Aw + (size_t)i * total,
                       (size_t)total * sizeof(double));
            }
            #pragma omp barrier
            /* repair replicas of nodes that had no thread at claim time */
            #pragma omp single
            for (int kk = 0; kk < nnode; kk++)
                while (claim[kk] < NDIR) {
                    int i = __atomic_fetch_add(&claim[kk], 1, __ATOMIC_RELAXED);
                    if (i < NDIR)
                        memcpy(Aw + ((size_t)(1 + kk) * NDIR + i) * total,
                               Aw + (size_t)i * total, (size_t)total * sizeof(double));
                }
            #pragma omp barrier
        }

        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) invD[i] = 1.0 / diag[i];

        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) { x[i] = 0.0; r[i] = b[i]; }

        #pragma omp single
        {
            int nd = MNODE();
            sweep_simd(yp + B2, r, NULL, invD, &fdesc[nd], N, 1);
            sweep_simd(zp + B2, diag, yp + B2, invD, &bdesc[nd], N, 0);
        }

        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) pp[B2 + i] = zp[B2 + i];

        static double dslot[256];
        double dpart = 0.0;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i += 32)
#if defined(__AVX512F__)
            DOT32(r, zp + B2, i, dpart);
#else
            DOTTAIL(r, zp + B2, i, dpart);
#endif
        dslot[omp_get_thread_num()] = dpart;
        #pragma omp barrier
        #pragma omp single
        {
            double s = 0.0;
            for (int t = 0; t < nt; t++) { s += dslot[t]; dslot[t] = 0.0; }
            rz = s;
        }

        int64_t it;
        for (it = 0; it < niter; it++) {
            #pragma omp for schedule(static)
            for (int64_t c = 0; c < nchunk; c++) {
                int64_t b0 = c * chunk;
                int64_t nb = nblk - b0; if (nb > chunk) nb = chunk;
                blk_spmv(q, pp + B2, diagp + B2, repbuf[MNODE()], aoff, N, nb, b0);
            }
            dpart = 0.0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i += 32)
#if defined(__AVX512F__)
                DOT32(pp + B2, q, i, dpart);
#else
                DOTTAIL(pp + B2, q, i, dpart);
#endif
            dslot[omp_get_thread_num()] = dpart;
            #pragma omp barrier
            #pragma omp single
            {
                double s = 0;
                for (int t = 0; t < nt; t++) { s += dslot[t]; dslot[t] = 0.0; }
                alpha = rz / (pq = s);
            }

            #pragma omp for schedule(static)
            for (int64_t c = 0; c < nchunk; c++) {
                int64_t b0 = c * chunk;
                int64_t nb = nblk - b0; if (nb > chunk) nb = chunk;
                blk_xr(x, r, q, pp + B2, alpha, N, nb, b0);
            }

            #pragma omp single
            {
                int nd = MNODE();
                sweep_simd(yp + B2, r, NULL, invD, &fdesc[nd], N, 1);
                sweep_simd(zp + B2, diag, yp + B2, invD, &bdesc[nd], N, 0);
            }
            dpart = 0.0;
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i += 32)
#if defined(__AVX512F__)
                DOT32(r, zp + B2, i, dpart);
#else
                DOTTAIL(r, zp + B2, i, dpart);
#endif
            dslot[omp_get_thread_num()] = dpart;
            #pragma omp barrier
            #pragma omp single
            {
                double s = 0;
                for (int t = 0; t < nt; t++) { s += dslot[t]; dslot[t] = 0.0; }
                beta = (rznew = s) / rz;
            }

            #pragma omp for schedule(static)
            for (int64_t c = 0; c < nchunk; c++) {
                int64_t b0 = c * chunk;
                int64_t nb = nblk - b0; if (nb > chunk) nb = chunk;
                blk_pud(pp + B2, zp + B2, beta, N, nb, b0);
            }
            #pragma omp single
            { rz = rznew; }
        }
    }

    if (bad) {
        free(Aw); free(r); free(q); free(diag); free(invD); free(omap);
        run_scalar(&a);
        return;
    }

    free(Aw); free(r); free(q); free(diag); free(invD); free(omap);
}
