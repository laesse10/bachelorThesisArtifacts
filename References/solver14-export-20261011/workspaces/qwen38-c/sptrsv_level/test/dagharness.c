#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include "../dag.c"
static char pathbuf[512];
static void mkpath(const char*d,const char*f){snprintf(pathbuf,512,"%s/%s",d,f);}
static void* rd(const char*d,const char*f,size_t n,size_t sz){
  mkpath(d,f); FILE*fp=fopen(pathbuf,"rb"); if(!fp){fprintf(stderr,"open %s fail\n",pathbuf);exit(1);}
  void*buf=malloc(n*sz); size_t g=fread(buf,1,n*sz,fp);
  if(g!=n*sz){fprintf(stderr,"short read %s\n",pathbuf);exit(1);} fclose(fp); return buf;
}
static double now(){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+ts.tv_nsec*1e-9;}
int main(int argc,char**argv){
  const char*d=argc>1?argv[1]:"m3";
  int reps=argc>2?atoi(argv[2]):20;
  int64_t hdr[2]; {mkpath(d,"hdr.i64");FILE*fp=fopen(pathbuf,"rb");fread(hdr,8,2,fp);fclose(fp);}
  int64_t N=hdr[0],nnz=hdr[1];
  double*data=rd(d,"data.f64",nnz,8);
  int64_t*indices=rd(d,"indices.i64",nnz,8);
  int64_t*indptr=rd(d,"indptr.i64",N+1,8);
  double*b=rd(d,"b.f64",N,8);
  int64_t*level_ptr=rd(d,"level_ptr.i64",N+1,8);
  int64_t*perm=rd(d,"perm.i64",N,8);
  double*xref=rd(d,"xref.f64",N,8);
  int nthreads=getenv("NT")?atoi(getenv("NT")):0;
  if(nthreads)omp_set_num_threads(nthreads);
  double*x=malloc(N*8);
  uint8_t*ws=malloc(1<<20);
  for(int w=0;w<3;w++){memset(x,0,N*8);sptrsv_level_csr_fp64(data,indices,indptr,b,level_ptr,perm,x,N,nnz,ws,1<<20);}
  double maxabs=0,maxrel=0;
  for(int64_t i=0;i<N;i++){double a=x[i],e=xref[i];double da=fabs(a-e);if(da>maxabs)maxabs=da;double dr=da/(fabs(e)+1e-300);if(dr>maxrel)maxrel=dr;}
  printf("%s N=%ld nnz=%ld maxabs=%.3e maxrel=%.3e correct=%s\n",d,N,nnz,maxabs,maxrel,(maxrel<1e-9)?"yes":"NO");
  double best=1e9;
  for(int r=0;r<reps;r++){
    memset(x,0,N*8);
    double t0=now();
    sptrsv_level_csr_fp64(data,indices,indptr,b,level_ptr,perm,x,N,nnz,ws,1<<20);
    double t1=now();
    if(t1-t0<best)best=t1-t0;
  }
  printf("  time best=%.1f us (threads=%d)\n",best*1e6,omp_get_max_threads());
  return 0;
}
