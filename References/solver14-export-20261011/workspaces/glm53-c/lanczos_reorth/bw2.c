#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static double now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
int main(void){
    long N = 53144100;
    double *a = malloc(N*8);
    for (long i=0;i<N;i++) a[i] = 1.0;
    double t0=now(); double s=0;
    for (long i=0;i<N;i++) s += a[i];
    double t1=now();
    printf("serial read: %.3f s  %.1f GB/s  (s=%g)\n", t1-t0, N*8.0/(t1-t0)/1e9, s);
    t0=now();
    #pragma omp parallel for reduction(+:s)
    for (long i=0;i<N;i++) s += a[i]*2.0;
    t1=now();
    printf("omp read: %.3f s  %.1f GB/s (s=%g)\n", t1-t0, N*8.0/(t1-t0)/1e9, s);
    return 0;
}
