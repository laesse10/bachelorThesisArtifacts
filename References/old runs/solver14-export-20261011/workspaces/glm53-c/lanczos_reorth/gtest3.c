#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
enum { KB_COLMAJOR = 102, KB_NOTRANS = 111, KB_TRANS = 112 };
void cblas_dgemv(int layout, int trans, int m, int n, double alpha,
                 const double *a, int lda, const double *x, int incx,
                 double beta, double *y, int incy);
static double now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
static int nthreads(void){
    DIR *d = opendir("/proc/self/task"); int n=0; struct dirent *e;
    while ((e = readdir(d))) if (e->d_name[0]>='0'&&e->d_name[0]<='9') n++;
    closedir(d); return n;
}
int main(int argc, char **argv){
    long N = 531441, k = argc>1?atol(argv[1]):400;
    double *QT = malloc((size_t)N*k*8), *w = malloc(N*8), *d = malloc(k*8);
    memset(QT, 0, (size_t)N*k*8);
    for (long i=0;i<N;i++) w[i] = 1.0;
    printf("threads before: %d\n", nthreads());
    double t0, t1;
    for (int r = 0; r < 4; r++) {
        printf("threads now: %d\n", nthreads());
        t0 = now();
        cblas_dgemv(KB_COLMAJOR, KB_TRANS, (int)N, (int)k, 1.0, QT, (int)N, w, 1, 0.0, d, 1);
        t1 = now();
        printf("gemv_t: %.4f s  %.1f GB/s\n", t1-t0, (double)N*k*8/(t1-t0)/1e9);
    }
    return 0;
}
