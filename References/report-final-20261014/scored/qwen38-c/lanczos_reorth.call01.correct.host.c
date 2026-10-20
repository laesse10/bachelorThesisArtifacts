#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#ifdef __AVX512F__
#include <immintrin.h>
#endif

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
    if (N <= 0 || m <= 0) return;

    int64_t T = omp_get_max_threads();
    if (T < 1) T = 1;
    if (T > N / 8) T = (N / 8) > 0 ? N / 8 : 1;
    const int64_t PS = ((m + 1 + 15) / 16) * 16; /* part row stride, 128B-aligned */
    int64_t need = 45 * N + T * PS * 8 + 8 * T + 256;
    int have_ws = (workspace != NULL) && workspace_size >= need;
    char *base = have_ws ? (char *)workspace : (char *)malloc((size_t)need);
    double *restrict w = (double *)base;
    double *restrict qprev = (double *)(base + 8 * N);
    double *restrict part = (double *)(base + 16 * N);      /* T*PS */
    double *restrict snp = part + T * PS;                   /* T    */
    int32_t *restrict off32 = (int32_t *)(snp + T);         /* 7N (d*m) */
    uint8_t *restrict nnz1 = (uint8_t *)(base + 16 * N + T * PS * 8 + T * 8 + 28 * N);
    int64_t i; /* shared worksharing-for loop variable (must not be private) */

    /* q_0 = b / ||b|| (column 0, row-major N x m) */
    double nrm = 0.0;
    #pragma omp parallel for schedule(static) reduction(+:nrm)
    for (int64_t i = 0; i < N; i++) nrm += b[i] * b[i];
    nrm = sqrt(nrm);
    const double inv_nrm = 1.0 / nrm;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) Q[i * m] = b[i] * inv_nrm;

    /* ---- one-time structure check: is A a "shift" matrix (7-point stencil)?
       row i entries only at i + d, d in {0, +-1, +-NZ, +-NZ*NY}.
       Precompute off32[i*7+k] = d*m (element offset into Q for column j). */
    int stencil = 1;
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        int bad = 0;
        #pragma omp for schedule(static)
        for (i = 0; i < N; i++) {
            const int64_t beg = A_indptr[i], end = A_indptr[i + 1];
            const int64_t nnz = end - beg;
            int64_t k;
            if (nnz > 7) { bad = 1; }
            for (k = beg; k < end && !bad; k++) {
                const int64_t d = A_indices[k] - i;
                if (d != 0 && d != 1 && d != -1 && d != NZ && d != -NZ &&
                    d != NZ * NY && d != -(NZ * NY))
                    bad = 1;
            }
            nnz1[i] = (uint8_t)nnz;
            if (!bad)
                for (k = beg; k < end; k++)
                    off32[i * 7 + (k - beg)] = (int32_t)((A_indices[k] - i) * m);
        }
        part[tid] = bad ? 1.0 : 0.0;
    }
    for (int64_t t = 0; t < T; t++) if (part[t] != 0.0) stencil = 0;

    /* subtile rows: per-thread chunk R/T rows x 8m bytes ~ 714KB (L2);
       per-CCD working set ~ 1.4MB (L3) regardless of m. */
    int64_t R = 1250000 / m;
    if (R < 256) R = 256;
    if (R > N) R = N;
    const int64_t S = (N + R - 1) / R;

    double beta_prev = 0.0;
    for (int64_t j = 0; j < m; j++) {
        double sn;

        /* 1) w = A q_j  (- beta_prev q_{j-1} for j>0) */
        if (stencil) {
            #pragma omp parallel for schedule(static)
            for (i = 0; i < N; i++) {
                const int64_t beg = A_indptr[i];
                const int nnz = nnz1[i];
                const int64_t fb = (size_t)i * 7;
                const int64_t obase = i * m + j;
                const double v0 = A_data[beg] * Q[obase + off32[fb]];
                const double v1 = nnz > 1 ? A_data[beg + 1] * Q[obase + off32[fb + 1]] : 0.0;
                const double v2 = nnz > 2 ? A_data[beg + 2] * Q[obase + off32[fb + 2]] : 0.0;
                const double v3 = nnz > 3 ? A_data[beg + 3] * Q[obase + off32[fb + 3]] : 0.0;
                const double v4 = nnz > 4 ? A_data[beg + 4] * Q[obase + off32[fb + 4]] : 0.0;
                const double v5 = nnz > 5 ? A_data[beg + 5] * Q[obase + off32[fb + 5]] : 0.0;
                const double v6 = nnz > 6 ? A_data[beg + 6] * Q[obase + off32[fb + 6]] : 0.0;
                double acc = (v0 + v1) + (v2 + v3) + (v4 + v5) + v6;
                if (j > 0) acc = acc - beta_prev * qprev[i];
                w[i] = acc;
            }
        } else {
            #pragma omp parallel for schedule(static)
            for (i = 0; i < N; i++) {
                const int64_t beg = A_indptr[i], end = A_indptr[i + 1];
                double acc = 0.0;
                int64_t k = beg;
                for (; k + 3 < end; k += 4) {
                    const double t0 = A_data[k] * Q[A_indices[k] * m + j];
                    const double t1 = A_data[k + 1] * Q[A_indices[k + 1] * m + j];
                    const double t2 = A_data[k + 2] * Q[A_indices[k + 2] * m + j];
                    const double t3 = A_data[k + 3] * Q[A_indices[k + 3] * m + j];
                    acc = acc + t0 + t1 + t2 + t3;
                }
                for (; k < end; k++)
                    acc = acc + A_data[k] * Q[A_indices[k] * m + j];
                if (j > 0) acc = acc - beta_prev * qprev[i];
                w[i] = acc;
            }
        }

        /* 2) full reorthogonalization, TWO fused passes (same structure as the
           reference: a single pass leaves an O(eps) non-orthogonality that the
           Lanczos dynamics amplify ~x2 per step -- after ~40 steps it reaches
           O(1) and destroys Q^T Q; the second pass crushes it to O(eps^2)).
           Pass 1: H1[p] = w . q_p, p = 0..j (H1[j] == alpha_j);  w -= Q H1.
           Pass 2: H2[p] = w . q_p;  w -= Q H2;  sn = w . w.
           Each pass = C (parallel 8-column dot partials) + split combine +
           D (subtiled row sweep; its Q re-read is L2/L3 hot from C). */
        double hdiag = 0.0;
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            double *restrict pt = part + (size_t)tid * PS;
            double *restrict H = part;
            double sn = 0.0;
            int64_t s, ii; /* ii: plain (non-worksharing) loop var, region-local */

            /* ---- pass 1 ---- */
            /* C1: per-thread row chunks, per subtile (no sync inside) */
            for (int64_t c = 0; c <= j; c++) pt[c] = 0.0;
            for (s = 0; s < S; s++) {
                const int64_t a0 = s * R, b1 = a0 + R > N ? N : a0 + R;
                const int64_t chunk = (b1 - a0 + T - 1) / T;
                int64_t st = a0 + (int64_t)tid * chunk;
                int64_t en = st + chunk;
                if (st > b1) st = b1;
                if (en > b1) en = b1;
                if (st >= en) continue;
                const double *wp = w + st;
                const int64_t n = en - st;
                int64_t c0 = 0;
#ifdef __AVX512F__
                for (; c0 + 8 <= j + 1; c0 += 8) {
                    __m512d d0 = _mm512_setzero_pd(), d1 = _mm512_setzero_pd();
                    int64_t i8 = 0;
                    for (; i8 + 8 <= n; i8 += 8) {
                        const __m512d w0 = _mm512_set1_pd(wp[i8]);
                        const __m512d w1 = _mm512_set1_pd(wp[i8 + 1]);
                        const __m512d w2 = _mm512_set1_pd(wp[i8 + 2]);
                        const __m512d w3 = _mm512_set1_pd(wp[i8 + 3]);
                        const __m512d w4 = _mm512_set1_pd(wp[i8 + 4]);
                        const __m512d w5 = _mm512_set1_pd(wp[i8 + 5]);
                        const __m512d w6 = _mm512_set1_pd(wp[i8 + 6]);
                        const __m512d w7 = _mm512_set1_pd(wp[i8 + 7]);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8) * m + c0), w0, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 1) * m + c0), w1, d1);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 2) * m + c0), w2, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 3) * m + c0), w3, d1);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 4) * m + c0), w4, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 5) * m + c0), w5, d1);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 6) * m + c0), w6, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 7) * m + c0), w7, d1);
                    }
                    {
                        double dv[8];
                        _mm512_storeu_pd(dv, _mm512_add_pd(d0, d1));
                        for (int c = 0; c < 8; c++) pt[c0 + c] += dv[c];
                    }
                    /* row tail */
                    for (; i8 < n; i8++) {
                        const double x = wp[i8];
                        const double *qr = Q + (size_t)(st + i8) * m + c0;
                        for (int c = 0; c < 8; c++) pt[c0 + c] += x * qr[c];
                    }
                }
