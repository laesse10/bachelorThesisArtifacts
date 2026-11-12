/* sptrsv_level: level-scheduled sparse triangular solve L x = b (serial).
 *
 * The level schedule is a topological order of the rows, so a single thread
 * walks perm[0..N) once.  Per row we branch on the entry count (small for
 * these matrices) and unroll the accumulation; the diagonal is the last CSR
 * entry, verified per row with one predicted branch, with a fallback scan
 * that is op-for-op identical to the reference otherwise.
 *
 * Rows are processed in groups of six to expose independent FMA
 * chains to the out-of-order core.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *restrict b,
    const int64_t *restrict level_ptr,
    const int64_t *restrict perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)NNZ_L; (void)workspace; (void)workspace_size; (void)level_ptr;
    if (N <= 0) return;

    int64_t i = 0;
    for (; i + 6 <= N; i += 6) {
        const int64_t row = perm[i];
        const int64_t ra = L_indptr[row], re = L_indptr[row + 1];
        const int64_t nnz = re - ra;
        if (nnz == 1) {
            x[row] = b[row] / L_data[ra];
        } else if (nnz > 1 && L_indices[re - 1] == row) {
            const int64_t *ic = L_indices + ra;
            const double *dc = L_data + ra;
            switch (nnz) {
            case 2: {
                { double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); x[row] = s / dc[1]; }
                break;
            }
            case 3: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s);
                x[row] = s / dc[2];
                break;
            }
            case 4: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s); s = __builtin_fma(-dc[2], x[ic[2]], s);
                x[row] = s / dc[3];
                break;
            }
            default: {
                double s = b[row];
                for (int64_t k = ra; k + 1 < re; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row] = s / L_data[re - 1];
            }
            }
        } else {
            double s = b[row], diag = 0.0;
            for (int64_t k = ra; k < re; k++) {
                const int64_t col = L_indices[k];
                if (col < row) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row) diag = L_data[k];
            }
            x[row] = s / diag;
        }
    
        const int64_t row1 = perm[i + 1];
        const int64_t ra1 = L_indptr[row1], re1 = L_indptr[row1 + 1];
        const int64_t nnz1 = re1 - ra1;
        if (nnz1 == 1) {
            x[row1] = b[row1] / L_data[ra1];
        } else if (nnz1 > 1 && L_indices[re1 - 1] == row1) {
            const int64_t *ic1 = L_indices + ra1;
            const double *dc1 = L_data + ra1;
            switch (nnz1) {
            case 2: {
                { double s = b[row1]; s = __builtin_fma(-dc1[0], x[ic1[0]], s); x[row1] = s / dc1[1]; }
                break;
            }
            case 3: {
                double s = b[row1]; s = __builtin_fma(-dc1[0], x[ic1[0]], s); s = __builtin_fma(-dc1[1], x[ic1[1]], s);
                x[row1] = s / dc1[2];
                break;
            }
            case 4: {
                double s = b[row1]; s = __builtin_fma(-dc1[0], x[ic1[0]], s); s = __builtin_fma(-dc1[1], x[ic1[1]], s); s = __builtin_fma(-dc1[2], x[ic1[2]], s);
                x[row1] = s / dc1[3];
                break;
            }
            default: {
                double s = b[row1];
                for (int64_t k = ra1; k + 1 < re1; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row1] = s / L_data[re1 - 1];
            }
            }
        } else {
            double s = b[row1], diag = 0.0;
            for (int64_t k = ra1; k < re1; k++) {
                const int64_t col = L_indices[k];
                if (col < row1) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row1) diag = L_data[k];
            }
            x[row1] = s / diag;
        }
    
        const int64_t row2 = perm[i + 2];
        const int64_t ra2 = L_indptr[row2], re2 = L_indptr[row2 + 1];
        const int64_t nnz2 = re2 - ra2;
        if (nnz2 == 1) {
            x[row2] = b[row2] / L_data[ra2];
        } else if (nnz2 > 1 && L_indices[re2 - 1] == row2) {
            const int64_t *ic2 = L_indices + ra2;
            const double *dc2 = L_data + ra2;
            switch (nnz2) {
            case 2: {
                { double s = b[row2]; s = __builtin_fma(-dc2[0], x[ic2[0]], s); x[row2] = s / dc2[1]; }
                break;
            }
            case 3: {
                double s = b[row2]; s = __builtin_fma(-dc2[0], x[ic2[0]], s); s = __builtin_fma(-dc2[1], x[ic2[1]], s);
                x[row2] = s / dc2[2];
                break;
            }
            case 4: {
                double s = b[row2]; s = __builtin_fma(-dc2[0], x[ic2[0]], s); s = __builtin_fma(-dc2[1], x[ic2[1]], s); s = __builtin_fma(-dc2[2], x[ic2[2]], s);
                x[row2] = s / dc2[3];
                break;
            }
            default: {
                double s = b[row2];
                for (int64_t k = ra2; k + 1 < re2; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row2] = s / L_data[re2 - 1];
            }
            }
        } else {
            double s = b[row2], diag = 0.0;
            for (int64_t k = ra2; k < re2; k++) {
                const int64_t col = L_indices[k];
                if (col < row2) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row2) diag = L_data[k];
            }
            x[row2] = s / diag;
        }
    
        const int64_t row3 = perm[i + 3];
        const int64_t ra3 = L_indptr[row3], re3 = L_indptr[row3 + 1];
        const int64_t nnz3 = re3 - ra3;
        if (nnz3 == 1) {
            x[row3] = b[row3] / L_data[ra3];
        } else if (nnz3 > 1 && L_indices[re3 - 1] == row3) {
            const int64_t *ic3 = L_indices + ra3;
            const double *dc3 = L_data + ra3;
            switch (nnz3) {
            case 2: {
                { double s = b[row3]; s = __builtin_fma(-dc3[0], x[ic3[0]], s); x[row3] = s / dc3[1]; }
                break;
            }
            case 3: {
                double s = b[row3]; s = __builtin_fma(-dc3[0], x[ic3[0]], s); s = __builtin_fma(-dc3[1], x[ic3[1]], s);
                x[row3] = s / dc3[2];
                break;
            }
            case 4: {
                double s = b[row3]; s = __builtin_fma(-dc3[0], x[ic3[0]], s); s = __builtin_fma(-dc3[1], x[ic3[1]], s); s = __builtin_fma(-dc3[2], x[ic3[2]], s);
                x[row3] = s / dc3[3];
                break;
            }
            default: {
                double s = b[row3];
                for (int64_t k = ra3; k + 1 < re3; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row3] = s / L_data[re3 - 1];
            }
            }
        } else {
            double s = b[row3], diag = 0.0;
            for (int64_t k = ra3; k < re3; k++) {
                const int64_t col = L_indices[k];
                if (col < row3) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row3) diag = L_data[k];
            }
            x[row3] = s / diag;
        }
    
        const int64_t row4 = perm[i + 4];
        const int64_t ra4 = L_indptr[row4], re4 = L_indptr[row4 + 1];
        const int64_t nnz4 = re4 - ra4;
        if (nnz4 == 1) {
            x[row4] = b[row4] / L_data[ra4];
        } else if (nnz4 > 1 && L_indices[re4 - 1] == row4) {
            const int64_t *ic4 = L_indices + ra4;
            const double *dc4 = L_data + ra4;
            switch (nnz4) {
            case 2: {
                { double s = b[row4]; s = __builtin_fma(-dc4[0], x[ic4[0]], s); x[row4] = s / dc4[1]; }
                break;
            }
            case 3: {
                double s = b[row4]; s = __builtin_fma(-dc4[0], x[ic4[0]], s); s = __builtin_fma(-dc4[1], x[ic4[1]], s);
                x[row4] = s / dc4[2];
                break;
            }
            case 4: {
                double s = b[row4]; s = __builtin_fma(-dc4[0], x[ic4[0]], s); s = __builtin_fma(-dc4[1], x[ic4[1]], s); s = __builtin_fma(-dc4[2], x[ic4[2]], s);
                x[row4] = s / dc4[3];
                break;
            }
            default: {
                double s = b[row4];
                for (int64_t k = ra4; k + 1 < re4; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row4] = s / L_data[re4 - 1];
            }
            }
        } else {
            double s = b[row4], diag = 0.0;
            for (int64_t k = ra4; k < re4; k++) {
                const int64_t col = L_indices[k];
                if (col < row4) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row4) diag = L_data[k];
            }
            x[row4] = s / diag;
        }
    
        const int64_t row5 = perm[i + 5];
        const int64_t ra5 = L_indptr[row5], re5 = L_indptr[row5 + 1];
        const int64_t nnz5 = re5 - ra5;
        if (nnz5 == 1) {
            x[row5] = b[row5] / L_data[ra5];
        } else if (nnz5 > 1 && L_indices[re5 - 1] == row5) {
            const int64_t *ic5 = L_indices + ra5;
            const double *dc5 = L_data + ra5;
            switch (nnz5) {
            case 2: {
                { double s = b[row5]; s = __builtin_fma(-dc5[0], x[ic5[0]], s); x[row5] = s / dc5[1]; }
                break;
            }
            case 3: {
                double s = b[row5]; s = __builtin_fma(-dc5[0], x[ic5[0]], s); s = __builtin_fma(-dc5[1], x[ic5[1]], s);
                x[row5] = s / dc5[2];
                break;
            }
            case 4: {
                double s = b[row5]; s = __builtin_fma(-dc5[0], x[ic5[0]], s); s = __builtin_fma(-dc5[1], x[ic5[1]], s); s = __builtin_fma(-dc5[2], x[ic5[2]], s);
                x[row5] = s / dc5[3];
                break;
            }
            default: {
                double s = b[row5];
                for (int64_t k = ra5; k + 1 < re5; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row5] = s / L_data[re5 - 1];
            }
            }
        } else {
            double s = b[row5], diag = 0.0;
            for (int64_t k = ra5; k < re5; k++) {
                const int64_t col = L_indices[k];
                if (col < row5) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row5) diag = L_data[k];
            }
            x[row5] = s / diag;
        }
    
    }
    for (; i < N; i++) {

        const int64_t row = perm[i];
        const int64_t ra = L_indptr[row], re = L_indptr[row + 1];
        const int64_t nnz = re - ra;
        if (nnz == 1) {
            x[row] = b[row] / L_data[ra];
        } else if (nnz > 1 && L_indices[re - 1] == row) {
            const int64_t *ic = L_indices + ra;
            const double *dc = L_data + ra;
            switch (nnz) {
            case 2: {
                { double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); x[row] = s / dc[1]; }
                break;
            }
            case 3: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s);
                x[row] = s / dc[2];
                break;
            }
            case 4: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s); s = __builtin_fma(-dc[2], x[ic[2]], s);
                x[row] = s / dc[3];
                break;
            }
            default: {
                double s = b[row];
                for (int64_t k = ra; k + 1 < re; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row] = s / L_data[re - 1];
            }
            }
        } else {
            double s = b[row], diag = 0.0;
            for (int64_t k = ra; k < re; k++) {
                const int64_t col = L_indices[k];
                if (col < row) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row) diag = L_data[k];
            }
            x[row] = s / diag;
        }
        }
}
