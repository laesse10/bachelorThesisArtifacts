/* Full-reorthogonalization Lanczos on a CSR 7-point stencil operator.
 *
 * Layout note: Q is N x m row-major, so a column of Q is strided.  We keep a
 * transposed copy qt (m rows of N contiguous doubles, qt[p*N+i] == Q[i,p]) in
 * the workspace so that every pass over the Krylov basis streams memory
 * contiguously, and write Q once at the end.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#include <omp.h>

#ifdef __AVX512F__
#include <immintrin.h>
static inline double vdot(const double *restrict a, const double *restrict b,
                          const int64_t n) {
    __m512d s0 = _mm512_setzero_pd(), s1 = _mm512_setzero_pd(),
            s2 = _mm512_setzero_pd(), s3 = _mm512_setzero_pd();
    int64_t i = 0;
    for (; i + 32 <= n; i += 32) {
        s0 = _mm512_fmadd_pd(_mm512_loadu_pd(a + i), _mm512_loadu_pd(b + i), s0);
        s1 = _mm512_fmadd_pd(_mm512_loadu_pd(a + i + 8), _mm512_loadu_pd(b + i + 8), s1);
        s2 = _mm512_fmadd_pd(_mm512_loadu_pd(a + i + 16), _mm512_loadu_pd(b + i + 16), s2);
        s3 = _mm512_fmadd_pd(_mm512_loadu_pd(a + i + 24), _mm512_loadu_pd(b + i + 24), s3);
    }
    for (; i + 8 <= n; i += 8)
        s0 = _mm512_fmadd_pd(_mm512_loadu_pd(a + i), _mm512_loadu_pd(b + i), s0);
    __m512d s = _mm512_add_pd(_mm512_add_pd(s0, s1), _mm512_add_pd(s2, s3));
    double r = _mm512_reduce_add_pd(s);
    for (; i < n; i++) r += a[i] * b[i];
    return r;
}
#else
static inline double vdot(const double *restrict a, const double *restrict b,
                          const int64_t n) {
    double s = 0.0;
    #pragma omp simd reduction(+:s)
    for (int64_t i = 0; i < n; i++) s += a[i] * b[i];
    return s;
}
#endif

#ifdef __AVX512F__
/* dst[i] = s * src[i]: streaming stores (first-touch writes, no RFO read) */
static inline void vstream(double *restrict dst, const double *restrict src,
                           const double s, const int64_t n) {
    const __m512d vs = _mm512_set1_pd(s);
    int64_t i = 0;
    while (i < n && ((uintptr_t)(dst + i) & 63u)) { dst[i] = s * src[i]; i++; }
    for (; i + 8 <= n; i += 8)
        _mm512_stream_pd(dst + i, _mm512_mul_pd(vs, _mm512_loadu_pd(src + i)));
    for (; i < n; i++) dst[i] = s * src[i];
}
#else
static inline void vstream(double *restrict dst, const double *restrict src,
                           const double s, const int64_t n) {
    for (int64_t i = 0; i < n; i++) dst[i] = s * src[i];
}
#endif

#define OFF_NONE 255

