#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
enum { KB_COLMAJOR = 102, KB_NOTRANS = 111, KB_TRANS = 112 };
void cblas_dgemv(int layout, int trans, int m, int n, double alpha,
                 const double *a, int lda, const double *x, int incx,
                 double beta, double *y, int incy);
static double now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
int main(int argc, char **argv){
    long N = 531441, k = 400;
    if (argc > 2) k = atol(argv[2]);
    double *QT = malloc((size_t)N*k*8), *w = malloc(N*8), *d = malloc(k*8);
    memset(QT, 0, (size_t)N*k*8);
    for (long i=0;i<N;i++) w[i] = 1.0;
    double t0, t1;
    for (int r = 0; r < 3; r++) {
        t0 = now();
        cblas_dgemv(KB_COLMAJOR, KB_TRANS, (int)N, (int)k, 1.0, QT, (int)N, w, 1, 0.0, d, 1);
        t1 = now();
        printf("gemv_t k=%ld: %.4f s  %.1f GB/s\n", k, t1-t0, (double)N*k*8/(t1-t0)/1e9);
        t0 = now();
        cblas_dgemv(KB_COLMAJOR, KB_NOTRANS, (int)N, (int)k, -1.0, QT, (int)N, d, 1, 1.0, w, 1);
        t1 = now();
        printf("gemv_n k=%ld: %.4f s  %.1f GB/s\n", k, t1-t0, (double)N*k*8/(t1-t0)/1e9);
    }
    return 0;
}
