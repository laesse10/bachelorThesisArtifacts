/* Geometric multigrid V-cycle on a cell-centered 3-D grid.
 * Optimized C port of the NumPy reference:
 *   - damped-Jacobi sweeps ping-pong between two buffers (no copy-back pass)
 *   - the first sweep after a zeroed level is specialized (u == 0)
 *   - one flat buffer per field with a per-level offset table
 *   - one omp parallel region per call; all passes are omp for over i-planes
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LMAX  24
#define NCOARSE 4
#define OMEGA  (6.0 / 7.0)
#define NCOARSE_SWEEPS 40
#ifndef PAR_MIN_N
#define PAR_MIN_N 32      /* smallest edge that runs the OpenMP path */
#endif

static double g_zero_row[8192];  /* .bss: stays zero, read-only */

/* one damped Jacobi sweep over i-planes [i0,i1):
   v[c] = (1-w) u[c] + (w/d) s + (w h2/d) f[c] */
static void sweep_planes(const double *restrict u, const double *restrict fv,
                         double *restrict v, const int64_t n, const double h2,
                         const double *restrict zrow,
                         const int64_t i0, const int64_t i1)
{
    const int64_t nn = n * n;
    const double om = OMEGA;
    const double a = 1.0 - om;
    for (int64_t i = i0; i < i1; i++) {
        const int64_t ib = i * nn;
        const int dbi = (i > 0 ? 0 : 1) + (i < n - 1 ? 0 : 1);
        for (int64_t j = 0; j < n; j++) {
            const int64_t c0 = ib + j * n;
            const double *rm = (i > 0)     ? u + c0 - nn : zrow;
            const double *rp = (i < n - 1) ? u + c0 + nn : zrow;
            const double *qm = (j > 0)     ? u + c0 - n : zrow;
            const double *qp = (j < n - 1) ? u + c0 + n : zrow;
            const double *ur = u + c0;
            const double *fr = fv + c0;
            double *vr = v + c0;
            const int d = 6 + dbi + (j > 0 ? 0 : 1) + (j < n - 1 ? 0 : 1);
            const double b  = om / (double)d;
            const double cf = om * h2 / (double)d;
            const double b1 = om / (double)(d + 1);
            const double cf1 = om * h2 / (double)(d + 1);
            double s;
            s = rm[0] + rp[0] + qm[0] + qp[0] + ur[1];
            vr[0] = a * ur[0] + b1 * s + cf1 * fr[0];
            for (int64_t k = 1; k < n - 1; k++) {
                s = rm[k] + rp[k] + qm[k] + qp[k] + ur[k - 1] + ur[k + 1];
                vr[k] = a * ur[k] + b * s + cf * fr[k];
            }
            s = rm[n - 1] + rp[n - 1] + qm[n - 1] + qp[n - 1] + ur[n - 2];
            vr[n - 1] = a * ur[n - 1] + b1 * s + cf1 * fr[n - 1];
        }
    }
}

/* first sweep on a level whose u is identically zero: v[c] = (w h2/d) f[c] */
static void sweep0_planes(const double *restrict fv, double *restrict v,
                          const int64_t n, const double h2,
                          const int64_t i0, const int64_t i1)
{
    const int64_t nn = n * n;
    const double om = OMEGA;
    for (int64_t i = i0; i < i1; i++) {
        const int64_t ib = i * nn;
        const int dbi = (i > 0 ? 0 : 1) + (i < n - 1 ? 0 : 1);
        for (int64_t j = 0; j < n; j++) {
            const int64_t c0 = ib + j * n;
            const double *fr = fv + c0;
            double *vr = v + c0;
            const int d = 6 + dbi + (j > 0 ? 0 : 1) + (j < n - 1 ? 0 : 1);
            const double cf = om * h2 / (double)d;
            const double cf1 = om * h2 / (double)(d + 1);
            vr[0] = cf1 * fr[0];
            for (int64_t k = 1; k < n - 1; k++) vr[k] = cf * fr[k];
            vr[n - 1] = cf1 * fr[n - 1];
        }
    }
}

