#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

void lanczos_reorth_csr_fp64(const double *A_data,
    const int64_t *A_indices, const int64_t *A_indptr,
    double *Q, double *alpha, const double *b, double *beta,
    int64_t nx, int64_t ny, int64_t nz, int64_t m,
    uint8_t *workspace, int64_t workspace_size);

static double now(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + 1e-9*ts.tv_nsec;
}

static void *rd(const char *tag, const char *ext, size_t n) {
    char fn[512]; snprintf(fn, sizeof fn, "%s_%s", tag, ext);
    FILE *g = fopen(fn, "r");
    if (!g) { fprintf(stderr, "open %s failed\n", fn); exit(3); }
    void *p = malloc(n);
    if (fread(p, 1, n, g) != n) { fprintf(stderr, "read %s\n", fn); exit(3); }
    fclose(g);
    return p;
}

int main(int argc, char **argv) {
    if (argc < 4) return 2;
    const char *tag = argv[1];
    const char *outtag = argv[2];
    int reps = atoi(argv[3]);
    char metafile[512]; snprintf(metafile, sizeof metafile, "%s_meta.txt", tag);
    FILE *f = fopen(metafile, "r");
    long nx, ny, nz, m, nnz, seed;
    if (!f || fscanf(f, "%ld %ld %ld %ld %ld %ld", &nx, &ny, &nz, &m, &nnz, &seed) != 6) return 2;
    fclose(f);
    long N = nx*ny*nz;
    double *data = rd(tag, "data.f64", (size_t)nnz*8);
    int64_t *ind = rd(tag, "ind.i64", (size_t)nnz*8);
    int64_t *ptr = rd(tag, "ptr.i64", (size_t)(N+1)*8);
    double *b = rd(tag, "b.f64", (size_t)N*8);
    double *Q = calloc((size_t)N*m, 8);
    double *alpha = malloc((size_t)m*8);
    double *beta = malloc((size_t)m*8);

    double best = 1e300, first = 0.0;
    for (int r = 0; r < reps; r++) {
        memset(Q, 0, (size_t)N*m*8);
        memset(alpha, 0, (size_t)m*8);
        memset(beta, 0, (size_t)m*8);
        double t0 = now();
        lanczos_reorth_csr_fp64(data, ind, ptr, Q, alpha, b, beta,
                                (int64_t)nx, (int64_t)ny, (int64_t)nz, (int64_t)m, NULL, 0);
        double t1 = now();
        if (r == 0) first = t1-t0;
        if (t1-t0 < best) best = t1-t0;
    }
    fprintf(stderr, "time_first %.4f time_best %.4f\n", first, best);

    char fn[512]; FILE *g;
    snprintf(fn, sizeof fn, "%s_Q.f64", outtag); g = fopen(fn, "w");
    fwrite(Q, 8, (size_t)N*m, g); fclose(g);
    snprintf(fn, sizeof fn, "%s_alpha.f64", outtag); g = fopen(fn, "w");
    fwrite(alpha, 8, (size_t)m, g); fclose(g);
    snprintf(fn, sizeof fn, "%s_beta.f64", outtag); g = fopen(fn, "w");
    fwrite(beta, 8, (size_t)m, g); fclose(g);
    return 0;
}
