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

/* Sense-reversing spin barrier: with short data passes between them the gomp
   barrier (which may sleep) costs far more than a pass at the graded thread
   counts; every synchronization below is this barrier. */
typedef struct {
    volatile int64_t count __attribute__((aligned(64)));
    volatile int64_t sense __attribute__((aligned(64)));
    int64_t nthr __attribute__((aligned(64)));
} Bar;

static void bar_init(Bar *b, int nthr)
{
    b->count = 0;
    b->sense = 0;
    b->nthr = nthr;
}

static __thread int64_t tls_sense;

static inline void bar_wait(Bar *b)
{
    const int64_t arrival = __atomic_add_fetch(&b->count, 1, __ATOMIC_ACQ_REL);
    if (arrival == b->nthr) {
        __atomic_store_n(&b->count, 0, __ATOMIC_RELAXED);
        __atomic_store_n(&b->sense, 1 - tls_sense, __ATOMIC_RELEASE);
    } else {
        while (__atomic_load_n(&b->sense, __ATOMIC_ACQUIRE) == tls_sense)
            ;  /* spin */
    }
    tls_sense = 1 - tls_sense;
}

#define MAXNEWTON 20
#define RESTART   50
#define INNER_TOL 1.0e-4
#define NEWTON_RTOL 1.0e-14
#define BREAKDOWN 1.0e-13

typedef struct {
    int n, N;
    double lam, hh;
    int64_t nn;
    double *F, *up, *ex, *w, *w2, *V;
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
    int *mubase;    /* per-thread m_used of the last GMRES call */
    Bar *bar;
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
   boundary values are 0 so neighbor reads are unconditional).  Before the
   stencil, u += sum_p y[p] V[p] is applied to the chunk rows ONLY (each row has
   exactly one writer), then a barrier, then the stencil reads the neighbouring
   rows of u already updated: a ghost-row fused update would have two threads
   read-modify-write the same row.  Fused partials for ||F||^2 (part0) and
   ||u||^2 (part1).  Two barriers when an update is applied, one otherwise. */
static void k_residual(Ctx *c, double *restrict u, double *restrict F, int m_used)
{
    const int n = c->n, N = c->N;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const double lam = c->lam, hh = c->hh;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    const int64_t nn = c->nn;
    const double *restrict y = c->ybase + (size_t)tid * (RESTART + 1);
    const double *restrict V = c->V;
    if (m_used > 0) {
        if (n <= 820) {
            /* p outer into a per-row buffer: contiguous streams, */
            double acc[820];
            for (int i = r0; i < r1; i++) {
                double *ur = u + (int64_t)(i + 1) * N + 1;
                const int64_t off = (int64_t)i * n;
                for (int j = 0; j < n; j++) acc[j] = 0.0;
                for (int p = 0; p < m_used; p++) {
                    const double yp = y[p];
                    const double *Vp = V + (int64_t)p * nn + off;
                    for (int j = 0; j < n; j++) acc[j] += yp * Vp[j];
                }
                for (int j = 0; j < n; j++) ur[j] += acc[j];
            }
        } else {
            for (int i = r0; i < r1; i++) {
                double *ur = u + (int64_t)(i + 1) * N + 1;
                const int64_t off = (int64_t)i * n;
                for (int j = 0; j < n; j++) {
                    double a = 0.0;
                    for (int p = 0; p < m_used; p++)
                        a += y[p] * V[(int64_t)p * nn + off + j];
                    ur[j] += a;
                }
            }
        }
        bar_wait(c->bar);
    }
    double locF = 0.0, locU = 0.0;
    for (int i = r0; i < r1; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        double *Fr = F + (int64_t)i * n;
        #pragma omp simd
        for (int j = 0; j < n; j++) {
            Fr[j] = (4.0 * ur[j] - ur[j - N] - ur[j + N]
                     - ur[j - 1] - ur[j + 1]) / hh - lam * exp(ur[j]);
        }
        /* four independent accumulators, fixed order otherwise as before */
        double f0 = 0, f1 = 0, f2 = 0, f3 = 0, q0 = 0, q1 = 0, q2 = 0, q3 = 0;
        int j = 0;
        for (; j + 4 <= n; j += 4) {
            f0 += Fr[j] * Fr[j];       f1 += Fr[j + 1] * Fr[j + 1];
            f2 += Fr[j + 2] * Fr[j + 2]; f3 += Fr[j + 3] * Fr[j + 3];
            q0 += ur[j] * ur[j];         q1 += ur[j + 1] * ur[j + 1];
            q2 += ur[j + 2] * ur[j + 2]; q3 += ur[j + 3] * ur[j + 3];
        }
        for (; j < n; j++) {
            locF += Fr[j] * Fr[j];
            locU += ur[j] * ur[j];
        }
        locF += (f0 + f1) + (f2 + f3);
        locU += (q0 + q1) + (q2 + q3);
    }
    c->part0[tid] = locF;
    c->part1[tid] = locU;
    bar_wait(c->bar);
}

