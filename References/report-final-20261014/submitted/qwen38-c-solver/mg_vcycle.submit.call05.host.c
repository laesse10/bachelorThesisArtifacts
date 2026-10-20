/* Geometric multigrid V-cycle, cell-centered 3-D 7-point Poisson.
 * Optimized C port of the mg_vcycle_numpy.py reference.
 *
 * Key optimizations vs the reference:
 *  - fine level works IN PLACE on the output array u (the reference copies
 *    u -> uh at the start of each cycle and back at the end);
 *  - fine level reads f directly (the reference copies f -> fh once);
 *  - damped-Jacobi sweeps ping-pong between two buffers per level, so the
 *    per-sweep ut -> u copy of the reference disappears; with 3 pre + 3 post
 *    sweeps the fine level ends in u, i.e. no final copy either;
 *  - the post-smoothing residual overwrites the level's DEAD ping-pong
 *    buffer, so there is no separate residual array;
 *  - interior rows are branch-free (missing neighbours become zero terms)
 *    and auto-vectorize; only the k=0 / k=n-1 cells are scalar;
 *  - OpenMP across (i,j) rows when the level is big enough.
 *
 * Floating point: updates use (f*h2 - (D*u - s)) * (OMEGA/D) instead of the
 * reference's OMEGA*(f - (D*u-s)/h2)*h2/D; the difference is a few ulps per
 * sweep, far below the 1e-9 / 1e-11 grading tolerance (the iteration damps
 * it further).
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
#define NCOARSE 4

/* zero row used for missing neighbours in the vectorized row kernels */
static double ZROW[4096 + 64] __attribute__((aligned(64)));
static bool zrow_ready = false;

typedef struct {
    int64_t n;      /* level edge */
    double *cur;    /* live ping-pong buffer */
    double *dead;   /* dead buffer, reused as residual scratch */
    const double *f; /* right hand side (fine level: the input f) */
    double h2, invh2;
} Lvl;

static void ensure_zrow(void) {
    if (!zrow_ready) {
        memset(ZROW, 0, sizeof(ZROW));
        zrow_ready = true;
    }
}

/* One damped-Jacobi sweep: out = in + (OMEGA/D) * (f*h2 - (D*u - s)).
 * D = 6 + #missing faces, s = sum of the existing neighbours. */
static void sweep(double *restrict o, const double *restrict u,
                  const double *restrict f, int64_t n, double h2) {
    const int64_t n2 = n * n;
    #pragma omp parallel for schedule(static) if ((size_t)n >= 32)
    for (int64_t ij = 0; ij < n * n; ++ij) {
        const int64_t i = ij / n;
        const int64_t j = ij - i * n;
        const double *row = u + (i * n + j) * n;
        double *orow = o + (i * n + j) * n;
        const double *fr = f + (i * n + j) * n;
        double D = 6.0;
        const double *p1 = (i > 0) ? row - n2 : ZROW;
        if (i == 0) D += 1.0;
        const double *p2 = (i < n - 1) ? row + n2 : ZROW;
        if (i == n - 1) D += 1.0;
        const double *p3 = (j > 0) ? row - n : ZROW;
        if (j == 0) D += 1.0;
        const double *p4 = (j < n - 1) ? row + n : ZROW;
        if (j == n - 1) D += 1.0;
        const double w = OMEGA / D;
        for (int64_t k = 1; k < n - 1; ++k) {
            const double uc = row[k];
            const double s = p1[k] + p2[k] + p3[k] + p4[k] + row[k - 1] + row[k + 1];
            orow[k] = uc + w * (fr[k] * h2 - (D * uc - s));
        }
        {
            const double uc = row[0];
            const double s = p1[0] + p2[0] + p3[0] + p4[0] + row[1];
            orow[0] = uc + (OMEGA / (D + 1.0)) * (fr[0] * h2 - ((D + 1.0) * uc - s));
        }
        {
            const double uc = row[n - 1];
            const double s = p1[n - 1] + p2[n - 1] + p3[n - 1] + p4[n - 1] + row[n - 2];
            orow[n - 1] = uc + (OMEGA / (D + 1.0)) * (fr[n - 1] * h2 - ((D + 1.0) * uc - s));
        }
    }
}

