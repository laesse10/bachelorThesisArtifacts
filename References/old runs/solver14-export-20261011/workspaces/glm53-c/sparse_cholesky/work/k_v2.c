#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

/* work[sc_row[t]] -= sc_val[t] * ljk  for t in [ts, te) */
static inline void suffix_update(double *restrict work, const int32_t *restrict sc_row,
                                 const double *restrict sc_val, int32_t ts, int32_t te,
                                 double ljk)
{
    const __m512d vljk = _mm512_set1_pd(ljk);
    int32_t t = ts;
    for (; t + 8 <= te; t += 8) {
        __m256i vidx = _mm256_loadu_si256((const __m256i *)(sc_row + t));
        __m512d vv = _mm512_loadu_pd(sc_val + t);
        __m512d wv = _mm512_i32gather_pd(vidx, work, 8);
        wv = _mm512_fnmadd_pd(vv, vljk, wv);
        _mm512_i32scatter_pd(work, vidx, wv, 8);
    }
    for (; t < te; t++)
        work[sc_row[t]] -= sc_val[t] * ljk;
}

void sparse_cholesky_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const int64_t *restrict L_to_Lc,
    double *restrict Lc_data,
    const int64_t *restrict Lc_indices,
    const int64_t *restrict Lc_indptr,
    const double *restrict b,
    double *restrict y,
    const int64_t EDGE,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const int64_t N = EDGE * EDGE * EDGE;
    const int64_t nnzL = Lc_indptr[N];

    double *work = (double *)malloc(N * sizeof(double));
    int32_t *sc_row = (int32_t *)malloc(nnzL * sizeof(int32_t));
    double *sc_val = (double *)malloc(nnzL * sizeof(double));
    int32_t *t_of_p = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int32_t *tstart = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int64_t *cursor = (int64_t *)malloc((N + 1) * sizeof(int64_t));
    memset(work, 0, N * sizeof(double));
    memcpy(cursor, Lc_indptr, (N + 1) * sizeof(int64_t));
    for (int64_t k = 0; k < N; k++) sc_row[Lc_indptr[k]] = (int32_t)k;
    for (int64_t i = 0; i < N; i++) {
        for (int64_t p = L_indptr[i]; p < L_indptr[i + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int64_t t = cursor[k]++;
            sc_row[t] = (int32_t)i;
            t_of_p[L_to_Lc[p]] = (int32_t)t;
            tstart[p] = (int32_t)t;
        }
    }

    for (int64_t j = 0; j < N; j++) {
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
            int64_t row = A_indices[p];
            if (row >= j) work[row] = A_data[p];
        }
        for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int32_t ts = tstart[p];
            double ljk = sc_val[ts];
            suffix_update(work, sc_row, sc_val, ts, (int32_t)Lc_indptr[k + 1], ljk);
        }
        double diag = work[j];
        work[j] = 0.0;
        double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj;
        sc_val[Lc_indptr[j]] = ljj;
        for (int64_t p = Lc_indptr[j] + 1; p < Lc_indptr[j + 1]; p++) {
            int64_t row = Lc_indices[p];
            double v = work[row] / ljj;
            work[row] = 0.0;
            Lc_data[p] = v;
            sc_val[t_of_p[p]] = v;
        }
    }

    for (int64_t j = 0; j < N; j++) y[j] = b[j];
    for (int64_t j = 0; j < N; j++) {
        double ljj = Lc_data[Lc_indptr[j]];
        y[j] = y[j] / ljj;
        const double yj = y[j];
        /* y[row] -= Lc_data[p] * yj */
        int64_t p = Lc_indptr[j] + 1, pe = Lc_indptr[j + 1];
        const __m512d vyj = _mm512_set1_pd(yj);
        for (; p + 8 <= pe; p += 8) {
            __m512i vidx = _mm512_loadu_si512((const __m512i *)(Lc_indices + p));
            __m512d vv = _mm512_loadu_pd(Lc_data + p);
            __m512d wv = _mm512_i64gather_pd(vidx, y, 8);
            wv = _mm512_fnmadd_pd(vv, vyj, wv);
            _mm512_i64scatter_pd(y, vidx, wv, 8);
        }
        for (; p < pe; p++) y[Lc_indices[p]] -= Lc_data[p] * yj;
    }
    for (int64_t jj = 0; jj < N; jj++) {
        int64_t col = N - 1 - jj;
        double ljj = Lc_data[Lc_indptr[col]];
        int64_t p = Lc_indptr[col] + 1, pe = Lc_indptr[col + 1];
        __m512d acc = _mm512_setzero_pd();
        for (; p + 8 <= pe; p += 8) {
            __m512i vidx = _mm512_loadu_si512((const __m512i *)(Lc_indices + p));
            __m512d vv = _mm512_loadu_pd(Lc_data + p);
            __m512d wv = _mm512_i64gather_pd(vidx, y, 8);
            acc = _mm512_fmadd_pd(vv, wv, acc);
        }
        double s = y[col] - _mm512_reduce_add_pd(acc);
        for (; p < pe; p++) s -= Lc_data[p] * y[Lc_indices[p]];
        y[col] = s / ljj;
    }
    free(work); free(sc_row); free(sc_val); free(t_of_p); free(tstart); free(cursor);
}
