#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
static double rd(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (double)ts.tv_sec+ts.tv_nsec*1e-9;}
#ifndef U
#define U 2
#endif
int main(int argc,char**argv){
  const char*d=argc>1?argv[1]:"m3"; char p[512];
  int64_t hdr[2];{snprintf(p,512,"%s/hdr.i64",d);FILE*fp=fopen(p,"rb");fread(hdr,8,2,fp);fclose(fp);}
  int64_t N=hdr[0],nnz=hdr[1];
  double*data;{snprintf(p,512,"%s/data.f64",d);FILE*fp=fopen(p,"rb");data=malloc(nnz*8);fread(data,1,nnz*8,fp);fclose(fp);}
  int64_t*idx;{snprintf(p,512,"%s/indices.i64",d);FILE*fp=fopen(p,"rb");idx=malloc(nnz*8);fread(idx,1,nnz*8,fp);fclose(fp);}
  int64_t*indptr;{snprintf(p,512,"%s/indptr.i64",d);FILE*fp=fopen(p,"rb");indptr=malloc((N+1)*8);fread(indptr,1,(N+1)*8,fp);fclose(fp);}
  double*b;{snprintf(p,512,"%s/b.f64",d);FILE*fp=fopen(p,"rb");b=malloc(N*8);fread(b,1,N*8,fp);fclose(fp);}
  double*x=malloc(N*8);
  double*xref;{snprintf(p,512,"%s/xref.f64",d);FILE*fp=fopen(p,"rb");xref=malloc(N*8);fread(xref,1,N*8,fp);fclose(fp);}
  double best=1e9;
  for(int r=0;r<20;r++){
    memset(x,0,N*8);
    double t0=rd();
#if U==1
    for(int64_t row=0;row<N;row++){
      int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];
      for(int64_t k=lo;k<hi-1;k++) s-=data[k]*x[idx[k]];
      x[row]=s/data[hi-1];
    }
#elif U>=2
    int64_t row=0;
    for(;row+U<=N;row+=U){
      // compute U consecutive rows in order, each after its predecessor's x
      for(int64_t j=0;j<U;j++){
        int64_t rr=row+j;
        int64_t lo=indptr[rr],hi=indptr[rr+1];double s=b[rr];
        for(int64_t k=lo;k<hi-1;k++) s-=data[k]*x[idx[k]];
        x[rr]=s/data[hi-1];
      }
    }
    for(;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];for(int64_t k=lo;k<hi-1;k++)s-=data[k]*x[idx[k]];x[row]=s/data[hi-1];}
#endif
    double t1=rd();
    if(t1-t0<best)best=t1-t0;
  }
  // correctness
  memset(x,0,N*8);
  { int64_t row=0;
#if U>=2
    for(;row+U<=N;row+=U) for(int64_t j=0;j<U;j++){int64_t rr=row+j;int64_t lo=indptr[rr],hi=indptr[rr+1];double s=b[rr];for(int64_t k=lo;k<hi-1;k++)s-=data[k]*x[idx[k]];x[rr]=s/data[hi-1];}
#endif
    for(;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];for(int64_t k=lo;k<hi-1;k++)s-=data[k]*x[idx[k]];x[row]=s/data[hi-1];}
  }
  double mx=0;for(int64_t i=0;i<N;i++){double e=fabs(x[i]-xref[i]);if(e>mx)mx=e;}
  printf("U=%d: %.1f us (%.2f ns/row) maxerr=%.2e correct=%s\n",U,best*1e6,best*1e9/N,mx,mx<1e-9?"yes":"NO");
  return 0;
}
