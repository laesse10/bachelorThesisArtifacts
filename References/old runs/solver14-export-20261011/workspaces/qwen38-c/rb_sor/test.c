#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <time.h>

void rb_sor_fp64(const double *restrict f, double *restrict u, const int64_t N,
                 const int64_t TSTEPS, const double omega, uint8_t *restrict workspace,
                 const int64_t workspace_size);

static double now_s(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    /* usage: test f.bin N TSTEPS omega reps [u_in.bin] [u_out.bin] [workspace_bytes] */
    if (argc < 6) { fprintf(stderr, "usage: test f.bin N TSTEPS omega reps [u_in] [u_out] [ws]\n"); return 2; }
    int64_t N = atoll(argv[2]);
    int64_t TSTEPS = atoll(argv[3]);
    double omega = atof(argv[4]);
    int reps = atoi(argv[5]);
    FILE *fp = fopen(argv[1], "rb"); if (!fp) { perror("f.bin"); return 1; }
    int64_t fsz = N * N * (int64_t)sizeof(double);
    double *f = malloc(fsz);
    if (fread(f, 1, fsz, fp) != (size_t)fsz) { fprintf(stderr, "short f read\n"); return 1; }
    fclose(fp);

    double *u = malloc(fsz);
    if (argc > 6 && argv[6][0]) {
        FILE *up = fopen(argv[6], "rb"); if (!up) { perror("u_in"); return 1; }
        fread(u, 1, fsz, up); fclose(up);
    } else { memset(u, 0, fsz); }

    uint8_t *ws = NULL; int64_t wsz = 0;
    if (argc > 8) { wsz = atoll(argv[8]); ws = malloc(wsz); }

    rb_sor_fp64(f, u, N, TSTEPS, omega, ws, wsz); /* warmup */
    double tmin = 1e30, tmax = 0, tsum = 0;
    for (int r = 0; r < reps; ++r) {
        if (r) memset(u, 0, fsz);   /* identical input every timed call; REF stays 1-run */
        double t0 = now_s();
        rb_sor_fp64(f, u, N, TSTEPS, omega, ws, wsz);
        double dt = now_s() - t0;
        tsum += dt; if (dt < tmin) tmin = dt; if (dt > tmax) tmax = dt;
    }
    printf("N=%ld TSTEPS=%ld omega=%g mean=%.3f ms min=%.3f max=%.3f\n",
           (long)N, (long)TSTEPS, omega, (tsum / reps) * 1e3, tmin * 1e3, tmax * 1e3);

    if (argc > 7 && argv[7][0]) {
        FILE *uo = fopen(argv[7], "wb"); fwrite(u, 1, fsz, uo); fclose(uo);
    }
    if (getenv("REF")) {
        FILE *rf = fopen(getenv("REF"), "rb"); if (rf) {
            double *ref = malloc(fsz); if (fread(ref, 1, fsz, rf) == (size_t)fsz) {
                double mad = 0; int64_t mi = 0;
                for (int64_t k = 0; k < N * N; ++k) {
                    double d = u[k] - ref[k]; if (d < 0) d = -d;
                    if (d > mad) { mad = d; mi = k; }
                }
                printf("maxdiff=%.3e at %ld\n", mad, (long)mi);
            } free(ref); fclose(rf);
        }
    }
    free(f); free(u); free(ws);
    return 0;
}
