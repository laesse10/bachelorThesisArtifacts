#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#if defined(__AVX2__) && defined(__FMA__)
#include <immintrin.h>
#define HQR_VEC 1
#endif

/* Householder QR factorization + least-squares solve, row-major (i,j) at i*N+j.
 *
 * Identical math to the reference, step for step:
 *   for k = 0..N-1:
 *     v = A[k:M,k] (in a buffer), v[0] = A[k,k] + sign*||A[k:M,k]||
 *     beta = 2/(v.v)  (0 if v.v == 0)
 *     w = beta * (v @ A[k:M, k:N]);  A[k:M, k:N] -= outer(v, w)
 *   Q = I (M x N);  for k = N-1..0: Q[k:M,:] -= beta_k * outer(v_k, v_k @ Q[k:M,:])
 *   g = Q^T b;  back-substitute R x = g.
 *
 * Each rank-1 application is two streaming passes (dot, then update) over the
 * sub-block, split over OpenMP row chunks; per-thread partial dots are reduced
 * by the master after a barrier before the update pass.
 */

enum { HQR_NCAP = 1024 };                     /* vector scratch capacity (N cap) */
enum { HQR_TMAX = 512 };                      /* max threads using the scratch  */
static double hqr_wp[(size_t)HQR_TMAX * (HQR_NCAP + 8)];

/* w[j] = scale * sum_i x[i] * base[i*stride + j0 + j], i in [0,len), j in [0,nc) */
static void hqr_dot(double *w, const double *base, long j0, const double *x,
                    long len, long nc, long stride, double scale, long nchunks,
                    int nt) {
    long CH = (len + nchunks - 1) / nchunks;
    if (CH < 1) CH = 1;
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        double *wp = hqr_wp + (size_t)tid * (HQR_NCAP + 8);
        #pragma omp for nowait schedule(static)
        for (long c = 0; c < nchunks; ++c) {
            long r0 = c * CH;
            long r1 = r0 + CH;
            if (r1 > len) r1 = len;
#ifdef HQR_VEC
            /* two 4-column groups per row sweep: the second half of each
             * 64B line is then an L1 hit instead of a fresh L3 fetch */
            for (long j = 0; j + 8 <= nc; j += 8) {
                __m256d accA = _mm256_setzero_pd();
                __m256d accB = _mm256_setzero_pd();
                for (long i = r0; i < r1; ++i) {
                    const double *row = base + i * stride + j0 + j;
                    __m256d xv = _mm256_set1_pd(x[i]);
                    accA = _mm256_fmadd_pd(_mm256_loadu_pd(row), xv, accA);
                    accB = _mm256_fmadd_pd(_mm256_loadu_pd(row + 4), xv, accB);
                }
                __m256d sA = _mm256_loadu_pd(wp + j);
                __m256d sB = _mm256_loadu_pd(wp + j + 4);
                _mm256_storeu_pd(wp + j, _mm256_add_pd(sA, accA));
                _mm256_storeu_pd(wp + j + 4, _mm256_add_pd(sB, accB));
            }
            for (long j = (nc / 8) * 8; j + 4 <= nc; j += 4) {
                __m256d acc = _mm256_setzero_pd();
                for (long i = r0; i < r1; ++i) {
                    const double *row = base + i * stride + j0 + j;
                    acc = _mm256_fmadd_pd(_mm256_loadu_pd(row), _mm256_set1_pd(x[i]), acc);
                }
                __m256d s = _mm256_loadu_pd(wp + j);
                _mm256_storeu_pd(wp + j, _mm256_add_pd(s, acc));
            }
            for (long j = (nc / 4) * 4; j < nc; ++j) {
                double acc = 0.0;
                for (long i = r0; i < r1; ++i)
                    acc += (base + i * stride + j0 + j)[0] * x[i];
                wp[j] += acc;
            }
#else
            for (long i = r0; i < r1; ++i) {
                const double *row = base + i * stride + j0;
                for (long j = 0; j < nc; ++j) wp[j] += row[j] * x[i];
            }
#endif
        }
        #pragma omp barrier
        if (tid == 0) {
            for (long j = 0; j < nc; ++j) {
                double s = 0.0;
                for (int t = 0; t < nt; ++t) s += hqr_wp[(size_t)t * (HQR_NCAP + 8) + j];
                w[j] = scale * s;
            }
        }
    }
}

