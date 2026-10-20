#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
void ilu0_fp64(double *restrict, const int64_t *restrict, const int64_t *restrict,
               const int64_t, const int64_t, uint8_t *restrict, const int64_t);
static void *rd(const char *f, size_t *n) {
    FILE *fp = fopen(f, "rb");
    if (!fp) { perror(f); exit(1); }
    fseek(fp, 0, SEEK_END); long sz = ftell(fp); fseek(fp, 0, SEEK_SET);
    void *p = malloc(sz);
    if (fread(p, 1, sz, fp) != (size_t)sz) { fprintf(stderr, "short read\n"); exit(1); }
    fclose(fp); *n = sz; return p;
}
int main(void) {
    size_t n;
    double *data = rd("data.bin", &n);
    int64_t nnz = n / 8;
    int64_t *indices = rd("indices.bin", &n);
    int64_t *indptr = rd("indptr.bin", &n);
    int64_t N = (n / 8) - 1;
    int64_t ws = 24 * N;
    uint8_t *w = aligned_alloc(64, ws + 64);
    int reps = 7;
    double best = 1e30;
    for (int r = 0; r < reps; r++) {
        double t0 = omp_get_wtime();
        ilu0_fp64(data, indices, indptr, N, nnz, w, ws);
        double t1 = omp_get_wtime();
        if (t1 - t0 < best) best = t1 - t0;
    }
    printf("best %.4f ms over %d\n", best * 1e3, reps);
    FILE *fp = fopen("out.bin", "wb");
    fwrite(data, 8, nnz, fp);
    fclose(fp);
    free(data); free(indices); free(indptr); free(w);
    return 0;
}
