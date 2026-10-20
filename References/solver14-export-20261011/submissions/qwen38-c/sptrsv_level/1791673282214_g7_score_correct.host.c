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
  if(N<=0) return;
  double xp=0.0;  /* x[row-1] held in a register: kills the store->load
                     forwarding stall and lets the per-row divisions pipeline */
  for(int64_t row=0;row<N;row++){
    int64_t lo=L_indptr[row], hi=L_indptr[row+1];
    double s=b[row];
    double diag=0.0;
    if(hi>lo && L_indices[hi-1]==row){
      diag=L_data[hi-1];
      for(int64_t k=lo;k<hi-1;k++){
        int64_t c=L_indices[k];
        if(c==row-1) s-=L_data[k]*xp;
        else         s-=L_data[k]*x[c];
      }
    } else {
      for(int64_t k=lo;k<hi;k++){
        int64_t c=L_indices[k];
        if(c==row)        diag=L_data[k];
        else if(c==row-1) s-=L_data[k]*xp;
        else if(c<row)    s-=L_data[k]*x[c];
      }
    }
    x[row]=s/diag;
    xp=x[row];
  }
}