static void hqr_wp_zero(long nc) {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        double *wp = hqr_wp + (size_t)tid * (HQR_NCAP + 8);
#ifdef HQR_VEC
        for (long j = 0; j < nc + 4; j += 4) {
            double z[4] = {0.0, 0.0, 0.0, 0.0};
            _mm256_storeu_pd(wp + j, _mm256_loadu_pd(z));
        }
#else
        for (long j = 0; j < nc; ++j) wp[j] = 0.0;
#endif
    }
}

/* base[i*stride + j0 + j] -= x[i] * w[j] */
static void hqr_update(double *base, long j0, const double *x, const double *w,
                       long len, long nc, long stride, long nchunks) {
    long CH = (len + nchunks - 1) / nchunks;
    if (CH < 1) CH = 1;
    #pragma omp parallel for schedule(static)
    for (long c = 0; c < nchunks; ++c) {
        long r0 = c * CH;
        long r1 = r0 + CH;
        if (r1 > len) r1 = len;
#ifdef HQR_VEC
        /* two 4-column groups per row sweep: the second half of each
         * 64B line is then an L1 hit instead of a fresh L3 fetch */
        const __m256d SIGN = _mm256_set1_pd(-0.0);
        for (long j = 0; j + 8 <= nc; j += 8) {
            double wbuf[4] = {0.0, 0.0, 0.0, 0.0};
            double wbuf2[4] = {0.0, 0.0, 0.0, 0.0};
            for (long q = 0; q < 4; ++q) { wbuf[q] = w[j + q]; wbuf2[q] = w[j + 4 + q]; }
            __m256d wvA = _mm256_loadu_pd(wbuf);
            __m256d wvB = _mm256_loadu_pd(wbuf2);
            for (long i = r0; i < r1; ++i) {
                double *row = base + i * stride + j0 + j;
                __m256d ngt = _mm256_xor_pd(_mm256_set1_pd(x[i]), SIGN);
                __m256d a = _mm256_loadu_pd(row);
                a = _mm256_fmadd_pd(ngt, wvA, a);
                _mm256_storeu_pd(row, a);
                __m256d b = _mm256_loadu_pd(row + 4);
                b = _mm256_fmadd_pd(ngt, wvB, b);
                _mm256_storeu_pd(row + 4, b);
            }
        }
        for (long j = (nc / 8) * 8; j + 4 <= nc; j += 4) {
            double wbuf[4] = {0.0, 0.0, 0.0, 0.0};
            for (long q = 0; q < 4; ++q) wbuf[q] = w[j + q];
            __m256d wv = _mm256_loadu_pd(wbuf);
            for (long i = r0; i < r1; ++i) {
                double *row = base + i * stride + j0 + j;
                __m256d a = _mm256_loadu_pd(row);
                a = _mm256_fmadd_pd(_mm256_xor_pd(_mm256_set1_pd(x[i]), SIGN), wv, a);
                _mm256_storeu_pd(row, a);
            }
        }
        for (long j = (nc / 4) * 4; j < nc; ++j) {
            for (long i = r0; i < r1; ++i) {
                double *row = base + i * stride + j0 + j;
                row[0] -= x[i] * w[j];
            }
        }
#else
        for (long i = r0; i < r1; ++i) {
            double *row = base + i * stride + j0;
            for (long j = 0; j < nc; ++j) row[j] -= x[i] * w[j];
        }
#endif
    }
}

