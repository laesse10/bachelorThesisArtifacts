/* ILU(0) factorization, in place on the CSR pattern of A (Saad, Alg. 10.4).
 *
 * The row loop of the algorithm is sequential: row i is eliminated against the
 * already-factored rows k < i from its own pattern.  Instead of scattering each
 * row into a dense length-N scratch vector (several random accesses per nonzero),
 * we exploit that CSR rows here are short and sorted: the row pattern stays in a
 * tiny per-row local buffer and each update is a merge-join of two sorted
 * patterns.  The arithmetic sequence is exactly the reference's.
 *
 * A non-canonical matrix (unsorted / duplicated columns, or a missing diagonal)
 * falls back to a faithful dense translation of the reference.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* ------------- dense fallback: exact translation of the NumPy reference ------------- */

static void ilu0_dense(double *restrict A_data, const int64_t *restrict A_indices,
                       const int64_t *restrict A_indptr, const int64_t N)
{
    double *restrict diag    = (double *)calloc((size_t)N, sizeof(double));
    double *restrict row_val = (double *)calloc((size_t)N, sizeof(double));
    int64_t *restrict in_row = (int64_t *)calloc((size_t)N, sizeof(int64_t));
    if (!diag || !row_val || !in_row) {
        free(diag); free(row_val); free(in_row);
        return;
    }

    for (int64_t i = 0; i < N; i++) {
        const int64_t row_start = A_indptr[i];
        const int64_t row_end   = A_indptr[i + 1];
        for (int64_t k = row_start; k < row_end; k++) {
            const int64_t col = A_indices[k];
            row_val[col] = A_data[k];
            in_row[col] = 1;
        }
        for (int64_t k = row_start; k < row_end; k++) {
            const int64_t col_k = A_indices[k];
            if (col_k >= i)
                continue;
            const double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik;
            for (int64_t m = A_indptr[col_k]; m < A_indptr[col_k + 1]; m++) {
                const int64_t col_j = A_indices[m];
                if (col_j > col_k && in_row[col_j] == 1)
                    row_val[col_j] -= l_ik * A_data[m];
            }
        }
        diag[i] = row_val[i];
        for (int64_t k = row_start; k < row_end; k++) {
            A_data[k] = row_val[A_indices[k]];
            in_row[A_indices[k]] = 0;
        }
    }

    free(diag); free(row_val); free(in_row);
}

/* ------------------- fast path: local row buffer + merge-join ----------------------- */

static inline size_t al64(size_t x) { return (x + 63u) & ~(size_t)63u; }

