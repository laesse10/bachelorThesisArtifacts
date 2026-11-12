#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

typedef struct { int32_t ustart; int32_t uend; } uinfo_t;

static double nowsec(void) { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return ts.tv_sec + 1e-9*ts.tv_nsec; }

static int64_t *load64(const char *path, const char *suf, size_t *n) {
    char fn[512]; snprintf(fn, sizeof fn, "%s%s.npy", path, suf);
    FILE *f = fopen(fn, "rb"); if (!f) { perror(fn); exit(1); }
    char hdr[10]; fread(hdr, 1, 10, f);
    uint16_t hlen; memcpy(&hlen, hdr+8, 2);
    size_t off = 10 + hlen + ((16 - (10 + hlen) % 16) % 16);
    fseek(f, 0, SEEK_END); long fsz = ftell(f); fseek(f, off, SEEK_SET);
    char *buf = malloc(fsz - off);
    if (fread(buf, 1, fsz-off, f) != (size_t)(fsz-off)) exit(1);
    fclose(f); *n = (fsz-off)/8; return (int64_t*)buf;
}
static double *loadarr(const char *path, const char *suf, size_t *n) {
    char fn[512]; snprintf(fn, sizeof fn, "%s%s.npy", path, suf);
    FILE *f = fopen(fn, "rb"); if (!f) { perror(fn); exit(1); }
    char hdr[10]; fread(hdr, 1, 10, f);
    uint16_t hlen; memcpy(&hlen, hdr+8, 2);
    size_t off = 10 + hlen + ((16 - (10 + hlen) % 16) % 16);
    fseek(f, 0, SEEK_END); long fsz = ftell(f); fseek(f, off, SEEK_SET);
    char *buf = malloc(fsz - off);
    if (fread(buf, 1, fsz-off, f) != (size_t)(fsz-off)) exit(1);
    fclose(f); *n = (fsz-off)/8; return (double*)buf;
}

int main(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : "big";
    int reps = argc > 2 ? atoi(argv[2]) : 7;
    size_t n_ip, n_ci, n_da;
    int64_t *indptr = load64(path, "_indptr", &n_ip);
    int64_t *indices = load64(path, "_indices", &n_ci);
    double *data = loadarr(path, "_data", &n_da);
    int64_t N = n_ip - 1;
    uinfo_t *uinfo = malloc(sizeof(uinfo_t) * N);
    double *diag = malloc(8.0 * N);
    uint32_t *pos = malloc(4.0 * N);
    int64_t maxdeg = 0;
    for (int64_t r = 0; r < N; r++) {
        int64_t rs = indptr[r], re = indptr[r+1], deg = re - rs;
        if (deg > maxdeg) maxdeg = deg;
        int64_t u = rs, prev = -1;
        for (int64_t m = rs; m < re; m++) {
            int64_t c = indices[m];
            if (c <= prev) exit(2);
            prev = c;
            if (c <= r) u = m + 1;
        }
        uinfo[r].ustart = (int32_t)u; uinfo[r].uend = (int32_t)re;
    }
    double *local = malloc(8.0 * maxdeg);
    double *work = malloc(8.0 * n_da);
    fprintf(stderr, "N=%ld nnz=%ld maxdeg=%ld\n", (long)N, (long)n_ci, (long)maxdeg);

    /* timing of skeleton (no inner) vs full, recip, fullrow via env ILU0_EXP */
    int mode = getenv("ILU0_EXP") ? atoi(getenv("ILU0_EXP")) : 0;
    const int noinner = mode & 1, recip = mode & 2, fullrow = mode & 4;

    double best = 1e9;
    for (int r = 0; r < reps; r++) {
        memcpy(work, data, 8.0*n_da);
        double t0 = nowsec();
        for (int64_t i = 0; i < N; i++) {
            const int64_t rs = indptr[i], re = indptr[i+1];
            const int64_t deg = re - rs;
            const int64_t *restrict rowi = indices + rs;
            double *restrict rowd = work + rs;
            int64_t diagpos = -1;
            for (int64_t j = 0; j < deg; j++) {
                pos[rowi[j]] = (uint32_t)j;
                local[j] = rowd[j];
                diagpos = (rowi[j] == i) ? j : diagpos;
            }
            for (int64_t k = 0; k < deg; k++) {
                const int64_t col_k = rowi[k];
                if (col_k >= i) break;
                double l_ik = recip ? local[k] * (1.0 / diag[col_k]) : local[k] / diag[col_k];
                local[k] = l_ik;
                if (fullrow) {
                    for (int64_t m = indptr[col_k]; m < indptr[col_k+1]; m++) {
                        const int64_t col_j = indices[m];
                        if (col_j > col_k) {
                            const uint32_t p = pos[col_j];
                            if (p < (uint32_t)deg && rowi[p] == col_j)
                                local[p] -= l_ik * work[m];
                        }
                    }
                } else if (!noinner) {
                    const uinfo_t ui = uinfo[col_k];
                    for (int64_t m = ui.ustart; m < ui.uend; m++) {
                        const int64_t col_j = indices[m];
                        const uint32_t p = pos[col_j];
                        if (p < (uint32_t)deg && rowi[p] == col_j)
                            local[p] -= l_ik * work[m];
                    }
                }
            }
            diag[i] = local[diagpos];
            for (int64_t j = 0; j < deg; j++) rowd[j] = local[j];
        }
        double dt = nowsec() - t0;
        if (dt < best) best = dt;
    }
    printf("mode=%d noinner=%d recip=%d fullrow=%d: best=%.4f ms\n", mode, noinner, recip, fullrow, best*1e3);
    return 0;
}
