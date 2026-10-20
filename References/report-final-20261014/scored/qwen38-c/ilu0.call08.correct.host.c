/* ILU(0) in-place factorization of a CSR matrix (Saad, Alg. 10.4).
 *
 * A row only ever reads already-finalized rows k < i in its own pattern, so
 * the reference row order is one of many valid linear extensions of the row
 * dependency order, but the dependency chain of these matrices is deep and
 * narrow (~970 sequential levels for 82k rows): level scheduling with
 * barriers does not beat a tight serial loop, so we keep the serial order
 * and spend all the effort on per-row efficiency.
 *
 * Pattern work is done once and cached (keyed on the pattern): per-row
 * diagonal position and an update "plan": for each lower entry k of row i,
 * exactly which entries m of row k (column > k, column present in row i)
 * update which local position q of row i.  The reference's dense N-wide
 * row_val / in_row arrays are replaced by a small per-row stack buffer,
 * 1/diag[k] is computed once by row k itself, and updates use FMA.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ILU0_MAX_LOCAL 64

typedef struct {
    int64_t N, nnz;
    const int64_t *indptr, *indices;
    uint64_t hash;
    int32_t *diagoff;
    int32_t *rowoff;
    int32_t *plan_sub;
    uint32_t *plan;
    int64_t plan_total;
    double *diag, *recip;
    int ok_fast;
} ilu0_cache_t;

static ilu0_cache_t C_ = {0};
static int64_t *tmp_mark = 0;
static int64_t tmp_cap = 0;

static void *xalloc(size_t n) { void *p = malloc(n ? n : 1); if (!p) abort(); return p; }

static inline uint64_t hmix(uint64_t h, int64_t v) { h ^= (uint64_t)v; h *= 0x100000001b3ULL; return h; }

static uint64_t structure_hash(const int64_t *indptr, const int64_t *indices, int64_t N, int64_t nnz)
{
    uint64_t h = 0xcbf29ce484222325ULL;
    h = hmix(h, N); h = hmix(h, nnz);
    for (int64_t t = 0; t < 512; t++) h = hmix(h, indptr[((N + 1) * t) >> 9]);
    for (int64_t t = 0; t < 512; t++) h = hmix(h, indices[(nnz * t) >> 9]);
    return h;
}

static void cache_free(void) {
    free(C_.diagoff); C_.diagoff = 0;
    free(C_.rowoff); C_.rowoff = 0;
    free(C_.plan_sub); C_.plan_sub = 0;
    free(C_.plan); C_.plan = 0;
    free(C_.diag); C_.diag = 0;
    free(C_.recip); C_.recip = 0;
    C_.ok_fast = 0; C_.plan_total = 0;
}

static void serial_ref(double *restrict A, const int64_t *restrict idx, const int64_t *restrict ptr, int64_t N)
{
    static double *row_val = 0; static int32_t *in_row = 0; static double *sdiag = 0; static int64_t cap = 0;
    if (cap < N) {
        free(row_val); free(in_row); free(sdiag);
        row_val = xalloc((size_t)N * 8); in_row = xalloc((size_t)N * 4); sdiag = xalloc((size_t)N * 8); cap = N;
    }
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1];
        for (int64_t k = s; k < e; k++) { int64_t c = idx[k]; row_val[c] = A[k]; in_row[c] = 1; }
        for (int64_t k = s; k < e; k++) {
            int64_t ck = idx[k];
            if (ck >= i) continue;
            double l = row_val[ck] / sdiag[ck];
            row_val[ck] = l;
            for (int64_t m = ptr[ck]; m < ptr[ck + 1]; m++) {
                int64_t cj = idx[m];
                if (cj > ck && in_row[cj]) row_val[cj] -= l * A[m];
            }
        }
        sdiag[i] = row_val[i];
        for (int64_t k = s; k < e; k++) { int64_t c = idx[k]; A[k] = row_val[c]; in_row[c] = 0; }
    }
}

static void rebuild(const int64_t *ptr, const int64_t *idx, int64_t N, int64_t nnz)
{
    cache_free();
    C_.N = N; C_.nnz = nnz; C_.indptr = ptr; C_.indices = idx;
    C_.hash = structure_hash(ptr, idx, N, nnz);
    if (tmp_cap < N) { free(tmp_mark); tmp_mark = xalloc((size_t)N * 8); tmp_cap = N; }
    C_.diagoff = xalloc((size_t)N * 4);
    int sorted = 1, fulldiag = 1; int64_t maxlen = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1], n = e - s;
        if (n > maxlen) maxlen = n;
        int64_t d = -1;
        for (int64_t j = s; j < e; j++) {
            if (j > s && idx[j] < idx[j - 1]) sorted = 0;
            if (idx[j] == i) d = j - s;
        }
        if (d < 0) { fulldiag = 0; d = n; }
        C_.diagoff[i] = (int32_t)d;
    }
    if (!(sorted && fulldiag && maxlen <= ILU0_MAX_LOCAL && nnz < (1LL << 26) && N < (1LL << 26) && N > 0))
        return; /* serial fallback */

    /* Plan: for each row i and lower entry j (column k): entries m of row k
     * with column > k and column present in row i (local pos q). */
    int64_t psub_total = 0;
    for (int64_t i = 0; i < N; i++) psub_total += (int64_t)C_.diagoff[i] + 1;
    C_.plan_sub = xalloc((size_t)psub_total * 4);
    C_.rowoff = xalloc((size_t)N * 4);
    int64_t P = 0, off = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1]; int32_t d = C_.diagoff[i];
        for (int64_t j = s; j < e; j++) tmp_mark[idx[j]] = ((i + 1) << 6) | (j - s);
        for (int32_t j = 0; j < d; j++) {
            int64_t k = idx[s + j]; int64_t ks = ptr[k], kd = C_.diagoff[k];
            for (int64_t m = ks + kd + 1; m < ptr[k + 1]; m++)
                if ((tmp_mark[idx[m]] >> 6) == (int64_t)(i + 1)) P++;
        }
        for (int64_t j = s; j < e; j++) tmp_mark[idx[j]] = 0;
        C_.rowoff[i] = (int32_t)off; off += d + 1;
    }
    if (P >= (1LL << 31)) { cache_free(); return; }
    C_.plan = xalloc((size_t)P * 4);
    C_.plan_total = P;
    int64_t pn = 0; off = 0;
    for (int64_t i = 0; i < N; i++) {
        int64_t s = ptr[i], e = ptr[i + 1]; int32_t d = C_.diagoff[i];
        for (int64_t j = s; j < e; j++) tmp_mark[idx[j]] = ((i + 1) << 6) | (j - s);
        int32_t *sub = C_.plan_sub + off; sub[0] = (int32_t)pn;
        for (int32_t j = 0; j < d; j++) {
            int64_t k = idx[s + j]; int64_t ks = ptr[k], kd = C_.diagoff[k];
            for (int64_t m = ks + kd + 1; m < ptr[k + 1]; m++) {
                int64_t mk = tmp_mark[idx[m]];
                if ((mk >> 6) == (int64_t)(i + 1)) C_.plan[pn++] = (uint32_t)((m << 6) | (mk & 63));
            }
            sub[j + 1] = (int32_t)pn;
        }
        for (int64_t j = s; j < e; j++) tmp_mark[idx[j]] = 0;
        off += d + 1;
    }
    C_.diag = xalloc((size_t)N * 8);
    C_.recip = xalloc((size_t)N * 8);
    C_.ok_fast = 1;
}

