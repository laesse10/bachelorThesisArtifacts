#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static double now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
int main(void){
    long N = 100000000;
    double *a = malloc(N*8), *b = malloc(N*8), *c = malloc(N*8);
    for (long i=0;i<N;i++){a[i]=1.0+i;b[i]=2.0-i;c[i]=0.0;}
    double t0=now();
    #pragma omp parallel for
    for (long i=0;i<N;i++) c[i]=a[i]+2.0*b[i];
    double t1=now();
    double bytes = 3.0*N*8;
    printf("%.1f GB/s\n", bytes/(t1-t0)/1e9);
    return 0;
}