/* r = f - A u over i-planes [i0,i1) */
static void residual_planes(const double *restrict u, const double *restrict fv,
                            double *restrict r, const int64_t n, const double h2,
                            const double *restrict zrow,
                            const int64_t i0, const int64_t i1)
{
    const int64_t nn = n * n;
    const double inv = 1.0 / h2;
    for (int64_t i = i0; i < i1; i++) {
        const int64_t ib = i * nn;
        const int dbi = (i > 0 ? 0 : 1) + (i < n - 1 ? 0 : 1);
        for (int64_t j = 0; j < n; j++) {
            const int64_t c0 = ib + j * n;
            const double *rm = (i > 0)     ? u + c0 - nn : zrow;
            const double *rp = (i < n - 1) ? u + c0 + nn : zrow;
            const double *qm = (j > 0)     ? u + c0 - n : zrow;
            const double *qp = (j < n - 1) ? u + c0 + n : zrow;
            const double *ur = u + c0;
            const double *fr = fv + c0;
            double *rr = r + c0;
            const int d = 6 + dbi + (j > 0 ? 0 : 1) + (j < n - 1 ? 0 : 1);
            const double dd = (double)d;
            const double dd1 = (double)(d + 1);
            double s;
            s = rm[0] + rp[0] + qm[0] + qp[0] + ur[1];
            rr[0] = fr[0] - (dd1 * ur[0] - s) * inv;
            for (int64_t k = 1; k < n - 1; k++) {
                s = rm[k] + rp[k] + qm[k] + qp[k] + ur[k - 1] + ur[k + 1];
                rr[k] = fr[k] - (dd * ur[k] - s) * inv;
            }
            s = rm[n - 1] + rp[n - 1] + qm[n - 1] + qp[n - 1] + ur[n - 2];
            rr[n - 1] = fr[n - 1] - (dd1 * ur[n - 1] - s) * inv;
        }
    }
}

/* full weighting: each coarse cell averages its 8 children */
static void restrict_planes(const double *restrict r, double *restrict fc,
                            const int64_t nc, const int64_t i0, const int64_t i1)
{
    const int64_t nf = 2 * nc;
    const int64_t nnf = nf * nf;
    for (int64_t i = i0; i < i1; i++) {
        const double *A = r + (2 * i) * nnf;
        const double *B = r + (2 * i + 1) * nnf;
        double *out = fc + i * nc * nc;
        for (int64_t j = 0; j < nc; j++) {
            const double *a0 = A + (2 * j) * nf;
            const double *a1 = A + (2 * j + 1) * nf;
            const double *b0 = B + (2 * j) * nf;
            const double *b1 = B + (2 * j + 1) * nf;
            double *o = out + j * nc;
            for (int64_t k = 0; k < nc; k++) {
                double s = a0[2 * k] + a0[2 * k + 1] + a1[2 * k] + a1[2 * k + 1]
                         + b0[2 * k] + b0[2 * k + 1] + b1[2 * k] + b1[2 * k + 1];
                o[k] = 0.125 * s;
            }
        }
    }
}