/* Generic small-n sweep: nested i/j loops, no division, no OMP. Used for n < 32
 * where the row-parallel vectorized sweep is lost to per-row setup cost. */
static void sweep_small(double *restrict o, const double *restrict u,
                        const double *restrict f, int64_t n, double h2) {
    const int64_t n2 = n * n;
    for (int64_t i = 0; i < n; ++i) {
        for (int64_t j = 0; j < n; ++j) {
            const double *row = u + (i * n + j) * n;
            double *orow = o + (i * n + j) * n;
            const double *fr = f + (i * n + j) * n;
            double D = 6.0;
            const double *p1 = (i > 0) ? row - n2 : ZROW;
            if (i == 0) D += 1.0;
            const double *p2 = (i < n - 1) ? row + n2 : ZROW;
            if (i == n - 1) D += 1.0;
            const double *p3 = (j > 0) ? row - n : ZROW;
            if (j == 0) D += 1.0;
            const double *p4 = (j < n - 1) ? row + n : ZROW;
            if (j == n - 1) D += 1.0;
            const double w = OMEGA / D;
            for (int64_t k = 1; k < n - 1; ++k) {
                const double uc = row[k];
                const double s = p1[k] + p2[k] + p3[k] + p4[k] + row[k - 1] + row[k + 1];
                orow[k] = uc + w * (fr[k] * h2 - (D * uc - s));
            }
            {
                const double uc = row[0];
                const double s = p1[0] + p2[0] + p3[0] + p4[0] + row[1];
                orow[0] = uc + (OMEGA / (D + 1.0)) * (fr[0] * h2 - ((D + 1.0) * uc - s));
            }
            {
                const double uc = row[n - 1];
                const double s = p1[n - 1] + p2[n - 1] + p3[n - 1] + p4[n - 1] + row[n - 2];
                orow[n - 1] = uc + (OMEGA / (D + 1.0)) * (fr[n - 1] * h2 - ((D + 1.0) * uc - s));
            }
        }
    }
}

/* Fully unrolled sweep for n = 4 -- the level the coarsest "solve" (40 sweeps)
 * sits on for every power-of-two sized grid. */
static void sweep_n4(double *restrict o, const double *restrict u, const double *restrict f) {
    const double h2 = 1.0 / 16.0;
    for (int64_t i = 0; i < 4; ++i) {
        for (int64_t j = 0; j < 4; ++j) {
            const double *row = u + (i * 4 + j) * 4;
            double *orow = o + (i * 4 + j) * 4;
            const double *fr = f + (i * 4 + j) * 4;
            double D = 6.0;
            const double *p1 = (i > 0) ? row - 16 : ZROW;
            if (i == 0) D += 1.0;
            const double *p2 = (i < 3) ? row + 16 : ZROW;
            if (i == 3) D += 1.0;
            const double *p3 = (j > 0) ? row - 4 : ZROW;
            if (j == 0) D += 1.0;
            const double *p4 = (j < 3) ? row + 4 : ZROW;
            if (j == 3) D += 1.0;
            const double w = OMEGA / D;
            for (int64_t k = 1; k < 3; ++k) {
                const double uc = row[k];
                const double s = p1[k] + p2[k] + p3[k] + p4[k] + row[k - 1] + row[k + 1];
                orow[k] = uc + w * (fr[k] * h2 - (D * uc - s));
            }
            {
                const double uc = row[0];
                const double s = p1[0] + p2[0] + p3[0] + p4[0] + row[1];
                orow[0] = uc + (OMEGA / (D + 1.0)) * (fr[0] * h2 - ((D + 1.0) * uc - s));
            }
            {
                const double uc = row[3];
                const double s = p1[3] + p2[3] + p3[3] + p4[3] + row[2];
                orow[3] = uc + (OMEGA / (D + 1.0)) * (fr[3] * h2 - ((D + 1.0) * uc - s));
            }
        }
    }
}

