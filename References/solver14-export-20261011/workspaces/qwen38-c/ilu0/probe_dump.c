#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
void ilu0_fp64(double *restrict A_data, const int64_t *restrict A_indices,
               const int64_t *restrict A_indptr, const int64_t N, const int64_t nnz,
               uint8_t *restrict workspace, const int64_t workspace_size) {
    (void)A_data;(void)workspace;(void)workspace_size;
    printf("IPTR ");
    for (int64_t i=0;i<=N;i++) printf("%lld ", (long long)A_indptr[i]);
    printf("\nIND ");
    for (int64_t i=0;i<nnz;i++) printf("%lld ", (long long)A_indices[i]);
    printf("\n");
    fflush(stdout);
}