/* trilinear prolongation with odd reflection at the edges */
static void prolong_planes(const double *restrict uc, double *restrict uf,
                           const int64_t nc, const int64_t i0, const int64_t i1)
{
    const int64_t nf = 2 * nc;
    const int64_t nnf = nf * nf;
    const int64_t nnc = nc * nc;
    for (int64_t i = i0; i < i1; i++) {
        const int64_t pi = i >> 1;
        const int64_t si = (i & 1) ? 1 : -1;
        int64_t ci1 = pi + si;
        double wi1 = 1.0;
        if (ci1 < 0)            { ci1 = 0;      wi1 = -1.0; }
        else if (ci1 > nc - 1)  { ci1 = nc - 1; wi1 = -1.0; }
        const double *r0 = uc + pi * nnc;
        const double *r1 = uc + ci1 * nnc;
        for (int64_t j = 0; j < nf; j++) {
            const int64_t pj = j >> 1;
            const int64_t sj = (j & 1) ? 1 : -1;
            int64_t cj1 = pj + sj;
            double wj1 = 1.0;
            if (cj1 < 0)           { cj1 = 0;      wj1 = -1.0; }
            else if (cj1 > nc - 1) { cj1 = nc - 1; wj1 = -1.0; }
            const double *s00 = r0 + pj * nc;
            const double *s01 = r0 + cj1 * nc;
            const double *s10 = r1 + pj * nc;
            const double *s11 = r1 + cj1 * nc;
            double *dst = uf + i * nnf + j * nf;
            const double w00 = 9.0;
            const double w01 = 3.0 * wj1;
            const double w10 = 3.0 * wi1;
            const double w11 = wi1 * wj1;
            for (int64_t m = 1; m < nc; m++) {
                const bool last = (m == nc - 1);
                double p0 = 3.0 * s00[m] + s00[m - 1];
                double q0 = 3.0 * s00[m] + (last ? -s00[m] : s00[m + 1]);
                double p1 = 3.0 * s01[m] + s01[m - 1];
                double q1 = 3.0 * s01[m] + (last ? -s01[m] : s01[m + 1]);
                double p2 = 3.0 * s10[m] + s10[m - 1];
                double q2 = 3.0 * s10[m] + (last ? -s10[m] : s10[m + 1]);
                double p3 = 3.0 * s11[m] + s11[m - 1];
                double q3 = 3.0 * s11[m] + (last ? -s11[m] : s11[m + 1]);
                dst[2 * m]     += (w00 * p0 + w01 * p1 + w10 * p2 + w11 * p3) * (1.0 / 64.0);
                dst[2 * m + 1] += (w00 * q0 + w01 * q1 + w10 * q2 + w11 * q3) * (1.0 / 64.0);
            }
            {   /* m = 0: k=0 reads 3c[0]-c[0] = 2c[0]; k=1 reads 3c[0]+c[1] (or 2c[0]) */
                const bool one = (nc > 1);
                double e0 = 2.0 * s00[0], e1 = 2.0 * s01[0];
                double e2 = 2.0 * s10[0], e3 = 2.0 * s11[0];
                double q0 = 3.0 * s00[0] + (one ? s00[1] : -s00[0]);
                double q1 = 3.0 * s01[0] + (one ? s01[1] : -s01[0]);
                double q2 = 3.0 * s10[0] + (one ? s10[1] : -s10[0]);
                double q3 = 3.0 * s11[0] + (one ? s11[1] : -s11[0]);
                dst[0] += (w00 * e0 + w01 * e1 + w10 * e2 + w11 * e3) * (1.0 / 64.0);
                dst[1] += (w00 * q0 + w01 * q1 + w10 * q2 + w11 * q3) * (1.0 / 64.0);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
typedef struct {
    double *uh, *fh, *rh, *th;
    double *zrow;
    int64_t ns[LMAX], offs[LMAX];
    int64_t nlevels;
} mg_state;

static void mg_body(const mg_state *S, const double *restrict f, double *restrict u,
                    const int64_t N, const int64_t ncycles)
{
    const int64_t *ns = S->ns, *offs = S->offs;
    const int64_t nlevels = S->nlevels;
    double *uh = S->uh, *fh = S->fh, *rh = S->rh, *th = S->th, *zrow = S->zrow;
    const int64_t nn0 = N * N;

    /* fh[level 0] = f */
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < N; i++)
        memcpy(fh + i * nn0, f + i * nn0, (size_t)nn0 * sizeof(double));

    for (int64_t cyc = 0; cyc < ncycles; cyc++) {
        /* ---- down ---- */
        for (int64_t lv = 0; lv + 1 < nlevels; lv++) {
            const int64_t n = ns[lv];
            const double h2 = 1.0 / ((double)n * (double)n);
            double *A = uh + offs[lv], *B = th + offs[lv];
            const double *fl = fh + offs[lv];
            const int64_t nc = ns[lv + 1];
            if (lv == 0 && cyc > 0) {
                /* level 0 keeps the previous cycle's solution: plain sweep */
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < n; i++) sweep_planes(A, fl, B, n, h2, zrow, i, i + 1);
            } else {
                /* u == 0 on this level: first sweep needs no stencil */
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < n; i++) sweep0_planes(fl, B, n, h2, i, i + 1);
            }
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) sweep_planes(B, fl, A, n, h2, zrow, i, i + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) sweep_planes(A, fl, B, n, h2, zrow, i, i + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) residual_planes(B, fl, rh + offs[lv], n, h2, zrow, i, i + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < nc; i++) restrict_planes(rh + offs[lv], fh + offs[lv + 1], nc, i, i + 1);
        }
        /* ---- coarsest ---- */
        {
            const int64_t nl = nlevels - 1;
            const int64_t n = ns[nl];
            const double h2 = 1.0 / ((double)n * (double)n);
            double *A = uh + offs[nl], *B = th + offs[nl];
            const double *fl = fh + offs[nl];
            #pragma omp single
            {
                sweep0_planes(fl, B, n, h2, 0, n);
                double *cur = B;
                for (int s = 1; s < NCOARSE_SWEEPS; s++) {
                    double *nxt = (cur == B) ? A : B;
                    sweep_planes(cur, fl, nxt, n, h2, g_zero_row, 0, n);
                    cur = nxt;
                }
            }
        }
        /* ---- up ---- */
        for (int64_t lu = 0; lu + 1 < nlevels; lu++) {
            const int64_t up = nlevels - 2 - lu;
            const int64_t n = ns[up];
            const int64_t nc = ns[up + 1];
            const double h2 = 1.0 / ((double)n * (double)n);
            double *A = uh + offs[up], *B = th + offs[up];
            const double *fl = fh + offs[up];
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < 2 * nc; i++) prolong_planes(uh + offs[up + 1], B, nc, i, i + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) sweep_planes(B, fl, A, n, h2, zrow, i, i + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) sweep_planes(A, fl, B, n, h2, zrow, i, i + 1);
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) sweep_planes(B, fl, A, n, h2, zrow, i, i + 1);
        }
        /* u = level 0 of uh */
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++)
            memcpy(u + i * nn0, uh + i * nn0, (size_t)nn0 * sizeof(double));
    }
}

