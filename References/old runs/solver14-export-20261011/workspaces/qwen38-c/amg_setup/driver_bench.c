#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
extern void amg_setup_csr_fp64(const double*,const int64_t*,const int64_t*,int64_t*,int64_t*,int64_t*,int64_t*,const int64_t,const int64_t,const int64_t,const double,uint8_t*,const int64_t);
int main(int argc, char**argv){
    FILE*f=fopen(argv[1],"rb"); int64_t NX,NY,NZ; double theta;
    fread(&NX,8,1,f);fread(&NY,8,1,f);fread(&NZ,8,1,f);fread(&theta,8,1,f);
    int64_t n0=NX*NY*NZ, nnz0=(3*NX-2)*(3*NY-2)*(3*NZ-2);
    double*A_data=malloc(nnz0*8); int64_t*A_indices=malloc(nnz0*8); int64_t*A_indptr=malloc((n0+1)*8);
    fread(A_data,8,nnz0,f);fread(A_indices,8,nnz0,f);fread(A_indptr,8,n0+1,f);fclose(f);
    int64_t*agg0=malloc(n0*8);int64_t*level_n=malloc(16*8);int64_t*level_nnz=malloc(16*8);int64_t*nlevels=malloc(8);
    amg_setup_csr_fp64(A_data,A_indices,A_indptr,agg0,level_n,level_nnz,nlevels,NX,NY,NZ,theta,NULL,0); // warmup
    double t0=omp_get_wtime();
    int iters=atoi(argv[2]);
    for(int it=0;it<iters;it++) amg_setup_csr_fp64(A_data,A_indices,A_indptr,agg0,level_n,level_nnz,nlevels,NX,NY,NZ,theta,NULL,0);
    double t1=omp_get_wtime();
    printf("nlevels %lld level_n", (long long)nlevels[0]);
    for(int i=0;i<nlevels[0];i++)printf(" %lld",(long long)level_n[i]);
    printf("\navg_ms %.3f\n",(t1-t0)/iters*1000.0);
    return 0;
}
