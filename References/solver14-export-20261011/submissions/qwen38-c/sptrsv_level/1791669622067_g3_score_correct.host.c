#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

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
  (void)level_ptr;(void)perm;(void)NNZ_L;(void)workspace;(void)workspace_size;
  for(int64_t r=0;r<N;r++){
    int64_t lo=L_indptr[r], hi=L_indptr[r+1];
    double s=b[r];
    if(hi>lo && L_indices[hi-1]==r){
      double diag=L_data[hi-1];
      const double *d=L_data+lo;
      const int64_t *ic=L_indices+lo;
      for(int64_t k=lo;k<hi-1;k++) s-=d[k-lo]*x[ic[k-lo]];
      x[r]=s/diag;
    } else {
      double diag=0.0;
      for(int64_t k=lo;k<hi;k++){
        int64_t c=L_indices[k];
        if(c<r) s-=L_data[k]*x[c];
        else if(c==r) diag=L_data[k];
      }
      x[r]=s/diag;
    }
  }
}
