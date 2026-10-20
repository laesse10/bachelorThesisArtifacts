#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
extern void amg_setup_csr_fp64(const double*,const int64_t*,const int64_t*,int64_t*,int64_t*,int64_t*,int64_t*,const int64_t,const int64_t,const int64_t,const double,uint8_t*,const int64_t);
int main(int argc, char**argv){
    FILE*f=fopen(argv[1],"rb");
    if(!f){perror("open");return 2;}
    int64_t NX,NY,NZ,theta8;
    /* read theta as double */
    int64_t NX2,NY2,NZ2;
    double theta;
    fread(&NX2,sizeof(int64_t),1,f);
    fread(&NY2,sizeof(int64_t),1,f);
    fread(&NZ2,sizeof(int64_t),1,f);
    fread(&theta,sizeof(double),1,f);
    NX=NX2;NY=NY2;NZ=NZ2;
    int64_t n0=NX*NY*NZ, nnz0=(3*NX-2)*(3*NY-2)*(3*NZ-2);
    double*A_data=malloc(nnz0*sizeof(double));
    int64_t*A_indices=malloc(nnz0*sizeof(int64_t));
    int64_t*A_indptr=malloc((n0+1)*sizeof(int64_t));
    fread(A_data,sizeof(double),nnz0,f);
    fread(A_indices,sizeof(int64_t),nnz0,f);
    fread(A_indptr,sizeof(int64_t),n0+1,f);
    fclose(f);
    int64_t*agg0=malloc(n0*sizeof(int64_t));
    int64_t*level_n=malloc(16*sizeof(int64_t));
    int64_t*level_nnz=malloc(16*sizeof(int64_t));
    int64_t*nlevels=malloc(sizeof(int64_t));
    amg_setup_csr_fp64(A_data,A_indices,A_indptr,agg0,level_n,level_nnz,nlevels,NX,NY,NZ,theta,NULL,0);
    printf("nlevels %lld\n",(long long)nlevels[0]);
    printf("level_n"); for(int i=0;i<16;i++) printf(" %lld",(long long)level_n[i]); printf("\n");
    printf("level_nnz"); for(int i=0;i<16;i++) printf(" %lld",(long long)level_nnz[i]); printf("\n");
    printf("agg0"); for(int64_t i=0;i<n0;i++) printf(" %lld",(long long)agg0[i]); printf("\n");
    return 0;
}
