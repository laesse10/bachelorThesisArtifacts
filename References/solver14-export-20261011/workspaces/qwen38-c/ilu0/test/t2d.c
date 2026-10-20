#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ilu0_fp64 ilu0_v1fn
#include "../ilu0_v1_serial.c"
#undef ilu0_fp64
#define ilu0_fp64 ilu0_v12fn
#include "../ilu0_v12.c"
#undef ilu0_fp64
static void *rd(const char *f, size_t *n){FILE*fp=fopen(f,"rb");fseek(fp,0,SEEK_END);long sz=ftell(fp);fseek(fp,0,SEEK_SET);void*p=malloc(sz);if(fread(p,1,sz,fp)!=(size_t)sz)exit(2);fclose(fp);*n=sz;return p;}
static void cmp(const char*tag,const double*a,const double*b,int64_t nnz){
  int64_t bad=0,first=-1;
  for(int64_t i=0;i<nnz;i++){double d=a[i]-b[i];if(d<0)d=-d; if(d>1e-12*(1.0+ (a[i]<0?-a[i]:a[i]))) {if(first<0)first=i;bad++;}}
  printf("%s: bad=%lld first=%lld a=%.17g b=%.17g\n",tag,bad,first,bad?a[first]:0,bad?b[first]:0);
}
int main(void){
  size_t n; double*data=rd("data.bin",&n); int64_t nnz=n/8;
  int64_t*indices=rd("indices.bin",&n); int64_t*indptr=rd("indptr.bin",&n);
  int64_t N=n/8-1;
  int64_t wsn=70*N; uint8_t*w1=malloc(wsn),*w2=malloc(wsn),*w3=malloc(wsn);
  double*d1=malloc(8*nnz),*d2=malloc(8*nnz),*d3=malloc(8*nnz);
  memcpy(d1,data,8*nnz); memcpy(d2,data,8*nnz); memcpy(d3,data,8*nnz);
  memset(w1,0,wsn); memset(w2,0,wsn); memset(w3,0,wsn);
  ilu0_v1fn(d1,indices,indptr,N,nnz,w1,wsn);
  ilu0_v12fn(d2,indices,indptr,N,nnz,w2,wsn);          /* cold */
  cmp("v9-cold vs v1",d1,d2,nnz);
  memcpy(d3,data,8*nnz);
  ilu0_v12fn(d3,indices,indptr,N,nnz,w2,wsn);          /* warm, same ws2 */
  cmp("v9-warm vs v1",d1,d3,nnz);
  cmp("v9-warm vs v9-cold",d2,d3,nnz);
  return 0;
}
