/* Optimized geometric multigrid V-cycle for a cell-centered 3-D grid.
 *
 * Mirrors the reference implementation in mg_vcycle_numpy.py.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

static const int LMAX = 24;                /* maximum number of levels */
static const int NCOARSE = 4;              /* edge length of coarsest grid */
static const double OMEGA = 6.0 / 7.0;      /* damped-Jacobi weight */
static const int NU1 = 3;                  /* pre-smoothing sweeps */
static const int NU2 = 3;                  /* post-smoothing sweeps */
static const int NCOARSE_SWEEPS = 40;      /* sweeps on the coarsest grid */

/* Damped-Jacobi smoother */
static void smooth(double *restrict u, const double *restrict f,
                   double *restrict tmp, int64_t off, int64_t n,
                   double h2, int nsweeps)
{
    // Two work buffers: cur holds the current solution, next holds the updated values.
    double *cur = u;
    double *next = tmp;
    for (int sweep = 0; sweep < nsweeps; ++sweep) {
        #pragma omp parallel for schedule(static) if(n >= 16)
        for (int i = 0; i < n; ++i) {
            int64_t off_i = off + (int64_t)i * n * n;
            for (int j = 0; j < n; ++j) {
                int64_t off_ij = off_i + (int64_t)j * n;
                #pragma omp simd
                for (int k = 0; k < n; ++k) {
                    int64_t c = off_ij + k;
                    double s = 0.0;
                    double d = 6.0;
                    if (i > 0) {
                        s += cur[c - n * n];
                    } else {
                        d += 1.0;
                    }
                    if (i < n - 1) {
                        s += cur[c + n * n];
                    } else {
                        d += 1.0;
                    }
                    if (j > 0) {
                        s += cur[c - n];
                    } else {
                        d += 1.0;
                    }
                    if (j < n - 1) {
                        s += cur[c + n];
                    } else {
                        d += 1.0;
                    }
                    if (k > 0) {
                        s += cur[c - 1];
                    } else {
                        d += 1.0;
                    }
                    if (k < n - 1) {
                        s += cur[c + 1];
                    } else {
                        d += 1.0;
                    }
                    double au = (d * cur[c] - s) / h2;
                    next[c] = cur[c] + OMEGA * (f[c] - au) * h2 / d;
                }
            }
        }
        // Swap buffers for next sweep
        double *swap = cur;
        cur = next;
        next = swap;
    }
    // Ensure final result is stored in the original u array
    if (cur != u) {
        #pragma omp parallel for schedule(static) if(n >= 16)
        for (int i = 0; i < n; ++i) {
            int64_t off_i = off + (int64_t)i * n * n;
            for (int j = 0; j < n; ++j) {
                int64_t off_ij = off_i + (int64_t)j * n;
                #pragma omp simd
                for (int k = 0; k < n; ++k) {
                    int64_t c = off_ij + k;
                    u[c] = cur[c];
                }
            }
        }
    }
}

/* Residual r = f - A u on one level */
#if 0
static void residual(const double *restrict u, const double *restrict f,
                     double *restrict r, int64_t off, int64_t n,
                     double h2)
{
    #pragma omp parallel for schedule(static) if(n >= 16)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            #pragma omp simd
                for (int k = 0; k < n; ++k) {
                int64_t c = off + (int64_t)(i * n + j) * n + k;
                double s = 0.0;
                double d = 6.0;
                if (i > 0) {
                    s += u[c - n * n];
                } else {
                    d += 1.0;
                }
                if (i < n - 1) {
                    s += u[c + n * n];
                } else {
                    d += 1.0;
                }
                if (j > 0) {
                    s += u[c - n];
                } else {
                    d += 1.0;
                }
                if (j < n - 1) {
                    s += u[c + n];
                } else {
                    d += 1.0;
                }
                if (k > 0) {
                    s += u[c - 1];
                } else {
                    d += 1.0;
                }
                if (k < n - 1) {
                    s += u[c + 1];
                } else {
                    d += 1.0;
                }
                r[c] = f[c] - (d * u[c] - s) / h2;
            }
        }
    }
}

#endif

/* Full-weighting restriction, averaging 8 fine cells into one coarse cell */

static void residual(const double *restrict u, const double *restrict f,
                     double *restrict r, int64_t off, int64_t n,
                     double h2)
{
    #pragma omp parallel for schedule(static) if(n >= 16)
    for (int i = 0; i < n; ++i) {
        int64_t off_i = off + (int64_t)i * n * n;
        for (int j = 0; j < n; ++j) {
            int64_t off_ij = off_i + (int64_t)j * n;
            #pragma omp simd
            for (int k = 0; k < n; ++k) {
                int64_t c = off_ij + k;
                double s = 0.0;
                double d = 6.0;
                if (i > 0) {
                    s += u[c - n * n];
                } else {
                    d += 1.0;
                }
                if (i < n - 1) {
                    s += u[c + n * n];
                } else {
                    d += 1.0;
                }
                if (j > 0) {
                    s += u[c - n];
                } else {
                    d += 1.0;
                }
                if (j < n - 1) {
                    s += u[c + n];
                } else {
                    d += 1.0;
                }
                if (k > 0) {
                    s += u[c - 1];
                } else {
                    d += 1.0;
                }
                if (k < n - 1) {
                    s += u[c + 1];
                } else {
                    d += 1.0;
                }
                r[c] = f[c] - (d * u[c] - s) / h2;
            }
        }
    }
}

