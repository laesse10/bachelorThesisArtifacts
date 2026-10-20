/* jfnk_bratu: faithful, fast port of the NumPy reference.
 *
 * The reference runs JFNK-Bratu: Newton (max 20 steps, stop at ||F|| <= 1e-14 f0)
 * around GMRES(restart=50, tol=1e-4) with a matrix-free finite-difference
 * Jacobian-vector product (eps = sqrt(macheps)*(1+||u||)/||v||).  With the loose
 * inner tolerance the outer loop converges only linearly, so the final iterate
 * depends on the whole trajectory: the arithmetic below reproduces the reference
 * (same eps, same FD formula, same Gram-Schmidt variant, same Givens rotations,
 * same break conditions and back substitution).
 *
 * Everything runs inside ONE persistent OpenMP parallel region.  Each data
 * parallel pass is a manually row-chunked loop (no worksharing constructs) that
 * writes per-thread partials and then joins on a single explicit barrier; the
 * scalar control flow (Givens rotations, back substitution, break conditions)
 * runs redundantly on every thread over thread-private copies, so all threads
 * always take identical decisions.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#pragma omp declare simd notinbranch
extern double exp(double);

#define MAXNEWTON 20
#define RESTART   50
#define INNER_TOL 1.0e-4
#define NEWTON_RTOL 1.0e-14
#define BREAKDOWN 1.0e-13

typedef struct {
    int n, N;
    double lam, hh;
    int64_t nn;
    double *F, *up, *ex, *w, *V;
    /* The Givens chain reads and rewrites the Hessenberg column, so the whole
       scalar section (H, cs, sn, g, y, hcol) is kept PER THREAD. */
    double *Hbase;  /* nthr_max * (RESTART+2)*RESTART */
    double *scbase; /* nthr_max * 2*(RESTART+2): cs then sn */
    double *gbase;  /* nthr_max * (RESTART+3) */
    double *ybase;  /* nthr_max * (RESTART+1) */
    double *hcolbase; /* nthr_max * (RESTART+1) */
    /* Three alternating scalar-partial buffers: a pass may only reuse a
       buffer no earlier than two passes after its last use, because a thread
       summing pass p can still be reading the buffer while the next pass of a
       faster thread overwrites it.  Adjacent passes always buffers differ. */
    double *part0;  /* residual ||F||^2, axpy ||w||^2 */
    double *part1;  /* residual ||u||^2, normalize ||V[k+1]||^2 */
    double *part2;  /* v0 ||V0||^2 */
    double *partD;  /* nthr*(RESTART+1) scratch for the dots pass */
} Ctx;

/* contiguous row chunk [r0, r1) of n rows for thread tid of nthr */
static inline void row_range(int n, int tid, int nthr, int *r0, int *r1)
{
    *r0 = (int)((int64_t)n * tid / nthr);
    *r1 = (int)((int64_t)n * (tid + 1) / nthr);
}

static inline double sum_part(const double *part, int nthr)
{
    double s = 0.0;
    for (int t = 0; t < nthr; t++) s += part[t];
    return s;
}

/* F = (4u - NSEW)/(h*h) - lam*exp(u) on the interior (u is the full N x N grid,
   boundary values are 0 so neighbor reads are unconditional).  Fused partials
   for ||F||^2 (part) and ||u||^2 (partA).  One barrier. */
static void k_residual(Ctx *c, const double *restrict u, double *restrict F)
{
    const int n = c->n, N = c->N;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const double lam = c->lam, hh = c->hh;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    double locF = 0.0, locU = 0.0;
    for (int i = r0; i < r1; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        double *Fr = F + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            const double val = (4.0 * ur[j] - ur[j - N] - ur[j + N]
                                - ur[j - 1] - ur[j + 1]) / hh - lam * exp(ur[j]);
            Fr[j] = val;
            locF += val * val;
            locU += ur[j] * ur[j];
        }
    }
    c->part0[tid] = locF;
    c->part1[tid] = locU;
    #pragma omp barrier
}

/* V0 = -F/beta on the interior; returns ||V0||_2.  One barrier. */
static double k_v0(Ctx *c, const double *restrict F, double beta)
{
    const int n = c->n;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    double *restrict V0 = c->V;
    double loc = 0.0;
    for (int i = r0; i < r1; i++) {
        const double *Fr = F + (int64_t)i * n;
        double *Vr = V0 + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            const double t = -Fr[j] / beta;
            Vr[j] = t;
            loc += t * t;
        }
    }
    c->part2[tid] = loc;
    #pragma omp barrier
    return sqrt(sum_part(c->part2, nthr));
}

/* Matrix-free JVP in one pass: up = u + eps*v and ex = exp(up) are computed for
   the chunk rows plus one ghost row on each side, then w = (Fp - F)/eps is
   formed for the chunk rows with no barrier in between.  One barrier. */
