#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdatomic.h>
#include <sched.h>
#include <omp.h>
static double rd(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return (double)ts.tv_sec+ts.tv_nsec*1e-9;}
int main(int argc,char**argv){
  int coreA=argc>1?atoi(argv[1]):10;
  int coreB=argc>2?atoi(argv[2]):24;
  int reps=argc>3?atoi(argv[3]):2000000;
  _Alignas(64) atomic_int counter=0;
  _Alignas(64) double shared[8]={1,2,3,4,5,6,7,8};
  double sink=0;
  omp_set_num_threads(2);
  // A pings counter, B reads it; measure ping-pong latency
  double t0=rd();
  #pragma omp parallel num_threads(2)
  {
    int tid=omp_get_thread_num();
    cpu_set_t cs; CPU_ZERO(&cs); CPU_SET(tid?coreB:coreA,&cs);
    sched_setaffinity(0,sizeof(cs),&cs);
    if(tid==0){
      for(int i=0;i<reps;i++){ atomic_fetch_add_explicit(&counter,1,memory_order_acq_rel); atomic_fetch_add_explicit(&counter,1,memory_order_acq_rel); }
    } else {
      for(int i=0;i<reps;i++){ atomic_fetch_add_explicit(&counter,1,memory_order_acq_rel); atomic_fetch_add_explicit(&counter,1,memory_order_acq_rel); }
      for(int j=0;j<8;j++) sink+=shared[j];
    }
  }
  double t1=rd();
  printf("cross-socket 2-thread contended fetch_add: %.2f ns/op (cores %d,%d)\n",(t1-t0)/(reps*2)*1e9,coreA,coreB);
  (void)sink;
  return 0;
}