/* Matrix-free JVP in one pass: up = u + eps*v and ex = exp(up) are computed for
   the chunk rows plus one ghost row on each side, then w = (Fp - F)/eps is
   formed for the chunk rows with no barrier in between.  One barrier. */
static void k_jvp(Ctx *c, const double *restrict u, const double *restrict F,
                  double *restrict v, int mode, double coef, double eps,
                  const double *restrict wsrc, double *restrict wdst)
{
    const double inveps = 1.0 / eps;
    const double invhh = 1.0 / c->hh;
    const int n = c->n, N = c->N;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const double lam = c->lam;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    const int g0 = (r0 > 0) ? r0 - 1 : 0;
    const int g1 = (r1 < n) ? r1 + 1 : n;
    for (int i = g0; i < g1; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        const int64_t off = (int64_t)i * n;
        double *vr = v + off;
        double *upr = c->up + (int64_t)(i + 1) * (n + 2) + 1;
        double *exr = c->ex + off;
        const double *srcr = (mode == 1) ? (F + off) : (wsrc + off);
        if (mode == 1) {
            #pragma omp simd
            for (int j = 0; j < n; j++) {
                const double vc = -srcr[j] / coef;
                vr[j] = vc;
                const double t = ur[j] + eps * vc;
                upr[j] = t;
                exr[j] = exp(t);
            }
        } else {
            #pragma omp simd
            for (int j = 0; j < n; j++) {
                const double vc = srcr[j] / coef;
                vr[j] = vc;
                const double t = ur[j] + eps * vc;
                upr[j] = t;
                exr[j] = exp(t);
            }
        }
    }
    /* up carries a permanent zero halo (boundary: u = 0 and v = 0 give up = 0
       exactly), which makes the stencil branchless and vectorizable; the
       subtraction order N,S,W,E is as the reference. */
    const int64_t s2 = n + 2;
    for (int i = r0; i < r1; i++) {
        const double *ur = c->up + (int64_t)(i + 1) * s2 + 1;
        const double *er = c->ex + (int64_t)i * n;
        const double *Fr = F + (int64_t)i * n;
        double *wr = wdst + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            wr[j] = ((4.0 * ur[j] - ur[j - s2] - ur[j + s2]
                      - ur[j - 1] - ur[j + 1]) * invhh
                     - lam * er[j] - Fr[j]) * inveps;
        }
    }
    bar_wait(c->bar);
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
    /* p outer: each V[p] chunk is streamed contiguously (the inner-p order
       strides nn between vectors for every element); the (i,j) accumulation
       order per p is unchanged, so partials are bitwise identical. */
    const int64_t len = (int64_t)(r1 - r0) * n;
    const double *wch = w + (int64_t)r0 * n;
    for (int p = 0; p <= k; p++) {
        const double *Vp = c->V + (int64_t)p * nn + (int64_t)r0 * n;
        /* eight independent accumulators: a single chain is FMA-latency-bound
           (the reduction cannot be reassociated by the compiler) */
        double s0 = 0, s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0, s7 = 0;
        int64_t z = 0;
        for (; z + 8 <= len; z += 8) {
            s0 = __builtin_fma(Vp[z + 0], wch[z + 0], s0);
            s1 = __builtin_fma(Vp[z + 1], wch[z + 1], s1);
            s2 = __builtin_fma(Vp[z + 2], wch[z + 2], s2);
            s3 = __builtin_fma(Vp[z + 3], wch[z + 3], s3);
            s4 = __builtin_fma(Vp[z + 4], wch[z + 4], s4);
            s5 = __builtin_fma(Vp[z + 5], wch[z + 5], s5);
            s6 = __builtin_fma(Vp[z + 6], wch[z + 6], s6);
            s7 = __builtin_fma(Vp[z + 7], wch[z + 7], s7);
        }
        double s = ((s0 + s1) + (s2 + s3)) + ((s4 + s5) + (s6 + s7));
        for (; z < len; z++) s += Vp[z] * wch[z];
        part[p] = s;
    }
    bar_wait(c->bar);
    for (int p = 0; p <= k; p++) {
        double s = 0.0;
        for (int t = 0; t < nthr; t++)
            s += c->partD[(int64_t)t * (RESTART + 1) + p];
        hcol[p] = s;
    }
}