static void k_jvp(Ctx *c, const double *restrict u, const double *restrict F,
                  const double *restrict v, double eps)
{
    const int n = c->n, N = c->N;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const double lam = c->lam, hh = c->hh;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    const int g0 = (r0 > 0) ? r0 - 1 : 0;
    const int g1 = (r1 < n) ? r1 + 1 : n;
    for (int i = g0; i < g1; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        const double *vr = v + (int64_t)i * n;
        double *upr = c->up + (int64_t)i * n;
        double *exr = c->ex + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            const double t = ur[j] + eps * vr[j];
            upr[j] = t;
            exr[j] = exp(t);
        }
    }
    for (int i = r0; i < r1; i++) {
        const double *ur = c->up + (int64_t)i * n;
        const double *er = c->ex + (int64_t)i * n;
        const double *Fr = F + (int64_t)i * n;
        double *wr = c->w + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double s = 4.0 * ur[j];
            if (i > 0)     s -= ur[j - n];
            if (i < n - 1) s -= ur[j + n];
            if (j > 0)     s -= ur[j - 1];
            if (j < n - 1) s -= ur[j + 1];
            wr[j] = (s / hh - lam * er[j] - Fr[j]) / eps;
        }
    }
    #pragma omp barrier
}

/* classical Gram-Schmidt: hcol[p] = <V[p], w> for p = 0..k.  One barrier. */
static void k_dots(Ctx *c, const double *restrict w, int k,
                   double *restrict hcol)
{
    const int n = c->n;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const int64_t nn = c->nn;
    double *restrict part = c->partD + (RESTART + 1) * tid;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    for (int p = 0; p <= k; p++) part[p] = 0.0;
    for (int i = r0; i < r1; i++) {
        const int64_t off = (int64_t)i * n;
        const double *wr = w + off;
        for (int j = 0; j < n; j++) {
            const double wi = wr[j];
            for (int p = 0; p <= k; p++)
                part[p] += c->V[(int64_t)p * nn + off + j] * wi;
        }
    }
    #pragma omp barrier
    for (int p = 0; p <= k; p++) {
        double s = 0.0;
        for (int t = 0; t < nthr; t++)
            s += c->partD[(int64_t)t * (RESTART + 1) + p];
        hcol[p] = s;
    }
}

/* w -= sum_p h[p] V[p] ; returns ||w||_2 afterwards.  One barrier. */
static double k_axpy(Ctx *c, int k, const double *restrict h)
{
    const int n = c->n;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const int64_t nn = c->nn;
    double *restrict w = c->w;
    double *restrict V = c->V;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    double loc = 0.0;
    for (int i = r0; i < r1; i++) {
        const int64_t off = (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double acc = 0.0;
            for (int p = 0; p <= k; p++)
                acc += h[p] * V[(int64_t)p * nn + off + j];
            const double nw = w[off + j] - acc;
            w[off + j] = nw;
            loc += nw * nw;
        }
    }
    c->part0[tid] = loc;
    #pragma omp barrier
    return sqrt(sum_part(c->part0, nthr));
}

/* V[k+1] = w/hnext ; returns ||V[k+1]||_2.  One barrier. */
static double k_normalize(Ctx *c, double hnext, int k)
{
    const int n = c->n;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    double *restrict dst = c->V + (int64_t)(k + 1) * c->nn;
    const double *restrict w = c->w;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    double loc = 0.0;
    for (int i = r0; i < r1; i++) {
        const int64_t off = (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            const double t = w[off + j] / hnext;
            dst[off + j] = t;
            loc += t * t;
        }
    }
    c->part1[tid] = loc;
    #pragma omp barrier
    return sqrt(sum_part(c->part1, nthr));
}

/* u += sum_p y[p] V[p] (the reference's du accumulation fused with the Newton
   update; per element the additions happen in the same p order).  One barrier. */
static void k_update(Ctx *c, double *restrict u, const double *restrict y,
                     int m_used)
{
    const int n = c->n, N = c->N;
    const int64_t nn = c->nn;
    double *restrict V = c->V;
    int r0, r1;
    row_range(n, omp_get_thread_num(), omp_get_num_threads(), &r0, &r1);
    for (int i = r0; i < r1; i++) {
        double *ur = u + (int64_t)(i + 1) * N + 1;
        const int64_t off = (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double acc = 0.0;
            for (int p = 0; p < m_used; p++)
                acc += y[p] * V[(int64_t)p * nn + off + j];
            ur[j] += acc;
        }
    }
    #pragma omp barrier
}

/* GMRES(RESTART) with FD Jacobian-vector products (mirrors bratu_gmres), ending
   with the fused Newton update.  Every thread runs the control flow redundantly
   on identical (identical) values of its private copies. */
