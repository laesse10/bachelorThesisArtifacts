#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// Constants from reference
static const int64_t LMAX = 24;
static const int64_t NCOARSE = 4;
static const double OMEGA = 6.0 / 7.0;
static const int NU1 = 3;
static const int NU2 = 3;
static const int NCOARSE_SWEEPS = 40;

static inline void smooth(double *restrict u, const double *restrict f, double *restrict ut,
                          int64_t off, int64_t n, double h2, int nsweeps) {
    for (int sweep = 0; sweep < nsweeps; ++sweep) {
        #pragma omp parallel for collapse(3) schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            for (int64_t j = 0; j < n; ++j) {
                for (int64_t k = 0; k < n; ++k) {
                    int64_t c = off + (i * n + j) * n + k;
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
                    double au = (d * u[c] - s) / h2;
                    ut[c] = u[c] + OMEGA * (f[c] - au) * h2 / d;
                }
            }
        }
        // copy back
        #pragma omp parallel for schedule(static)
        for (int64_t idx = off; idx < off + n * n * n; ++idx) {
            u[idx] = ut[idx];
        }    }}static inline void residual(const double *restrict u, const double *restrict f, double *restrict r,
                              int64_t off, int64_t n, double h2) {
    #pragma omp parallel for collapse(3) schedule(static)
    for (int64_t i = 0; i < n; ++i) {
        for (int64_t j = 0; j < n; ++j) {
            for (int64_t k = 0; k < n; ++k) {
                int64_t c = off + (i * n + j) * n + k;
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
static inline void restrict_full(const double *restrict r, double *restrict f,
                                 int64_t off_f, int64_t off_c, int64_t nc) {
    int64_t nf = 2 * nc;
    #pragma omp parallel for collapse(3) schedule(static)
    for (int64_t i = 0; i < nc; ++i) {
        for (int64_t j = 0; j < nc; ++j) {
            for (int64_t k = 0; k < nc; ++k) {
                double s = 0.0;
                for (int di = 0; di < 2; ++di) {
                    int64_t fi = 2 * i + di;
                    for (int dj = 0; dj < 2; ++dj) {
                        int64_t fj = 2 * j + dj;
                        for (int dk = 0; dk < 2; ++dk) {
                            int64_t fk = 2 * k + dk;
                            s += r[off_f + (fi * nf + fj) * nf + fk];
                        }
                    }
                }
                f[off_c + (i * nc + j) * nc + k] = 0.125 * s;
            }
        }
    }
}static inline void prolong(double *restrict u,
                             int64_t off_f, int64_t off_c, int64_t nc) {
    int64_t nf = 2 * nc;
    #pragma omp parallel for collapse(3) schedule(static)
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
}void mg_vcycle_fp64(const double *restrict f, double *restrict u, const int64_t N,
                    const int64_t ncycles, uint8_t *restrict workspace, const int64_t workspace_size) {
    // Compute flat buffer size
    int64_t flat = (8 * N * N * N) / 7 + 64;
    // Expected workspace size (in bytes)
    int64_t required = flat * (int64_t)sizeof(double) * 4 + (int64_t)sizeof(int64_t) * LMAX * 2;
    // If workspace is too small, allocate buffers dynamically
    double *uh_ptr = NULL;
    double *fh_ptr = NULL;
    double *rh_ptr = NULL;
    double *th_ptr = NULL;
    int64_t *ns_ptr = NULL;
    int64_t *offs_ptr = NULL;
    bool allocated = false;
    uint8_t *allocated_buf = NULL;
    if (workspace_size >= required) {
        uh_ptr = (double *)workspace;
        fh_ptr = uh_ptr + flat;
        rh_ptr = fh_ptr + flat;
        th_ptr = rh_ptr + flat;
        ns_ptr = (int64_t *)(th_ptr + flat);
        offs_ptr = ns_ptr + LMAX;
    } else {
        // allocate a single contiguous buffer
        size_t total_bytes = (size_t)flat * sizeof(double) * 4 + (size_t)sizeof(int64_t) * LMAX * 2;
        allocated_buf = (uint8_t *)malloc(total_bytes);
        if (!allocated_buf) return; // allocation failed
        allocated = true;
        uh_ptr = (double *)allocated_buf;
        fh_ptr = uh_ptr + flat;
        rh_ptr = fh_ptr + flat;
        th_ptr = rh_ptr + flat;
        ns_ptr = (int64_t *)(th_ptr + flat);
        offs_ptr = ns_ptr + LMAX;
    }
    double *uh = uh_ptr;
    double *fh = fh_ptr;
    double *rh = rh_ptr;
    double *th = th_ptr;
    int64_t *ns = ns_ptr;
    int64_t *offs = offs_ptr;

    // Determine number of multigrid levels
    int64_t nlevels = 1;
    int64_t edge = N;
    while (edge > NCOARSE) {
        edge = edge / 2;
        nlevels++;
    }
    // Build level size and offset tables
    int64_t pos = 0;
    edge = N;
    for (int64_t lv = 0; lv < nlevels; ++lv) {
        ns[lv] = edge;
        offs[lv] = pos;
        pos += edge * edge * edge;
        edge = edge / 2;
    }
    int64_t N3 = N * N * N;
    // Initialize fine RHS and zero initial guess
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N3; ++i) {
        fh[i] = f[i];
        u[i] = 0.0;
    }
    // Main V-cycle loop
    for (int64_t cyc = 0; cyc < ncycles; ++cyc) {
        // Copy current solution into working buffer
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N3; ++i) {
            uh[i] = u[i];
        }
        // Downward sweep (pre‑smoothing, residual, restriction)
        for (int64_t lv = 0; lv < nlevels - 1; ++lv) {
            int64_t n = ns[lv];
            double h2 = 1.0 / ((double)n * (double)n);
            smooth(uh, fh, th, offs[lv], n, h2, NU1);
            residual(uh, fh, rh, offs[lv], n, h2);
            int64_t nc = ns[lv + 1];
            restrict_full(rh, fh, offs[lv], offs[lv + 1], nc);
            // Zero coarse solution buffer before solving on that level
            int64_t base = offs[lv + 1];
            int64_t nc3 = nc * nc * nc;
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < nc3; ++i) {
                uh[base + i] = 0.0;
            }
        }
        // Coarsest grid solve (many smoothing sweeps)
        int64_t lvlc = nlevels - 1;
        int64_t n = ns[lvlc];
        double h2 = 1.0 / ((double)n * (double)n);
        smooth(uh, fh, th, offs[lvlc], n, h2, NCOARSE_SWEEPS);
        // Upward sweep (prolongation, post‑smoothing)
        for (int64_t lu = 0; lu < nlevels - 1; ++lu) {
            int64_t up = nlevels - 2 - lu;
            int64_t nc = ns[up + 1];
            prolong(uh, offs[up], offs[up + 1], nc);
            n = ns[up];
            h2 = 1.0 / ((double)n * (double)n);
            smooth(uh, fh, th, offs[up], n, h2, NU2);
        }
        // Copy result back to user output array
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < N3; ++i) {
            u[i] = uh[i];
        }
    }
    // Free allocated workspace buffer if allocated
    if (allocated) {
        free(allocated_buf);
    }
}