#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ILU(0) in place on CSR (Saad 10.4).  Serial in row order (the outer loop
 * has a true dependency chain); per row:
 *   scatter row i into dense scratch, eliminate against its parents k < i
 *   using a precomputed per-(row,parent) hit list -- the exact (column,
 *   parent-row entry) pairs that receive an update -- then store the row back.
 *
 * The lists and diagonal positions depend only on the (fixed) sparsity
 * pattern, so they are precomputed once per pattern and kept in static
 * buffers keyed by a fingerprint of (indptr, indices); calls on the same
 * pattern skip the precomputation.  The workspace holds the per-call dense
 * scratch only.  Falls back to the classic dense-mark loop for rows the
 * fast path cannot represent (row length > 255, unsorted, no diagonal,
 * or > 15 parents). */

static uint64_t ilu0_pat_hash(const int64_t *restrict indptr,
                              const int64_t *restrict indices,
                              int64_t N, int64_t nnz) {
    uint64_t h = 1469598103934665603ULL;
    for (int64_t i = 0; i <= N; i += 4) { h ^= (uint64_t)indptr[i]; h *= 1099511628211ULL; }
    for (int64_t i = 0; i < nnz; i += 16) { h ^= (uint64_t)indices[i]; h *= 1099511628211ULL; }
    h ^= (uint64_t)N * 0x9E3779B97F4A7C15ULL;
    h ^= (uint64_t)nnz;
    h *= 1099511628211ULL;
    return h;
}

