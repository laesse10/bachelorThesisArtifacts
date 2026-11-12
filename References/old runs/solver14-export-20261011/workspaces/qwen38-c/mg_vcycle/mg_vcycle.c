/* Geometric multigrid V-cycle, optimized C port of mg_vcycle_numpy.py.
 *
 * Differences from the reference that do not change the results:
 *  - The working solution lives in the workspace across cycles (the reference
 *    copies u -> uh at the start and uh -> u at the end of every cycle; the
 *    only cross-cycle state is the finest-level solution).
 *  - Smoothing sweeps alternate between two buffers (pointer swap) instead of
 *    a ut -> u copy after every sweep.
 *  - A level that is zero-initialized (every coarse level, once per cycle) gets
 *    its first sweep as ut = OMEGA*f*h2/d with the u-read and the explicit
 *    zero-fill elided (both are exactly zero).
 *  - Division by h2 becomes multiplication by 1/h2 (bit-identical: for every
 *    grid edge n that can be generated, h2 = 1/n^2 is an exact power of two).
 *  - Division by the integer diagonal d becomes multiplication by 1/d (1 ulp).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define OMEGA (6.0 / 7.0)
#define NU1 3
#define NU2 3
#define NCOARSE_SWEEPS 40
#define NCOARSE_EDGE 4
#define LMAX 24

static inline long flat_size(long n) { return (8 * n * n * n) / 7 + 64; }

/* One damped-Jacobi sweep:
 *   ut[c] = u[c] + OMEGA * (f[c] - (d*u[c] - s)/h2) * h2 / d
 * with odd-reflection ghosts (missing face raises d by 1).
 */
