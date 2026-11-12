#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include <math.h>

void ilu0_naive(double *restrict, const int64_t *restrict, const int64_t *restrict, const int64_t, const int64_t);
void ilu0_fp64(double *restrict, const int64_t *restrict, const int64_t *restrict, const int64_t, const int64_t, uint8_t *restrict, const int64_t);

static double nowsec(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + 1e-9*ts.tv_nsec;
}

static double *loadarr(const char *path, const char *suf, size_t *n, size_t esz) {
    char fn[512]; snprintf(fn, sizeof fn, "%s%s.npy", path, suf);
    FILE *f = fopen(fn, "rb"); if (!f) { perror(fn); exit(1); }
    char hdr[128]; fread(hdr, 1, 128, f);
    /* numpy .npy v1: magic(6) ver(2) hdrlen(2) header */
    uint16_t hlen; memcpy(&hlen, hdr+8, 2);
    size_t off = 10 + hlen + ((16 - (10 + hlen) % 16) % 16);
    fseek(f, 0, SEEK_END); long fsz = ftell(f);
    size_t cnt = (fsz - off) / esz;
    fseek(f, off, SEEK_SET);
    char *buf = malloc(fsz - off);
    if (fread(buf, 1, fsz - off, f) != (size_t)(fsz-off)) { perror("read"); exit(1); }
    fclose(f);
    *n = cnt;
    return (void*)buf;
}

int main(int argc, char **argv) {
    const char *path = argv[1];
    int reps = argc > 2 ? atoi(argv[2]) : 5;
    int threads = argc > 3 ? atoi(argv[3]) : 0;
    if (threads > 0) omp_set_num_threads(threads);

    size_t n_ip, n_ci, n_da;
    int64_t *indptr = (int64_t*)loadarr(path, "_indptr", &n_ip, 8);
    int64_t *indices = (int64_t*)loadarr(path, "_indices", &n_ci, 8);
    double *data = (double*)loadarr(path, "_data", &n_da, 8);
    int64_t N = n_ip - 1, nnz = n_ci;
    fprintf(stderr, "%s: N=%ld nnz=%ld threads=%d\n", path, (long)N, (long)nnz, threads?threads:omp_get_max_threads());

    double *base = malloc(n_da*8);
    memcpy(base, data, n_da*8);
    ilu0_naive(base, indices, indptr, N, nnz);


    double *cand = malloc(n_da*8);
    memcpy(cand, data, n_da*8);
    ilu0_fp64(cand, indices, indptr, N, nnz, NULL, 0);
    double maxrel = 0, maxabs = 0; int64_t bad = -1;
    for (size_t k = 0; k < n_da; k++) {
        double d = fabs(cand[k]-base[k]);
        double r = d / (fabs(base[k]) > 0 ? fabs(base[k]) : 1.0);
        if (r > maxrel) { maxrel = r; }
        if (d > maxabs) { maxabs = d; }
        if (d != 0.0 && bad < 0) bad = k;
    }
    printf("check vs naive: maxrel=%.3e maxabs=%.3e firstdiff=%ld %s\n", maxrel, maxabs, (long)bad, bad<0?"BITWISE-OK":"DIFF");

    /* timing */
    double t_naive[32], t_cand[32];
    for (int r = 0; r < reps; r++) {
        memcpy(base, data, n_da*8);
        double t0 = nowsec();
        ilu0_naive(base, indices, indptr, N, nnz);
        t_naive[r] = nowsec()-t0;
        memcpy(cand, data, n_da*8);
        t0 = nowsec();
        ilu0_fp64(cand, indices, indptr, N, nnz, NULL, 0);
        t_cand[r] = nowsec()-t0;
    }
    int cmp(const void*a, const void*b){double x=*(const double*)a-*(const double*)b; return x<0?-1:x>0;}
    double tn[32], tc[32]; memcpy(tn, t_naive, sizeof tn); memcpy(tc, t_cand, sizeof tc);
    qsort(tn, reps, sizeof(double), cmp); qsort(tc, reps, sizeof(double), cmp);
    printf("naive: med=%.4f ms (min %.4f)\ncand : med=%.4f ms (min %.4f)  speedup=%.2fx\n",
        tn[reps/2]*1e3, tn[0]*1e3, tc[reps/2]*1e3, tc[0]*1e3, tn[reps/2]/tc[reps/2]);
    (void)maxabs;
    return 0;
}