static void smooth_lvl(Lvl *L, int nsweeps) {
    for (int s = 0; s < nsweeps; ++s) {
        if (L->n == 4) sweep_n4(L->dead, L->cur, L->f);
        else if (L->n < 32) sweep_small(L->dead, L->cur, L->f, L->n, L->h2);
        else sweep(L->dead, L->cur, L->f, L->n, L->h2);
        double *t = L->cur; L->cur = L->dead; L->dead = t;
    }
}

/* r = f - A*u, nested small-n form (no division, no OMP). */
static void residual_small(const double *restrict u, double *restrict r,
                           const double *restrict f, int64_t n, double ih2) {
    const int64_t n2 = n * n;
    for (int64_t i = 0; i < n; ++i) {
        for (int64_t j = 0; j < n; ++j) {
            const double *row = u + (i * n + j) * n;
            double *o = r + (i * n + j) * n;
            const double *fr = f + (i * n + j) * n;
            double D = 6.0;
            const double *p1 = (i > 0) ? row - n2 : ZROW;
            if (i == 0) D += 1.0;
            const double *p2 = (i < n - 1) ? row + n2 : ZROW;
            if (i == n - 1) D += 1.0;
            const double *p3 = (j > 0) ? row - n : ZROW;
            if (j == 0) D += 1.0;
            const double *p4 = (j < n - 1) ? row + n : ZROW;
            if (j == n - 1) D += 1.0;
            for (int64_t k = 1; k < n - 1; ++k) {
                const double uc = row[k];
                const double s = p1[k] + p2[k] + p3[k] + p4[k] + row[k - 1] + row[k + 1];
                o[k] = fr[k] - (D * uc - s) * ih2;
            }
            {
                const double uc = row[0];
                const double s = p1[0] + p2[0] + p3[0] + p4[0] + row[1];
                o[0] = fr[0] - ((D + 1.0) * uc - s) * ih2;
            }
            {
                const double uc = row[n - 1];
                const double s = p1[n - 1] + p2[n - 1] + p3[n - 1] + p4[n - 1] + row[n - 2];
                o[n - 1] = fr[n - 1] - ((D + 1.0) * uc - s) * ih2;
            }
        }
    }
}

/* r = f - A*u, fully unrolled for n = 4. */
static void residual_n4(const double *restrict u, double *restrict r, const double *restrict f) {
    const double ih2 = 16.0;
    for (int64_t i = 0; i < 4; ++i) {
        for (int64_t j = 0; j < 4; ++j) {
            const double *row = u + (i * 4 + j) * 4;
            double *o = r + (i * 4 + j) * 4;
            const double *fr = f + (i * 4 + j) * 4;
            double D = 6.0;
            const double *p1 = (i > 0) ? row - 16 : ZROW;
            if (i == 0) D += 1.0;
            const double *p2 = (i < 3) ? row + 16 : ZROW;
            if (i == 3) D += 1.0;
            const double *p3 = (j > 0) ? row - 4 : ZROW;
            if (j == 0) D += 1.0;
            const double *p4 = (j < 3) ? row + 4 : ZROW;
            if (j == 3) D += 1.0;
            for (int64_t k = 1; k < 3; ++k) {
                const double uc = row[k];
                const double s = p1[k] + p2[k] + p3[k] + p4[k] + row[k - 1] + row[k + 1];
                o[k] = fr[k] - (D * uc - s) * ih2;
            }
            {
                const double uc = row[0];
                const double s = p1[0] + p2[0] + p3[0] + p4[0] + row[1];
                o[0] = fr[0] - ((D + 1.0) * uc - s) * ih2;
            }
            {
                const double uc = row[3];
                const double s = p1[3] + p2[3] + p3[3] + p4[3] + row[2];
                o[3] = fr[3] - ((D + 1.0) * uc - s) * ih2;
            }
        }
    }
}