static void k_gmres(Ctx *c, double *restrict u, const double *restrict F,
                    double beta, double nu)
{
    const int64_t nn = c->nn;
    const int tid = omp_get_thread_num();
    double (*H)[RESTART] =
        (double (*)[RESTART])(c->Hbase + (size_t)tid * (RESTART + 2) * RESTART);
    double *cs = c->scbase + (size_t)tid * 2 * (RESTART + 2);
    double *sn = cs + (RESTART + 2);
    double *g = c->gbase + (size_t)tid * (RESTART + 3);
    double *y = c->ybase + (size_t)tid * (RESTART + 1);
    double *hcol = c->hcolbase + (size_t)tid * (RESTART + 1);
    if (beta == 0.0) return;      /* du = 0: the update is a no-op */
    double nv = k_v0(c, F, beta);
    g[0] = beta;
    int m_used = RESTART;
    int k;
    for (k = 0; k < RESTART; k++) {
        const double eps = 1.4901161193847656e-08 * (1.0 + nu) / nv;
        k_jvp(c, u, F, c->V + (int64_t)k * nn, eps);
        k_dots(c, c->w, k, hcol);
        for (int p = 0; p <= k; p++) H[p][k] = hcol[p];
        const double hnext = k_axpy(c, k, hcol);
        H[k + 1][k] = hnext;
        for (int p = 0; p < k; p++) {
            double temp = cs[p] * H[p][k] + sn[p] * H[p + 1][k];
            H[p + 1][k] = -sn[p] * H[p][k] + cs[p] * H[p + 1][k];
            H[p][k] = temp;
        }
        const double denom = sqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
        cs[k] = H[k][k] / denom;
        sn[k] = H[k + 1][k] / denom;
        H[k][k] = cs[k] * H[k][k] + sn[k] * H[k + 1][k];
        H[k + 1][k] = 0.0;
        const double temp = cs[k] * g[k];
        g[k + 1] = -sn[k] * g[k];
        g[k] = temp;
        const double rel = fabs(g[k + 1]) / beta;
        if (rel < INNER_TOL || hnext < BREAKDOWN || k == RESTART - 1) {
            m_used = k + 1;
            break;
        }
        nv = k_normalize(c, hnext, k);
    }
    (void) k;
    for (int row = 0; row < m_used; row++) {
        const int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; col++) s -= H[rr][col] * y[col];
        y[rr] = s / H[rr][rr];
    }
    k_update(c, u, y, m_used);
}

void jfnk_bratu_fp64(
    double *restrict u,
    const int64_t N,
    const double lam,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    const int n = (int)(N - 2);
    if (n < 1) return;
    const double h = 1.0 / (double)(N - 1);
    const int nthr_max = omp_get_max_threads();
    const size_t perthr = (size_t)(RESTART + 2) * RESTART   /* H */
                        + 2 * (RESTART + 2)                  /* cs, sn */
                        + (RESTART + 3)                      /* g */
                        + (RESTART + 1)                      /* y */
                        + (RESTART + 1);                     /* hcol */
    const size_t scratch = perthr * (size_t)(nthr_max + 1)
                         + (size_t)(2 * RESTART + 16) * (nthr_max + 1) + 4096;
    const size_t total = (size_t)(RESTART + 5) * (size_t)n * n + scratch + 64;

    double *base;
    int used_ws = 0;
    if (workspace && workspace_size >= (int64_t)(total * sizeof(double))) {
        uintptr_t p = (uintptr_t)workspace;
        p = (p + 63) & ~(uintptr_t)63;
        base = (double *)p;
        used_ws = 1;
    } else {
        base = (double *)malloc(total * sizeof(double));
        if (!base) return;
    }
    double *ptr = base;
    Ctx c;
    c.n = n; c.N = (int)N; c.lam = lam; c.hh = h * h;
    c.nn = (int64_t)n * n;
    c.F  = ptr; ptr += c.nn;
    c.up = ptr; ptr += c.nn;
    c.ex = ptr; ptr += c.nn;
    c.w  = ptr; ptr += c.nn;
    c.V  = ptr; ptr += (size_t)RESTART * c.nn;
    c.Hbase    = ptr; ptr += perthr * (nthr_max + 1);
    c.scbase   = ptr; ptr += perthr * (nthr_max + 1);
    c.gbase    = ptr; ptr += perthr * (nthr_max + 1);
    c.ybase    = ptr; ptr += perthr * (nthr_max + 1);
    c.hcolbase = ptr; ptr += perthr * (nthr_max + 1);
    c.part0 = ptr; ptr += nthr_max + 1;
    c.part1 = ptr; ptr += nthr_max + 1;
    c.part2 = ptr; ptr += nthr_max + 1;
    c.partD = ptr;

    #pragma omp parallel
    {
        const int nthr = omp_get_num_threads();
        /* Newton loop, exactly as in the reference */
        k_residual(&c, u, c.F);          /* leaves ||F||^2 in part, ||u||^2 in partA */
        double f2 = sum_part(c.part0, nthr);
        const double f0 = sqrt(f2);
        double nu = sqrt(sum_part(c.part1, nthr));
        for (int step = 0; step < MAXNEWTON; step++) {
            const double fnow = sqrt(f2); /* F is unchanged since the last residual */
            if (fnow <= NEWTON_RTOL * f0) break;
            k_gmres(&c, u, c.F, fnow, nu);
            k_residual(&c, u, c.F);
            f2 = sum_part(c.part0, nthr);
            nu = sqrt(sum_part(c.part1, nthr));
        }
    }
    if (!used_ws) free(base);
}
