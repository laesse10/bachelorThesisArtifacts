#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
void ilu0_fp64(double *restrict, const int64_t *restrict, const int64_t *restrict,
               const int64_t, const int64_t, uint8_t *restrict, const int64_t);
static void *rd(const char *f, size_t *n){FILE*fp=fopen(f,"rb");if(!fp){perror(f);exit(1);}
  fseek(fp,0,SEEK_END);long sz=ftell(fp);fseek(fp,0,SEEK_SET);void*p=malloc(sz);
  if(fread(p,1,sz,fp)!=(size_t)sz)exit(2);fclose(fp);*n=sz;return p;}
int main(int argc,char**argv){
    size_t n; double*data=rd("data.bin",&n); int64_t nnz=n/8;
    int64_t*indices=rd("indices.bin",&n); int64_t*indptr=rd("indptr.bin",&n);
    int64_t N=n/8-1;
    int wsn = argc>1?atoi(argv[1]):800;
    int reps= argc>2?atoi(argv[2]):9;
    int64_t ws=(int64_t)wsn*N;
    uint8_t*w=aligned_alloc(64,ws+64); memset(w,0,ws);
    double data0_copy[1]; (void)data0_copy;
    // save original data for repeated correct runs
    double*orig=malloc(8*nnz); memcpy(orig,data,8*nnz);
    double best=1e30,sum=0; int ok=1;
    for(int r=0;r<reps;r++){
        memcpy(data,orig,8*nnz);
        double t0=omp_get_wtime();
        ilu0_fp64(data,indices,indptr,N,nnz,w,ws);
        double t1=omp_get_wtime();
        sum+=t1-t0; if(t1-t0<best)best=t1-t0;
    }
    printf("best %.4f ms  avg %.4f ms  (N=%ld nnz=%ld)\n",best*1e3,sum/reps*1e3,N,nnz);
    FILE*fp=fopen("out.bin","wb"); fwrite(data,8,nnz,fp); fclose(fp);
    free(data);free(indices);free(indptr);free(w);free(orig);
    return 0;
}
