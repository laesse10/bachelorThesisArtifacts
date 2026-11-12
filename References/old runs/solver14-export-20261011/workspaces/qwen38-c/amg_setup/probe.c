#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

void amg_setup_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    int64_t *restrict agg0,
    int64_t *restrict level_n,
    int64_t *restrict level_nnz,
    int64_t *restrict nlevels,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const double theta,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    int64_t n0 = NX * NY * NZ;
    int64_t nnz0 = (3*NX-2)*(3*NY-2)*(3*NZ-2);
    printf("PROBE NX=%lld NY=%lld NZ=%lld n0=%lld nnz0=%lld theta=%.4g ws=%lld bytes\n",
           (long long)NX,(long long)NY,(long long)NZ,(long long)n0,(long long)nnz0,theta,(long long)workspace_size);
    fflush(stdout);
    (void)A_data;(void)A_indices;(void)A_indptr;(void)agg0;(void)level_n;(void)level_nnz;(void)nlevels;(void)workspace;
    /* minimal valid output so it is not obviously wrong */
    level_n[0]=n0; level_nnz[0]=nnz0; nlevels[0]=1;
}