static void ilu0_local(double *restrict A_data, const int64_t *restrict A_indices,
                       const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
                       const int64_t maxrow, uint8_t *restrict buf)
{
    /* scratch layout: diag[8N] | ustart[4N] | idx32[4*nnz] | cols[4*maxrow] | vals[8*maxrow],
       each segment 64-byte aligned. */
    size_t off = 0;
    double   *restrict diag   = (double *)buf;
    off += al64((size_t)8 * (size_t)N);
    uint32_t *restrict ustart = (uint32_t *)(buf + off);
    off += al64((size_t)4 * (size_t)N);
    int32_t  *restrict idx32 = (int32_t *)(buf + off);
    off += al64((size_t)4 * (size_t)nnz);
    uint32_t *restrict cols  = (uint32_t *)(buf + off);
    off += al64((size_t)4 * (size_t)maxrow);
    double   *restrict vals  = (double *)(buf + off);
    off += al64((size_t)8 * (size_t)maxrow);
    (void)off;

    /* int32 copy of the column indices: halves the index traffic in the hot loop. */
    for (int64_t j = 0; j < nnz; j++)
        idx32[j] = (int32_t)A_indices[j];

    /* ustart[k]: offset of the first entry of row k whose column is > k (the U part). */
    for (int64_t k = 0; k < N; k++) {
        const int64_t s = A_indptr[k];
        const int64_t e = A_indptr[k + 1];
        int64_t m = s;
        while (m < e && (int64_t)idx32[m] <= k)
            m++;
        ustart[k] = (uint32_t)(m - s);
    }

    for (int64_t i = 0; i < N; i++) {
        const int64_t rs = A_indptr[i];
        const int64_t re = A_indptr[i + 1];
        const int nnz_i = (int)(re - rs);
        int diagpos = -1;

        /* scatter row i into the local buffer */
        for (int t = 0; t < nnz_i; t++) {
            const int c = idx32[rs + t];
            cols[t] = (uint32_t)c;
            vals[t] = A_data[rs + t];
            if (c == (int)i)
                diagpos = t;
        }

        /* eliminate against every already-factored row k < i in the pattern */
        for (int t = 0; t < nnz_i; t++) {
            const int ck = (int)cols[t];
            if (ck >= i)
                break; /* row is sorted: no further k < i */
            const double lik = vals[t] / diag[ck];
            vals[t] = lik;

            const int64_t ms = A_indptr[ck] + (int64_t)ustart[ck];
            const int64_t me = A_indptr[ck + 1];
            int p = t + 1;
            int64_t m = ms;
            while (p < nnz_i && m < me) {
                const int cp = (int)cols[p];
                const int cj = idx32[m];
                if (cp < cj) {
                    p++;
                } else if (cp > cj) {
                    m++;
                } else {
                    vals[p] -= lik * A_data[m];
                    p++;
                    m++;
                }
            }
        }

        diag[i] = (diagpos >= 0) ? vals[diagpos] : 0.0;

        /* gather the finished row back */
        for (int t = 0; t < nnz_i; t++)
            A_data[rs + t] = vals[t];
    }
}

/* -------------------------------------- entry ---------------------------------------- */

void ilu0_fp64(
    double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const int64_t N,
    const int64_t nnz,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    if (N <= 0 || nnz < 0)
        return;

    /* Validate that the matrix is canonical CSR (sorted, no duplicates, every
       diagonal present) and find the widest row. */
    int64_t maxrow = 1;
    int canonical = 1;
    for (int64_t i = 0; i < N; i++) {
        const int64_t s = A_indptr[i];
        const int64_t e = A_indptr[i + 1];
        if (e < s || e > nnz) {
            canonical = 0;
            break;
        }
        if (e - s > maxrow)
            maxrow = e - s;
        int64_t prev = -1;
        int have_diag = 0;
        for (int64_t m = s; m < e; m++) {
            const int64_t c = A_indices[m];
            if (c <= prev || c < 0 || c >= N) {
                canonical = 0;
                break;
            }
            prev = c;
            if (c == i)
                have_diag = 1;
        }
        if (canonical && !have_diag)
            canonical = 0;
        if (!canonical)
            break;
    }

    if (!canonical) {
        ilu0_dense(A_data, A_indices, A_indptr, N);
        return;
    }

    const size_t need = al64((size_t)8 * (size_t)N)
                      + al64((size_t)4 * (size_t)N)
                      + al64((size_t)4 * (size_t)nnz)
                      + al64((size_t)4 * (size_t)maxrow)
                      + al64((size_t)8 * (size_t)maxrow)
                      + 64;
    uint8_t *buf = NULL;
    int from_malloc = 0;
    if (workspace != NULL && workspace_size > 0 && (size_t)workspace_size >= need) {
        buf = (uint8_t *)(((uintptr_t)workspace + 63u) & ~(uintptr_t)63u);
    } else {
        buf = (uint8_t *)malloc(need);
        from_malloc = 1;
    }
    if (buf == NULL) {
        ilu0_dense(A_data, A_indices, A_indptr, N);
        return;
    }

    ilu0_local(A_data, A_indices, A_indptr, N, nnz, maxrow, buf);

    if (from_malloc)
        free(buf);
}
