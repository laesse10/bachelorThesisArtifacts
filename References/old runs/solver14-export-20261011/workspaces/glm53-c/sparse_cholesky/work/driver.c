#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

extern void sparse_cholesky_csr_fp64(
    const double *restrict, const int64_t *restrict, const int64_t *restrict,
    const int64_t *restrict, const int64_t *restrict, const int64_t *restrict,
    double *restrict, const int64_t *restrict, const int64_t *restrict,
    const double *restrict, double *restrict, const int64_t,
    uint8_t *restrict, const int64_t);

/* simple npz loader via generated header */
#include "testdata.h"

static double now_s(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return ts.tv_sec + 1e-9*ts.tv_nsec; }

int main(int argc, char **argv) {
    int reps = argc > 1 ? atoi(argv[1]) : 20;
    int64_t N = EDGE*EDGE*EDGE;
    double *y_out = malloc(N*sizeof(double));
    double maxdiff_y = 0, maxdiff_L = 0;
    int64_t nnzL = Lc_indptr[N];
    double *Lc_out = malloc(nnzL*sizeof(double));
    /* correctness: run once, compare */
    for (int64_t i = 0; i < N; i++) y_out[i] = -12345.0;
    sparse_cholesky_csr_fp64(A_data, A_indices, A_indptr, L_indices, L_indptr, L_to_Lc,
                             Lc_out, Lc_indices, Lc_indptr, b, y_out, EDGE, NULL, 0);
    for (int64_t i = 0; i < N; i++) {
        double d = fabs(y_out[i]-y_ref[i]);
        if (d > maxdiff_y) maxdiff_y = d;
    }
    for (int64_t p = 0; p < nnzL; p++) {
        double d = fabs(Lc_out[p]-Lc_data_ref[p]);
        if (d > maxdiff_L) maxdiff_L = d;
    }
    double tol = 1e-9;
    int ok = 1;
    for (int64_t i = 0; i < N; i++) {
        double a = fabs(y_ref[i]);
        double lim = tol*(a>1.0?a:1.0) + 1e-11;
        if (fabs(y_out[i]-y_ref[i]) > lim) ok = 0;
    }
    for (int64_t p = 0; p < nnzL; p++) {
        double a = fabs(Lc_data_ref[p]);
        double lim = tol*(a>1.0?a:1.0) + 1e-11;
        if (fabs(Lc_out[p]-Lc_data_ref[p]) > lim) ok = 0;
    }
    printf("maxdiff y=%.3e Lc=%.3e  %s\n", maxdiff_y, maxdiff_L, ok?"OK":"FAIL");
    /* timing */
    double t0 = now_s();
    for (int r = 0; r < reps; r++)
        sparse_cholesky_csr_fp64(A_data, A_indices, A_indptr, L_indices, L_indptr, L_to_Lc,
                                 Lc_out, Lc_indices, Lc_indptr, b, y_out, EDGE, NULL, 0);
    double t1 = now_s();
    printf("time/call: %.3f ms\n", (t1-t0)/reps*1e3);
    return 0;
}