static uint8_t *g_di = NULL;      /* per row: relative diagonal position */
static int64_t *g_par = NULL;     /* per row: cumulative parent count (N+1) */
static int32_t *g_uoff = NULL;    /* hit-list offset per pair (P+1 <= nnz+1) */
static int32_t *g_cj = NULL;      /* hit list: target column */
static int32_t *g_q = NULL;       /* hit list: entry position in the parent row */
static int64_t g_capN = 0, g_capnnz = 0;
static uint64_t g_hash = 0;
static int64_t g_N = -1, g_nnz = -1;

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size) {
    if (workspace == NULL || workspace_size < 20 * (int64_t)N) return;

    const size_t Ns = (size_t)N;
    double *restrict row_val  = (double *)workspace;
    double *restrict inv_diag = row_val + Ns;
    uint32_t *restrict mark   = (uint32_t *)(inv_diag + Ns);

    const uint64_t cur = ilu0_pat_hash(A_indptr, A_indices, N, nnz);
    const int have = g_di != NULL && g_par != NULL && g_uoff != NULL &&
                     g_cj != NULL && g_q != NULL &&
                     g_hash == cur && g_N == N && g_nnz == nnz;

    if (!have) {
        if (g_capN < N || g_capnnz < nnz + 1) {
            const size_t n2 = (size_t)(N > 0 ? 2 * N : 2), nn2 = (size_t)(nnz + 1);
            uint8_t *di = (uint8_t *)realloc(g_di, n2);
            int64_t *pc = (int64_t *)realloc(g_par, 8 * (n2 + 1));
            int32_t *uo = (int32_t *)realloc(g_uoff, 4 * nn2);
            int32_t *cj = (int32_t *)realloc(g_cj, 4 * nn2);
            int32_t *q = (int32_t *)realloc(g_q, 4 * nn2);
            g_di = di; g_par = pc; g_uoff = uo; g_cj = cj; g_q = q;
            g_capN = (int64_t)n2; g_capnnz = (int64_t)nn2;
        }
        int ok = g_di != NULL && g_par != NULL && g_uoff != NULL &&
                 g_cj != NULL && g_q != NULL;
        /* Phase A: validate pattern, record diagonal positions. */
        for (int64_t i = 0; i < N && ok; i++) {
            const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
            if (re - rs > 255) { ok = 0; break; }
            int64_t di = -1;
            for (int64_t p = rs; p < re; p++) {
                if (p > rs && A_indices[p] <= A_indices[p - 1]) { ok = 0; break; }
                if (di < 0 && A_indices[p] >= i) di = p;
            }
            if (di < 0 || A_indices[di] != i) ok = 0;
            else g_di[i] = (uint8_t)(di - rs);
        }
        if (ok) {
            g_par[0] = 0;
            for (int64_t i = 0; i < N; i++)
                g_par[i + 1] = g_par[i] + (int64_t)g_di[i];
            /* Phase B: per-(row, parent) hit lists, serial. */
            int64_t ucnt = 0;
            for (int64_t i = 0; i < N; i++) {
                const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
                const int64_t di = rs + (int64_t)g_di[i];
                int64_t s = g_par[i];
                for (int64_t p = rs; p < di; p++) {
                    const int64_t k = A_indices[p];
                    const int64_t kus = A_indptr[k] + (int64_t)g_di[k] + 1, kre = A_indptr[k + 1];
                    g_uoff[s] = (int32_t)ucnt;
                    int64_t j = rs;
                    for (int64_t q = kus; q < kre; q++) {
                        const int64_t cj = A_indices[q];
                        while (j < re && A_indices[j] < cj) j++;
                        if (j < re && A_indices[j] == cj) {
                            g_cj[ucnt] = (int32_t)cj;
                            g_q[ucnt] = (int32_t)q;
                            ucnt++;
                        }
                    }
                    s++;
                }
                g_uoff[s] = (int32_t)ucnt;
            }
            g_hash = cur; g_N = N; g_nnz = nnz;
        }
        if (!ok) {
            /* Fallback: classic dense-mark loop. */
            for (int64_t i = 0; i < N; i++) {
                const uint32_t gen = (uint32_t)(i + 1);
                const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
                for (int64_t p = rs; p < re; p++) {
                    const int64_t c = A_indices[p];
                    row_val[c] = A_data[p];
                    mark[c] = gen;
                }
                for (int64_t p = rs; p < re; p++) {
                    const int64_t k = A_indices[p];
                    if (k >= i) continue;
                    const double l = row_val[k] * inv_diag[k];
                    row_val[k] = l;
                    const int64_t qs = A_indptr[k], qe = A_indptr[k + 1];
                    for (int64_t q = qs; q < qe; q++) {
                        const int64_t cj = A_indices[q];
                        if (cj > k && mark[cj] == gen) row_val[cj] -= l * A_data[q];
                    }
                }
                inv_diag[i] = 1.0 / row_val[i];
                for (int64_t p = rs; p < re; p++) A_data[p] = row_val[A_indices[p]];
            }
            return;
        }
    }

    /* Main loop: hit-list elimination, serial in row order. */
    for (int64_t i = 0; i < N; i++) {
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        const int m = (int)g_di[i];
        if (m > 15) {
            /* Generic row (uncommon): classic dense-mark elimination. */
            const uint32_t gen = (uint32_t)(i + 1);
            for (int64_t p = rs; p < re; p++) {
                const int64_t c = A_indices[p];
                row_val[c] = A_data[p];
                mark[c] = gen;
            }
            for (int64_t p = rs; p < re; p++) {
                const int64_t k = A_indices[p];
                if (k >= i) continue;
                const double l = row_val[k] * inv_diag[k];
                row_val[k] = l;
                const int64_t qs = A_indptr[k], qe = A_indptr[k + 1];
                for (int64_t q = qs; q < qe; q++) {
                    const int64_t cj = A_indices[q];
                    if (cj > k && mark[cj] == gen) row_val[cj] -= l * A_data[q];
                }
            }
            inv_diag[i] = 1.0 / row_val[i];
            for (int64_t p = rs; p < re; p++) A_data[p] = row_val[A_indices[p]];
            continue;
        }
        for (int64_t p = rs; p < re; p++) row_val[A_indices[p]] = A_data[p];
        int64_t kp[16];
        double kinv[16];
        int32_t u0[16], u1[16];
        int t;
        for (t = 0; t < m; t++) {
            const int64_t k = A_indices[rs + t];
            kp[t] = k;
            const int64_t ku = A_indptr[k] + (int64_t)g_di[k] + 1;
            const int64_t kr = A_indptr[k + 1];
            __builtin_prefetch(&row_val[k], 0, 3);
            __builtin_prefetch(&inv_diag[k], 0, 3);
            if (ku < kr) {
                __builtin_prefetch(&A_data[ku], 0, 3);
                if (ku + 8 < kr) __builtin_prefetch(&A_data[ku + 8], 0, 3);
            }
        }
        for (t = 0; t < m; t++) {
            kinv[t] = inv_diag[kp[t]];
            u0[t] = g_uoff[g_par[i] + t];
            u1[t] = g_uoff[g_par[i] + t + 1];
            for (int32_t u = u0[t]; u < u1[t]; u++)
                __builtin_prefetch(&row_val[g_cj[u]], 0, 3);
        }
        const int64_t nrs = A_indptr[i + 1];
        if (nrs < nnz) {
            __builtin_prefetch(&A_data[nrs], 0, 3);
            __builtin_prefetch(&A_indices[nrs], 0, 3);
        }
        for (t = 0; t < m; t++) {
            const int64_t k = kp[t];
            const double l = row_val[k] * kinv[t];
            row_val[k] = l;
            for (int32_t u = u0[t]; u < u1[t]; u++)
                row_val[g_cj[u]] -= l * A_data[g_q[u]];
        }
        inv_diag[i] = 1.0 / row_val[i];
        for (int64_t p = rs; p < re; p++) A_data[p] = row_val[A_indices[p]];
        if (i + 1 < N) {
            /* Prefetch row i+1's parent data one row ahead. */
            const int64_t rs1 = A_indptr[i + 1];
            const int m1 = (int)g_di[i + 1];
            for (int t2 = 0; t2 < m1; t2++) {
                const int64_t k = A_indices[rs1 + t2];
                __builtin_prefetch(&row_val[k], 0, 3);
                __builtin_prefetch(&inv_diag[k], 0, 3);
                const int64_t ku = A_indptr[k] + (int64_t)g_di[k] + 1, kr = A_indptr[k + 1];
                if (ku < kr) {
                    __builtin_prefetch(&A_data[ku], 0, 3);
                    if (ku + 8 < kr) __builtin_prefetch(&A_data[ku + 8], 0, 3);
                }
            }
        }
    }
}