#endif
                for (; c0 <= j; c0++) {
                    const double *qp = Q + c0 + (size_t)st * m;
                    double s2 = 0.0;
                    for (ii = 0; ii < n; ii++) s2 = s2 + wp[ii] * qp[ii * m];
                    pt[c0] += s2;
                }
            }

            /* C1 done (no worksharing) -- sync before reading others' partials */
            #pragma omp barrier

            /* split H1 combine across threads (each p written by one thread) */
            for (int64_t p = tid; p <= j; p += T) {
                double ssum = 0.0;
                for (int64_t t = 0; t < T; t++)
                    ssum = ssum + part[(size_t)t * PS + p];
                H[p] = ssum;
            }
            /* D's omp for is NOT a barrier, and part[j] is written by thread
               (j % T) in the combine: sync before anyone re-reads part */
            #pragma omp barrier
            if (tid == 0) hdiag = H[j];

            /* D1: per subtile, w -= Q H1 (Q rows hot in L2/L3 from C1) */
            for (s = 0; s < S; s++) {
                const int64_t a0 = s * R, b1 = a0 + R > N ? N : a0 + R;
                #pragma omp for schedule(static)
                for (i = a0; i < b1; i++) {
                    const int64_t row = (size_t)i * m;
                    double ssum;
#ifdef __AVX512F__
                    __m512d sv = _mm512_setzero_pd();
                    int64_t p = 0;
                    for (; p + 8 <= j + 1; p += 8)
                        sv = _mm512_add_pd(sv,
                            _mm512_mul_pd(_mm512_loadu_pd(Q + row + p),
                                          _mm512_loadu_pd(H + p)));
                    ssum = _mm512_reduce_add_pd(sv);
                    for (; p <= j; p++) ssum += H[p] * Q[row + p];
#else
                    ssum = 0.0;
                    for (int64_t p = 0; p <= j; p++) ssum += H[p] * Q[row + p];
#endif
                    w[i] = w[i] - ssum;
                }
            }

            /* H lives in thread 0's pt row; pass 2's reset would clobber it
               while late D1 threads might still read it -- sync first */
            #pragma omp barrier

            /* ---- pass 2 (identical structure; computes the final norm) ---- */
            for (int64_t c = 0; c <= j; c++) pt[c] = 0.0;
            for (s = 0; s < S; s++) {
                const int64_t a0 = s * R, b1 = a0 + R > N ? N : a0 + R;
                const int64_t chunk = (b1 - a0 + T - 1) / T;
                int64_t st = a0 + (int64_t)tid * chunk;
                int64_t en = st + chunk;
                if (st > b1) st = b1;
                if (en > b1) en = b1;
                if (st >= en) continue;
                const double *wp = w + st;
                const int64_t n = en - st;
                int64_t c0 = 0;
#ifdef __AVX512F__
                for (; c0 + 8 <= j + 1; c0 += 8) {
                    __m512d d0 = _mm512_setzero_pd(), d1 = _mm512_setzero_pd();
                    int64_t i8 = 0;
                    for (; i8 + 8 <= n; i8 += 8) {
                        const __m512d w0 = _mm512_set1_pd(wp[i8]);
                        const __m512d w1 = _mm512_set1_pd(wp[i8 + 1]);
                        const __m512d w2 = _mm512_set1_pd(wp[i8 + 2]);
                        const __m512d w3 = _mm512_set1_pd(wp[i8 + 3]);
                        const __m512d w4 = _mm512_set1_pd(wp[i8 + 4]);
                        const __m512d w5 = _mm512_set1_pd(wp[i8 + 5]);
                        const __m512d w6 = _mm512_set1_pd(wp[i8 + 6]);
                        const __m512d w7 = _mm512_set1_pd(wp[i8 + 7]);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8) * m + c0), w0, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 1) * m + c0), w1, d1);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 2) * m + c0), w2, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 3) * m + c0), w3, d1);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 4) * m + c0), w4, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 5) * m + c0), w5, d1);
                        d0 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 6) * m + c0), w6, d0);
                        d1 = _mm512_fmadd_pd(_mm512_loadu_pd(Q + ((size_t)st + i8 + 7) * m + c0), w7, d1);
                    }
                    {
                        double dv[8];
                        _mm512_storeu_pd(dv, _mm512_add_pd(d0, d1));
                        for (int c = 0; c < 8; c++) pt[c0 + c] += dv[c];
                    }
                    /* row tail */
                    for (; i8 < n; i8++) {
                        const double x = wp[i8];
                        const double *qr = Q + (size_t)(st + i8) * m + c0;
                        for (int c = 0; c < 8; c++) pt[c0 + c] += x * qr[c];
                    }
                }
