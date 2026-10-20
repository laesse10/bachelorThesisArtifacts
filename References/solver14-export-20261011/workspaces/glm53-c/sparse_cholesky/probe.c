#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

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
    int64_t N = EDGE*EDGE*EDGE;
    printf("EDGE=%lld N=%lld workspace_size=%lld maxthreads=%d\n",
           (long long)EDGE, (long long)N, (long long)workspace_size, omp_get_max_threads());
    printf("nnzL=%lld nnzA=%lld\n",
           (long long)Lc_indptr[N], (long long)A_indptr[N]);
    fflush(stdout);
    for (int64_t j = 0; j < N; j++) y[j] = b[j];
}