void mg_vcycle_fp64(const double *restrict f, double *restrict u, const int64_t N,
                    const int64_t ncycles, uint8_t *restrict workspace,
                    const int64_t workspace_size)
{
    if (N < 1) return;
    const int64_t n3 = N * N * N;
    if (ncycles < 1) {
        for (int64_t i = 0; i < n3; i++) u[i] = 0.0;
        return;
    }
    int64_t ns[LMAX], offs[LMAX];
    int64_t nlevels = 1, edge = N;
    while (edge > NCOARSE) { edge /= 2; nlevels++; }
    if (nlevels > LMAX) nlevels = LMAX;
    {
        int64_t pos = 0;
        edge = N;
        for (int64_t lv = 0; lv < nlevels; lv++) {
            ns[lv] = edge;
            offs[lv] = pos;
            pos += edge * edge * edge;
            edge /= 2;
        }
    }
    const int64_t flat = (8 * n3) / 7 + 64;
    const int64_t need = (4 * flat + 8192) * (int64_t)sizeof(double);
    double *uh, *free_me = NULL;
    if (workspace_size >= need && workspace) {
        uh = (double *)(void *)workspace;
    } else {
        free_me = (double *)malloc((size_t)need);
        if (!free_me) return;
        uh = free_me;
    }
    double *fh = uh + flat;
    double *rh = fh + flat;
    double *th = rh + flat;
    double *zrow = th + flat;

    mg_state S;
    S.uh = uh; S.fh = fh; S.rh = rh; S.th = th; S.zrow = zrow;
    for (int64_t lv = 0; lv < nlevels; lv++) { S.ns[lv] = ns[lv]; S.offs[lv] = offs[lv]; }
    S.nlevels = nlevels;

    if (N >= PAR_MIN_N && omp_get_max_threads() > 1) {
        #pragma omp parallel
        { mg_body(&S, f, u, N, ncycles); }
    } else {
        mg_body(&S, f, u, N, ncycles);
    }
    free(free_me);
}
