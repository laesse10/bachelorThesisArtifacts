#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdatomic.h>
#include <time.h>
#include <omp.h>
static double now(){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+ts.tv_nsec*1e-9;}
typedef struct { _Alignas(64) atomic_int count; _Alignas(64) atomic_int phase; } Bar;
static Bar bar;
static void mybar(int nt){
  int ph = atomic_load_explicit(&bar.phase, memory_order_acquire);
  int c = atomic_fetch_add_explicit(&bar.count,1,memory_order_acq_rel);
  if(c==nt-1){ atomic_store_explicit(&bar.count,0,memory_order_release);
               atomic_fetch_add_explicit(&bar.phase,1,memory_order_release); }
  else { while(atomic_load_explicit(&bar.phase,memory_order_acquire)==ph){} }
}
int main(int argc,char**argv){
  int nt=argc>1?atoi(argv[1]):13; int reps=argc>2?atoi(argv[2]):200000;
  double t0=now();
  #pragma omp parallel num_threads(nt)
  { for(int i=0;i<reps;i++) mybar(nt); }
  double t1=now();
  printf("custom mybar tight: %.2f ns (nt=%d)\n",(t1-t0)/reps*1e9,nt);
  t0=now();
  #pragma omp parallel num_threads(nt)
  { for(int i=0;i<reps;i++){ _Pragma("omp barrier") } }
  double t1b=now();
  printf("omp barrier tight:  %.2f ns (nt=%d)\n",(t1b-t0)/reps*1e9,nt);
  return 0;
}
