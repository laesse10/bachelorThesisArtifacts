#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
static double rd(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (double)ts.tv_sec+ts.tv_nsec*1e-9;}
#ifndef V
#define V 0
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
#if V==0
    for(int64_t row=0;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];for(int64_t k=lo;k<hi-1;k++)s-=data[k]*x[idx[k]];x[row]=s/data[hi-1];}
#elif V==1  // x_prev register, branch per offdiag
    double xp=0;
    for(int64_t row=0;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];
      for(int64_t k=lo;k<hi-1;k++){int64_t c=idx[k]; if(c==row-1) s-=data[k]*xp; else s-=data[k]*x[c];}
      x[row]=s/data[hi-1]; xp=x[row];}
#elif V==2  // x_prev fast path: last offdiag is row-1
    double xp=0;
    for(int64_t row=0;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];
      int64_t lasto=hi-2;
      if(lasto>=lo && idx[lasto]==row-1){
        for(int64_t k=lo;k<lasto;k++) s-=data[k]*x[idx[k]];
        s-=data[lasto]*xp;
      } else {
        for(int64_t k=lo;k<hi-1;k++) s-=data[k]*x[idx[k]];
      }
      x[row]=s/data[hi-1]; xp=x[row];}
#elif V==3  // prefetch division input (diag) + next b
    for(int64_t row=0;row<N;row++){
      __builtin_prefetch(&b[row+1],0,1);
      int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];
      for(int64_t k=lo;k<hi-1;k++) s-=data[k]*x[idx[k]];
      x[row]=s/data[hi-1];}
#endif
    double t1=rd();
    if(t1-t0<best)best=t1-t0;
  }
  // correctness with V logic
  memset(x,0,N*8);
#if V==1
  {double xp=0;for(int64_t row=0;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];for(int64_t k=lo;k<hi-1;k++){int64_t c=idx[k];if(c==row-1)s-=data[k]*xp;else s-=data[k]*x[c];}x[row]=s/data[hi-1];xp=x[row];}}
#elif V==2
  {double xp=0;for(int64_t row=0;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];int64_t lasto=hi-2;if(lasto>=lo&&idx[lasto]==row-1){for(int64_t k=lo;k<lasto;k++)s-=data[k]*x[idx[k]];s-=data[lasto]*xp;}else{for(int64_t k=lo;k<hi-1;k++)s-=data[k]*x[idx[k]];}x[row]=s/data[hi-1];xp=x[row];}}
#else
  for(int64_t row=0;row<N;row++){int64_t lo=indptr[row],hi=indptr[row+1];double s=b[row];for(int64_t k=lo;k<hi-1;k++)s-=data[k]*x[idx[k]];x[row]=s/data[hi-1];}
#endif
  double mx=0;for(int64_t i=0;i<N;i++){double e=fabs(x[i]-xref[i]);if(e>mx)mx=e;}
  printf("V=%d: %.1f us (%.2f ns/row) maxerr=%.2e correct=%s\n",V,best*1e6,best*1e9/N,mx,mx<1e-9?"yes":"NO");
  return 0;
}
