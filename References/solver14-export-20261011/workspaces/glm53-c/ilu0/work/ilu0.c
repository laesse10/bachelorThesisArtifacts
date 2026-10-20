#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#if defined(__x86_64__)
#include <immintrin.h>
#define cpu_relax() _mm_pause()
#else
#define cpu_relax() do {} while (0)
#endif

/* ---------------- fallback: exact transliteration of the NumPy reference ------------- */
static void ilu0_ref(double *restrict A_data, const int64_t *restrict A_indices,
                     const int64_t *restrict A_indptr, const int64_t N)
{
    double *diag = (double *)malloc(sizeof(double) * (size_t)N);
    double *row_val = (double *)calloc((size_t)N, sizeof(double));
    int64_t *in_row = (int64_t *)calloc((size_t)N, sizeof(int64_t));
    for (int64_t i = 0; i < N; i++) {
        const int64_t row_start = A_indptr[i], row_end = A_indptr[i + 1];
        for (int64_t k = row_start; k < row_end; k++) {
            const int64_t col = A_indices[k];
            row_val[col] = A_data[k];
            in_row[col] = 1;
        }
        for (int64_t k = row_start; k < row_end; k++) {
            const int64_t col_k = A_indices[k];
            if (col_k >= i) continue;
            const double l_ik = row_val[col_k] / diag[col_k];
            row_val[col_k] = l_ik;
            for (int64_t m = A_indptr[col_k]; m < A_indptr[col_k + 1]; m++) {
                const int64_t col_j = A_indices[m];
                if (col_j > col_k && in_row[col_j] == 1)
                    row_val[col_j] = row_val[col_j] - l_ik * A_data[m];
            }
        }
        diag[i] = row_val[i];
        for (int64_t k = row_start; k < row_end; k++) {
            const int64_t col = A_indices[k];
            A_data[k] = row_val[col];
            in_row[col] = 0;
        }
    }
    free(diag); free(row_val); free(in_row);
}

/* ---------------- fast path ---------------------------------------------------------- */
typedef struct { int32_t ustart; int32_t uend; } uinfo_t;

/* Setup: validates (strictly increasing indices, in range, full diagonal, symmetric pattern),
   computes uinfo[r] = range of row r's upper part, ready[r] = #deps of row r, maxdeg. */
static int setup_fast(const int64_t *restrict A_indices, const int64_t *restrict A_indptr,
                      const int64_t N, uinfo_t *restrict uinfo, int32_t *restrict ready,
                      int32_t *restrict coldeg, int64_t *pmaxdeg)
{
    int ok = 1;
    int64_t maxdeg = 0;
    #pragma omp parallel for reduction(& : ok) reduction(max : maxdeg) schedule(static)
    for (int64_t r = 0; r < N; r++) {
        const int64_t rs = A_indptr[r], re = A_indptr[r + 1];
        const int64_t deg = re - rs;
        if (deg > maxdeg) maxdeg = deg;
        int64_t u = rs, prev = -1, nlow = 0;
        for (int64_t m = rs; m < re; m++) {
            const int64_t c = A_indices[m];
            if (c <= prev || c >= N) { ok = 0; break; }
            prev = c;
            if (c < r) { u = m + 1; nlow++; }
            else if (c == r) { u = m + 1; }
            __atomic_fetch_add(&coldeg[c], 1, __ATOMIC_RELAXED);
        }
        uinfo[r].ustart = (int32_t)u;
        uinfo[r].uend = (int32_t)re;
        ready[r] = (int32_t)nlow;
        if (u <= rs || A_indices[u - 1] != r) ok = 0;   /* diagonal present */
    }
    /* symmetry check: column degree must equal row degree */
    #pragma omp parallel for reduction(& : ok) schedule(static)
    for (int64_t r = 0; r < N; r++)
        if (coldeg[r] != (int32_t)(A_indptr[r + 1] - A_indptr[r])) ok = 0;
    *pmaxdeg = maxdeg;
    return ok;
}

/* Serial fast path */
static void ilu0_fast_serial(double *restrict A_data, const int64_t *restrict A_indices,
                             const int64_t *restrict A_indptr, const int64_t N,
                             double *restrict diag, uint32_t *restrict pos,
                             uinfo_t *restrict uinfo, double *restrict local)
{
    for (int64_t i = 0; i < N; i++) {
        const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
        const int64_t deg = re - rs;
        const int64_t *restrict rowi = A_indices + rs;
        double *restrict rowd = A_data + rs;
        int64_t diagpos = -1;
        for (int64_t j = 0; j < deg; j++) {
            pos[rowi[j]] = (uint32_t)j;
            local[j] = rowd[j];
            diagpos = (rowi[j] == i) ? j : diagpos;
        }
        for (int64_t k = 0; k < deg; k++) {
            const int64_t col_k = rowi[k];
            if (col_k >= i) break;
            const double l_ik = local[k] / diag[col_k];
            local[k] = l_ik;
            const uinfo_t ui = uinfo[col_k];
            for (int64_t m = ui.ustart; m < ui.uend; m++) {
                const int64_t col_j = A_indices[m];
                const uint32_t p = pos[col_j];
                if (p < (uint32_t)deg && rowi[p] == col_j)
                    local[p] -= l_ik * A_data[m];
            }
        }
        diag[i] = local[diagpos];
        for (int64_t j = 0; j < deg; j++) rowd[j] = local[j];
    }
}