void householder_qr_fp64(
    double *restrict A,
    double *restrict Q,
    double *restrict R,
    const double *restrict b,
    double *restrict x,
    const int64_t M,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {

    const int nt = omp_get_max_threads();
    const bool par = (N + 8 <= HQR_NCAP) && (nt <= HQR_TMAX);
    /* chunk count = mult * threads; measured per-size: 192 wins only for the
     * largest wide (N=256) cases, 128 is best (or within noise) everywhere else. */
    const long mult = (N == 256 && M >= 31000) ? 192L : 128L;
    const long nchunks = par ? mult * nt : 1;

    /* scratch: all v_k (vtotal doubles), then beta/w/g vectors, then offsets. */
    const int64_t vtotal = M * N - N * (N - 1) / 2;
    const int64_t need = vtotal + 5 * N + 16;   /* vs | beta | w | g | off (N int64) */
    double *sb;
    bool used_ws = false;
    if (workspace && workspace_size >= need * (int64_t)sizeof(double)) {
        sb = (double *)workspace;
        used_ws = true;
    } else {
        sb = (double *)malloc((size_t)need * sizeof(double));
    }
    double *vs   = sb;
    double *beta = sb + vtotal;
    double *w    = sb + vtotal + N + 8;
    double *g    = sb + vtotal + 2 * N + 16;
    int64_t *off = (int64_t *)(sb + vtotal + 3 * N + 16);
    for (int64_t j = 0; j < N + 8; ++j) w[j] = 0.0;
    int64_t pos = 0;
    for (int64_t k = 0; k < N; ++k) { off[k] = pos; pos += M - k; }

    /* ================= A factorization ================= */
    for (int64_t k = 0; k < N; ++k) {
        const long len = (long)(M - k);
        const long nc  = (long)(N - k);
        double *Akrow  = A + k * N;
        double *vk     = vs + off[k];

        /* one strided pass over column k: store vk, accumulate rest = sum_{i>=1} col[i]^2 */
        const double a00 = Akrow[k];
        vk[0] = a00;
        double rest = 0.0;
        /* strided column copy + sum-of-squares in one pass (stride N, scalar) */
        for (long i = 1; i < len; ++i) {
            double a = A[(k + i) * N + k];
            vk[i] = a;
            rest = fma(a, a, rest);
        }
        const double n2 = a00 * a00 + rest;
        const double normx = sqrt(n2);
        const double s = (a00 >= 0.0) ? 1.0 : -1.0;
        const double v0 = a00 + s * normx;
        vk[0] = v0;
        const double vv = v0 * v0 + rest;
        beta[k] = (vv > 0.0) ? (2.0 / vv) : 0.0;
        if (vv <= 0.0) continue;

        if (par) {
            hqr_wp_zero(nc);
            hqr_dot(w, Akrow, k, vk, len, nc, (long)N, beta[k], nchunks, nt);
            hqr_update(Akrow, k, vk, w, len, nc, (long)N, nchunks);
        } else {
            /* small/serial fallback: w = beta * (vk . each column), then update */
            for (long j = 0; j < nc; ++j) {
                double d = 0.0;
                for (long i = 0; i < len; ++i) d = fma(vk[i], A[(k + i) * N + k + j], d);
                w[j] = beta[k] * d;
            }
            for (long i = 0; i < len; ++i) {
                double *row = A + (k + i) * N + k;
                for (long j = 0; j < nc; ++j) row[j] -= vk[i] * w[j];
            }
        }
    }

    /* R = upper triangle of factored A */
    for (int64_t i = 0; i < N; ++i)
        memcpy(R + i * N + i, A + i * N + i, (size_t)(N - i) * sizeof(double));

    /* ================= Q accumulation ================= */
    memset(Q, 0, (size_t)M * (size_t)N * sizeof(double));
    for (int64_t i = 0; i < N; ++i) Q[i * N + i] = 1.0;

    if (par) {
        for (int64_t k = N - 1; k >= 0; --k) {
            const long len = (long)(M - k);
            const double bk = beta[k];
            if (bk == 0.0) continue;
            double *vk = vs + off[k];
            hqr_wp_zero((long)N);
            hqr_dot(w, Q + k * N, 0, vk, len, (long)N, (long)N, bk, nchunks, nt);
            hqr_update(Q + k * N, 0, vk, w, len, (long)N, (long)N, nchunks);
        }
    } else {
        for (int64_t k = N - 1; k >= 0; --k) {
            const long len = (long)(M - k);
            const double bk = beta[k];
            if (bk == 0.0) continue;
            double *vk = vs + off[k];
            for (long j = 0; j < N; ++j) {
                double d = 0.0;
                for (long i = 0; i < len; ++i) d = fma(vk[i], Q[(k + i) * N + j], d);
                w[j] = bk * d;
            }
            for (long i = 0; i < len; ++i) {
                double *row = Q + (k + i) * N;
                for (long j = 0; j < N; ++j) row[j] -= vk[i] * w[j];
            }
        }
    }

    /* ================= g = Q^T b, back-substitute ================= */
    if (par) {
        hqr_wp_zero((long)N);
        hqr_dot(g, Q, 0, b, (long)M, (long)N, (long)N, 1.0, nchunks, nt);
    } else {
        for (long j = 0; j < N; ++j) {
            double d = 0.0;
            for (long i = 0; i < M; ++i) d = fma(Q[i * N + j], b[i], d);
            g[j] = d;
        }
    }
    for (int64_t i = N - 1; i >= 0; --i) {
        double ssum = 0.0;
        for (int64_t j = i + 1; j < N; ++j) ssum = fma(R[i * N + j], x[j], ssum);
        x[i] = (g[i] - ssum) / R[i * N + i];
    }

    if (!used_ws) free(sb);
}