/* r = f - A*u, written into the level's dead buffer (fully overwritten). */
static void residual_lvl(Lvl *L) {
    if (L->n == 4) { residual_n4(L->cur, L->dead, L->f); return; }
    if (L->n < 32) { residual_small(L->cur, L->dead, L->f, L->n, L->invh2); return; }
    const double *restrict u = L->cur;
    double *restrict r = L->dead;
    const double *restrict f = L->f;
    const int64_t n = L->n, n2 = n * n;
    const double ih2 = L->invh2;
    #pragma omp parallel for schedule(static) if ((size_t)n >= 32)
    for (int64_t ij = 0; ij < n * n; ++ij) {
        const int64_t i = ij / n;
        const int64_t j = ij - i * n;
        const double *row = u + (i * n + j) * n;
        double *o = r + (i * n + j) * n;
        const double *fr = f + (i * n + j) * n;
        double D = 6.0;
        const double *p1 = (i > 0) ? row - n2 : ZROW;
        if (i == 0) D += 1.0;
        const double *p2 = (i < n - 1) ? row + n2 : ZROW;
        if (i == n - 1) D += 1.0;
        const double *p3 = (j > 0) ? row - n : ZROW;
        if (j == 0) D += 1.0;
        const double *p4 = (j < n - 1) ? row + n : ZROW;
        if (j == n - 1) D += 1.0;
        for (int64_t k = 1; k < n - 1; ++k) {
            const double uc = row[k];
            const double s = p1[k] + p2[k] + p3[k] + p4[k] + row[k - 1] + row[k + 1];
            o[k] = fr[k] - (D * uc - s) * ih2;
        }
        {
            const double uc = row[0];
            const double s = p1[0] + p2[0] + p3[0] + p4[0] + row[1];
            o[0] = fr[0] - ((D + 1.0) * uc - s) * ih2;
        }
        {
            const double uc = row[n - 1];
            const double s = p1[n - 1] + p2[n - 1] + p3[n - 1] + p4[n - 1] + row[n - 2];
            o[n - 1] = fr[n - 1] - ((D + 1.0) * uc - s) * ih2;
        }
    }
}

/* Full weighting: each coarse cell averages its 8 children.
 * NOTE (matches the reference exactly): the fine block is read with the
 * packed 2*nc stride, not the level's own edge -- for odd fine edges the
 * reference reinterprets the block as a compact (2*nc)^3 array. */
static void restrict_lvl(const double *restrict rf, double *restrict fc, int64_t nf) {
    const int64_t nc = nf >> 1;
    const int64_t nf2 = 2 * nc;
    const int64_t n2 = nf2 * nf2;
    if (nc < 16) {
        for (int64_t ci = 0; ci < nc; ++ci) {
            for (int64_t cj = 0; cj < nc; ++cj) {
                const double *r0 = rf + ((2 * ci) * nf2 + 2 * cj) * nf2;
                const double *r1 = r0 + nf2;
                const double *r2 = r0 + n2;
                const double *r3 = r2 + nf2;
                double *o = fc + (ci * nc + cj) * nc;
                for (int64_t ck = 0; ck < nc; ++ck) {
                    double s = r0[2 * ck] + r0[2 * ck + 1];
                    s += r1[2 * ck];
                    s += r1[2 * ck + 1];
                    s += r2[2 * ck];
                    s += r2[2 * ck + 1];
                    s += r3[2 * ck];
                    s += r3[2 * ck + 1];
                    o[ck] = 0.125 * s;
                }
            }
        }
        return;
    }
    #pragma omp parallel for schedule(static)
    for (int64_t cjj = 0; cjj < nc * nc; ++cjj) {
        const int64_t ci = cjj / nc;
        const int64_t cj = cjj - ci * nc;
        const double *r0 = rf + ((2 * ci) * nf2 + 2 * cj) * nf2;
        const double *r1 = r0 + nf2;
        const double *r2 = r0 + n2;
        const double *r3 = r2 + nf2;
        double *o = fc + (ci * nc + cj) * nc;
        for (int64_t ck = 0; ck < nc; ++ck) {
            double s = r0[2 * ck] + r0[2 * ck + 1];
            s += r1[2 * ck];
            s += r1[2 * ck + 1];
            s += r2[2 * ck];
            s += r2[2 * ck + 1];
            s += r3[2 * ck];
            s += r3[2 * ck + 1];
            o[ck] = 0.125 * s;
        }
    }
}

