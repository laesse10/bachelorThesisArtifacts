/* Optimized geometric multigrid V-cycle, cell-centered 3-D grid.
 * Semantics: literal translation of mg_vcycle_numpy.py.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define OMEGA (6.00 / 7.0)
#define LMAX 24
#define NCOARSE 4
#define NU1 3
#define NU2 3
#define NCOARSE_SWEEPS 40

#ifndef PTH
#define PTH 32
#endif

/* adaptive thread count: keep at least ~per cells of work per thread so the
 * barrier never dominates; computed once per kernel call. */
static int g_T = 1;

static inline int nthr_for(int64_t cells, int64_t per) {
    int64_t k = cells / per;
    if (k < 1) k = 1;
    if (k > g_T) k = g_T;
    return (int)k;
}

/* ---------------- exact scalar cell kernels (mirror the reference) ----------- */

static inline double smooth_cell(const double *restrict u, const double *restrict f,
                                 int64_t c, int64_t nn, int64_t n,
                                 int64_t i, int64_t j, int64_t k, double h2) {
    double s = 0.0;
    double d = 6.0;
    if (i > 0) s = s + u[c - nn]; else d = d + 1.0;
    if (i < n - 1) s = s + u[c + nn]; else d = d + 1.0;
    if (j > 0) s = s + u[c - n]; else d = d + 1.0;
    if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
    if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
    if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
    double au = (d * u[c] - s) / h2;
    return u[c] + OMEGA * (f[c] - au) * h2 / d;
}

static inline double resid_cell(const double *restrict u, const double *restrict f,
                                int64_t c, int64_t nn, int64_t n,
                                int64_t i, int64_t j, int64_t k, double h2) {
    double s = 0.0;
    double d = 6.0;
    if (i > 0) s = s + u[c - nn]; else d = d + 1.0;
    if (i < n - 1) s = s + u[c + nn]; else d = d + 1.0;
    if (j > 0) s = s + u[c - n]; else d = d + 1.0;
    if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
    if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
    if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
    return f[c] - (d * u[c] - s) / h2;
}

/* ---------------- one damped-Jacobi sweep: dst <- smooth(src) ---------------- */


