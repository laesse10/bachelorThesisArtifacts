#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

typedef struct { int32_t pos; int32_t row; int32_t len; } Run;

/* work[r0 .. r0+len) -= sc_val[t0 .. t0+len) * ljk  (contiguous) */
static inline void upd_run(double *restrict work, const double *restrict sc_val,
                           int32_t t0, int32_t r0, int32_t len, double ljk)
{
    const __m512d vl = _mm512_set1_pd(ljk);
    int32_t t = t0, r = r0;
    for (; len >= 8; len -= 8) {
        __m512d w = _mm512_loadu_pd(work + r);
        __m512d v = _mm512_loadu_pd(sc_val + t);
        _mm512_storeu_pd(work + r, _mm512_fnmadd_pd(v, vl, w));
        t += 8; r += 8;
    }
    if (len > 0) {
        __mmask8 m = (__mmask8)((1u << len) - 1);
        __m512d w = _mm512_loadu_pd(work + r);
        __m512d v = _mm512_maskz_loadu_pd(m, sc_val + t);
        _mm512_mask_storeu_pd(work + r, m, _mm512_mask3_fnmadd_pd(v, vl, w, m));
    }
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
    int32_t *sc_pos = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int32_t *tstart = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int32_t *trun = (int32_t *)malloc(nnzL * sizeof(int32_t));
    int32_t *cr_ptr = (int32_t *)malloc((N + 1) * sizeof(int32_t));
    int64_t *cursor = (int64_t *)malloc((N + 1) * sizeof(int64_t));
    Run *runs = (Run *)malloc((size_t)(nnzL + N) * sizeof(Run));
    memset(work, 0, N * sizeof(double));
    memcpy(cursor, Lc_indptr, (N + 1) * sizeof(int64_t));

    /* pass 1: transpose CSR pattern -> sorted columns; sc_pos maps sorted->orig */
    for (int64_t k = 0; k < N; k++) {
        sc_row[Lc_indptr[k]] = (int32_t)k;
        sc_pos[Lc_indptr[k]] = (int32_t)Lc_indptr[k];
    }
    for (int64_t i = 0; i < N; i++) {
        for (int64_t p = L_indptr[i]; p < L_indptr[i + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int64_t t = cursor[k]++;
            sc_row[t] = (int32_t)i;
            sc_pos[t] = (int32_t)L_to_Lc[p];
            tstart[p] = (int32_t)t;
        }
    }
    /* pass 2: run structure per column */
    int32_t nruns = 0;
    for (int64_t k = 0; k < N; k++) {
        cr_ptr[k] = nruns;
        int32_t t = (int32_t)Lc_indptr[k], te = (int32_t)Lc_indptr[k + 1];
        while (t < te) {
            int32_t r0 = sc_row[t];
            int32_t t0 = t;
            t++;
            while (t < te && sc_row[t] == sc_row[t - 1] + 1) t++;
            runs[nruns].pos = t0;
            runs[nruns].row = r0;
            runs[nruns].len = t - t0;
            nruns++;
        }
    }
    cr_ptr[N] = nruns;
    /* pass 3: run id for each CSR pattern entry p (suffix start of (row, k)) */
    for (int64_t i = 0; i < N; i++) {
        for (int64_t p = L_indptr[i]; p < L_indptr[i + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int32_t t = tstart[p];
            /* run of column k containing position t: linear from cr_ptr[k] via row offset */
            const Run *R = runs + cr_ptr[k];
            int32_t lo = cr_ptr[k], hi = cr_ptr[k + 1];
            while (lo + 1 < hi) {
                int32_t mid = (lo + hi) >> 1;
                if (R[mid].row <= sc_row[t]) lo = mid; else hi = mid;
            }
            trun[p] = lo;
        }
    }

    /* factorization */
    for (int64_t j = 0; j < N; j++) {
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
            int64_t row = A_indices[p];
            if (row >= j) work[row] = A_data[p];
        }
        for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
            int64_t k = L_indices[p];
            int32_t t = tstart[p];
            double ljk = sc_val[t];
            int32_t ri = trun[p];
            const Run *R = runs + cr_ptr[k];
            int32_t nr = cr_ptr[k + 1];
            Run r = R[ri];
            int32_t off = t - r.pos;
            upd_run(work, sc_val, r.pos + off, r.row + off, r.len - off, ljk);
            for (ri++; ri < nr; ri++) {
                r = R[ri];
                upd_run(work, sc_val, r.pos, r.row, r.len, ljk);
            }
        }
        double diag = work[j];
        work[j] = 0.0;
        double ljj = sqrt(diag);
        Lc_data[Lc_indptr[j]] = ljj;
        sc_val[Lc_indptr[j]] = ljj;
        /* finalize: runs of column j, skipping the diagonal (first element of run 0) */
        for (int32_t ri = cr_ptr[j]; ri < cr_ptr[j + 1]; ri++) {
            Run r = runs[ri];
            int32_t t0 = r.pos, r0 = r.row, len = r.len;
            if (r0 == (int32_t)j) { t0++; r0++; len--; }
            if (len <= 0) continue;
            const __m512d vl = _mm512_set1_pd(ljj);
            int32_t t = t0, rr = r0;
            for (; len >= 8; len -= 8) {
                __m512d w = _mm512_loadu_pd(work + rr);
                __m512d v = _mm512_div_pd(w, vl);
                _mm512_storeu_pd(sc_val + t, v);
                _mm512_storeu_pd(work + rr, _mm512_setzero_pd());
                for (int q = 0; q < 8; q++) Lc_data[sc_pos[t + q]] = v[q];
                t += 8; rr += 8;
            }
            if (len > 0) {
                __mmask8 m = (__mmask8)((1u << len) - 1);
                __m512d w = _mm512_maskz_loadu_pd(m, work + rr);
                __m512d v = _mm512_div_pd(w, vl);
                _mm512_mask_storeu_pd(sc_val + t, m, v);
                _mm512_mask_storeu_pd(work + rr, m, _mm512_setzero_pd());
                for (int q = 0; q < len; q++) Lc_data[sc_pos[t + q]] = v[q];
            }
        }
    }

    /* solves */
    for (int64_t j = 0; j < N; j++) y[j] = b[j];
    for (int64_t j = 0; j < N; j++) {
        double ljj = Lc_data[Lc_indptr[j]];
        y[j] = y[j] / ljj;
        const double yj = y[j];
        for (int32_t ri = cr_ptr[j]; ri < cr_ptr[j + 1]; ri++) {
            Run r = runs[ri];
            int32_t t0 = r.pos, r0 = r.row, len = r.len;
            if (r0 == (int32_t)j) { t0++; r0++; len--; }
            if (len <= 0) continue;
            const __m512d vy = _mm512_set1_pd(yj);
            int32_t t = t0, rr = r0;
            for (; len >= 8; len -= 8) {
                __m512d w = _mm512_loadu_pd(y + rr);
                __m512d v = _mm512_loadu_pd(sc_val + t);
                _mm512_storeu_pd(y + rr, _mm512_fnmadd_pd(v, vy, w));
                t += 8; rr += 8;
            }
            if (len > 0) {
                __mmask8 m = (__mmask8)((1u << len) - 1);
                __m512d w = _mm512_maskz_loadu_pd(m, y + rr);
                __m512d v = _mm512_maskz_loadu_pd(m, sc_val + t);
                _mm512_mask_storeu_pd(y + rr, m, _mm512_fnmadd_pd(v, vy, w));
            }
        }
    }
    for (int64_t jj = 0; jj < N; jj++) {
        int64_t col = N - 1 - jj;
        double ljj = Lc_data[Lc_indptr[col]];
        double s = y[col];
        for (int64_t p = Lc_indptr[col] + 1; p < Lc_indptr[col + 1]; p++) {
            int64_t row = Lc_indices[p];
            s -= Lc_data[p] * y[row];
        }
        y[col] = s / ljj;
    }
    free(work); free(sc_row); free(sc_val); free(sc_pos);
    free(tstart); free(trun); free(cr_ptr); free(cursor); free(runs);
}
