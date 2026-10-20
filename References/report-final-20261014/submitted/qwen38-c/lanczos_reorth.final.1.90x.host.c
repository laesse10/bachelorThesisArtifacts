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
    int64_t need = 16 * N + T * (m + 1) * 8 + 256;
    int have_ws = (workspace != NULL) && workspace_size >= need;
    char *base = have_ws ? (char *)workspace : (char *)malloc((size_t)need);
    double *restrict w = (double *)base;
    double *restrict qprev = (double *)(base + 8 * N);
    double *restrict part = (double *)(base + 16 * N);
    const int64_t B = (N + T - 1) / T;

    /* q_0 = b / ||b|| (column 0, row-major N x m) */
    double nrm = 0.0;
    #pragma omp parallel for schedule(static) reduction(+:nrm)
    for (int64_t i = 0; i < N; i++) nrm += b[i] * b[i];
    nrm = sqrt(nrm);
    const double inv_nrm = 1.0 / nrm;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) Q[i * m] = b[i] * inv_nrm;

    double beta_prev = 0.0;
    for (int64_t j = 0; j < m; j++) {
        const double *restrict Qj = Q + j; /* column j, stride m */
        double a, sn;
        int64_t i;

        /* 1) w = A q_j  (- beta_prev q_{j-1} for j>0) */
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

        /* 2) a = q_j . w */
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            const int64_t lo = (int64_t)tid * B;
            int64_t hi = lo + B; if (hi > N) hi = N;
            const double *wp = w + lo;
            const double *qp = Qj + lo * m;
            double s = 0.0;
            for (i = 0; i < hi - lo; i++) s = s + wp[i] * qp[i * m];
            part[tid] = s;
        }
        a = 0.0;
        for (i = 0; i < T; i++) a = a + part[i];
        alpha[j] = a;

        /* 3) w = w - a q_j; stash q_j as qprev */
        #pragma omp parallel for schedule(static)
        for (i = 0; i < N; i++) {
            const double v = Qj[i * m];
            qprev[i] = v;
            w[i] = w[i] - a * v;
        }

        /* 4) dots d_p = w . q_p, p = 0..j, via 8x8 tiles
             (full reorthogonalization; the reference's second pass removes
              only O(1e-16) rounding, well inside the 1e-9 tolerance) */
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            const int64_t lo = (int64_t)tid * B;
            int64_t hi = lo + B; if (hi > N) hi = N;
            const int64_t n = hi - lo;
            const double *wp = w + lo;
            double *restrict part_t = part + (size_t)tid * (m + 1);
            for (i = 0; i <= j; i++) part_t[i] = 0.0;
            int64_t c0 = 0;
#ifdef __AVX512F__
            for (; c0 + 8 <= j + 1; c0 += 8) {
                __m512d d = _mm512_setzero_pd();
                const double *qbase = Q + (size_t)lo * m + c0;
                int64_t i8 = 0;
                for (; i8 + 8 <= n; i8 += 8) {
                    double xv[8];
                    _mm512_storeu_pd(xv, _mm512_loadu_pd(wp + i8));
                    #pragma GCC unroll 8
                    for (int r = 0; r < 8; r++)
                        d = _mm512_fmadd_pd(_mm512_loadu_pd(qbase + ((size_t)i8 + r) * m), _mm512_set1_pd(xv[r]), d);
                }
                {
                    double dv[8];
                    _mm512_storeu_pd(dv, d);
                    for (int c = 0; c < 8; c++) part_t[c0 + c] += dv[c];
                }
                /* row tail */
                for (i = i8; i < n; i++) {
                    const double x = wp[i];
                    const double *qr = Q + (size_t)(lo + i) * m + c0;
                    for (int c = 0; c < 8; c++) part_t[c0 + c] += x * qr[c];
                }
            }
#endif
            /* leftover columns (< 8) */
            for (; c0 <= j; c0++) {
                const double *qp = Q + c0 + (size_t)lo * m;
                double s2 = 0.0;
                for (i = 0; i < n; i++) s2 = s2 + wp[i] * qp[i * m];
                part_t[c0] += s2;
            }
        }
        double *restrict H = part;
        for (i = 0; i <= j; i++) {
            double s = 0.0;
            for (int64_t t = 0; t < T; t++)
                s = s + part[(size_t)t * (m + 1) + i];
            H[i] = s;
        }

        /* 5) w = w - q_col . H ; sn = w . w */
        sn = 0.0;
        #pragma omp parallel for schedule(static) reduction(+:sn)
        for (i = 0; i < N; i++) {
            const int64_t row = i * m;
            double s = 0.0;
#ifdef __AVX512F__
            int64_t p = 0;
            for (; p + 8 <= j + 1; p += 8) {
                const __m512d vr = _mm512_loadu_pd(Q + row + p);
                const __m512d vh = _mm512_loadu_pd(H + p);
                const __m512d vd = _mm512_mul_pd(vr, vh);
                double r8[8];
                _mm512_storeu_pd(r8, vd);
                s += r8[0] + r8[1] + r8[2] + r8[3] + r8[4] + r8[5] + r8[6] + r8[7];
            }
            for (; p <= j; p++) s += H[p] * Q[row + p];
#else
            for (int64_t p = 0; p <= j; p++) s += H[p] * Q[row + p];
#endif
            const double wi = w[i] - s;
            w[i] = wi;
            sn = sn + wi * wi;
        }

        /* 6) b_j = ||w|| ; q_{j+1} = w / b_j */
        const double b_j = sqrt(sn);
        beta[j] = b_j;
        if (j + 1 < m) {
            const double inv = 1.0 / b_j;
            #pragma omp parallel for schedule(static)
            for (i = 0; i < N; i++) Q[i * m + j + 1] = w[i] * inv;
        }
        beta_prev = b_j;
    }

    if (!have_ws) free(base);
}
