#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include <stdatomic.h>
#define fmin(a,b) (((a)<(b))?(a):(b))

static double now(){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+ts.tv_nsec*1e-9;}

// custom sense-reversal barrier

typedef struct { _Alignas(64) atomic_int count; _Alignas(64) atomic_int phase; } Bar;
static Bar bar;
static void mybar(int nt){
  int ph = atomic_load_explicit(&bar.phase, memory_order_acquire);
  int c = atomic_fetch_add_explicit(&bar.count,1,memory_order_acq_rel);
  if(c==nt-1){
    atomic_store_explicit(&bar.count,0,memory_order_release);
    atomic_fetch_add_explicit(&bar.phase,1,memory_order_release);
  } else {
    while(atomic_load_explicit(&bar.phase,memory_order_acquire)==ph){ /*spin*/ }
  }
}

int main(int argc,char**argv){
  int nt=argc>1?atoi(argv[1]):13;
  int reps=argc>2?atoi(argv[2]):20000;
  FILE*fp=fopen("m3/widths_nonzero.i32","rb"); int32_t L;fread(&L,4,1,fp);
  int32_t*w=malloc(L*4); fread(w,4,L,fp); fclose(fp);
  int64_t totalrows=0; for(int i=0;i<L;i++)totalrows+=w[i];
  double*dummy=malloc(1<<20);
  omp_set_num_threads(nt);
  double best[3]={1e9,1e9,1e9};
  for(int it=0;it<3;it++){
    // A: omp for lockstep
    double t0=now();
    #pragma omp parallel num_threads(nt)
    { for(int lvl=0;lvl<L;lvl++){
        #pragma omp for schedule(static)
        for(int32_t p=0;p<w[lvl];p++){ volatile double v=dummy[p&1023]; (void)v; }
    }}
    best[0]=fmin(best[0],now()-t0);
    // B: custom barrier + manual division
    t0=now();
    #pragma omp parallel num_threads(nt)
    { int tid=omp_get_thread_num();
      for(int lvl=0;lvl<L;lvl++){
        int32_t wdt=w[lvl]; int32_t per=(wdt+nt-1)/nt;
        int32_t a=tid*per, c=a+per; if(c>wdt)c=wdt;
        for(int32_t p=a;p<c;p++){ volatile double v=dummy[p&1023]; (void)v; }
        mybar(nt);
      }
    }
    best[1]=fmin(best[1],now()-t0);
    // C: no sync, manual division, barrier only at end (WRONG but measures floor)
    t0=now();
    #pragma omp parallel num_threads(nt)
    { int tid=omp_get_thread_num();
      for(int lvl=0;lvl<L;lvl++){
        int32_t wdt=w[lvl]; int32_t per=(wdt+nt-1)/nt;
        int32_t a=tid*per,c=a+per; if(c>wdt)c=wdt;
        for(int32_t p=a;p<c;p++){ volatile double v=dummy[p&1023]; (void)v; }
      }
      #pragma omp barrier
    }
    best[2]=fmin(best[2],now()-t0);
  }
  printf("nt=%d L=%d totalrows=%lld\n",nt,L,(long long)totalrows);
  printf("  A omp-for-lockstep:  %.1f us  (%.2f us/level)\n",best[0]*1e6,best[0]/L*1e6);
  printf("  B custombar+div:     %.1f us  (%.2f us/level)\n",best[1]*1e6,best[1]/L*1e6);
  printf("  C no-sync-floor:     %.1f us  (%.2f us/level)\n",best[2]*1e6,best[2]/L*1e6);
  return 0;
}
