/* jfnk_bratu: faithful, fast port of the NumPy reference.
 *
 * The reference runs JFNK-Bratu: Newton (max 20 steps, stop at ||F|| <= 1e-14 f0)
 * around GMRES(restart=50, tol=1e-4) with a matrix-free finite-difference
 * Jacobian-vector product (eps = sqrt(macheps)*(1+||u||)/||v||).  With the loose
 * inner tolerance the outer loop converges only linearly, so the final
 * iterate depends on the whole trajectory: the arithmetic below reproduces
 * the reference (same eps, same FD formula, same Gram-Schmidt, same Givens
 * rotations, same break conditions and back substitution) while running every
 * data-parallel parts (stencils, exp, Gram-Schmidt) under one persistent
 * OpenMP region.
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
    int n, N, nthr;
    double lam, hh, nu;
    int64_t nn;
    double *F, *du, *up, *ex, *w, *V;
    /* The Givens rotation chain reads and rewrites the Hessenberg column, so the
       whole scalar section (H, cs, sn, g, y, hcol) is kept PER THREAD: every
       thread runs it redundantly on private copies, which cannot race. */
    double *Hbase;  /* nthr_max * (RESTART+2)*RESTART */
    double *scbase; /* nthr_max * 2*(RESTART+2): cs[0..RESTART+1], sn[0..RESTART+1] */
    double *gbase;  /* nthr_max * (RESTART+3) */
    double *ybase;  /* nthr_max * (RESTART+1) */
    double *hcolbase; /* nthr_max * (RESTART+1) */
    double *part;   /* nthr scratch for scalar reductions */
    double *partA;  /* nthr scratch for the norm after axpy */
    double *partD;  /* (RESTART+1)*nthr scratch for the dots */
} Ctx;

/* F = (4u - NSEW)/(h*h) - lam*exp(u) on the interior; u is the full N x N grid */
static void k_residual(Ctx *c, const double *restrict u, double *restrict F)
{
    const int n = c->n, N = c->N;
    const double lam = c->lam, hh = c->hh;
    #pragma omp for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        double *Fr = F + (int64_t)i * n;
        for (int j = 0; j < n; j++)
            Fr[j] = (4.0 * ur[j] - ur[j - N] - ur[j + N] - ur[j - 1] - ur[j + 1])
                    / hh - lam * exp(ur[j]);
    }
}

/* s = <a,b> (all threads get the same value; one barrier) */
static double k_dot(Ctx *c, const double *restrict a, const double *restrict b,
                     double *restrict part)
{
    const int64_t nn = c->nn;
    const int nthr = c->nthr;
    const int tid = omp_get_thread_num();
    double loc = 0.0;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < nn; i++) loc += a[i] * b[i];
    part[tid] = loc;
    #pragma omp barrier
    double s = 0.0;
    for (int t = 0; t < nthr; t++) s += part[t];
    return s;
}

/* ||u||_2 over the interior of the full-grid u (boundary rows are 0) */
static double k_norm_u(Ctx *c, const double *restrict u)
{
    const int n = c->n, N = c->N;
    const int nthr = c->nthr;
    const int tid = omp_get_thread_num();
    double loc = 0.0;
    #pragma omp for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        for (int j = 0; j < n; j++) loc += ur[j] * ur[j];
    }
    c->part[tid] = loc;
    #pragma omp barrier
    double s = 0.0;
    for (int t = 0; t < nthr; t++) s += c->part[t];
    return sqrt(s);
}

/* up = u + eps*v ; ex = exp(up) */
static void k_jvp_up(Ctx *c, const double *restrict u, const double *restrict v,
                     double eps)
{
    const int n = c->n, N = c->N;
    #pragma omp for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        const double *vr = v + (int64_t)i * n;
        double *upr = c->up + (int64_t)i * n;
        double *exr = c->ex + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double t = ur[j] + eps * vr[j];
            upr[j] = t;
            exr[j] = exp(t);
        }
    }
}

/* w = (Fp - F)/eps with Fp = (4*up - NSEW)/(h*h) - lam*ex */
static void k_jvp_w(Ctx *c, const double *restrict F, double *restrict w,
                    double eps)
{
    const int n = c->n;
    const double lam = c->lam, hh = c->hh;
    #pragma omp for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = c->up + (int64_t)i * n;
        const double *er = c->ex + (int64_t)i * n;
        const double *Fr = F + (int64_t)i * n;
        double *wr = w + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double s = 4.0 * ur[j];
            if (i > 0)     s -= ur[j - n];
            if (i < n - 1) s -= ur[j + n];
            if (j > 0)     s -= ur[j - 1];
            if (j < n - 1) s -= ur[j + 1];
            wr[j] = (s / hh - lam * er[j] - Fr[j]) / eps;
        }
    }
}

/* classical Gram-Schmidt: h[p] = <V[p], w> for p = 0..k (one pass + barrier) */
static void k_dots(Ctx *c, const double *restrict w, int k,
                    double *restrict hcol)
{
    const int64_t nn = c->nn;
    const int nthr = c->nthr;
    const int tid = omp_get_thread_num();
    double *restrict part = c->partD + (RESTART + 1) * tid;
    for (int p = 0; p <= k; p++) part[p] = 0.0;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < nn; i++) {
        const double wi = w[i];
        for (int p = 0; p <= k; p++) part[p] += c->V[(int64_t)p * nn + i] * wi;
    }
    #pragma omp barrier
    for (int p = 0; p <= k; p++) {
        double s = 0.0;
        for (int t = 0; t < nthr; t++) s += c->partD[(int64_t)t * (RESTART + 1) + p];
        hcol[p] = s;
    }
    #pragma omp barrier
}