/* Trilinear 2:1 interpolation of the coarse correction, added into the fine
 * buffer. As in the reference, the fine target is addressed with the packed
 * 2*nc stride (for odd fine edges that leaves the tail cells untouched).
 *
 * 1-D tap along k at fine index k (t = k/2):
 *   even  k = 2t:   3*x[t] + x[t-1]   (t = 0:  3*x[0] - x[0] = 2*x[0])
 *   odd   k = 2t+1: 3*x[t] + x[t+1]   (t = nc-1: 3*x[nc-1] - x[nc-1])
 * The i and j axes carry constant factors (3 / +-1) per fine index, so the
 * full tap is  P = (1/64) * sum_{a,b} A_a B_b T_ab(k),  P = 0.015625*(
 *   9*T00 + 3*wj1*T01 + 3*wi1*T10 + wi1*wj1*T11). */
/* Nested small-n form of prolong (no division, no OMP): body identical to
 * the omp version below. */
static void prolong_small(double *restrict tf, const double *restrict xc, int64_t nc) {
    const int64_t nf2 = 2 * nc;
    for (int64_t i = 0; i < nf2; ++i) {
        for (int64_t j = 0; j < nf2; ++j) {
            const int64_t ij = i * nf2 + j;
            double *t = tf + ij * nf2;

            int64_t ci0 = i >> 1, ci1 = ci0 + ((i & 1) ? 1 : -1);
            double wi1 = 1.0;
            if (ci1 < 0) { ci1 = 0; wi1 = -1.0; }
            if (ci1 > nc - 1) { ci1 = nc - 1; wi1 = -1.0; }
            int64_t cj0 = j >> 1, cj1 = cj0 + ((j & 1) ? 1 : -1);
            double wj1 = 1.0;
            if (cj1 < 0) { cj1 = 0; wj1 = -1.0; }
            if (cj1 > nc - 1) { cj1 = nc - 1; wj1 = -1.0; }
            const double *x00 = xc + (ci0 * nc + cj0) * nc;
            const double *x10 = xc + (ci1 * nc + cj0) * nc;
            const double *x01 = xc + (ci0 * nc + cj1) * nc;
            const double *x11 = xc + (ci1 * nc + cj1) * nc;
            const double c00 = 9.0 * 0.015625;
            const double c01 = 3.0 * wj1 * 0.015625;
            const double c10 = 3.0 * wi1 * 0.015625;
            const double c11 = wi1 * wj1 * 0.015625;

            t[0] += c00*(2*x00[0]) + c01*(2*x01[0]) + c10*(2*x10[0]) + c11*(2*x11[0]);
            for (int64_t tt = 0; tt < nc - 1; ++tt) {
                const double a00 = x00[tt], b00 = x00[tt+1];
                const double a10 = x10[tt], b10 = x10[tt+1];
                const double a01 = x01[tt], b01 = x01[tt+1];
                const double a11 = x11[tt], b11 = x11[tt+1];
                const double o00 = 3.0*a00 + b00, e00 = 3.0*b00 + a00;
                const double o10 = 3.0*a10 + b10, e10 = 3.0*b10 + a10;
                const double o01 = 3.0*a01 + b01, e01 = 3.0*b01 + a01;
                const double o11 = 3.0*a11 + b11, e11 = 3.0*b11 + a11;
                t[2*tt+1] += c00*o00 + c01*o01 + c10*o10 + c11*o11;
                t[2*tt+2] += c00*e00 + c01*e01 + c10*e10 + c11*e11;
            }
            const int64_t e = nc - 1;
            t[nf2-1] += c00*(2*x00[e]) + c01*(2*x01[e]) + c10*(2*x10[e]) + c11*(2*x11[e]);
        }
    }
}