static void smooth_sweep(const double *restrict u, const double *restrict f,
                         double *restrict ut, long n, double h2, double invh2,
                         int do_omp) {
  long n2 = n * n;
  if (do_omp) {
    #pragma omp for collapse(2) schedule(static)
    for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++) {
        long c0 = (i * n + j) * n;
        const double *uc = u + c0;
        const double *fc = f + c0;
        double *tc = ut + c0;
        if (i > 0 && i < n - 1 && j > 0 && j < n - 1) {
          const double *ujm = uc - n, *ujp = uc + n;
          const double *ujm2 = uc - n2, *ujp2 = uc + n2;
          { /* k = 0, missing k- neighbor -> d = 7 */
            double s = uc[1] + ujm[0] + ujp[0] + ujm2[0] + ujp2[0];
            double c = uc[0];
            double au = (7.0 * c - s) * invh2;
            tc[0] = c + OMEGA * (fc[0] - au) * h2 * (1.0 / 7.0);
          }
          for (long k = 1; k < n - 1; k++) { /* interior, d = 6 */
            double s = uc[k - 1] + uc[k + 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            double c = uc[k];
            double au = (6.0 * c - s) * invh2;
            tc[k] = c + OMEGA * (fc[k] - au) * h2 * (1.0 / 6.0);
          }
          { /* k = n-1, missing k+ neighbor -> d = 7 */
            long k = n - 1;
            double s = uc[k - 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            double c = uc[k];
            double au = (7.0 * c - s) * invh2;
            tc[k] = c + OMEGA * (fc[k] - au) * h2 * (1.0 / 7.0);
          }
        } else { /* boundary row: faithful reference logic */
          for (long k = 0; k < n; k++) {
            long c = c0 + k;
            double s = 0.0;
            double d = 6.0;
            if (i > 0) s = s + u[c - n2]; else d = d + 1.0;
            if (i < n - 1) s = s + u[c + n2]; else d = d + 1.0;
            if (j > 0) s = s + u[c - n]; else d = d + 1.0;
            if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
            if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
            if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
            double au = (d * u[c] - s) * invh2;
            tc[k] = u[c] + OMEGA * (fc[k] - au) * h2 * (1.0 / d);
          }
        }
      }
  } else {
    for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++) {
        long c0 = (i * n + j) * n;
        const double *uc = u + c0;
        const double *fc = f + c0;
        double *tc = ut + c0;
        if (i > 0 && i < n - 1 && j > 0 && j < n - 1) {
          const double *ujm = uc - n, *ujp = uc + n;
          const double *ujm2 = uc - n2, *ujp2 = uc + n2;
          {
            double s = uc[1] + ujm[0] + ujp[0] + ujm2[0] + ujp2[0];
            double c = uc[0];
            double au = (7.0 * c - s) * invh2;
            tc[0] = c + OMEGA * (fc[0] - au) * h2 * (1.0 / 7.0);
          }
          for (long k = 1; k < n - 1; k++) {
            double s = uc[k - 1] + uc[k + 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            double c = uc[k];
            double au = (6.0 * c - s) * invh2;
            tc[k] = c + OMEGA * (fc[k] - au) * h2 * (1.0 / 6.0);
          }
          {
            long k = n - 1;
            double s = uc[k - 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            double c = uc[k];
            double au = (7.0 * c - s) * invh2;
            tc[k] = c + OMEGA * (fc[k] - au) * h2 * (1.0 / 7.0);
          }
        } else {
          for (long k = 0; k < n; k++) {
            long c = c0 + k;
            double s = 0.0;
            double d = 6.0;
            if (i > 0) s = s + u[c - n2]; else d = d + 1.0;
            if (i < n - 1) s = s + u[c + n2]; else d = d + 1.0;
            if (j > 0) s = s + u[c - n]; else d = d + 1.0;
            if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
            if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
            if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
            double au = (d * u[c] - s) * invh2;
            tc[k] = u[c] + OMEGA * (fc[k] - au) * h2 * (1.0 / d);
          }
        }
      }
  }
}

/* First sweep on a level whose current solution is exactly zero:
 *   ut[c] = u[c] + OMEGA*(f[c] - 0)*h2/d = OMEGA*h2*f[c]/d
 */
static void smooth_sweep_zero(const double *restrict f, double *restrict ut,
                              long n, double h2, int do_omp) {
  if (do_omp) {
    #pragma omp for collapse(2) schedule(static)
    for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++) {
        long c0 = (i * n + j) * n;
        const double *fc = f + c0;
        double *tc = ut + c0;
        double db = 6.0 + (i == 0) + (i == n - 1) + (j == 0) + (j == n - 1);
        if (db == 6.0) {
          tc[0] = (OMEGA * (fc[0] * h2)) * (1.0 / 7.0);
          for (long k = 1; k < n - 1; k++)
            tc[k] = (OMEGA * (fc[k] * h2)) * (1.0 / 6.0);
          tc[n - 1] = (OMEGA * (fc[n - 1] * h2)) * (1.0 / 7.0);
        } else {
          for (long k = 0; k < n; k++) {
            double d = db + (k == 0) + (k == n - 1);
            tc[k] = (OMEGA * (fc[k] * h2)) * (1.0 / d);
          }
        }
      }
  } else {
    for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++) {
        long c0 = (i * n + j) * n;
        const double *fc = f + c0;
        double *tc = ut + c0;
        double db = 6.0 + (i == 0) + (i == n - 1) + (j == 0) + (j == n - 1);
        if (db == 6.0) {
          tc[0] = (OMEGA * (fc[0] * h2)) * (1.0 / 7.0);
          for (long k = 1; k < n - 1; k++)
            tc[k] = (OMEGA * (fc[k] * h2)) * (1.0 / 6.0);
          tc[n - 1] = (OMEGA * (fc[n - 1] * h2)) * (1.0 / 7.0);
        } else {
          for (long k = 0; k < n; k++) {
            double d = db + (k == 0) + (k == n - 1);
            tc[k] = (OMEGA * (fc[k] * h2)) * (1.0 / d);
          }
        }
      }
  }
}