static void restrict_grid(const double *restrict r, double *restrict f,
                          int64_t off_f, int64_t off_c, int64_t nc)
{
    int64_t nf = 2 * nc;
    #pragma omp parallel for schedule(static) if(nc >= 32)
    for (int64_t i = 0; i < nc; ++i) {
        for (int64_t j = 0; j < nc; ++j) {
            for (int64_t k = 0; k < nc; ++k) {
                int64_t i2 = 2 * i;
                int64_t j2 = 2 * j;
                int64_t k2 = 2 * k;
                double sum = r[off_f + (i2 * nf + j2) * nf + k2];
                sum += r[off_f + ((i2 + 1) * nf + j2) * nf + k2];
                sum += r[off_f + (i2 * nf + (j2 + 1)) * nf + k2];
                sum += r[off_f + ((i2 + 1) * nf + (j2 + 1)) * nf + k2];
                sum += r[off_f + (i2 * nf + j2) * nf + (k2 + 1)];
                sum += r[off_f + ((i2 + 1) * nf + j2) * nf + (k2 + 1)];
                sum += r[off_f + (i2 * nf + (j2 + 1)) * nf + (k2 + 1)];
                sum += r[off_f + ((i2 + 1) * nf + (j2 + 1)) * nf + (k2 + 1)];
                f[off_c + (i * nc + j) * nc + k] = 0.125 * sum;
            }
        }
    }
}

/* Trilinear prolongation (adds the interpolated correction to the fine level) */
static void prolong(double *restrict u,
                    int64_t off_f, int64_t off_c, int64_t nc)
{
    int64_t nf = 2 * nc;
    #pragma omp parallel for schedule(static) if(nc >= 32)
    for (int64_t i = 0; i < nf; ++i) {
        for (int64_t j = 0; j < nf; ++j) {
            for (int64_t k = 0; k < nf; ++k) {
                int64_t pi = i / 2;
                int64_t pj = j / 2;
                int64_t pk = k / 2;
                int si = 2 * (int)(i % 2) - 1;
                int sj = 2 * (int)(j % 2) - 1;
                int sk = 2 * (int)(k % 2) - 1;
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

/* Entry point required by the benchmark driver. */
void mg_vcycle_fp64(const double *restrict f, double *restrict u,
                    const int64_t N, const int64_t ncycles,
                    uint8_t *restrict workspace, const int64_t workspace_size)
{
    int64_t ns[LMAX];
    int64_t offs[LMAX];
    int64_t nlevels = 1;
    int64_t edge = N;
    while (edge > NCOARSE) {
        edge = edge / 2;
        nlevels = nlevels + 1;
    }
    int64_t pos = 0;
    edge = N;
    for (int64_t lv = 0; lv < nlevels; ++lv) {
        ns[lv] = edge;
        offs[lv] = pos;
        pos += edge * edge * edge;
        edge = edge / 2;
    }
    int64_t total_cells = pos;

    int64_t required = total_cells * 4 * (int64_t)sizeof(double);
    double *uh, *fh, *rh, *th;
    int allocated = 0;
    if (workspace_size >= required) {
        uh = (double *)workspace;
    } else {
        uh = (double *)malloc(required);
        if (uh == NULL) {
            return;
        }
        allocated = 1;
    }
    fh = uh + total_cells;
    rh = fh + total_cells;
    th = rh + total_cells;

    memcpy(fh, f, (size_t)N * N * N * sizeof(double));
    memset(u, 0, (size_t)N * N * N * sizeof(double));

        for (int64_t cycle = 0; cycle < ncycles; ++cycle) {
        memcpy(uh, u, (size_t)N * N * N * sizeof(double));

        for (int64_t lv = 0; lv < nlevels - 1; ++lv) {
            int64_t n = ns[lv];
            double h2 = 1.0 / ((double)n * (double)n);
            smooth(uh, fh, th, offs[lv], n, h2, NU1);
            residual(uh, fh, rh, offs[lv], n, h2);
            int64_t nc = ns[lv + 1];
            restrict_grid(rh, fh, offs[lv], offs[lv + 1], nc);
            int64_t base = offs[lv + 1];
            int64_t coarse_cells = nc * nc * nc;
            memset(uh + base, 0, (size_t)coarse_cells * sizeof(double));
        }

        int64_t nl = nlevels - 1;
        int64_t n = ns[nl];
        double h2c = 1.0 / ((double)n * (double)n);
        smooth(uh, fh, th, offs[nl], n, h2c, NCOARSE_SWEEPS);

        for (int64_t lu = 0; lu < nlevels - 1; ++lu) {
            int64_t up = nlevels - 2 - lu;
            int64_t nc = ns[up + 1];
            prolong(uh, offs[up], offs[up + 1], nc);
            int64_t nf = ns[up];
            double h2f = 1.0 / ((double)nf * (double)nf);
            smooth(uh, fh, th, offs[up], nf, h2f, NU2);
        }

        memcpy(u, uh, (size_t)N * N * N * sizeof(double));
    }
    if (allocated) free(uh);
}