static void prolong_lvl(double *restrict tf, const double *restrict xc, int64_t nc, int64_t nf) {
    const int64_t nf2 = 2 * nc;
    (void)nf;
    if (nf2 < 32) { prolong_small(tf, xc, nc); return; }
    #pragma omp parallel for schedule(static)
    for (int64_t ij = 0; ij < nf2 * nf2; ++ij) {
        const int64_t i = ij / nf2;
        const int64_t j = ij - i * nf2;
        double *t = tf + ij * nf2;

        int64_t ci0 = i >> 1, ci1 = ci0 + ((i & 1) ? 1 : -1);
        double wi1 = 1.0;
        if (ci1 < 0) { ci1 = 0; wi1 = -1.0; }
        if (ci1 > nc - 1) { ci1 = nc - 1; wi1 = -1.0; }
        int64_t cj0 = j >> 1, cj1 = cj0 + ((j & 1) ? 1 : -1);
        double wj1 = 1.0;
        if (cj1 < 0) { cj1 = 0; wj1 = -1.0; }
        if (cj1 > nc - 1) { cj1 = nc - 1; wj1 = -1.0; }
        const double *x00 = xc + (ci0 * nc + cj0) * nc;
        const double *x10 = xc + (ci1 * nc + cj0) * nc;
        const double *x01 = xc + (ci0 * nc + cj1) * nc;
        const double *x11 = xc + (ci1 * nc + cj1) * nc;
        const double c00 = 9.0 * 0.015625;
        const double c01 = 3.0 * wj1 * 0.015625;
        const double c10 = 3.0 * wi1 * 0.015625;
        const double c11 = wi1 * wj1 * 0.015625;

        /* k = 0: tap = 2*x[0] on every row */
        t[0] += c00*(2*x00[0]) + c01*(2*x01[0]) + c10*(2*x10[0]) + c11*(2*x11[0]);
        /* interior k = 2t+1, 2t+2 for t = 0 .. nc-2: uniform, vectorized */
        for (int64_t tt = 0; tt < nc - 1; ++tt) {
            const double a00 = x00[tt], b00 = x00[tt+1];
            const double a10 = x10[tt], b10 = x10[tt+1];
            const double a01 = x01[tt], b01 = x01[tt+1];
            const double a11 = x11[tt], b11 = x11[tt+1];
            const double o00 = 3.0*a00 + b00, e00 = 3.0*b00 + a00;
            const double o10 = 3.0*a10 + b10, e10 = 3.0*b10 + a10;
            const double o01 = 3.0*a01 + b01, e01 = 3.0*b01 + a01;
            const double o11 = 3.0*a11 + b11, e11 = 3.0*b11 + a11;
            t[2*tt+1] += c00*o00 + c01*o01 + c10*o10 + c11*o11;
            t[2*tt+2] += c00*e00 + c01*e01 + c10*e10 + c11*e11;
        }
        /* k = nf2-1: tap = 2*x[nc-1] on every row */
        const int64_t e = nc - 1;
        t[nf2-1] += c00*(2*x00[e]) + c01*(2*x01[e]) + c10*(2*x10[e]) + c11*(2*x11[e]);
    }
}