void lanczos_reorth_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    double *restrict Q,
    double *restrict alpha,
    const double *restrict b,
    double *restrict beta,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t m,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    const int64_t N = NX * NY * NZ;
    const size_t NN = (size_t)N;
    if (N <= 0 || m <= 0) return;

    int ntmax = omp_get_max_threads();
    if (ntmax < 1) ntmax = 1;
    /* stride of a per-thread partial row; 64B-aligned so adjacent threads'
     * partials never share a cache line (false sharing) */
    const size_t PSTR = (size_t)(((m + 2) + 7) & ~(int64_t)7);

    size_t need = sizeof(double) * ((size_t)m * NN + NN
                                    + PSTR * (size_t)ntmax * 2u
                                    + (size_t)(m + 1) * (size_t)ntmax * 2u)
                + sizeof(int) * (size_t)ntmax + 64;
    uint8_t *ws = workspace;
    int owns = 0;
    if (ws == NULL || (size_t)workspace_size < need) {
        ws = (uint8_t *)malloc(need);
        if (ws == NULL) return;
        owns = 1;
    }
    double *qt   = (double *)ws;                     /* qt[p*N+i] = Q[i,p] */
    double *w    = qt + (size_t)m * NN;
    double *part = w + NN;                           /* buf A: [tid*PSTR + p] */
    double *pb   = part + PSTR * (size_t)ntmax;      /* buf B: [tid*PSTR + p] */
    double *dv   = pb + PSTR * (size_t)ntmax;        /* [tid*(m+1)*2 + p] d1/d2 */
    int *flags   = (int *)(dv + (size_t)(m + 1) * (size_t)ntmax * 2u);

    /* position of each of the 7 stencil slots inside a CSR row, per neighbour mask */
    uint8_t tbl[64][7];
    uint8_t cnt[64];
    for (int mask = 0; mask < 64; mask++) {
        int t = 0;
        tbl[mask][0] = (mask & 1)  ? (uint8_t)t++ : OFF_NONE; /* x-1 */
        tbl[mask][1] = (mask & 4)  ? (uint8_t)t++ : OFF_NONE; /* y-1 */
        tbl[mask][2] = (mask & 16) ? (uint8_t)t++ : OFF_NONE; /* z-1 */
        tbl[mask][3] = (uint8_t)t++;                          /* diag */
        tbl[mask][4] = (mask & 32) ? (uint8_t)t++ : OFF_NONE; /* z+1 */
        tbl[mask][5] = (mask & 8)  ? (uint8_t)t++ : OFF_NONE; /* y+1 */
        tbl[mask][6] = (mask & 2)  ? (uint8_t)t++ : OFF_NONE; /* x+1 */
        cnt[mask] = (uint8_t)t;
    }

    double ss_all = 0.0, nrm_sh = 0.0;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int nt  = omp_get_num_threads();
        const int64_t i0 = (int64_t)(((__int128)N * tid) / nt);
        const int64_t i1 = (int64_t)(((__int128)N * (tid + 1)) / nt);

        double *pa   = part + PSTR * (size_t)tid;
        double *pbuf = pb   + PSTR * (size_t)tid;
        double *d1   = dv + (size_t)(m + 1) * 2u * (size_t)tid;
        double *d2   = d1 + (m + 1);

        /* ---- initial vector (fused into this region; streaming stores) ---- */
        {
            #pragma omp for reduction(+:ss_all) schedule(static)
            for (int64_t i = 0; i < N; i++) ss_all += b[i] * b[i];
            #pragma omp single
            nrm_sh = sqrt(ss_all);
            const double nrm = nrm_sh;
            #pragma omp for schedule(static) nowait
            for (int64_t c = 0; c < N; c += 4096) {
                const int64_t ce = (c + 4096 < N) ? c + 4096 : N;
                vstream(qt + c, b + c, 1.0 / nrm, ce - c);
            }
            _mm_sfence();
            #pragma omp barrier
        }

        /* ---- does the CSR exactly match the 7-point stencil? ---- */
        {
            int ok = 1;
            const int64_t sX = NY * NZ, sY = NZ;
            int64_t i = i0, x, y, z;
            z = i % NZ;
            int64_t r = i / NZ;
            y = r % NY;
            x = r / NY;
            for (; i < i1 && ok; ) {
                const int64_t ke = A_indptr[i + 1];
                int64_t t = 0;
                if (x > 0)      { if (A_indices[A_indptr[i] + t] != i - sX) ok = 0; t++; }
                if (y > 0)      { if (A_indices[A_indptr[i] + t] != i - sY) ok = 0; t++; }
                if (z > 0)      { if (A_indices[A_indptr[i] + t] != i - 1)   ok = 0; t++; }
                                  t++;
                if (z + 1 < NZ) { if (A_indices[A_indptr[i] + t] != i + 1)   ok = 0; t++; }
                if (y + 1 < NY) { if (A_indices[A_indptr[i] + t] != i + sY) ok = 0; t++; }
                if (x + 1 < NX) { if (A_indices[A_indptr[i] + t] != i + sX) ok = 0; t++; }
                if (ke - A_indptr[i] != t) ok = 0;
                z++; i++;
                if (z == NZ) { z = 0; y++; if (y == NY) { y = 0; x++; } }
            }
            flags[tid] = ok;
        }
        #pragma omp barrier
        int stencil = 1;
        for (int t = 0; t < nt; t++) stencil = stencil && flags[t];
        const int64_t sX = NY * NZ, sY = NZ;
        int64_t ip0 = A_indptr[i0];

        double beta_prev = 0.0;

        for (int64_t j = 0; j < m; j++) {
            const double *q     = qt + (size_t)j * NN;
            const double *qprev = qt + (size_t)(j - 1) * NN;
            const double bp = (j > 0) ? beta_prev : 0.0;

            /* ---- sparse matvec: w = A*q - beta_prev*q_prev ---- */
            if (stencil) {
                int64_t i = i0, x, y, z, ip = ip0;
                z = i % NZ;
                int64_t r = i / NZ;
                y = r % NY;
                x = r / NY;
                while (i < i1) {
                    const int interior = (x > 0 && x + 1 < NX && y > 0 && y + 1 < NY);
                    if (interior && z >= 1 && z + 1 < NZ) {
                        /* bulk: 7 plane values per row, unit-stride streams */
                        int64_t zend = NZ - 1;
                        if (zend - z > i1 - i) zend = z + (i1 - i);
                        const int64_t len = zend - z;
                        const int64_t ibase = i;
                        const double *rdb = A_data + ip;
                        if (j > 0) {
                            for (int64_t k = 0; k < len; k++) {
                                const int64_t ii = ibase + k;
                                const double *rr = rdb + 7 * k;
                                w[ii] = rr[0] * q[ii - sX]
                                      + rr[1] * q[ii - sY]
                                      + rr[2] * q[ii - 1]
                                      + rr[3] * q[ii]
                                      + rr[4] * q[ii + 1]
                                      + rr[5] * q[ii + sY]
                                      + rr[6] * q[ii + sX]
                                      - bp * qprev[ii];
                            }
                        } else {
                            for (int64_t k = 0; k < len; k++) {
                                const int64_t ii = ibase + k;
                                const double *rr = rdb + 7 * k;
                                w[ii] = rr[0] * q[ii - sX]
                                      + rr[1] * q[ii - sY]
                                      + rr[2] * q[ii - 1]
                                      + rr[3] * q[ii]
                                      + rr[4] * q[ii + 1]
                                      + rr[5] * q[ii + sY]
                                      + rr[6] * q[ii + sX];
                            }
                        }
                        i += len;
                        z = zend;
                        ip += 7 * len;
                    } else {
                        const int64_t mx = (x > 0 ? 1 : 0) | (x + 1 < NX ? 2 : 0)
                                         | (y > 0 ? 4 : 0) | (y + 1 < NY ? 8 : 0)
                                         | (z > 0 ? 16 : 0) | (z + 1 < NZ ? 32 : 0);
                        const uint8_t *o = tbl[mx];
                        const double *rd = A_data + ip;
                        double acc = rd[o[3]] * q[i];
                        if (mx & 1)  acc += rd[o[0]] * q[i - sX];
                        if (mx & 4)  acc += rd[o[1]] * q[i - sY];
                        if (mx & 16) acc += rd[o[2]] * q[i - 1];
                        if (mx & 32) acc += rd[o[4]] * q[i + 1];
                        if (mx & 8)  acc += rd[o[5]] * q[i + sY];
                        if (mx & 2)  acc += rd[o[6]] * q[i + sX];
                        w[i] = acc - bp * qprev[i];
                        ip += cnt[mx];
                        z++; i++;
                        if (z == NZ) { z = 0; y++; if (y == NY) { y = 0; x++; } }
                    }
                }
            } else {
                for (int64_t i = i0; i < i1; i++) {
                    double acc = 0.0;
                    const int64_t ke = A_indptr[i + 1];
                    for (int64_t k = A_indptr[i]; k < ke; k++)
                        acc += A_data[k] * q[A_indices[k]];
                    w[i] = acc - bp * qprev[i];
                }
            }

            /* ---- pass 1: dots of w against every column built so far ---- */
            for (int64_t p = 0; p <= j; p++) {
                const double *row = qt + (size_t)p * NN + i0;
                pa[p] = vdot(row, w + i0, i1 - i0);
            }
            #pragma omp barrier
            for (int64_t p = 0; p <= j; p++) {
                double s = 0.0;
                for (int t = 0; t < nt; t++) s += part[PSTR * (size_t)t + p];
                d1[p] = s;
            }
            if (tid == 0) alpha[j] = d1[j];

            /* ---- fused sweep: apply d1 and, blocked over i for L2, recompute
             *      the dots against the fully updated w (all rows per block).
             *      Also accumulates ||w1|^2 for the step norm below. ---- */
            for (int64_t p = 0; p <= j; p++) pbuf[p] = 0.0;
            double ss1 = 0.0;
            {
                /* block length: keep the (j+1) x B panel piece inside L2 */
                int64_t B = 49152 / (j + 1);
                if (B > 8192) B = 8192;
                if (B < 64) B = 64;
                B &= ~(int64_t)7;
                for (int64_t a = i0; a < i1; a += B) {
                    const int64_t e = (a + B < i1) ? a + B : i1;
                    for (int64_t p = 0; p <= j; p++) {
                        const double *row = qt + (size_t)p * NN;
                        const double dp = d1[p];
                        #pragma omp simd
                        for (int64_t i = a; i < e; i++) w[i] -= dp * row[i];
                    }
                    for (int64_t p = 0; p <= j; p++) {
                        const double *row = qt + (size_t)p * NN + a;
                        pbuf[p] += vdot(row, w + a, e - a);
                    }
                    ss1 += vdot(w + a, w + a, e - a);
                }
            }
            pbuf[m + 1] = ss1;
            #pragma omp barrier
            double b_j, inv;
            {
                double dd = 0.0;
                for (int64_t p = 0; p <= j; p++) {
                    double s = 0.0;
                    for (int t = 0; t < nt; t++) s += pb[PSTR * (size_t)t + p];
                    d2[p] = s;
                    dd += s * s;
                }
                double ss = 0.0;
                for (int t = 0; t < nt; t++) ss += pb[PSTR * (size_t)t + m + 1];
                /* |w2|^2 = |w1|^2 - |d2|^2  (w2 = w1 - Q d2, and Q^T w1 = d2) */
                b_j = sqrt(ss - dd > 0.0 ? ss - dd : 0.0);
                inv = 1.0 / b_j;
                if (tid == 0) beta[j] = b_j;
            }

            /* ---- apply d2, then scale into the next Krylov column ---- */
            for (int64_t p = 0; p <= j; p++) {
                const double *row = qt + (size_t)p * NN;
                const double dp = d2[p];
                #pragma omp simd
                for (int64_t i = i0; i < i1; i++) w[i] -= dp * row[i];
            }
            if (j + 1 < m) {
                double *row = qt + (size_t)(j + 1) * NN;
                vstream(row + i0, w + i0, inv, i1 - i0);
                _mm_sfence();
            }
            beta_prev = b_j;
            #pragma omp barrier
        }

        /* ---- transpose qt (m x N) back into Q (N x m), streaming ---- */
        #pragma omp for schedule(static) nowait
        for (int64_t ib = 0; ib < N; ib += 8) {
            const int64_t ie = (ib + 8 < N) ? ib + 8 : N;
            const int64_t nr = ie - ib;
            if (m <= 128) {
                double tmp[8 * 128];
                for (int64_t p = 0; p < m; p++) {
                    const double *row = qt + (size_t)p * NN;
                    for (int64_t i = 0; i < nr; i++)
                        tmp[(size_t)i * m + p] = row[ib + i];
                }
                vstream(Q + (size_t)ib * m, tmp, 1.0, (int64_t)((size_t)nr * m));
            } else {
                for (int64_t p = 0; p < m; p++) {
                    const double *row = qt + (size_t)p * NN;
                    for (int64_t i = ib; i < ie; i++)
                        Q[(size_t)i * (size_t)m + (size_t)p] = row[i];
                }
            }
        }
        _mm_sfence();
    }

    if (owns) free(ws);
}
