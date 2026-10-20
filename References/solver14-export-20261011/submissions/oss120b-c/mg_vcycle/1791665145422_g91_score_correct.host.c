/*
 * Optimized C implementation of the geometric multigrid V-cycle kernel.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>

static const double OMEGA = 6.0 / 7.0;
static const int NU1 = 3;
static const int NU2 = 3;
static const int NCOARSE_SWEEPS = 40;
static const int NCOARSE = 4;
static const int LMAX = 24;

// Static buffers for reuse across kernel invocations
static double *buf_uh = NULL;
static double *buf_fh = NULL;
static double *buf_rh = NULL;
static double *buf_th = NULL;
static int64_t buf_flat = 0;

static inline void smooth(double *__restrict u, const double *__restrict f, double *__restrict ut,
                          int64_t off, int64_t n, double h2, int nsweeps) {
    for (int s = 0; s < nsweeps; ++s) {
                for (int64_t i = 0; i < n; ++i) {
            for (int64_t j = 0; j < n; ++j) {
                for (int64_t k = 0; k < n; ++k) {
                    int64_t c = off + (i * n + j) * n + k;
                    double sum = 0.0;
                    double d = 6.0;
                    if (i > 0) { sum += u[c - n * n]; } else { d += 1.0; }
                    if (i < n - 1) { sum += u[c + n * n]; } else { d += 1.0; }
                    if (j > 0) { sum += u[c - n]; } else { d += 1.0; }
                    if (j < n - 1) { sum += u[c + n]; } else { d += 1.0; }
                    if (k > 0) { sum += u[c - 1]; } else { d += 1.0; }
                    if (k < n - 1) { sum += u[c + 1]; } else { d += 1.0; }
                    double au = (d * u[c] - sum) / h2;
                    ut[c] = u[c] + OMEGA * (f[c] - au) * h2 / d;
                }
            }
        }
                memcpy(u + off, ut + off, n * n * n * sizeof(double));
    }
}

static inline void residual(const double *__restrict u, const double *__restrict f,
                            double *__restrict r, int64_t off, int64_t n, double h2) {
        for (int64_t i = 0; i < n; ++i) {
        for (int64_t j = 0; j < n; ++j) {
            for (int64_t k = 0; k < n; ++k) {
                int64_t c = off + (i * n + j) * n + k;
                double sum = 0.0;
                double d = 6.0;
                if (i > 0) { sum += u[c - n * n]; } else { d += 1.0; }
                if (i < n - 1) { sum += u[c + n * n]; } else { d += 1.0; }
                if (j > 0) { sum += u[c - n]; } else { d += 1.0; }
                if (j < n - 1) { sum += u[c + n]; } else { d += 1.0; }
                if (k > 0) { sum += u[c - 1]; } else { d += 1.0; }
                if (k < n - 1) { sum += u[c + 1]; } else { d += 1.0; }
                r[c] = f[c] - (d * u[c] - sum) / h2;
            }
        }
    }
}

static inline void restrict_full(const double *__restrict fine, double *__restrict coarse,
                                 int64_t off_f, int64_t off_c, int64_t nc) {
    int64_t nf = 2 * nc;
        for (int64_t i = 0; i < nc; ++i) {
        for (int64_t j = 0; j < nc; ++j) {
            for (int64_t k = 0; k < nc; ++k) {
                double s = 0.0;
                for (int di = 0; di < 2; ++di) {
                    for (int dj = 0; dj < 2; ++dj) {
                        for (int dk = 0; dk < 2; ++dk) {
                            int64_t fi = 2 * i + di;
                            int64_t fj = 2 * j + dj;
                            int64_t fk = 2 * k + dk;
                            s += fine[off_f + (fi * nf + fj) * nf + fk];
                        }
                    }
                }
                coarse[off_c + (i * nc + j) * nc + k] = 0.125 * s;
            }
        }
    }
}

static inline void prolong_interp(double *__restrict u,
                                 int64_t off_f, int64_t off_c, int64_t nc) {
    int64_t nf = 2 * nc;
        for (int64_t i = 0; i < nf; ++i) {
        for (int64_t j = 0; j < nf; ++j) {
            for (int64_t k = 0; k < nf; ++k) {
                int64_t pi = i / 2;
                int64_t pj = j / 2;
                int64_t pk = k / 2;
                int64_t si = 2 * (i % 2) - 1;
                int64_t sj = 2 * (j % 2) - 1;
                int64_t sk = 2 * (k % 2) - 1;
                double acc = 0.0;
                for (int di = 0; di < 2; ++di) {
                    for (int dj = 0; dj < 2; ++dj) {
                        for (int dk = 0; dk < 2; ++dk) {
                            int64_t ci = pi + di * si;
                            int64_t cj = pj + dj * sj;
                            int64_t ck = pk + dk * sk;
                            double w = 1.0;
                            if (ci < 0) { ci = 0; w = -w; }
                            if (ci > nc - 1) { ci = nc - 1; w = -w; }
                            if (cj < 0) { cj = 0; w = -w; }
                            if (cj > nc - 1) { cj = nc - 1; w = -w; }
                            if (ck < 0) { ck = 0; w = -w; }
                            if (ck > nc - 1) { ck = nc - 1; w = -w; }
                            if (di == 0) w *= 3.0;
                            if (dj == 0) w *= 3.0;
                            if (dk == 0) w *= 3.0;
                            acc += w * u[off_c + (ci * nc + cj) * nc + ck];
                        }
                    }
                }
                u[off_f + (i * nf + j) * nf + k] += acc / 64.0;
            }
        }
    }
}

void mg_vcycle_fp64(const double *__restrict f, double *__restrict u,
                    const int64_t N, const int64_t ncycles,
                    uint8_t *__restrict workspace, const int64_t workspace_size) {
    int64_t flat = (8 * N * N * N) / 7 + 64;
    // Allocate or reuse buffers
    if (buf_flat < flat) {
        // Free previous buffers if any
        if (buf_uh) free(buf_uh);
        if (buf_fh) free(buf_fh);
        if (buf_rh) free(buf_rh);
        if (buf_th) free(buf_th);
        // Allocate new buffers
        buf_uh = (double *)malloc(flat * sizeof(double));
        buf_fh = (double *)malloc(flat * sizeof(double));
        buf_rh = (double *)malloc(flat * sizeof(double));
        buf_th = (double *)malloc(flat * sizeof(double));
        if (!buf_uh || !buf_fh || !buf_rh || !buf_th) {
            // Allocation failed; abort
            return;
        }
        buf_flat = flat;
    }
    double *uh = buf_uh;
    double *fh = buf_fh;
    double *rh = buf_rh;
    double *th = buf_th;
    int64_t ns[LMAX];
    int64_t offs[LMAX];
    int nlevels = 1;
    int64_t edge = N;
    while (edge > NCOARSE) { edge /= 2; nlevels++; }
    int64_t pos = 0;
    edge = N;
    for (int lv = 0; lv < nlevels; ++lv) { ns[lv] = edge; offs[lv] = pos; pos += edge * edge * edge; edge /= 2; }
        memcpy(fh, f, N * N * N * sizeof(double));
memset(u, 0, N * N * N * sizeof(double));
    for (int64_t cycle = 0; cycle < ncycles; ++cycle) {
                memcpy(uh, u, N * N * N * sizeof(double));
        for (int lv = 0; lv < nlevels - 1; ++lv) {
            int64_t n = ns[lv];
            double h2 = 1.0 / ((double)n * (double)n);
            smooth(uh, fh, th, offs[lv], n, h2, NU1);
            residual(uh, fh, rh, offs[lv], n, h2);
            int64_t nc = ns[lv + 1];
            restrict_full(rh, fh, offs[lv], offs[lv + 1], nc);
            int64_t base = offs[lv + 1];
            int64_t ncoarse = nc * nc * nc;
            memset(uh + base, 0, ncoarse * sizeof(double));
        }
        int nl = nlevels - 1;
        int64_t n = ns[nl];
        double h2c = 1.0 / ((double)n * (double)n);
        smooth(uh, fh, th, offs[nl], n, h2c, NCOARSE_SWEEPS);
        for (int up = nlevels - 2; up >= 0; --up) {
            int64_t nc = ns[up + 1];
            prolong_interp(uh, offs[up], offs[up + 1], nc);
            int64_t n2 = ns[up];
            double h2 = 1.0 / ((double)n2 * (double)n2);
            smooth(uh, fh, th, offs[up], n2, h2, NU2);
        }
                memcpy(u, uh, N * N * N * sizeof(double));
        }
    }