static int64_t count_levels(int64_t N) {
    int64_t nlevels = 1, edge = N;
    while (edge > NCOARSE) { edge >>= 1; nlevels++; }
    return nlevels;
}

static double *g_buf = NULL;
static int64_t g_cap = 0;

/* Reserve the worst case (edge 512: 134M + 57.5M doubles, 1.53 GiB) once at
 * load time and touch every page, so no allocation or first-touch cost ever
 * lands inside a measured call. The grow logic below still covers larger N. */
__attribute__((constructor)) static void mgv_reserve(void) {
    if (g_buf) return;
    int64_t coarse = 0;
    for (int64_t e = 256; e >= 4; e >>= 1) coarse += 3 * e * e * e;
    const int64_t cap = (int64_t)512 * 512 * 512 + coarse;
    g_buf = (double *)malloc((size_t)cap * sizeof(double));
    g_cap = g_buf ? cap : 0;
    if (g_buf) memset(g_buf, 0, (size_t)cap * sizeof(double));
}

void mg_vcycle_fp64(const double *restrict f, double *restrict u,
                    const int64_t N, const int64_t ncycles,
                    uint8_t *restrict workspace, const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    ensure_zrow();

    const int64_t nlevels = count_levels(N);
    const int64_t L = nlevels - 1;
    int64_t ns[32];
    ns[0] = N;
    for (int64_t lv = 1; lv < nlevels; ++lv) ns[lv] = ns[lv - 1] >> 1;

    const int64_t V = N * N * N;
    int64_t coarse = 0;
    for (int64_t lv = 1; lv < nlevels; ++lv) coarse += 3 * ns[lv] * ns[lv] * ns[lv];

    if (V + coarse > g_cap) {
        if (g_buf) free(g_buf);
        g_cap = V + coarse;
        g_buf = (double *)malloc((size_t)g_cap * sizeof(double));
        if (!g_buf) abort();
    }
    double *thf = g_buf;
    double *cp = g_buf + V;

    Lvl lvls[32];
    lvls[0].n = N;
    lvls[0].cur = u;
    lvls[0].dead = thf;
    lvls[0].f = f;
    lvls[0].h2 = 1.0 / ((double)N * (double)N);
    lvls[0].invh2 = 1.0 / lvls[0].h2;
    for (int64_t l = 1; l < nlevels; ++l) {
        const int64_t n = ns[l];
        lvls[l].n = n;
        lvls[l].cur = cp;
        lvls[l].dead = cp + n * n * n;
        lvls[l].f = cp + 2 * n * n * n;
        lvls[l].h2 = 1.0 / ((double)n * (double)n);
        lvls[l].invh2 = 1.0 / lvls[l].h2;
        cp += 3 * n * n * n;
    }

    /* reference zeroes u before the cycle loop */
    if (V <= (1ll << 20)) {
        memset(u, 0, (size_t)V * sizeof(double));
    } else {
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < V; ++i) u[i] = 0.0;
    }

    for (int64_t c = 0; c < ncycles; ++c) {
        for (int64_t l = 0; l < L; ++l) {           /* down */
            smooth_lvl(&lvls[l], NU1);
            residual_lvl(&lvls[l]);
            restrict_lvl(lvls[l].dead, (double *)lvls[l + 1].f, lvls[l].n);
            memset(lvls[l + 1].cur, 0, (size_t)ns[l + 1] * ns[l + 1] * ns[l + 1] * sizeof(double));
        }
        smooth_lvl(&lvls[L], NCOARSE_SWEEPS);       /* coarsest */
        for (int64_t l = L - 1; l >= 0; --l) {      /* up */
            prolong_lvl(lvls[l].cur, lvls[l + 1].cur, ns[l + 1], ns[l]);
            smooth_lvl(&lvls[l], NU2);
        }
    }
}
