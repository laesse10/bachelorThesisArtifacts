#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ilu0_fp64 ilu0_v1fn
#include "../ilu0_v1_serial.c"
#undef ilu0_fp64
#define ilu0_fp64 ilu0_v9fn
#include "../ilu0_v9.c"
#undef ilu0_fp64
static void *rd(const char *f, size_t *n){FILE*fp=fopen(f,"rb");fseek(fp,0,SEEK_END);long sz=ftell(fp);fseek(fp,0,SEEK_SET);void*p=malloc(sz);if(fread(p,1,sz,fp)!=(size_t)sz)exit(2);fclose(fp);*n=sz;return p;}
int main(void){
  size_t n; double*data=rd("data.bin",&n); int64_t nnz=n/8;
  int64_t*indices=rd("indices.bin",&n); int64_t*indptr=rd("indptr.bin",&n);
  int64_t N=n/8-1;
  int64_t wsn=70*N; uint8_t*w1=malloc(wsn),*w2=malloc(wsn);
  double*d1=malloc(8*nnz),*d2=malloc(8*nnz);
  memcpy(d1,data,8*nnz); memcpy(d2,data,8*nnz);
  memset(w1,0,wsn); memset(w2,0,wsn);
  ilu0_v1fn(d1,indices,indptr,N,nnz,w1,wsn);
  ilu0_v9fn(d2,indices,indptr,N,nnz,w2,wsn);
  int64_t rbad=-1;
  for(int64_t r=0;r<N;r++){
    for(int64_t p=indptr[r];p<indptr[r+1];p++){
      double a=d1[p],b=d2[p],d=a-b;if(d<0)d=-d; double sc=1.0+(a<0?-a:a);
      if(d>1e-12*sc){ if(rbad<0){printf("first bad row=%lld col=%lld idx=%lld\n",r,indices[p],p);
        printf("row %lld cols:",r); for(int64_t q=indptr[r];q<indptr[r+1];q++) printf(" %lld",indices[q]);
        printf("\n v1:"); for(int64_t q=indptr[r];q<indptr[r+1];q++) printf(" %.12g",d1[q]);
        printf("\n v9:"); for(int64_t q=indptr[r];q<indptr[r+1];q++) printf(" %.12g",d2[q]);
        printf("\n"); rbad=r; } break; }
    }
    if(rbad>=0) break;
  }
  printf("rbad=%lld\n",rbad);
  return 0;
}
