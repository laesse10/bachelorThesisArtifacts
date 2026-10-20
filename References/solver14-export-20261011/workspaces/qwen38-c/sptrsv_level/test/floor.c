#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <omp.h>
static double now(){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+ts.tv_nsec*1e-9;}
int main(int argc,char**argv){
  const char*d=argc>1?argv[1]:"m3"; int nt=argc>2?atoi(argv[2]):13;
  char p[512];
  #define RD(f,n,sz) do{snprintf(p,512,"%s/%s",d,f);FILE*fp=fopen(p,"rb");void*b=malloc((n)*(sz));if(!b||fread(b,1,(n)*(sz),fp)!=(size_t)(n)*(sz)){fprintf(stderr,"fail %s\n",f);exit(1);}fclose(fp);return b;}while(0)
  int64_t hdr[2];{snprintf(p,512,"%s/hdr.i64",d);FILE*fp=fopen(p,"rb");fread(hdr,8,2,fp);fclose(fp);}
  int64_t N=hdr[0],nnz=hdr[1];
  double*data;{snprintf(p,512,"%s/data.f64",d);FILE*fp=fopen(p,"rb");data=malloc(nnz*8);fread(data,1,nnz*8,fp);fclose(fp);}
  int64_t*idx;{snprintf(p,512,"%s/indices.i64",d);FILE*fp=fopen(p,"rb");idx=malloc(nnz*8);fread(idx,1,nnz*8,fp);fclose(fp);}
  int64_t*indptr;{snprintf(p,512,"%s/indptr.i64",d);FILE*fp=fopen(p,"rb");indptr=malloc((N+1)*8);fread(indptr,1,(N+1)*8,fp);fclose(fp);}
  double*b;{snprintf(p,512,"%s/b.f64",d);FILE*fp=fopen(p,"rb");b=malloc(N*8);fread(b,1,N*8,fp);fclose(fp);}
  double*x=malloc(N*8);
  omp_set_num_threads(nt);
  double best=1e9;
  for(int r=0;r<10;r++){
    memset(x,0,N*8);
    double t0=now();
    #pragma omp parallel num_threads(nt)
    {
      int T=omp_get_num_threads(), tid=omp_get_thread_num();
      int64_t per=(N+T-1)/T, a=tid*per, c=a+per; if(c>N)c=N;
      for(int64_t row=a;row<c;row++){
        int64_t lo=indptr[row], hi=indptr[row+1];
        double s=b[row];
        for(int64_t k=lo;k<hi-1;k++) s-=data[k]*x[idx[k]];
        x[row]=s/data[hi-1];
      }
    }
    double t1=now();
    if(t1-t0<best)best=t1-t0;
  }
  printf("compute floor nt=%d: %.1f us  (%.2f ns/row agg)\n",nt,best*1e6,best*1e9/N);
  return 0;
}