/* w -= sum_p h[p] V[p] ; returns ||w||_2 afterwards (one pass + barrier) */
static double k_axpy(Ctx *c, int k, const double *restrict h)
{
    const int64_t nn = c->nn;
    const int nthr = c->nthr;
    const int tid = omp_get_thread_num();
    double *restrict w = c->w;
    double *restrict V = c->V;
    double loc = 0.0;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < nn; i++) {
        double acc = 0.0;
        for (int p = 0; p <= k; p++) acc += h[p] * V[(int64_t)p * nn + i];
        const double nw = w[i] - acc;
        w[i] = nw;
        loc += nw * nw;
    }
    c->partA[tid] = loc;
    #pragma omp barrier
    double s = 0.0;
    for (int t = 0; t < nthr; t++) s += c->partA[t];
    return sqrt(s);
}

/* V[k+1] = w/hext ; returns ||V[k+1]||_2 (fused norm of the new basis vector) */
static double k_normalize(Ctx *c, double hnext, int k)
{
    const int64_t nn = c->nn;
    const int nthr = c->nthr;
    const int tid = omp_get_thread_num();
    double *restrict dst = c->V + (int64_t)(k + 1) * nn;
    const double *restrict w = c->w;
    double loc = 0.0;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < nn; i++) {
        const double t = w[i] / hnext;
        dst[i] = t;
        loc += t * t;
    }
    c->part[tid] = loc;
    #pragma omp barrier
    double s = 0.0;
    for (int t = 0; t < nthr; t++) s += c->part[t];
    return sqrt(s);
}

/* GMRES(RESTART) with FD Jacobian-vector products (mirrors bratu_gmres).
 * Runs inside the persistent parallel region; every thread executes the
 * control flow redundantly on identical scalar values. */
static void k_gmres(Ctx *c, const double *restrict u, const double *restrict F,
                    double *restrict du)
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
    const double beta = sqrt(k_dot(c, F, F, c->part));
    if (beta == 0.0) {
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nn; i++) du[i] = 0.0;
        return;
    }
    double *restrict V0 = c->V;
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < nn; i++) V0[i] = -F[i] / beta;
    #pragma omp barrier
    double nv = sqrt(k_dot(c, V0, V0, c->part));
    g[0] = beta;
    int m_used = RESTART;
    int k;
    for (k = 0; k < RESTART; k++) {
        const double eps = 1.4901161193847656e-08 * (1.0 + c->nu) / nv;
        k_jvp_up(c, u, c->V + (int64_t)k * nn, eps);
        k_jvp_w(c, F, c->w, eps);
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
    #pragma omp for schedule(static)
    for (int64_t i = 0; i < nn; i++) du[i] = 0.0;
    for (int p = 0; p < m_used; p++) {
        const double yp = y[p];
        if (yp == 0.0) continue;
        const double *Vp = c->V + (int64_t)p * nn;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nn; i++) du[i] += yp * Vp[i];
    }
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
                         + (size_t)(RESTART + 12) * (nthr_max + 1) + 4096;
    const size_t total = (size_t)(RESTART + 6) * (size_t)n * n + scratch + 64;

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
    c.du = ptr; ptr += c.nn;
    c.up = ptr; ptr += c.nn;
    c.ex = ptr; ptr += c.nn;
    c.w  = ptr; ptr += c.nn;
    c.V  = ptr; ptr += (size_t)RESTART * c.nn;
    c.Hbase   = ptr; ptr += perthr * (nthr_max + 1);
    c.scbase  = ptr; ptr += perthr * (nthr_max + 1);
    c.gbase   = ptr; ptr += perthr * (nthr_max + 1);
    c.ybase   = ptr; ptr += perthr * (nthr_max + 1);
    c.hcolbase = ptr; ptr += perthr * (nthr_max + 1);
    c.part  = ptr; ptr += nthr_max + 1;
    c.partA = ptr; ptr += nthr_max + 1;
    c.partD = ptr;

    #pragma omp parallel
    {
        c.nthr = omp_get_num_threads();
        /* Newton loop, exactly as in the reference */
        k_residual(&c, u, c.F);
        #pragma omp barrier
        const double f0 = sqrt(k_dot(&c, c.F, c.F, c.part));
        for (int step = 0; step < MAXNEWTON; step++) {
            const double fnow = sqrt(k_dot(&c, c.F, c.F, c.part));
            if (fnow <= NEWTON_RTOL * f0) break;
            c.nu = k_norm_u(&c, u);
            k_gmres(&c, u, c.F, c.du);
            const int nn = c.n, N = c.N;
            #pragma omp for schedule(static)
            for (int i = 0; i < nn; i++) {
                double *ur = u + (int64_t)(i + 1) * N + 1;
                const double *dr = c.du + (int64_t)i * nn;
                for (int j = 0; j < nn; j++) ur[j] += dr[j];
            }
            k_residual(&c, u, c.F);
        }
    }
    if (!used_ws) free(base);
}