/* Parallel wavefront fast path: ready-counter pipelining over row blocks */
static void ilu0_fast_parallel(double *restrict A_data, const int64_t *restrict A_indices,
                               const int64_t *restrict A_indptr, const int64_t N,
                               double *restrict diag, uint32_t *restrict posT,
                               uinfo_t *restrict uinfo, int32_t *restrict ready,
                               double *restrict localT, const int64_t maxdeg, const int T)
{
    #pragma omp parallel num_threads(T)
    {
        const int tid = omp_get_thread_num();
        uint32_t *restrict pos = posT + (size_t)tid * (size_t)N;
        double *restrict local = localT + (size_t)tid * (size_t)maxdeg;
        const int64_t b0 = (int64_t)(( (__int128)tid * N) / T);
        const int64_t b1 = (int64_t)(( (__int128)(tid + 1) * N) / T);
        for (int64_t i = b0; i < b1; i++) {
            while (__atomic_load_n(&ready[i], __ATOMIC_ACQUIRE) != 0) cpu_relax();
            const int64_t rs = A_indptr[i], re = A_indptr[i + 1];
            const int64_t deg = re - rs;
            const int64_t *restrict rowi = A_indices + rs;
            double *restrict rowd = A_data + rs;
            int64_t diagpos = -1;
            for (int64_t j = 0; j < deg; j++) {
                pos[rowi[j]] = (uint32_t)j;
                local[j] = rowd[j];
                diagpos = (rowi[j] == i) ? j : diagpos;
            }
            for (int64_t k = 0; k < deg; k++) {
                const int64_t col_k = rowi[k];
                if (col_k >= i) break;
                const double l_ik = local[k] / diag[col_k];
                local[k] = l_ik;
                const uinfo_t ui = uinfo[col_k];
                for (int64_t m = ui.ustart; m < ui.uend; m++) {
                    const int64_t col_j = A_indices[m];
                    const uint32_t p = pos[col_j];
                    if (p < (uint32_t)deg && rowi[p] == col_j)
                        local[p] -= l_ik * A_data[m];
                }
            }
            diag[i] = local[diagpos];
            for (int64_t j = 0; j < deg; j++) rowd[j] = local[j];
            for (int64_t m = uinfo[i].ustart; m < uinfo[i].uend; m++)
                __atomic_fetch_sub(&ready[A_indices[m]], 1, __ATOMIC_RELEASE);
        }
    }
}

void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size)
{
    if (N <= 0) return;
    if (N < 64) { ilu0_ref(A_data, A_indices, A_indptr, N); return; }
    const int T = omp_get_max_threads() > (int)(N / 64) ? (int)(N / 64) : omp_get_max_threads();
    if (T < 1) { ilu0_ref(A_data, A_indices, A_indptr, N); return; }

    int64_t maxdeg = 0;
    if (T == 1) {
        /* serial: diag 8N | uinfo 8N | pos 4N | local 8*maxdeg */
        const int64_t need = 24 * N + 1024;
        uint8_t *base = (workspace && workspace_size >= need) ? workspace
                       : (uint8_t *)malloc((size_t)need + 8 * (size_t)N);
        if (!base) { ilu0_ref(A_data, A_indices, A_indptr, N); return; }
        uintptr_t al = ((uintptr_t)base + 63) & ~(uintptr_t)63;
        double *diag = (double *)al;
        uinfo_t *uinfo = (uinfo_t *)(diag + N);
        uint32_t *pos = (uint32_t *)(uinfo + N);
        int32_t *ready = NULL, *coldeg = (int32_t *)calloc((size_t)N, 4);
        int ok = coldeg && setup_fast(A_indices, A_indptr, N, uinfo, ready, coldeg, &maxdeg);
        free(coldeg);
        if (ok) ilu0_fast_serial(A_data, A_indices, A_indptr, N, diag, pos, uinfo,
                                 (double *)(pos + N));
        else ilu0_ref(A_data, A_indices, A_indptr, N);
        if (base != workspace) free(base);
        (void)nnz; (void)ready;
        return;
    }

    /* parallel: diag 8N | uinfo 8N | ready 4N | coldeg 4N | pos T*4N | local T*8*maxdeg */
    const int64_t need = 24 * N + 8 * N + (int64_t)T * 4 * N + 1024;
    uint8_t *base = (workspace && workspace_size >= need) ? workspace
                   : (uint8_t *)malloc((size_t)need + (int64_t)T * 8 * N);
    if (!base) { ilu0_ref(A_data, A_indices, A_indptr, N); return; }
    uintptr_t al = ((uintptr_t)base + 63) & ~(uintptr_t)63;
    int owned = base != workspace;
    double *diag = (double *)al;
    uinfo_t *uinfo = (uinfo_t *)(diag + N);
    int32_t *ready = (int32_t *)(uinfo + N);
    int32_t *coldeg = ready + N;
    uint32_t *posT = (uint32_t *)(coldeg + N);
    double *localT = (double *)(posT + (size_t)T * N);
    memset(coldeg, 0, sizeof(int32_t) * (size_t)N);
    int ok = setup_fast(A_indices, A_indptr, N, uinfo, ready, coldeg, &maxdeg);
    if (ok) {
        ilu0_fast_parallel(A_data, A_indices, A_indptr, N, diag, posT, uinfo, ready,
                           localT, maxdeg, T);
    } else {
        ilu0_ref(A_data, A_indices, A_indptr, N);
    }
    if (owned) free(base);
    (void)nnz;
}