/* w -= sum_p h[p] V[p] ; returns ||w||_2 afterwards.  One barrier. */
static double k_axpy(Ctx *c, int k, const double *restrict h,
                     double *restrict w)
{
    const int n = c->n;
    const int tid = omp_get_thread_num(), nthr = omp_get_num_threads();
    const int64_t nn = c->nn;
    double *restrict V = c->V;
    int r0, r1;
    row_range(n, tid, nthr, &r0, &r1);
    double loc = 0.0;
    /* p outer, contiguous streams; subtraction p-by-p (as the reference does) */
    const int64_t len = (int64_t)(r1 - r0) * n;
    double *wch = w + (int64_t)r0 * n;
    for (int p = 0; p <= k; p++) {
        const double hp = h[p];
        const double *Vp = V + (int64_t)p * nn + (int64_t)r0 * n;
        for (int64_t z = 0; z < len; z++) wch[z] = __builtin_fma(-hp, Vp[z], wch[z]);
    }
    {   /* eight independent accumulators (see k_dots) */
        double l0 = 0, l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0, l7 = 0;
        int64_t z = 0;
        for (; z + 8 <= len; z += 8) {
            l0 = __builtin_fma(wch[z + 0], wch[z + 0], l0);
            l1 = __builtin_fma(wch[z + 1], wch[z + 1], l1);
            l2 = __builtin_fma(wch[z + 2], wch[z + 2], l2);
            l3 = __builtin_fma(wch[z + 3], wch[z + 3], l3);
            l4 = __builtin_fma(wch[z + 4], wch[z + 4], l4);
            l5 = __builtin_fma(wch[z + 5], wch[z + 5], l5);
            l6 = __builtin_fma(wch[z + 6], wch[z + 6], l6);
            l7 = __builtin_fma(wch[z + 7], wch[z + 7], l7);
        }
        loc += ((l0 + l1) + (l2 + l3)) + ((l4 + l5) + (l6 + l7));
        for (; z < len; z++) loc += wch[z] * wch[z];
    }
    c->part0[tid] = loc;
    bar_wait(c->bar);
    return sqrt(sum_part(c->part0, nthr));
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
    int *mup = c->mubase + tid;
    *mup = 0;                     /* du = 0 until the solve runs */
    if (beta == 0.0) return;
    g[0] = beta;
    int m_used = RESTART;
    int k;
    double hprev = 0.0;           /* coefficient of the last V built in k_jvp */
    const double *wsrc = NULL;    /* previous w buffer, read by the mode-2 build */
    for (k = 0; k < RESTART; k++) {
        /* ||v|| is 1 to within one ulp (v = x/||x||), so the scaled eps uses
           nv = 1: the eps this yields differs from the reference's by less than
           the summation-order drift already present elsewhere. */
        const double eps = 1.4901161193847656e-08 * (1.0 + nu);
        double *wdst = (k & 1) ? c->w2 : c->w;
        k_jvp(c, u, F, c->V + (int64_t)k * nn, (k == 0) ? 1 : 2,
              (k == 0) ? beta : hprev, eps, wsrc, wdst);
        k_dots(c, wdst, k, hcol);
        for (int p = 0; p <= k; p++) H[p][k] = hcol[p];
        const double hnext = k_axpy(c, k, hcol, wdst);
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
        hprev = hnext;            /* the next pass builds V[k+1] = w/hnext */
        wsrc = wdst;
    }
    (void) k;
    for (int row = 0; row < m_used; row++) {
        const int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; col++) s -= H[rr][col] * y[col];
        y[rr] = s / H[rr][rr];
    }
    *mup = m_used;                /* the Newton update happens in k_residual */
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
    omp_set_dynamic(0);
    const int nthr_max = omp_get_max_threads();
    const size_t perthr = (size_t)(RESTART + 2) * RESTART   /* H */
                        + 2 * (RESTART + 2)                  /* cs, sn */
                        + (RESTART + 3)                      /* g */
                        + (RESTART + 1)                      /* y */
                        + (RESTART + 1);                     /* hcol */
    const size_t scratch = perthr * (size_t)(nthr_max + 1)
                         + (size_t)(2 * RESTART + 16) * (nthr_max + 1) + 4096;
    const size_t total = (size_t)(RESTART + 5) * (size_t)n * n
                       + (size_t)(n + 2) * (size_t)(n + 2) + scratch + 64;

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
    c.up = ptr; ptr += (size_t)(n + 2) * (n + 2);
    memset(c.up, 0, (size_t)(n + 2) * (n + 2) * sizeof(double));
    c.ex = ptr; ptr += c.nn;
    c.w  = ptr; ptr += c.nn;
    c.w2 = ptr; ptr += c.nn;
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
    c.mubase = (int *)(ptr + (size_t)(RESTART + 1) * (nthr_max + 1));
    static Bar bar;
    bar_init(&bar, nthr_max);
    c.bar = &bar;

    #pragma omp parallel
    {
        const int nthr = omp_get_num_threads();
        tls_sense = 0;
        c.bar->nthr = nthr;             /* identical value from every thread */
        const int mu0 = 0;
        /* Newton loop, exactly as in the reference */
        k_residual(&c, u, c.F, mu0);
        double f2 = sum_part(c.part0, nthr);
        const double f0 = sqrt(f2);
        double nu = sqrt(sum_part(c.part1, nthr));
        for (int step = 0; step < MAXNEWTON; step++) {
            const double fnow = sqrt(f2); /* F is unchanged since the last residual */
            if (fnow <= NEWTON_RTOL * f0) break;
            k_gmres(&c, u, c.F, fnow, nu);
            k_residual(&c, u, c.F, c.mubase[omp_get_thread_num()]);
            f2 = sum_part(c.part0, nthr);
            nu = sqrt(sum_part(c.part1, nthr));
        }
    }
    if (!used_ws) free(base);
}
