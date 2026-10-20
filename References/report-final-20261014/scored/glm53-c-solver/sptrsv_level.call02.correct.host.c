/* Level-scheduled sparse triangular solve:  x = L^-1 b.
 *
 * The schedule (level_ptr, perm) is given; rows of one level are independent and the
 * level loop is sequential.  On this machine a level's work (a few hundred ns on
 * average) is below the cost of synchronizing even two threads, so the solve runs
 * on one thread and the remaining time goes to the memory pattern of the CSR walk.
 */
#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Solve rows perm[lo..hi) of one level.  The reference computes, per row,
 *   s = b[row]; for k: if col<row s -= a*x[col]; elif col==row diag = a;  x = s/diag.
 * Column indices are ascending within a row and L is lower triangular, so when the
 * diagonal is stored it is the LAST entry of the row: the common case skips the
 * per-entry comparison, and a single-entry row is a pure division.  The arithmetic
 * order inside a row is the reference's own, so the results agree bit for bit. */
static void do_rows(const double *restrict L_data, const int64_t *restrict L_indices,
                    const int64_t *restrict L_indptr, const double *restrict b,
                    const int64_t *restrict perm, double *restrict x,
                    int64_t lo, int64_t hi)
{
    for (int64_t idx = lo; idx < hi; idx++) {
        const int64_t row = perm[idx];
        const int64_t rs = L_indptr[row], re = L_indptr[row + 1];
        if (re - rs == 1 && L_indices[rs] == row) {
            x[row] = b[row] / L_data[rs];
        } else {
            double s = b[row];
            const int64_t re1 = re - 1;
            if (L_indices[re1] == row) {
                for (int64_t k = rs; k < re1; k++)
                    s -= L_data[k] * x[L_indices[k]];
                x[row] = s / L_data[re1];
            } else {
                double diag = 0.0;
                for (int64_t k = rs; k < re; k++) {
                    const int64_t c = L_indices[k];
                    if (c < row)       s -= L_data[k] * x[c];
                    else if (c == row) diag = L_data[k];
                }
                x[row] = s / diag;
            }
        }
    }
}

void sptrsv_level_csr_fp64(
    const double *restrict L_data, const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr, const double *b,
    const int64_t *level_ptr, const int64_t *perm, double *restrict x,
    const int64_t N, const int64_t NNZ_L, uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)NNZ_L; (void)workspace; (void)workspace_size;
    if (N <= 0) return;

    /* Levels are contiguous 0..nlev-1: level_ptr is nondecreasing with level_ptr[N]==N,
     * so the first entry holding N is the number of levels; any levels past it hold no
     * rows and the reference's level loop over them is empty. */
    int64_t lo = 0, hi = N;
    while (lo < hi) {
        int64_t m = lo + (hi - lo) / 2;
        if (level_ptr[m] < N) lo = m + 1; else hi = m;
    }
    const int64_t nlev = lo;

    for (int64_t lvl = 0; lvl < nlev; lvl++) {
        const int64_t s = level_ptr[lvl], e = level_ptr[lvl + 1];
        do_rows(L_data, L_indices, L_indptr, b, perm, x, s, e);
    }
}