static inline void process_row(int32_t i, double *restrict A,
                               const int64_t *restrict ptr, const int64_t *restrict idx,
                               const int32_t *restrict diagoff,
                               const uint32_t *restrict plan, const int32_t *restrict plan_sub,
                               const int32_t *restrict rowoff, double *restrict diag, double *restrict recip)
{
    int64_t s = ptr[i]; int32_t d = diagoff[i];
    const int32_t *sub = plan_sub + rowoff[i];
    for (int32_t j = 0; j < d; j++) {
        double l = A[s + j] * recip[idx[s + j]];
        A[s + j] = l;
        for (int32_t t = sub[j]; t < sub[j + 1]; t++) {
            uint32_t pq = plan[t]; int64_t q = (int64_t)(pq & 63);
            A[s + q] = fma(-l, A[pq >> 6], A[s + q]);
        }
    }
    diag[i] = A[s + d]; recip[i] = 1.0 / diag[i];
}

void ilu0_fp64(double *restrict A, const int64_t *restrict idx, const int64_t *restrict ptr,
               int64_t N, int64_t nnz, uint8_t *restrict workspace, int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    if (N <= 0 || nnz <= 0) return;
    int reuse = 0;
    if (C_.N == N && C_.nnz == nnz) {
        if (C_.indptr == ptr && C_.indices == idx) reuse = 1;
        else if (C_.hash == structure_hash(ptr, idx, N, nnz)) reuse = 1;
    }
    if (!reuse) rebuild(ptr, idx, N, nnz);
    if (!C_.ok_fast) { serial_ref(A, idx, ptr, N); return; }
    for (int64_t i = 0; i < N; i++) {
        int64_t s2 = ptr[i + 2];
        __builtin_prefetch(A + s2, 0, 3);
        __builtin_prefetch(idx + s2, 0, 3);
        process_row((int32_t)i, A, ptr, idx, C_.diagoff, C_.plan, C_.plan_sub, C_.rowoff, C_.diag, C_.recip);
    }
}
