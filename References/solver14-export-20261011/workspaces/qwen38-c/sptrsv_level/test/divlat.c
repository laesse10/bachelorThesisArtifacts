#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 4000000
static double rd(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (double)ts.tv_sec+ts.tv_nsec*1e-9;}
static inline void bar(double *x){__asm__ volatile("" : "+xm"(*x));}
int main(){
  double a=1.0000001, d=1.5;
  double x=a;
  double t0=rd();
  for(long i=0;i<N;i++){ x=x/d; if(i&63) bar(&x); }
  double t1=rd();
  printf("div latency (dependent): %.2f ns/op\n",(t1-t0)/N*1e9);
  double f=1.0;
  t0=rd();
  for(long i=0;i<N;i++){ f=f*d+a; if(i&63) bar(&f); }
  t1=rd();
  printf("fma latency (dependent): %.2f ns/op\n",(t1-t0)/N*1e9);
  double y[8]={1,2,3,4,5,6,7,8};
  t0=rd();
  for(long i=0;i<N;i++){ for(int j=0;j<8;j++) y[j]=y[j]/d; }
  t1=rd();
  printf("div throughput (8-wide indep): %.2f ns/op\n",(t1-t0)/(N*8)*1e9);
  return 0;
}
