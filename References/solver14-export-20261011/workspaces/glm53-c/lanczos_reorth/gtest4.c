
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <omp.h>
enum { KB_COLMAJOR = 102, KB_NOTRANS = 111, KB_TRANS = 112 };
void cblas_dgemv(int layout, int trans, int m, int n, double alpha,
                 const double *a, int lda, const double *x, int incx,
                 double beta, double *y, int incy);
static int nthreads(void){
    DIR *d = opendir("/proc/self/task"); int n=0; struct dirent *e;
    while ((e = readdir(d))) if (e->d_name[0]>='0'&&e->d_name[0]<='9') n++;
    closedir(d); return n;
}
static double now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
int main(void){
    long N = 531441, k = 400;
    double *QT = malloc((size_t)N*k*8), *w = malloc(N*8), *d = malloc(k*8);
    memset(QT, 0, (size_t)N*k*8);
    long i;
    for (i=0;i<N;i++) w[i] = 1.0;
    double t0=now();
    #pragma omp parallel for
    for (i=0;i<N;i++) w[i] = QT[i]+1.0;
    printf("after omp phase: %.3f s, omp max threads=%d\n", now()-t0, omp_get_max_threads());
    printf("threads: %d\n", nthreads());
    t0=now();
    cblas_dgemv(KB_COLMAJOR, KB_TRANS, (int)N, (int)k, 1.0, QT, (int)N, w, 1, 0.0, d, 1);
    printf("first gemv: %.4f s (%.1f GB/s), threads now: %d\n", now()-t0, (double)N*k*8/(now()-t0)/1e9, nthreads());
    int r;
    for (r=0;r<3;r++){
        t0=now();
        cblas_dgemv(KB_COLMAJOR, KB_TRANS, (int)N, (int)k, 1.0, QT, (int)N, w, 1, 0.0, d, 1);
        printf("gemv_t: %.4f s  (%.1f GB/s)\n", now()-t0, (double)N*k*8/(now()-t0)/1e9);
    }
    return 0;
}
