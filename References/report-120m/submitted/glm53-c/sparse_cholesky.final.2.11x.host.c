/* sparse_cholesky: up-looking column Cholesky over a precomputed pattern + solves.
 * Stage B: columns presorted (rows ascending) so each update is a contiguous,
 * branch-free AXPY suffix; int32 index arrays. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int32_t *Sidx;        /* nnz: row of each sorted position */
    double  *Sv;          /* nnz: value of each sorted position */
    int32_t *origOfSorted;/* nnz: original Lc position of sorted position */
    int32_t *sortedOfOrig;/* nnz: sorted position of original Lc position */
    int64_t *cost;        /* N: inner-loop iteration estimate per column */
} scratch_t;

/* ---- scratch arena (grow-only, reused across calls) ---- */
static void *arena = NULL;
static size_t arena_cap = 0;
static void *arena_alloc(size_t bytes) {
    if (bytes > arena_cap) {
        void *p = realloc(arena, bytes);
        if (!p) return NULL;
        arena = p; arena_cap = bytes;
    }
    return arena;
}

void sparse_cholesky_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t *L_indices,
    const int64_t *L_indptr,
    const int64_t *L_to_Lc,
    double *restrict Lc_data,
    const int64_t *Lc_indices,
    const int64_t *Lc_indptr,
    const double *restrict b,
    double *restrict y,
    const int64_t EDGE,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const int64_t N = EDGE * EDGE * EDGE;
    const int64_t nnz = Lc_indptr[N];

    size_t need = (size_t)nnz * (4 + 8 + 4 + 4) + (size_t)N * (8 + 8);
    uint8_t *base = (uint8_t *)arena_alloc(need);
    if (!base) { /* fallback: direct malloc (not retained) */
        base = (uint8_t *)malloc(need);
    }
    int32_t *Sidx = (int32_t *)(base);
    int32_t *origOfSorted = (int32_t *)(base + (size_t)nnz * 4);
    int32_t *sortedOfOrig = (int32_t *)(base + (size_t)nnz * 8);
    double  *Sv = (double *)(base + (size_t)nnz * 12);
    double  *work = (double *)(base + (size_t)nnz * 20);
    int64_t *cost = (int64_t *)(base + (size_t)nnz * 20 + (size_t)N * 8);

    /* ---- preprocessing: sorted column layout via the CSR pattern of L ----
     * Rows of L ascend; L_to_Lc[p] is the (unsorted) Lc position of entry
     * (row r, col k). Filling column buckets in row order yields, for each
     * column k, its Lc positions in ascending-row order. */
    {
        /* cursor[k] = Lc_indptr[k], then single sweep */
        int64_t *cursor = (int64_t *)malloc((size_t)(N + 1) * 8);
        memcpy(cursor, Lc_indptr, (size_t)(N + 1) * 8);
        for (int64_t r = 0; r < N; r++) {
            for (int64_t p = L_indptr[r]; p < L_indptr[r + 1]; p++) {
                int64_t k = L_indices[p];
                origOfSorted[cursor[k]++] = (int32_t)L_to_Lc[p];
            }
        }
        free(cursor);
        for (int64_t q = 0; q < nnz; q++) {
            int32_t o = origOfSorted[q];
            Sidx[q] = (int32_t)Lc_indices[o];
            sortedOfOrig[o] = (int32_t)q;
            Sv[q] = 0.0;
        }
    }

    /* per-column cost (inner-loop iteration estimate) */
    for (int64_t j = 0; j < N; j++) cost[j] = 0;
    for (int64_t j = 0; j < N; j++) {
        int64_t c = 0;
        for (int64_t p = L_indptr[j]; p < L_indptr[j + 1] - 1; p++) {
            int64_t k = L_indices[p];
            c += Lc_indptr[k + 1] - sortedOfOrig[L_to_Lc[p]];
        }
        cost[j] = c + (Lc_indptr[j + 1] - Lc_indptr[j]);
    }

    memset(work, 0, (size_t)N * sizeof(double));

    /* ---- factorization ---- */
    for (int64_t j = 0; j < N; j++) {
        for (int64_t p = A_indptr[j]; p < A_indptr[j + 1]; p++) {
            int64_t row = A_indices[p];
            if (row >= j) work[row] = A_data[p];
        }
        const int64_t js = L_indptr[j], je = L_indptr[j + 1] - 1;
        for (int64_t p = js; p < je; p++) {
            const int64_t k = L_indices[p];
            const int32_t qs = sortedOfOrig[L_to_Lc[p]];
            const double ljk = Sv[qs];
            const int32_t *restrict idx = Sidx;
            const int64_t qe = Lc_indptr[k + 1];
            for (int64_t q = qs; q < qe; q++) {
                work[idx[q]] -= Sv[q] * ljk;
            }
        }
        const int64_t jd = Lc_indptr[j];
        double diag = work[j];
        work[j] = 0.0;
        double ljj = sqrt(diag);
        Sv[jd] = ljj;
        for (int64_t q = jd + 1; q < Lc_indptr[j + 1]; q++) {
            int64_t row = Sidx[q];
            Sv[q] = work[row] / ljj;
            work[row] = 0.0;
        }
    }

    /* ---- scatter values back to the original Lc layout ---- */
    for (int64_t q = 0; q < nnz; q++) Lc_data[origOfSorted[q]] = Sv[q];

    /* ---- triangular solves (sorted layout) ---- */
    for (int64_t j = 0; j < N; j++) y[j] = b[j];
    for (int64_t j = 0; j < N; j++) {
        const int64_t jd = Lc_indptr[j], je = Lc_indptr[j + 1];
        double ljj = Sv[jd];
        y[j] = y[j] / ljj;
        const double yj = y[j];
        for (int64_t q = jd + 1; q < je; q++) y[Sidx[q]] -= Sv[q] * yj;
    }
    for (int64_t jj = 0; jj < N; jj++) {
        const int64_t col = N - 1 - jj;
        const int64_t jd = Lc_indptr[col], je = Lc_indptr[col + 1];
        double ljj = Sv[jd];
        double s = y[col];
        for (int64_t q = jd + 1; q < je; q++) s -= Sv[q] * y[Sidx[q]];
        y[col] = s / ljj;
    }
}