#endif
                for (; c0 <= j; c0++) {
                    const double *qp = Q + c0 + (size_t)st * m;
                    double s2 = 0.0;
                    for (ii = 0; ii < n; ii++) s2 = s2 + wp[ii] * qp[ii * m];
                    pt[c0] += s2;
                }
            }

            /* C2 done -- sync before reading others' partials */
            #pragma omp barrier

            /* split H2 combine across threads */
            for (int64_t p = tid; p <= j; p += T) {
                double ssum = 0.0;
                for (int64_t t = 0; t < T; t++)
                    ssum = ssum + part[(size_t)t * PS + p];
                H[p] = ssum;
            }
            #pragma omp barrier

            /* D2: per subtile, w -= Q H2, sn += w*w */
            for (s = 0; s < S; s++) {
                const int64_t a0 = s * R, b1 = a0 + R > N ? N : a0 + R;
                #pragma omp for schedule(static)
                for (i = a0; i < b1; i++) {
                    const int64_t row = (size_t)i * m;
                    double ssum;
#ifdef __AVX512F__
                    __m512d sv = _mm512_setzero_pd();
                    int64_t p = 0;
                    for (; p + 8 <= j + 1; p += 8)
                        sv = _mm512_add_pd(sv,
                            _mm512_mul_pd(_mm512_loadu_pd(Q + row + p),
                                          _mm512_loadu_pd(H + p)));
                    ssum = _mm512_reduce_add_pd(sv);
                    for (; p <= j; p++) ssum += H[p] * Q[row + p];
#else
                    ssum = 0.0;
                    for (int64_t p = 0; p <= j; p++) ssum += H[p] * Q[row + p];
#endif
                    const double wi = w[i] - ssum;
                    w[i] = wi;
                    sn = sn + wi * wi;
                }
            }
            snp[tid] = sn;
        }

        sn = 0.0;
        for (i = 0; i < T; i++) sn = sn + snp[i];
        alpha[j] = hdiag;
        const double b_j = sqrt(sn);
        beta[j] = b_j;
        if (j + 1 < m) {
            const double inv = 1.0 / b_j;
            #pragma omp parallel for schedule(static)
            for (i = 0; i < N; i++) { Q[i * m + j + 1] = w[i] * inv; qprev[i] = Q[i * m + j]; }
        }
        beta_prev = b_j;
    }

    if (!have_ws) free(base);
}