/* r[c] = f[c] - (d*u[c] - s)/h2 on one level. */
static void residual(const double *restrict u, const double *restrict f,
                     double *restrict r, long n, double invh2, int do_omp) {
  long n2 = n * n;
  if (do_omp) {
    #pragma omp for collapse(2) schedule(static)
    for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++) {
        long c0 = (i * n + j) * n;
        const double *uc = u + c0;
        const double *fc = f + c0;
        double *tc = r + c0;
        if (i > 0 && i < n - 1 && j > 0 && j < n - 1) {
          const double *ujm = uc - n, *ujp = uc + n;
          const double *ujm2 = uc - n2, *ujp2 = uc + n2;
          {
            double s = uc[1] + ujm[0] + ujp[0] + ujm2[0] + ujp2[0];
            tc[0] = fc[0] - (7.0 * uc[0] - s) * invh2;
          }
          for (long k = 1; k < n - 1; k++) {
            double s = uc[k - 1] + uc[k + 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            tc[k] = fc[k] - (6.0 * uc[k] - s) * invh2;
          }
          {
            long k = n - 1;
            double s = uc[k - 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            tc[k] = fc[k] - (7.0 * uc[k] - s) * invh2;
          }
        } else {
          for (long k = 0; k < n; k++) {
            long c = c0 + k;
            double s = 0.0;
            double d = 6.0;
            if (i > 0) s = s + u[c - n2]; else d = d + 1.0;
            if (i < n - 1) s = s + u[c + n2]; else d = d + 1.0;
            if (j > 0) s = s + u[c - n]; else d = d + 1.0;
            if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
            if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
            if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
            tc[k] = fc[k] - (d * u[c] - s) * invh2;
          }
        }
      }
  } else {
    for (long i = 0; i < n; i++)
      for (long j = 0; j < n; j++) {
        long c0 = (i * n + j) * n;
        const double *uc = u + c0;
        const double *fc = f + c0;
        double *tc = r + c0;
        if (i > 0 && i < n - 1 && j > 0 && j < n - 1) {
          const double *ujm = uc - n, *ujp = uc + n;
          const double *ujm2 = uc - n2, *ujp2 = uc + n2;
          {
            double s = uc[1] + ujm[0] + ujp[0] + ujm2[0] + ujp2[0];
            tc[0] = fc[0] - (7.0 * uc[0] - s) * invh2;
          }
          for (long k = 1; k < n - 1; k++) {
            double s = uc[k - 1] + uc[k + 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            tc[k] = fc[k] - (6.0 * uc[k] - s) * invh2;
          }
          {
            long k = n - 1;
            double s = uc[k - 1] + ujm[k] + ujp[k] + ujm2[k] + ujp2[k];
            tc[k] = fc[k] - (7.0 * uc[k] - s) * invh2;
          }
        } else {
          for (long k = 0; k < n; k++) {
            long c = c0 + k;
            double s = 0.0;
            double d = 6.0;
            if (i > 0) s = s + u[c - n2]; else d = d + 1.0;
            if (i < n - 1) s = s + u[c + n2]; else d = d + 1.0;
            if (j > 0) s = s + u[c - n]; else d = d + 1.0;
            if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
            if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
            if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
            tc[k] = fc[k] - (d * u[c] - s) * invh2;
          }
        }
      }
  }
}

/* Full weighting: each coarse cell averages its 8 children. */
static void restrict_(const double *restrict r, double *restrict fc,
                      long nf, long nc, int do_omp) {
  long n2f = nf * nf;
  long n3c = nc * nc * nc;
  if (do_omp) {
    #pragma omp for schedule(static)
    for (long c = 0; c < n3c; c++) {
      long i = c / (nc * nc);
      long rem = c - i * (nc * nc);
      long j = rem / nc;
      long k = rem - j * nc;
      long base = ((2 * i) * nf + (2 * j)) * nf + (2 * k);
      double s = 0.0;
      for (long di = 0; di < 2; di++)
        for (long dj = 0; dj < 2; dj++)
          for (long dk = 0; dk < 2; dk++)
            s = s + r[base + di * n2f + dj * nf + dk];
      fc[c] = 0.125 * s;
    }
  } else {
    for (long c = 0; c < n3c; c++) {
      long i = c / (nc * nc);
      long rem = c - i * (nc * nc);
      long j = rem / nc;
      long k = rem - j * nc;
      long base = ((2 * i) * nf + (2 * j)) * nf + (2 * k);
      double s = 0.0;
      for (long di = 0; di < 2; di++)
        for (long dj = 0; dj < 2; dj++)
          for (long dk = 0; dk < 2; dk++)
            s = s + r[base + di * n2f + dj * nf + dk];
      fc[c] = 0.125 * s;
    }
  }
}

/* Trilinear interpolation of the coarse correction (27/9/3/1 over 64),
 * with odd-reflection at the boundary (sign flip). Added into u. */
static void prolong(double *restrict uf, const double *restrict uc,
                    long off_f, long off_c, long nc, int do_omp) {
  long nf = 2 * nc;
  long n2f = nf * nf;
  long n3f = n2f * nf;
  if (do_omp) {
    #pragma omp for schedule(static)
    for (long c = 0; c < n3f; c++) {
      long i = c / n2f;
      long rem = c - i * n2f;
      long j = rem / nf;
      long k = rem - j * nf;
      long pi = i / 2, pj = j / 2, pk = k / 2;
      long si = 2 * (i % 2) - 1, sj = 2 * (j % 2) - 1, sk = 2 * (k % 2) - 1;
      double acc = 0.0;
      for (long di = 0; di < 2; di++)
        for (long dj = 0; dj < 2; dj++)
          for (long dk = 0; dk < 2; dk++) {
            long ci = pi + di * si, cj = pj + dj * sj, ck = pk + dk * sk;
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
            acc = acc + w * uc[off_c + (ci * nc + cj) * nc + ck];
          }
      uf[off_f + c] += acc * (1.0 / 64.0);
    }
  } else {
    for (long c = 0; c < n3f; c++) {
      long i = c / n2f;
      long rem = c - i * n2f;
      long j = rem / nf;
      long k = rem - j * nf;
      long pi = i / 2, pj = j / 2, pk = k / 2;
      long si = 2 * (i % 2) - 1, sj = 2 * (j % 2) - 1, sk = 2 * (k % 2) - 1;
      double acc = 0.0;
      for (long di = 0; di < 2; di++)
        for (long dj = 0; dj < 2; dj++)
          for (long dk = 0; dk < 2; dk++) {
            long ci = pi + di * si, cj = pj + dj * sj, ck = pk + dk * sk;
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
            acc = acc + w * uc[off_c + (ci * nc + cj) * nc + ck];
          }
      uf[off_f + c] += acc * (1.0 / 64.0);
    }
  }
}

void mg_vcycle_fp64(const double *restrict f, double *restrict u,
                    const int64_t N, const int64_t ncycles,
                    uint8_t *restrict workspace,
                    const int64_t workspace_size) {
  (void)workspace_size;
  long n = (long)N;
  long flat = flat_size(n);
  long n3 = n * n * n;

  double *A = (double *)workspace;
  double *B = A + flat;
  double *F = B + flat;
  double *R = F + flat;

  long ns[LMAX], offs[LMAX];
  int nlevels = 1;
  long edge = n;
  while (edge > NCOARSE_EDGE) { edge /= 2; nlevels++; }
  long pos = 0;
  edge = n;
  for (int lv = 0; lv < nlevels; lv++) {
    ns[lv] = edge;
    offs[lv] = pos;
    pos += edge * edge * edge;
    edge /= 2;
  }

  #pragma omp parallel
  {
    /* Finest-level RHS scaling: not used (kept faithful), but the F buffer
     * holds the input f itself at the finest level. */
    #pragma omp for schedule(static)
    for (long i = 0; i < n3; i++) F[i] = f[i];

    /* u starts at zero, in buffer A. cur[lv] is the base (A or B) of the
     * buffer holding level lv's current solution. */
    memset(A, 0, n3 * sizeof(double));
    int cur[LMAX];
    for (int lv = 0; lv < nlevels; lv++) cur[lv] = 0;
    double *const BB[2] = {A, B};

    for (long cy = 0; cy < ncycles; cy++) {
      /* Down: smooth, take the residual, restrict it to the coarse RHS. */
      for (int lv = 0; lv < nlevels - 1; lv++) {
        long lvn = ns[lv];
        long off = offs[lv];
        long nc = ns[lv + 1];
        double h2 = 1.0 / (double)(lvn * lvn);
        double invh2 = 1.0 / h2;
        int do_omp = (lvn >= 16);
        const double *flv = F + off;
        int b = cur[lv];
        double *c = BB[b] + off, *a = BB[b ^ 1] + off;
        if (lv == 0) {
          for (int s = 0; s < NU1; s++) {
            smooth_sweep(c, flv, a, lvn, h2, invh2, do_omp);
            b = b ^ 1;
            c = BB[b] + off;
            a = BB[b ^ 1] + off;
          }
        } else {
          smooth_sweep_zero(flv, a, lvn, h2, do_omp);
          b = b ^ 1;
          for (int s = 1; s < NU1; s++) {
            c = BB[b] + off; a = BB[b ^ 1] + off;
            smooth_sweep(c, flv, a, lvn, h2, invh2, do_omp);
            b = b ^ 1;
          }
        }
        cur[lv] = b;
        /* Residual in R, restricted into F at the coarse level. */
        double *r = R + off;
        residual(BB[b] + off, flv, r, lvn, invh2, do_omp);
        restrict_(r, F + offs[lv + 1], lvn, nc, (nc >= 8));
        /* The coarse solution is zero for the next stage (elided: the first
         * pre-smoothing sweep below starts from zero). */
        cur[lv + 1] = 0;
      }

      /* Coarsest grid: smoothed to convergence. */
      {
        int nl = nlevels - 1;
        long lvn = ns[nl];
        long off = offs[nl];
        double h2 = 1.0 / (double)(lvn * lvn);
        double invh2 = 1.0 / h2;
        const double *flv = F + off;
        int b = 0;
        smooth_sweep_zero(flv, BB[1] + off, lvn, h2, 0);
        b = 1;
        for (int s = 1; s < NCOARSE_SWEEPS; s++) {
          smooth_sweep(BB[b] + off, flv, BB[b ^ 1] + off, lvn, h2, invh2, 0);
          b = b ^ 1;
        }
        cur[nl] = b;
      }

      /* Up: interpolate the correction, then post-smooth. */
      for (int lu = 0; lu < nlevels - 1; lu++) {
        int up = nlevels - 2 - lu;
        long nc = ns[up + 1];
        long nup = ns[up];
        long off = offs[up];
        double h2 = 1.0 / (double)(nup * nup);
        double invh2 = 1.0 / h2;
        int do_omp = (nup >= 16);
        const double *flv = F + off;
        int b = cur[up];
        double *c = BB[b] + off;
        prolong(BB[b], BB[cur[up + 1]], off, offs[up + 1], nc, do_omp);
        for (int s = 0; s < NU2; s++) {
          smooth_sweep(c, flv, BB[b ^ 1] + off, nup, h2, invh2, do_omp);
          b = b ^ 1;
          c = BB[b] + off;
        }
        cur[up] = b;
      }
    }

    /* Write the result back. */
    double *c0 = BB[cur[0]];
    #pragma omp for schedule(static)
    for (long i = 0; i < n3; i++) u[i] = c0[i];
  }
}