/* fully unrolled n=4 sweep for the coarsest solve (compile-time flags) */
static void sweep_n4(double *restrict dst, const double *restrict u,
                      const double *restrict f, double h2) {
    const double ih2 = 1.0 / h2;
    const double k6 = OMEGA * h2 * (1.0 / 6.0);
    const double k7 = OMEGA * h2 * (1.0 / 7.0);
    const double k8 = OMEGA * h2 * (1.0 / 8.0);
    const double k9 = OMEGA * h2 * (1.0 / 9.0);
    { const int64_t k = 0;
        double s = u[0+k+16] + u[0+k+4] + u[0+k+1];
        double au = (9.0 * u[0+k] - s) * ih2;
        dst[0+k] = u[0+k] + (f[0+k] - au) * k9; }
    { const int64_t k = 1;
        double s = u[0+k+16] + u[0+k+4] + u[0+k-1] + u[0+k+1];
        double au = (8.0 * u[0+k] - s) * ih2;
        dst[0+k] = u[0+k] + (f[0+k] - au) * k8; }
    { const int64_t k = 2;
        double s = u[0+k+16] + u[0+k+4] + u[0+k-1] + u[0+k+1];
        double au = (8.0 * u[0+k] - s) * ih2;
        dst[0+k] = u[0+k] + (f[0+k] - au) * k8; }
    { const int64_t k = 3;
        double s = u[0+k+16] + u[0+k+4] + u[0+k-1];
        double au = (9.0 * u[0+k] - s) * ih2;
        dst[0+k] = u[0+k] + (f[0+k] - au) * k9; }
    { const int64_t k = 0;
        double s = u[4+k+16] + u[4+k-4] + u[4+k+4] + u[4+k+1];
        double au = (8.0 * u[4+k] - s) * ih2;
        dst[4+k] = u[4+k] + (f[4+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[4+k+16] + u[4+k-4] + u[4+k+4] + u[4+k-1] + u[4+k+1];
        double au = (7.0 * u[4+k] - s) * ih2;
        dst[4+k] = u[4+k] + (f[4+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[4+k+16] + u[4+k-4] + u[4+k+4] + u[4+k-1] + u[4+k+1];
        double au = (7.0 * u[4+k] - s) * ih2;
        dst[4+k] = u[4+k] + (f[4+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[4+k+16] + u[4+k-4] + u[4+k+4] + u[4+k-1];
        double au = (8.0 * u[4+k] - s) * ih2;
        dst[4+k] = u[4+k] + (f[4+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[8+k+16] + u[8+k-4] + u[8+k+4] + u[8+k+1];
        double au = (8.0 * u[8+k] - s) * ih2;
        dst[8+k] = u[8+k] + (f[8+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[8+k+16] + u[8+k-4] + u[8+k+4] + u[8+k-1] + u[8+k+1];
        double au = (7.0 * u[8+k] - s) * ih2;
        dst[8+k] = u[8+k] + (f[8+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[8+k+16] + u[8+k-4] + u[8+k+4] + u[8+k-1] + u[8+k+1];
        double au = (7.0 * u[8+k] - s) * ih2;
        dst[8+k] = u[8+k] + (f[8+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[8+k+16] + u[8+k-4] + u[8+k+4] + u[8+k-1];
        double au = (8.0 * u[8+k] - s) * ih2;
        dst[8+k] = u[8+k] + (f[8+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[12+k+16] + u[12+k-4] + u[12+k+1];
        double au = (9.0 * u[12+k] - s) * ih2;
        dst[12+k] = u[12+k] + (f[12+k] - au) * k9; }
    { const int64_t k = 1;
        double s = u[12+k+16] + u[12+k-4] + u[12+k-1] + u[12+k+1];
        double au = (8.0 * u[12+k] - s) * ih2;
        dst[12+k] = u[12+k] + (f[12+k] - au) * k8; }
    { const int64_t k = 2;
        double s = u[12+k+16] + u[12+k-4] + u[12+k-1] + u[12+k+1];
        double au = (8.0 * u[12+k] - s) * ih2;
        dst[12+k] = u[12+k] + (f[12+k] - au) * k8; }
    { const int64_t k = 3;
        double s = u[12+k+16] + u[12+k-4] + u[12+k-1];
        double au = (9.0 * u[12+k] - s) * ih2;
        dst[12+k] = u[12+k] + (f[12+k] - au) * k9; }
    { const int64_t k = 0;
        double s = u[16+k-16] + u[16+k+16] + u[16+k+4] + u[16+k+1];
        double au = (8.0 * u[16+k] - s) * ih2;
        dst[16+k] = u[16+k] + (f[16+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[16+k-16] + u[16+k+16] + u[16+k+4] + u[16+k-1] + u[16+k+1];
        double au = (7.0 * u[16+k] - s) * ih2;
        dst[16+k] = u[16+k] + (f[16+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[16+k-16] + u[16+k+16] + u[16+k+4] + u[16+k-1] + u[16+k+1];
        double au = (7.0 * u[16+k] - s) * ih2;
        dst[16+k] = u[16+k] + (f[16+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[16+k-16] + u[16+k+16] + u[16+k+4] + u[16+k-1];
        double au = (8.0 * u[16+k] - s) * ih2;
        dst[16+k] = u[16+k] + (f[16+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[20+k-16] + u[20+k+16] + u[20+k-4] + u[20+k+4] + u[20+k+1];
        double au = (7.0 * u[20+k] - s) * ih2;
        dst[20+k] = u[20+k] + (f[20+k] - au) * k7; }
    { const int64_t k = 1;
        double s = u[20+k-16] + u[20+k+16] + u[20+k-4] + u[20+k+4] + u[20+k-1] + u[20+k+1];
        double au = (6.0 * u[20+k] - s) * ih2;
        dst[20+k] = u[20+k] + (f[20+k] - au) * k6; }
    { const int64_t k = 2;
        double s = u[20+k-16] + u[20+k+16] + u[20+k-4] + u[20+k+4] + u[20+k-1] + u[20+k+1];
        double au = (6.0 * u[20+k] - s) * ih2;
        dst[20+k] = u[20+k] + (f[20+k] - au) * k6; }
    { const int64_t k = 3;
        double s = u[20+k-16] + u[20+k+16] + u[20+k-4] + u[20+k+4] + u[20+k-1];
        double au = (7.0 * u[20+k] - s) * ih2;
        dst[20+k] = u[20+k] + (f[20+k] - au) * k7; }
    { const int64_t k = 0;
        double s = u[24+k-16] + u[24+k+16] + u[24+k-4] + u[24+k+4] + u[24+k+1];
        double au = (7.0 * u[24+k] - s) * ih2;
        dst[24+k] = u[24+k] + (f[24+k] - au) * k7; }
    { const int64_t k = 1;
        double s = u[24+k-16] + u[24+k+16] + u[24+k-4] + u[24+k+4] + u[24+k-1] + u[24+k+1];
        double au = (6.0 * u[24+k] - s) * ih2;
        dst[24+k] = u[24+k] + (f[24+k] - au) * k6; }
    { const int64_t k = 2;
        double s = u[24+k-16] + u[24+k+16] + u[24+k-4] + u[24+k+4] + u[24+k-1] + u[24+k+1];
        double au = (6.0 * u[24+k] - s) * ih2;
        dst[24+k] = u[24+k] + (f[24+k] - au) * k6; }
    { const int64_t k = 3;
        double s = u[24+k-16] + u[24+k+16] + u[24+k-4] + u[24+k+4] + u[24+k-1];
        double au = (7.0 * u[24+k] - s) * ih2;
        dst[24+k] = u[24+k] + (f[24+k] - au) * k7; }
    { const int64_t k = 0;
        double s = u[28+k-16] + u[28+k+16] + u[28+k-4] + u[28+k+1];
        double au = (8.0 * u[28+k] - s) * ih2;
        dst[28+k] = u[28+k] + (f[28+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[28+k-16] + u[28+k+16] + u[28+k-4] + u[28+k-1] + u[28+k+1];
        double au = (7.0 * u[28+k] - s) * ih2;
        dst[28+k] = u[28+k] + (f[28+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[28+k-16] + u[28+k+16] + u[28+k-4] + u[28+k-1] + u[28+k+1];
        double au = (7.0 * u[28+k] - s) * ih2;
        dst[28+k] = u[28+k] + (f[28+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[28+k-16] + u[28+k+16] + u[28+k-4] + u[28+k-1];
        double au = (8.0 * u[28+k] - s) * ih2;
        dst[28+k] = u[28+k] + (f[28+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[32+k-16] + u[32+k+16] + u[32+k+4] + u[32+k+1];
        double au = (8.0 * u[32+k] - s) * ih2;
        dst[32+k] = u[32+k] + (f[32+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[32+k-16] + u[32+k+16] + u[32+k+4] + u[32+k-1] + u[32+k+1];
        double au = (7.0 * u[32+k] - s) * ih2;
        dst[32+k] = u[32+k] + (f[32+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[32+k-16] + u[32+k+16] + u[32+k+4] + u[32+k-1] + u[32+k+1];
        double au = (7.0 * u[32+k] - s) * ih2;
        dst[32+k] = u[32+k] + (f[32+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[32+k-16] + u[32+k+16] + u[32+k+4] + u[32+k-1];
        double au = (8.0 * u[32+k] - s) * ih2;
        dst[32+k] = u[32+k] + (f[32+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[36+k-16] + u[36+k+16] + u[36+k-4] + u[36+k+4] + u[36+k+1];
        double au = (7.0 * u[36+k] - s) * ih2;
        dst[36+k] = u[36+k] + (f[36+k] - au) * k7; }
    { const int64_t k = 1;
        double s = u[36+k-16] + u[36+k+16] + u[36+k-4] + u[36+k+4] + u[36+k-1] + u[36+k+1];
        double au = (6.0 * u[36+k] - s) * ih2;
        dst[36+k] = u[36+k] + (f[36+k] - au) * k6; }
    { const int64_t k = 2;
        double s = u[36+k-16] + u[36+k+16] + u[36+k-4] + u[36+k+4] + u[36+k-1] + u[36+k+1];
        double au = (6.0 * u[36+k] - s) * ih2;
        dst[36+k] = u[36+k] + (f[36+k] - au) * k6; }
    { const int64_t k = 3;
        double s = u[36+k-16] + u[36+k+16] + u[36+k-4] + u[36+k+4] + u[36+k-1];
        double au = (7.0 * u[36+k] - s) * ih2;
        dst[36+k] = u[36+k] + (f[36+k] - au) * k7; }
    { const int64_t k = 0;
        double s = u[40+k-16] + u[40+k+16] + u[40+k-4] + u[40+k+4] + u[40+k+1];
        double au = (7.0 * u[40+k] - s) * ih2;
        dst[40+k] = u[40+k] + (f[40+k] - au) * k7; }
    { const int64_t k = 1;
        double s = u[40+k-16] + u[40+k+16] + u[40+k-4] + u[40+k+4] + u[40+k-1] + u[40+k+1];
        double au = (6.0 * u[40+k] - s) * ih2;
        dst[40+k] = u[40+k] + (f[40+k] - au) * k6; }
    { const int64_t k = 2;
        double s = u[40+k-16] + u[40+k+16] + u[40+k-4] + u[40+k+4] + u[40+k-1] + u[40+k+1];
        double au = (6.0 * u[40+k] - s) * ih2;
        dst[40+k] = u[40+k] + (f[40+k] - au) * k6; }
    { const int64_t k = 3;
        double s = u[40+k-16] + u[40+k+16] + u[40+k-4] + u[40+k+4] + u[40+k-1];
        double au = (7.0 * u[40+k] - s) * ih2;
        dst[40+k] = u[40+k] + (f[40+k] - au) * k7; }
    { const int64_t k = 0;
        double s = u[44+k-16] + u[44+k+16] + u[44+k-4] + u[44+k+1];
        double au = (8.0 * u[44+k] - s) * ih2;
        dst[44+k] = u[44+k] + (f[44+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[44+k-16] + u[44+k+16] + u[44+k-4] + u[44+k-1] + u[44+k+1];
        double au = (7.0 * u[44+k] - s) * ih2;
        dst[44+k] = u[44+k] + (f[44+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[44+k-16] + u[44+k+16] + u[44+k-4] + u[44+k-1] + u[44+k+1];
        double au = (7.0 * u[44+k] - s) * ih2;
        dst[44+k] = u[44+k] + (f[44+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[44+k-16] + u[44+k+16] + u[44+k-4] + u[44+k-1];
        double au = (8.0 * u[44+k] - s) * ih2;
        dst[44+k] = u[44+k] + (f[44+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[48+k-16] + u[48+k+4] + u[48+k+1];
        double au = (9.0 * u[48+k] - s) * ih2;
        dst[48+k] = u[48+k] + (f[48+k] - au) * k9; }
    { const int64_t k = 1;
        double s = u[48+k-16] + u[48+k+4] + u[48+k-1] + u[48+k+1];
        double au = (8.0 * u[48+k] - s) * ih2;
        dst[48+k] = u[48+k] + (f[48+k] - au) * k8; }
    { const int64_t k = 2;
        double s = u[48+k-16] + u[48+k+4] + u[48+k-1] + u[48+k+1];
        double au = (8.0 * u[48+k] - s) * ih2;
        dst[48+k] = u[48+k] + (f[48+k] - au) * k8; }
    { const int64_t k = 3;
        double s = u[48+k-16] + u[48+k+4] + u[48+k-1];
        double au = (9.0 * u[48+k] - s) * ih2;
        dst[48+k] = u[48+k] + (f[48+k] - au) * k9; }
    { const int64_t k = 0;
        double s = u[52+k-16] + u[52+k-4] + u[52+k+4] + u[52+k+1];
        double au = (8.0 * u[52+k] - s) * ih2;
        dst[52+k] = u[52+k] + (f[52+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[52+k-16] + u[52+k-4] + u[52+k+4] + u[52+k-1] + u[52+k+1];
        double au = (7.0 * u[52+k] - s) * ih2;
        dst[52+k] = u[52+k] + (f[52+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[52+k-16] + u[52+k-4] + u[52+k+4] + u[52+k-1] + u[52+k+1];
        double au = (7.0 * u[52+k] - s) * ih2;
        dst[52+k] = u[52+k] + (f[52+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[52+k-16] + u[52+k-4] + u[52+k+4] + u[52+k-1];
        double au = (8.0 * u[52+k] - s) * ih2;
        dst[52+k] = u[52+k] + (f[52+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[56+k-16] + u[56+k-4] + u[56+k+4] + u[56+k+1];
        double au = (8.0 * u[56+k] - s) * ih2;
        dst[56+k] = u[56+k] + (f[56+k] - au) * k8; }
    { const int64_t k = 1;
        double s = u[56+k-16] + u[56+k-4] + u[56+k+4] + u[56+k-1] + u[56+k+1];
        double au = (7.0 * u[56+k] - s) * ih2;
        dst[56+k] = u[56+k] + (f[56+k] - au) * k7; }
    { const int64_t k = 2;
        double s = u[56+k-16] + u[56+k-4] + u[56+k+4] + u[56+k-1] + u[56+k+1];
        double au = (7.0 * u[56+k] - s) * ih2;
        dst[56+k] = u[56+k] + (f[56+k] - au) * k7; }
    { const int64_t k = 3;
        double s = u[56+k-16] + u[56+k-4] + u[56+k+4] + u[56+k-1];
        double au = (8.0 * u[56+k] - s) * ih2;
        dst[56+k] = u[56+k] + (f[56+k] - au) * k8; }
    { const int64_t k = 0;
        double s = u[60+k-16] + u[60+k-4] + u[60+k+1];
        double au = (9.0 * u[60+k] - s) * ih2;
        dst[60+k] = u[60+k] + (f[60+k] - au) * k9; }
    { const int64_t k = 1;
        double s = u[60+k-16] + u[60+k-4] + u[60+k-1] + u[60+k+1];
        double au = (8.0 * u[60+k] - s) * ih2;
        dst[60+k] = u[60+k] + (f[60+k] - au) * k8; }
    { const int64_t k = 2;
        double s = u[60+k-16] + u[60+k-4] + u[60+k-1] + u[60+k+1];
        double au = (8.0 * u[60+k] - s) * ih2;
        dst[60+k] = u[60+k] + (f[60+k] - au) * k8; }
    { const int64_t k = 3;
        double s = u[60+k-16] + u[60+k-4] + u[60+k-1];
        double au = (9.0 * u[60+k] - s) * ih2;
        dst[60+k] = u[60+k] + (f[60+k] - au) * k9; }
}

/* one k-row of a Jacobi sweep; the i/neighbor flags are compile-time per
 * call site so the vector loop carries no branches. im/ip/jm/jp: the
 * i-/i+/j-/j+ neighbor exists. */
static inline void row_smooth(double *restrict dst, const double *restrict u,
                              const double *restrict f, int64_t base,
                              int64_t nn, int64_t n, double h2,
                              const int im, const int ip,
                              const int jm, const int jp) {
    const int64_t n1 = n - 1;
    const double d = 6.0 + (im ? 0.0 : 1.0) + (ip ? 0.0 : 1.0)
                   + (jm ? 0.0 : 1.0) + (jp ? 0.0 : 1.0);
    const double ih2 = 1.0 / h2;
    const double kd = OMEGA * h2 / d;
    for (int64_t k = 1; k < n1; k++) {
        double s = 0.0;
        if (im) s += u[base + k - nn];
        if (ip) s += u[base + k + nn];
        if (jm) s += u[base + k - n];
        if (jp) s += u[base + k + n];
        s = s + u[base + k - 1] + u[base + k + 1];
        double au = (d * u[base + k] - s) * ih2;
        dst[base + k] = u[base + k] + (f[base + k] - au) * kd;
    }
    {
        const double kdd = kd * (d / (d + 1.0));
        double s = 0.0;
        if (im) s += u[base - nn];
        if (ip) s += u[base + nn];
        if (jm) s += u[base - n];
        if (jp) s += u[base + n];
        s += u[base + 1];
        const double au = ((d + 1.0) * u[base] - s) * ih2;
        dst[base] = u[base] + (f[base] - au) * kdd;
    }
    {
        const double kdd = kd * (d / (d + 1.0));
        double s = 0.0;
        if (im) s += u[base + n1 - nn];
        if (ip) s += u[base + n1 + nn];
        if (jm) s += u[base + n1 - n];
        if (jp) s += u[base + n1 + n];
        s += u[base + n1 - 1];
        const double au = ((d + 1.0) * u[base + n1] - s) * ih2;
        dst[base + n1] = u[base + n1] + (f[base + n1] - au) * kdd;
    }
}

static inline void row_resid(double *restrict r, const double *restrict u,
                             const double *restrict f, int64_t base,
                             int64_t nn, int64_t n, double h2,
                             const int im, const int ip,
                             const int jm, const int jp) {
    const int64_t n1 = n - 1;
    const double d = 6.0 + (im ? 0.0 : 1.0) + (ip ? 0.0 : 1.0)
                   + (jm ? 0.0 : 1.0) + (jp ? 0.0 : 1.0);
    const double ih2 = 1.0 / h2;
    for (int64_t k = 1; k < n1; k++) {
        double s = 0.0;
        if (im) s += u[base + k - nn];
        if (ip) s += u[base + k + nn];
        if (jm) s += u[base + k - n];
        if (jp) s += u[base + k + n];
        s = s + u[base + k - 1] + u[base + k + 1];
        r[base + k] = f[base + k] - (d * u[base + k] - s) * ih2;
    }
    {
        double s = 0.0;
        if (im) s += u[base - nn];
        if (ip) s += u[base + nn];
        if (jm) s += u[base - n];
        if (jp) s += u[base + n];
        s += u[base + 1];
        double dd = d + 1.0;
        r[base] = f[base] - (dd * u[base] - s) * ih2;
    }
    {
        double s = 0.0;
        if (im) s += u[base + n1 - nn];
        if (ip) s += u[base + n1 + nn];
        if (jm) s += u[base + n1 - n];
        if (jp) s += u[base + n1 + n];
        s += u[base + n1 - 1];
        double dd = d + 1.0;
        r[base + n1] = f[base + n1] - (dd * u[base + n1] - s) * ih2;
    }
}

static void sweep_rows(double *restrict dst, const double *restrict u,
                        const double *restrict f, int64_t n, double h2,
                        int64_t i0, int64_t i1) {
    const int64_t nn = n * n;
    const int64_t n1 = n - 1;
    for (int64_t i = i0; i < i1; i++) {
        const int64_t irow = i * n;
        if (i == 0) {
            row_smooth(dst, u, f, irow * n, nn, n, h2, 0, 1, 0, 1);
            for (int64_t j = 1; j < n1; j++)
                row_smooth(dst, u, f, (irow + j) * n, nn, n, h2, 0, 1, 1, 1);
            row_smooth(dst, u, f, (irow + n1) * n, nn, n, h2, 0, 1, 1, 0);
        } else if (i == n1) {
            row_smooth(dst, u, f, irow * n, nn, n, h2, 1, 0, 0, 1);
            for (int64_t j = 1; j < n1; j++)
                row_smooth(dst, u, f, (irow + j) * n, nn, n, h2, 1, 0, 1, 1);
            row_smooth(dst, u, f, (irow + n1) * n, nn, n, h2, 1, 0, 1, 0);
        } else {
            row_smooth(dst, u, f, irow * n, nn, n, h2, 1, 1, 0, 1);
            for (int64_t j = 1; j < n1; j++)
                row_smooth(dst, u, f, (irow + j) * n, nn, n, h2, 1, 1, 1, 1);
            row_smooth(dst, u, f, (irow + n1) * n, nn, n, h2, 1, 1, 1, 0);
        }
    }
}

static void sweep(double *restrict dst, const double *restrict u,
                  const double *restrict f, int64_t n, double h2) {
    if (n == 4) { sweep_n4(dst, u, f, h2); return; }
    const int k = nthr_for(n * n * n, 16384);
    if (k >= 2) {
#pragma omp parallel for schedule(static) num_threads(k)
        for (int64_t i = 0; i < n; i++) sweep_rows(dst, u, f, n, h2, i, i + 1);
    } else {
        sweep_rows(dst, u, f, n, h2, 0, n);
    }
}

/* ping-pong smoothing; returns the buffer holding the result */
static double *smooth_pp(double *a, double *b, const double *f, int64_t n,
                         double h2, int nsweeps) {
    for (int s = 0; s < nsweeps; s++) {
        sweep(b, a, f, n, h2);
        double *t = a; a = b; b = t;
    }
    return a;
}

/* ---------------- residual pass (shares the reference arithmetic) ------------- */

static void residual_rows(double *restrict r, const double *restrict u,
                          const double *restrict f, int64_t n, double h2,
                          int64_t i0, int64_t i1) {
    const int64_t nn = n * n;
    const int64_t n1 = n - 1;
    for (int64_t i = i0; i < i1; i++) {
        const int64_t irow = i * n;
        if (i == 0) {
            row_resid(r, u, f, irow * n, nn, n, h2, 0, 1, 0, 1);
            for (int64_t j = 1; j < n1; j++)
                row_resid(r, u, f, (irow + j) * n, nn, n, h2, 0, 1, 1, 1);
            row_resid(r, u, f, (irow + n1) * n, nn, n, h2, 0, 1, 1, 0);
        } else if (i == n1) {
            row_resid(r, u, f, irow * n, nn, n, h2, 1, 0, 0, 1);
            for (int64_t j = 1; j < n1; j++)
                row_resid(r, u, f, (irow + j) * n, nn, n, h2, 1, 0, 1, 1);
            row_resid(r, u, f, (irow + n1) * n, nn, n, h2, 1, 0, 1, 0);
        } else {
            row_resid(r, u, f, irow * n, nn, n, h2, 1, 1, 0, 1);
            for (int64_t j = 1; j < n1; j++)
                row_resid(r, u, f, (irow + j) * n, nn, n, h2, 1, 1, 1, 1);
            row_resid(r, u, f, (irow + n1) * n, nn, n, h2, 1, 1, 1, 0);
        }
    }
}

static void residual_pass(double *restrict r, const double *restrict u,
                          const double *restrict f, int64_t n, double h2) {
    const int k = nthr_for(n * n * n, 16384);
    if (k >= 2) {
#pragma omp parallel for schedule(static) num_threads(k)
        for (int64_t i = 0; i < n; i++) residual_rows(r, u, f, n, h2, i, i + 1);
    } else {
        residual_rows(r, u, f, n, h2, 0, n);
    }
}
/* -------- restriction: coarse rhs = 0.125 * the sum of 8 child residuals ---- */

static void restrict_rows(double *restrict fc, const double *restrict r,
                           int64_t off_f, int64_t off_c, int64_t nc,
                           int64_t i0, int64_t i1) {
    const int64_t nf = 2 * nc;
    for (int64_t i = i0; i < i1; i++) {
        for (int64_t j = 0; j < nc; j++) {
            const int64_t fbase = off_f + (2 * i * nf + 2 * j) * nf;
            const int64_t cbase = off_c + (i * nc + j) * nc;
            const double *R00 = r + fbase;
            const double *R01 = R00 + nf;
            const double *R10 = R00 + nf * nf;
            const double *R11 = R10 + nf;
            for (int64_t m = 0; m < nc; m++) {
                double s = R00[2 * m] + R00[2 * m + 1];
                s = s + R01[2 * m] + R01[2 * m + 1];
                s = s + R10[2 * m] + R10[2 * m + 1];
                s = s + R11[2 * m] + R11[2 * m + 1];
                fc[cbase + m] = 0.125 * s;
            }
        }
    }
}

static void restrict_pass(double *restrict fc, const double *restrict r,
                           int64_t off_f, int64_t off_c, int64_t nc) {
    const int k = nthr_for(nc * nc * nc, 8192);
    if (k >= 2) {
#pragma omp parallel for schedule(static) num_threads(k)
        for (int64_t i = 0; i < nc; i++)
            restrict_rows(fc, r, off_f, off_c, nc, i, i + 1);
    } else {
        restrict_rows(fc, r, off_f, off_c, nc, 0, nc);
    }
}

/* -------- fused residual + restriction --------------------------------------
 * The coarse right-hand side is 0.125 * the sum of the 8 child residuals, each
 * formed on the fly from (u, f); the fine residual block is never written or
 * re-read.  Each fine k-pair (2m, 2m+1) lands in two half-length streams so the
 * fold keeps the reference's left-to-right summation order (bit-compatible).
 * Missing i/j neighbours read a zero buffer instead of branching. */

static const double g_zbuf[2048] __attribute__((aligned(64)));

static inline void row_pair_resid(double *restrict dA, double *restrict dB,
                                  const double *restrict u, const double *restrict f,
                                  int64_t base, int64_t n, double h2,
                                  const double *pm, const double *pp,
                                  const double *qm, const double *qp, double d) {
    const int64_t nh = n >> 1;
    const double ih2 = 1.0 / h2;
    for (int64_t m = 1; m < nh - 1; m++) {
        const int64_t k = m + m;
        const int64_t k1 = k + 1;
        double sa = pm[k] + pp[k] + qm[k] + qp[k] + u[base + k - 1] + u[base + k + 1];
        dA[m] = f[base + k] - (d * u[base + k] - sa) * ih2;
        double sb = pm[k1] + pp[k1] + qm[k1] + qp[k1] + u[base + k1 - 1] + u[base + k1 + 1];
        dB[m] = f[base + k1] - (d * u[base + k1] - sb) * ih2;
    }
    {   /* pair (0, 1): k = 0 has no left neighbour (diagonal + 1) */
        double sa = pm[0] + pp[0] + qm[0] + qp[0] + u[base + 1];
        dA[0] = f[base] - ((d + 1.0) * u[base] - sa) * ih2;
        double sb = pm[1] + pp[1] + qm[1] + qp[1] + u[base] + u[base + 2];
        dB[0] = f[base + 1] - (d * u[base + 1] - sb) * ih2;
    }
    {   /* pair (n - 2, n - 1): k = n - 1 has no right neighbour */
        const int64_t k = n - 2, k1 = n - 1;
        double sa = pm[k] + pp[k] + qm[k] + qp[k] + u[base + k - 1] + u[base + k + 1];
        dA[nh - 1] = f[base + k] - (d * u[base + k] - sa) * ih2;
        double sb = pm[k1] + pp[k1] + qm[k1] + qp[k1] + u[base + k1 - 1];
        dB[nh - 1] = f[base + k1] - ((d + 1.0) * u[base + k1] - sb) * ih2;
    }
}

static void fused_rr_rows(double *restrict fc, const double *restrict u,
                          const double *restrict f, int64_t off_f, int64_t off_c,
                          int64_t nc, double h2, int64_t ci0, int64_t ci1) {
    const int64_t nf = 2 * nc;
    const int64_t nn = nf * nf;
    double buf[8][1026];
    for (int64_t ci = ci0; ci < ci1; ci++) {
        const int64_t fi0 = 2 * ci;
        for (int64_t cj = 0; cj < nc; cj++) {
            const int64_t fj0 = 2 * cj;
            for (int t = 0; t < 4; t++) {
                const int64_t fi = fi0 + (t >> 1);
                const int64_t fj = fj0 + (t & 1);
                const int im = fi > 0, ip = fi < nf - 1;
                const int jm = fj > 0, jp = fj < nf - 1;
                const double d = 6.0 + (im ? 0.0 : 1.0) + (ip ? 0.0 : 1.0)
                               + (jm ? 0.0 : 1.0) + (jp ? 0.0 : 1.0);
                const int64_t base = off_f + (fi * nf + fj) * nf;
                row_pair_resid(buf[2 * t], buf[2 * t + 1], u, f, base, nf, h2,
                                im ? u + base - nn : g_zbuf,
                                ip ? u + base + nn : g_zbuf,
                                jm ? u + base - nf : g_zbuf,
                                jp ? u + base + nf : g_zbuf, d);
            }
            double *fr = fc + off_c + (ci * nc + cj) * nc;
            for (int64_t m = 0; m < nc; m++) {
                double s = buf[0][m] + buf[1][m];
                s = s + buf[2][m];
                s = s + buf[3][m];
                s = s + buf[4][m];
                s = s + buf[5][m];
                s = s + buf[6][m];
                s = s + buf[7][m];
                fr[m] = 0.125 * s;
            }
        }
    }
}

static void fused_rr_pass(double *restrict fc, const double *restrict u,
                          const double *restrict f, int64_t off_f, int64_t off_c,
                          int64_t nc, double h2) {
    const int k = nthr_for(8 * nc * nc * nc, 16384);
    if (k >= 2) {
#pragma omp parallel for schedule(static) num_threads(k)
        for (int64_t i = 0; i < nc; i++)
            fused_rr_rows(fc, u, f, off_f, off_c, nc, h2, i, i + 1);
    } else {
        fused_rr_rows(fc, u, f, off_f, off_c, nc, h2, 0, nc);
    }
}

/* -------- fused separable prolongation (trilinear, odd reflection) ----------
 * value = (3a+b)(3c+d)(3e+f)/64 per axis with the odd-reflection neighbor.
 * Row-local temporaries only; overwrites (target known zero) or accumulates. */

static inline void expand_row(const double *restrict C, double *restrict E, int64_t nc) {
    const int64_t nc1 = nc - 1;
    for (int64_t m = 0; m < nc; m++) {
        double a = C[m];
        double bl = (m > 0) ? C[m - 1] : -a;
        double bh = (m < nc1) ? C[m + 1] : -a;
        E[2 * m] = 3.0 * a + bl;
        E[2 * m + 1] = 3.0 * a + bh;
    }
}

static void prolong_rows(double *restrict uf, const double *restrict uc,
                          int64_t nf, int64_t fi0, int64_t fi1,
                          double *B0, double *B1, double *B2,
                          double *B3, double *B4, double *B5) {
    const int64_t nc = nf / 2;
    const int64_t nc1 = nc - 1;
    const double inv64 = 1.0 / 64.0;
    for (int64_t fi = fi0; fi < fi1; fi++) {
        const int64_t pi = fi >> 1;
        const int64_t si = (fi & 1) ? 1 : -1;
        const int64_t ci1 = pi + si;
        const int iok = (ci1 >= 0 && ci1 <= nc1);
        const int64_t rowpi = pi * nc;
        const int64_t rowci = (iok ? ci1 : 0) * nc;
        double *EprevA = B0, *EcurA = B1, *EnextA = B2;
        double *EprevB = B3, *EcurB = B4, *EnextB = B5;
        expand_row(uc + (rowpi + 0) * nc, EcurA, nc);
        if (iok) expand_row(uc + (rowci + 0) * nc, EcurB, nc);
        for (int64_t pj = 0; pj < nc; pj++) {
            /* fine row 2*pj: leans to pj-1 */
            {
                double *out = uf + (fi * nf + 2 * pj) * nf;
                const double *curA = EcurA;
                const double *prevA = (pj > 0) ? EprevA : EcurA;
                const double spa = (pj > 0) ? 1.0 : -1.0;
                if (iok) {
                    const double *curB = EcurB;
                    const double *prevB = (pj > 0) ? EprevB : EcurB;
                    for (int64_t k = 0; k < nf; k++) {
                        double t1 = 3.0 * curA[k] + spa * prevA[k];
                        double t2 = 3.0 * curB[k] + spa * prevB[k];
                        out[k] += (3.0 * t1 + t2) * inv64;
                    }
                } else {
                    for (int64_t k = 0; k < nf; k++) {
                        double t1 = 3.0 * curA[k] + spa * prevA[k];
                        out[k] += (3.0 * t1 - t1) * inv64;
                    }
                }
            }
            if (pj < nc1) {
                expand_row(uc + (rowpi + pj + 1) * nc, EnextA, nc);
                if (iok) expand_row(uc + (rowci + pj + 1) * nc, EnextB, nc);
            }
            /* fine row 2*pj+1: leans to pj+1 */
            {
                double *out = uf + (fi * nf + 2 * pj + 1) * nf;
                const double *curA = EcurA;
                const double *nextA = (pj < nc1) ? EnextA : EcurA;
                const double sna = (pj < nc1) ? 1.0 : -1.0;
                if (iok) {
                    const double *curB = EcurB;
                    const double *nextB = (pj < nc1) ? EnextB : EcurB;
                    for (int64_t k = 0; k < nf; k++) {
                        double t1 = 3.0 * curA[k] + sna * nextA[k];
                        double t2 = 3.0 * curB[k] + sna * nextB[k];
                        out[k] += (3.0 * t1 + t2) * inv64;
                    }
                } else {
                    for (int64_t k = 0; k < nf; k++) {
                        double t1 = 3.0 * curA[k] + sna * nextA[k];
                        out[k] += (3.0 * t1 - t1) * inv64;
                    }
                }
            }
            {
                double *t = EprevA; EprevA = EcurA; EcurA = EnextA; EnextA = t;
            }
            {
                double *t = EprevB; EprevB = EcurB; EcurB = EnextB; EnextB = t;
            }
        }
    }
}

static void prolong_fused(double *restrict uf, const double *restrict uc,
                          int64_t nf) {
    const int k = nthr_for(nf * nf * nf, 16384);
    if (k >= 2) {
#pragma omp parallel num_threads(k)
        {
            double *B[6];
            for (int q = 0; q < 6; q++) B[q] = (double *)malloc((size_t)nf * sizeof(double));
#pragma omp for schedule(static)
            for (int64_t fi = 0; fi < nf; fi++)
                prolong_rows(uf, uc, nf, fi, fi + 1,
                             B[0], B[1], B[2], B[3], B[4], B[5]);
            for (int q = 0; q < 6; q++) free(B[q]);
        }
    } else if (nf <= 512) {
        double B[6 * 512];
        prolong_rows(uf, uc, nf, 0, nf, B, B + nf, B + 2 * nf,
                     B + 3 * nf, B + 4 * nf, B + 5 * nf);
    } else {
        double *B[6];
        for (int q = 0; q < 6; q++) B[q] = (double *)malloc((size_t)nf * sizeof(double));
        prolong_rows(uf, uc, nf, 0, nf, B[0], B[1], B[2], B[3], B[4], B[5]);
        for (int q = 0; q < 6; q++) free(B[q]);
    }
}

/* ---------------- literal fallback (exact reference semantics) --------------- */

static void lit_smooth(double *u, const double *f, double *ut, int64_t off,
                       int64_t n, double h2, int nsweeps) {
    for (int s = 0; s < nsweeps; s++) {
        for (int64_t i = 0; i < n; i++)
            for (int64_t j = 0; j < n; j++)
                for (int64_t k = 0; k < n; k++) {
                    int64_t c = off + (i * n + j) * n + k;
                    ut[c] = smooth_cell(u, f, c, n * n, n, i, j, k, h2);
                }
        for (int64_t i = 0; i < n; i++)
            for (int64_t j = 0; j < n; j++)
                for (int64_t k = 0; k < n; k++) {
                    int64_t c = off + (i * n + j) * n + k;
                    u[c] = ut[c];
                }
    }
}

static void lit_residual(const double *u, const double *f, double *r, int64_t off,
                         int64_t n, double h2) {
    for (int64_t i = 0; i < n; i++)
        for (int64_t j = 0; j < n; j++)
            for (int64_t k = 0; k < n; k++) {
                int64_t c = off + (i * n + j) * n + k;
                r[c] = resid_cell(u, f, c, n * n, n, i, j, k, h2);
            }
}

static void lit_restrict(const double *r, double *f, int64_t off_f, int64_t off_c,
                         int64_t nc) {
    int64_t nf = 2 * nc;
    for (int64_t i = 0; i < nc; i++)
        for (int64_t j = 0; j < nc; j++)
            for (int64_t k = 0; k < nc; k++) {
                double s = 0.0;
                for (int di = 0; di < 2; di++)
                    for (int dj = 0; dj < 2; dj++)
                        for (int dk = 0; dk < 2; dk++) {
                            int64_t fi = 2 * i + di;
                            int64_t fj = 2 * j + dj;
                            int64_t fk = 2 * k + dk;
                            s = s + r[off_f + (fi * nf + fj) * nf + fk];
                        }
                f[off_c + (i * nc + j) * nc + k] = 0.125 * s;
            }
}

static void lit_prolong(double *u, int64_t off_f, int64_t off_c, int64_t nc) {
    int64_t nf = 2 * nc;
    for (int64_t i = 0; i < nf; i++)
        for (int64_t j = 0; j < nf; j++)
            for (int64_t k = 0; k < nf; k++) {
                int64_t pi = i / 2, pj = j / 2, pk = k / 2;
                int64_t si = 2 * (i % 2) - 1;
                int64_t sj = 2 * (j % 2) - 1;
                int64_t sk = 2 * (k % 2) - 1;
                double acc = 0.0;
                for (int di = 0; di < 2; di++)
                    for (int dj = 0; dj < 2; dj++)
                        for (int dk = 0; dk < 2; dk++) {
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
                            if (di == 0) w = w * 3.0;
                            if (dj == 0) w = w * 3.0;
                            if (dk == 0) w = w * 3.0;
                            acc = acc + w * u[off_c + (ci * nc + cj) * nc + ck];
                        }
                u[off_f + (i * nf + j) * nf + k] += acc / 64.0;
            }
}

static void lit_vcycle(const double *f, double *u, int64_t N, int64_t ncycles) {
    int64_t flat = (8 * N * N * N) / 7 + 64;
    double *uh = (double *)malloc((size_t)flat * sizeof(double));
    double *fh = (double *)malloc((size_t)flat * sizeof(double));
    double *rh = (double *)malloc((size_t)flat * sizeof(double));
    double *th = (double *)malloc((size_t)flat * sizeof(double));
    int64_t ns[LMAX], offs[LMAX];
    int nlevels = 1;
    int64_t edge = N;
    while (edge > NCOARSE) { edge = edge / 2; nlevels = nlevels + 1; }
    int64_t pos = 0;
    edge = N;
    for (int lv = 0; lv < nlevels; lv++) {
        ns[lv] = edge; offs[lv] = pos;
        pos += edge * edge * edge;
        edge = edge / 2;
    }
    for (int64_t i = 0; i < N * N * N; i++) { fh[i] = f[i]; u[i] = 0.0; }
    for (int c = 0; c < ncycles; c++) {
        for (int64_t i = 0; i < N * N * N; i++) uh[i] = u[i];
        for (int lv = 0; lv < nlevels - 1; lv++) {
            int64_t n = ns[lv];
            double h2 = 1.0 / ((double)n * (double)n);
            lit_smooth(uh, fh, th, offs[lv], n, h2, NU1);
            lit_residual(uh, fh, rh, offs[lv], n, h2);
            int64_t nc = ns[lv + 1];
            lit_restrict(rh, fh, offs[lv], offs[lv + 1], nc);
            int64_t base = offs[lv + 1];
            for (int64_t i = 0; i < nc * nc * nc; i++) uh[base + i] = 0.0;
        }
        int nl = nlevels - 1;
        int64_t n = ns[nl];
        double h2 = 1.0 / ((double)n * (double)n);
        lit_smooth(uh, fh, th, offs[nl], n, h2, NCOARSE_SWEEPS);
        for (int lu = 0; lu < nlevels - 1; lu++) {
            int up = nlevels - 2 - lu;
            int64_t nc = ns[up + 1];
            lit_prolong(uh, offs[up], offs[up + 1], nc);
            int64_t n2 = ns[up];
            double h22 = 1.0 / ((double)n2 * (double)n2);
            lit_smooth(uh, fh, th, offs[up], n2, h22, NU2);
        }
        for (int64_t i = 0; i < N * N * N; i++) u[i] = uh[i];
    }
    free(uh); free(fh); free(rh); free(th);
}

/* ---------------- main driver ---------------------------------------------- */

static void zero_rows(double *restrict p, int64_t x0, int64_t x1) {
    for (int64_t x = x0; x < x1; x++) p[x] = 0.0;
}

static void zero_block(double *restrict p, int64_t cnt) {
    const int k = nthr_for(cnt, 131072);
    if (k >= 2) {
#pragma omp parallel for schedule(static) num_threads(k)
        for (int64_t x = 0; x < cnt; x++) zero_rows(p, x, x + 1);
    } else {
        zero_rows(p, 0, cnt);
    }
}

void mg_vcycle_fp64(const double *restrict f, double *restrict u, const int64_t N,
                    const int64_t ncycles, uint8_t *restrict workspace,
                    const int64_t workspace_size) {
    if (N < 1) return;
    g_T = omp_get_max_threads();
    if (g_T < 1) g_T = 1;
    int64_t ns[LMAX], offs[LMAX];
    int nlevels = 1;
    int64_t edge = N;
    while (edge > NCOARSE) { edge = edge / 2; nlevels = nlevels + 1; }
    int64_t pos = 0;
    edge = N;
    for (int lv = 0; lv < nlevels; lv++) {
        ns[lv] = edge; offs[lv] = pos;
        pos += edge * edge * edge;
        edge = edge / 2;
    }
    int clean = 1;
    for (int lv = 0; lv + 1 < nlevels; lv++)
        if (ns[lv] != 2 * ns[lv + 1]) { clean = 0; break; }
    if (!clean || ncycles <= 0) {
        lit_vcycle(f, u, N, ncycles);
        return;
    }

    const int64_t flat = (8 * N * N * N) / 7 + 64;
    const size_t chunk = (((size_t)flat * sizeof(double)) + 63u) & ~(size_t)63u;
    const size_t need = 4u * chunk;
    double *b[4];
    uintptr_t wp = ((uintptr_t)workspace + 63u) & ~(uintptr_t)63u;
    int64_t avail = workspace ? (workspace_size - (int64_t)(wp - (uintptr_t)workspace))
                              : -1;
    if (avail >= (int64_t)need) {
        for (int i = 0; i < 4; i++) b[i] = (double *)(wp + (size_t)i * chunk);
    } else {
        for (int i = 0; i < 4; i++) b[i] = (double *)aligned_alloc(64, chunk);
    }
    double *A = b[0];   /* iterate buffer 1 */
    double *Fh = b[1];  /* coarse right-hand sides */
    double *R = b[2];   /* residual */
    double *T = b[3];   /* iterate buffer 2 */

    double *Ucur[LMAX];
    for (int lv = 0; lv < nlevels; lv++) Ucur[lv] = A + offs[lv];
    zero_block(A, N * N * N);

    for (int cyc = 0; cyc < ncycles; cyc++) {
        /* down: smooth, residual, restrict */
        for (int lv = 0; lv < nlevels - 1; lv++) {
            const int64_t n = ns[lv];
            const double h2 = 1.0 / ((double)n * (double)n);
            const double *fr = (lv == 0) ? f : Fh + offs[lv];
            double *other = (Ucur[lv] == A + offs[lv]) ? T + offs[lv] : A + offs[lv];
            Ucur[lv] = smooth_pp(Ucur[lv], other, fr, n, h2, NU1);
            if (n >= 128) {
                fused_rr_pass(Fh + offs[lv + 1], Ucur[lv], fr, 0, 0,
                              ns[lv + 1], h2);
            } else {
                residual_pass(R + offs[lv], Ucur[lv], fr, n, h2);
                restrict_pass(Fh + offs[lv + 1], R + offs[lv], 0, 0, ns[lv + 1]);
            }
            {
                const int64_t nc2 = ns[lv + 1];
                zero_block(Ucur[lv + 1], nc2 * nc2 * nc2);
            }
        }
        /* coarsest solve (initial guess zero) */
        {
            const int nl = nlevels - 1;
            const int64_t n = ns[nl];
            const double h2 = 1.0 / ((double)n * (double)n);
            const double *fr = (nl == 0) ? f : Fh + offs[nl];
            double *other = (Ucur[nl] == A + offs[nl]) ? T + offs[nl] : A + offs[nl];
            Ucur[nl] = smooth_pp(Ucur[nl], other, fr, n, h2, NCOARSE_SWEEPS);
        }
        /* up: prolongate the correction, post-smooth */
        for (int lu = 0; lu < nlevels - 1; lu++) {
            const int up = nlevels - 2 - lu;
            const int64_t nf = ns[up];
            prolong_fused(Ucur[up], Ucur[up + 1], nf);
            const double h2 = 1.0 / ((double)nf * (double)nf);
            const double *fr = (up == 0) ? f : Fh + offs[up];
            double *other = (Ucur[up] == A + offs[up]) ? T + offs[up] : A + offs[up];
            Ucur[up] = smooth_pp(Ucur[up], other, fr, nf, h2, NU2);
        }
    }
    memcpy(u, Ucur[0], (size_t)(N * N * N) * sizeof(double));
    if (avail < (int64_t)need)
        for (int i = 0; i < 4; i++) free(b[i]);
}
