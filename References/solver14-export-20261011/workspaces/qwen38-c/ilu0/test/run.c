#include <stdint.h>
#include <stddef.h>
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
    if (fread(p, 1, sz, fp) != (size_t)sz) { fprintf(stderr, "short read %s\n", f); exit(1); }
    fclose(fp); *n = sz;
    return p;
}
int main(int argc, char **argv) {
    size_t n;
    double *data = rd("data.bin", &n);
    int64_t nnz = n / 8;
    int64_t *indices = rd("indices.bin", &n);
    int64_t *indptr = rd("indptr.bin", &n);
    int64_t N = (n / 8) - 1;
    int64_t ws = 20 * N;
    uint8_t *wsmem = aligned_alloc(64, ws + 64);
    memset(wsmem, 0, ws);
    double best = 1e30;
    for (int r = 0; r < 7; r++) {
        double t0 = omp_get_wtime();
        ilu0_fp64(data, indices, indptr, N, nnz, wsmem, ws);
        double t1 = omp_get_wtime();
        if (t1 - t0 < best) best = t1 - t0;
    }
    printf("best %.4f ms\n", best * 1e3);
    FILE *fp = fopen("out.bin", "wb");
    fwrite(data, 8, nnz, fp);
    fclose(fp);
    free(data); free(indices); free(indptr); free(wsmem);
    return 0;
}
