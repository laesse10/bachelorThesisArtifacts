/* Geometric multigrid V-cycle, cell-centered 3-D grid (HPGMG-style).
 * Optimized C: OpenMP over (i-slice block, j-row block), vectorized over j.
 * Semantics match mg_vcycle_numpy.py (damped Jacobi w=6/7, odd-reflection ghosts,
 * full weighting, 27/9/3/1 trilinear prolongation, 40-sweep coarse solve).
 *
 * Sizing policy: the workload is memory-bound; this machine scales best with a
 * small fixed team (about one 3-core group per NUMA node), so N>=128 runs with
 * 12 threads and N<=64 runs fully serial (OpenMP region overhead would cost
 * more than it saves on the small grids).
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define OMEGA (6.0/7.0)
#define NU1 3
#define NU2 3
#define NCOARSE_SWEEPS 40
#define NCOARSE 4
#define LMAX 24
#define BW 16   /* vector width over j (compile-time, auto-vectorization) */

typedef struct {
    int64_t n, off;          /* fine edge and offset */
    int64_t nc, offc;        /* coarse edge and offset (0 if coarsest) */
    double invh2;            /* 1/h2 */
    /* damped-Jacobi constants indexed by (d - 6) in {0,1,2,3}, precomputed once
     * per level so the hot row loop does no divisions. d = 6 + (i on boundary) +
     * (j on boundary) + (k on boundary); the interior branch always has d = 6. */
    double af[4], ass[4], afk[4], ask[4];
} Lvl;

static double *g_fb = NULL;
static int64_t g_fb_n = 0;
static int g_nt = 1;         /* team size chosen per call (1 = serial path) */
static double g_P[64 * 64];  /* coarse 40-sweep affine map (from 0), built once */
static int g_P_built = 0;

/* ---- generic block/row runners: OpenMP team, or plain loop when serial ---- */

typedef void (*blk_fn)(void *ctx, int64_t is, int64_t jb);
typedef void (*row_fn)(void *ctx, int64_t r);

static void run_blocks2d(int64_t nsl, int64_t nblk, blk_fn fn, void *ctx)
{
    if (g_nt > 1) {
        #pragma omp parallel for collapse(2) schedule(static) num_threads(g_nt)
        for (int64_t is = 0; is < nsl; is++)
            for (int64_t jb = 0; jb < nblk; jb++)
                fn(ctx, is, jb);
    } else {
        for (int64_t is = 0; is < nsl; is++)
            for (int64_t jb = 0; jb < nblk; jb++)
                fn(ctx, is, jb);
    }
}

static void run_rows(int64_t nr, row_fn fn, void *ctx)
{
    if (g_nt > 1) {
        #pragma omp parallel for schedule(static) num_threads(g_nt)
        for (int64_t r = 0; r < nr; r++)
            fn(ctx, r);
    } else {
        for (int64_t r = 0; r < nr; r++)
            fn(ctx, r);
    }
}

/* ---- coarse grid: 40 damped-Jacobi sweeps from zero == affine map P @ f ---- */

/* exact reference-style 1 damped-Jacobi sweep on a 4^3 grid (scalar, reference order) */
static void coarse_sweep_1(double u[64], const double f[64])
{
    double ut[64];
    const int n = NCOARSE;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++) {
                int c = (i * n + j) * n + k;
                double s = 0.0, d = 6.0;
                if (i > 0) s = s + u[c - n * n]; else d = d + 1.0;
                if (i < n - 1) s = s + u[c + n * n]; else d = d + 1.0;
                if (j > 0) s = s + u[c - n]; else d = d + 1.0;
                if (j < n - 1) s = s + u[c + n]; else d = d + 1.0;
                if (k > 0) s = s + u[c - 1]; else d = d + 1.0;
                if (k < n - 1) s = s + u[c + 1]; else d = d + 1.0;
                double h2 = 1.0 / (double)n / (double)n;
                double au = (d * u[c] - s) / h2;
                ut[c] = u[c] + OMEGA * (f[c] - au) * h2 / d;
            }
    for (int c = 0; c < 64; c++) u[c] = ut[c];
}

static void build_coarse_P(void)
{
    if (g_P_built) return;
    for (int j = 0; j < 64; j++) {
        double u[64], f[64];
        memset(u, 0, sizeof u);
        memset(f, 0, sizeof f);
        f[j] = 1.0;
        for (int it = 0; it < NCOARSE_SWEEPS; it++) coarse_sweep_1(u, f);
        for (int i = 0; i < 64; i++) g_P[i * 64 + j] = u[i];
    }
    g_P_built = 1;
}

/* ---- per-block operators: one (i-slice block, j-row block) ---- */

/* One smooth_block covers i = i0, rows jr in [j0, j0+BW):
 * damped Jacobi ut = u + OMEGA*(f - A u)*h2/d with odd-reflection boundaries:
 * a missing neighbor raises d by 1 (no ghost value is added). Interior rows
 * (all six neighbors present) have constant d = 6+delta_i and run a straight
 * vectorized loop; only the j=0 / j=n-1 rows and i-boundary slices take the
 * generic guarded form. Algebra: ut = (1-OMEGA)*u + OMEGA*h2/d*f + OMEGA/d*S. */
static void smooth_block(double *restrict u, const double *restrict f, double *restrict ut,
                         const Lvl *L, int64_t i0, int64_t jb, int di)
{
    const int64_t n = L->n;
    const int64_t row = n * n;
    const int64_t j0 = jb * BW;
    if (j0 >= n) return;
    const int has_im = (i0 > 0), has_ip = (i0 < n - 1);
    const int64_t nreal = n - j0 < BW ? n - j0 : BW;
    double *c0 = u + i0 * row + j0 * n;
    const double *fr0 = f + i0 * row + j0 * n;
    double *ut0 = ut + i0 * row + j0 * n;
    const double *im = c0 - row;   /* i-1 (valid only when has_im) */
    const double *ip = c0 + row;   /* i+1 (valid only when has_ip) */
    for (int64_t t = 0; t < nreal; t++) {
        const int64_t jr = j0 + t;
        if (jr > 0 && jr < n - 1 && has_im && has_ip) {
            const double AF = L->af[0], AS = L->ass[0], AM = 1.0 - OMEGA;
            const double AFk = L->afk[0], ASk = L->ask[0];
            const double *nj = c0 - n;
            const double *pj = c0 + n;
            #pragma omp simd
            for (int64_t k = 1; k < n - 1; k++) {
                double S = c0[k - 1] + c0[k + 1] + nj[k] + pj[k] + im[k] + ip[k];
                ut0[k] = AM * c0[k] + AF * fr0[k] + AS * S;
            }
            double S = c0[1] + nj[0] + pj[0] + im[0] + ip[0];
            ut0[0] = AM * c0[0] + AFk * fr0[0] + ASk * S;
            S = c0[n - 2] + nj[n - 1] + pj[n - 1] + im[n - 1] + ip[n - 1];
            ut0[n - 1] = AM * c0[n - 1] + AFk * fr0[n - 1] + ASk * S;
        } else {
            const int idx = di + ((jr == 0 || jr == n - 1) ? 1 : 0);
            const double AF = L->af[idx], AS = L->ass[idx], AM = 1.0 - OMEGA;
            const double AFk = L->afk[idx], ASk = L->ask[idx];
            const double *nj = c0 - n;
            const double *pj = c0 + n;
            for (int64_t k = 1; k < n - 1; k++) {
                double S = c0[k - 1] + c0[k + 1];
                if (jr < n - 1) S += pj[k];
                if (jr > 0) S += nj[k];
                if (has_im) S += im[k];
                if (has_ip) S += ip[k];
                ut0[k] = AM * c0[k] + AF * fr0[k] + AS * S;
            }
            double S = c0[1];
            if (jr < n - 1) S += pj[0];
            if (jr > 0) S += nj[0];
            if (has_im) S += im[0];
            if (has_ip) S += ip[0];
            ut0[0] = AM * c0[0] + AFk * fr0[0] + ASk * S;
            S = c0[n - 2];
            if (jr < n - 1) S += pj[n - 1];
            if (jr > 0) S += nj[n - 1];
            if (has_im) S += im[n - 1];
            if (has_ip) S += ip[n - 1];
            ut0[n - 1] = AM * c0[n - 1] + AFk * fr0[n - 1] + ASk * S;
        }
        c0 += n; fr0 += n; ut0 += n; im += n; ip += n;
    }
}

static void resid_block(const double *restrict u, const double *restrict f,
                        double *restrict r, const Lvl *L, int64_t i0, int64_t jb, double delta_i)
{
    const int64_t n = L->n;
    const int64_t row = n * n;
    const double invh2 = L->invh2;
    const int64_t j0 = jb * BW;
    if (j0 >= n) return;
    const int has_im = (i0 > 0), has_ip = (i0 < n - 1);
    const int64_t nreal = n - j0 < BW ? n - j0 : BW;
    const double *c0 = u + i0 * row + j0 * n;
    const double *fr0 = f + i0 * row + j0 * n;
    double *r0 = r + i0 * row + j0 * n;
    const double *im = c0 - row;
    const double *ip = c0 + row;
    for (int64_t t = 0; t < nreal; t++) {
        const int64_t jr = j0 + t;
        const double *nj = c0 - n;
        const double *pj = c0 + n;
        if (jr > 0 && jr < n - 1 && has_im && has_ip) {
            const double B = (6.0 + delta_i) * invh2, C = invh2;
            #pragma omp simd
            for (int64_t k = 1; k < n - 1; k++) {
                double S = c0[k - 1] + c0[k + 1] + nj[k] + pj[k] + im[k] + ip[k];
                r0[k] = fr0[k] - B * c0[k] + C * S;
            }
            r0[0] = fr0[0] - (B + invh2) * c0[0] + C * (c0[1] + nj[0] + pj[0] + im[0] + ip[0]);
            r0[n - 1] = fr0[n - 1] - (B + invh2) * c0[n - 1]
                        + C * (c0[n - 2] + nj[n - 1] + pj[n - 1] + im[n - 1] + ip[n - 1]);
        } else {
            const double d = 6.0 + delta_i + (jr == 0) + (jr == n - 1);
            const double B = d * invh2, C = invh2, Bk = (d + 1.0) * invh2;
            for (int64_t k = 1; k < n - 1; k++) {
                double S = c0[k - 1] + c0[k + 1];
                if (jr < n - 1) S += pj[k];
                if (jr > 0) S += nj[k];
                if (has_im) S += im[k];
                if (has_ip) S += ip[k];
                r0[k] = fr0[k] - B * c0[k] + C * S;
            }
            double S = c0[1];
            if (jr < n - 1) S += pj[0];
            if (jr > 0) S += nj[0];
            if (has_im) S += im[0];
            if (has_ip) S += ip[0];
            r0[0] = fr0[0] - Bk * c0[0] + C * S;
            S = c0[n - 2];
            if (jr < n - 1) S += pj[n - 1];
            if (jr > 0) S += nj[n - 1];
            if (has_im) S += im[n - 1];
            if (has_ip) S += ip[n - 1];
            r0[n - 1] = fr0[n - 1] - Bk * c0[n - 1] + C * S;
        }
        c0 += n; fr0 += n; r0 += n; im += n; ip += n;
    }
}

typedef struct {
    double *u, *ut;
    const double *fh;
    const Lvl *L;
    int S;
} Sctx;

static void blk_smooth(void *p, int64_t is, int64_t jb)
{
    Sctx *c = (Sctx *)p;
    const Lvl *L = c->L;
    const int64_t n = L->n;
    int64_t i1 = (is + 1) * (int64_t)c->S;
    if (i1 > n) i1 = n;
    for (int64_t i = is * (int64_t)c->S; i < i1; i++) {
        int di = (i > 0 && i < n - 1) ? 0 : 1;
        smooth_block(c->u, c->fh, c->ut, L, i, jb, di);
    }
}

typedef struct {
    const double *u, *fh;
    double *r;
    const Lvl *L;
    int S;
} Rctx;

static void blk_resid(void *p, int64_t is, int64_t jb)
{
    Rctx *c = (Rctx *)p;
    const Lvl *L = c->L;
    const int64_t n = L->n;
    int64_t i1 = (is + 1) * (int64_t)c->S;
    if (i1 > n) i1 = n;
    for (int64_t i = is * (int64_t)c->S; i < i1; i++) {
        double di = (i > 0 && i < n - 1) ? 0.0 : 1.0;
        resid_block(c->u, c->fh, c->r, L, i, jb, di);
    }
}

/* First Jacobi sweep from a zero field: ut = OMEGA * h2 / d * f elementwise.
   Coarse levels restart at zero on every cycle and the fine level starts at
   zero on cycle 0, so this replaces one full sweep (7 reads + 1 write per
   element) with one read + one write, and removes the zeroing pass. */
static void blk_scale(void *p, int64_t is, int64_t jb)
{
    Sctx *c = (Sctx *)p;
    const Lvl *L = c->L;
    const int64_t n = L->n, row = n * n;
    int64_t i1 = (is + 1) * (int64_t)c->S;
    if (i1 > n) i1 = n;
    const int64_t j0 = jb * BW;
    if (j0 >= n) return;
    const int64_t nreal = n - j0 < BW ? n - j0 : BW;
    for (int64_t i = is * (int64_t)c->S; i < i1; i++) {
        const int di = (i > 0 && i < n - 1) ? 0 : 1;
        const double cv = L->af[di], c0v = L->af[di + 1];
        const double cvb = L->af[di + 1], c0vb = L->af[di + 2];
        const double *f0 = c->fh + i * row + j0 * n;
        double *o = c->ut + i * row + j0 * n;
        for (int64_t t = 0; t < nreal; t++) {
            const int bd = (j0 + t == 0 || j0 + t == n - 1);
            const double cvx = bd ? cvb : cv;
            const double c0x = bd ? c0vb : c0v;
            #pragma omp simd
            for (int64_t k = 1; k < n - 1; k++) o[k] = f0[k] * cvx;
            o[0] = f0[0] * c0x;
            o[n - 1] = f0[n - 1] * c0x;
            f0 += n; o += n;
        }
    }
}

/* ---- prolongation: 1-D factors of the 27/9/3/1 stencil, three passes ---- */

/* x[0]=.5 v0; x[2m]=.75 v[m]+.25 v[m-1]; x[2m+1]=.75 v[m]+.25 v[m+1]; x[-1]=.5 v[-1]
   out has length n (fine), in has length nc = n/2. */
static void filt_row(double *out, const double *in, int64_t n, int add)
{
    out[0] = add ? out[0] + 0.5 * in[0] : 0.5 * in[0];
    for (int64_t k = 1; k < n - 1; k++) {
        int64_t p = k / 2;
        double v;
        if (k & 1) v = 0.75 * in[p] + 0.25 * in[p + 1];
        else       v = 0.75 * in[p] + 0.25 * in[p - 1];
        out[k] = add ? out[k] + v : v;
    }
    out[n - 1] = add ? out[n - 1] + 0.5 * in[(n - 1) / 2] : 0.5 * in[(n - 1) / 2];
}

static void row2tap(double *restrict out, const double *restrict A, const double *restrict B,
                    int64_t n, int add)
{
    #pragma omp simd
    for (int64_t k = 0; k < n; k++) {
        double v = 0.75 * A[k] + 0.25 * B[k];
        out[k] = add ? out[k] + v : v;
    }
}

typedef struct {
    double *tmp, *uf;
    const double *uc;
    int64_t n;
} Pctx;

/* k pass (fine rows of slice i) + j pass (in place over the slice, j descending:
 * source rows j/2, j/2+-1 are all < j, still unmodified). */
static void row_prolong_kj(void *p, int64_t i)
{
    Pctx *c = (Pctx *)p;
    const int64_t n = c->n, nc = n / 2, r = n * n;
    double *base = c->tmp + i * r;
    for (int64_t j = 0; j < nc; j++) filt_row(base + j * n, c->uc + (i * nc + j) * nc, n, 0);
    for (int64_t k = 0; k < n; k++) base[(n - 1) * n + k] = 0.5 * base[(nc - 1) * n + k]; /* j = n-1 */
    for (int64_t j = n - 2; j >= 2; j--) {
        if (j & 1) row2tap(base + j * n, base + (j / 2) * n, base + (j / 2 + 1) * n, n, 0);
        else row2tap(base + j * n, base + (j / 2) * n, base + (j / 2 - 1) * n, n, 0);
    }
    for (int64_t k = 0; k < n; k++) base[n + k] = 0.75 * base[k] + 0.25 * base[n + k]; /* j = 1 */
    for (int64_t k = 0; k < n; k++) base[k] = 0.5 * base[k]; /* j = 0 */
}

/* i pass: fine slice i (n*n doubles) += P_i over T2 slices (stride n*n). */
static void row_prolong_i(void *p, int64_t i)
{
    Pctx *c = (Pctx *)p;
    const int64_t n = c->n, nc = n / 2, r = n * n;
    double *o = c->uf + i * r;
    if (i == 0) {
        const double *A = c->tmp;
        #pragma omp simd
        for (int64_t k = 0; k < r; k++) o[k] += 0.5 * A[k];
    } else if (i == n - 1) {
        const double *A = c->tmp + (nc - 1) * r;
        #pragma omp simd
        for (int64_t k = 0; k < r; k++) o[k] += 0.5 * A[k];
    } else if (i & 1)
        row2tap(o, c->tmp + (i / 2) * r, c->tmp + (i / 2 + 1) * r, r, 1);
    else
        row2tap(o, c->tmp + (i / 2) * r, c->tmp + (i / 2 - 1) * r, r, 1);
}

/* uf, uc, tmp are level regions (already offset). T1/T2 live in tmp, fine-sized. */
static void prolong_lvl(double *restrict uf, const double *restrict uc, double *restrict tmp,
                        const Lvl *L)
{
    const int64_t n = L->n, nc = n / 2;
    Pctx c = { tmp, uf, uc, n };
    run_rows(nc, row_prolong_kj, &c);
    run_rows(n, row_prolong_i, &c);
}

/* ---- restriction: full weighting, each coarse cell averages its 8 children ---- */

typedef struct {
    const double *r;
    double *o;
    int64_t nc, nf;
} RestrictCtx;

static void row_restrict(void *p, int64_t rr)
{
    RestrictCtx *c = (RestrictCtx *)p;
    const int64_t nc = c->nc, nf = c->nf, n2 = nf * nf;
    int64_t i = rr / nc, j = rr % nc;
    const double *a = c->r + (2 * i) * n2 + (2 * j) * nf;
    const double *b = a + nf;
    const double *cc = a + n2;
    const double *d = b + n2;
    double *o_ = c->o + (i * nc + j) * nc;
    for (int64_t k = 0; k < nc; k++)
        o_[k] = 0.125 * (a[2 * k] + a[2 * k + 1] + b[2 * k] + b[2 * k + 1]
                         + cc[2 * k] + cc[2 * k + 1] + d[2 * k] + d[2 * k + 1]);
}

static void restrict_lvl(const double *restrict rf, double *restrict fc, const Lvl *L)
{
    RestrictCtx c = { rf, fc, L->nc, L->n };
    run_rows(L->nc * L->nc, row_restrict, &c);
}

/* ---- level drivers ---- */

static int slice_step(int64_t n)
{
    if (n >= 128) return 8;
    if (n >= 64) return 2;
    return 1;
}

/* Jacobi sweeps: reads *u, writes ut, swaps per sweep -- no n**3 copy-back.
   On return *u points at the freshest solution; the other buffer is scratch. */
static void smooth_lvl(double **u, const double *fh, double *ut, Lvl *L, int nsweeps)
{
    const int64_t n = L->n;
    const int64_t nblk = (n + BW - 1) / BW;
    const int S = slice_step(n);
    const int64_t nsl = (n + S - 1) / S;
    Sctx c = { *u, ut, fh, L, S };
    for (int s = 0; s < nsweeps; s++) {
        run_blocks2d(nsl, nblk, blk_smooth, &c);
        double *t = c.u;
        c.u = c.ut;
        c.ut = t;
    }
    *u = c.u;
}

static void resid_lvl(const double *u, const double *fh, double *r, Lvl *L)
{
    const int64_t n = L->n;
    const int64_t nblk = (n + BW - 1) / BW;
    const int S = slice_step(n);
    const int64_t nsl = (n + S - 1) / S;
    Rctx c = { u, fh, r, L, S };
    run_blocks2d(nsl, nblk, blk_resid, &c);
}

static void scale_f_lvl(double *ut, const double *fh, Lvl *L)
{
    const int64_t n = L->n;
    const int64_t nblk = (n + BW - 1) / BW;
    const int S = slice_step(n);
    const int64_t nsl = (n + S - 1) / S;
    Sctx c = { NULL, ut, fh, L, S };
    run_blocks2d(nsl, nblk, blk_scale, &c);
}

/* ---- whole-buffer copies (64 KiB chunks) ---- */

typedef struct {
    double *d;
    const double *s;
    int64_t len;
} CopyCtx;

#define COPY_CHUNK 8192
static void row_copy(void *p, int64_t r)
{
    CopyCtx *c = (CopyCtx *)p;
    int64_t i0 = r * COPY_CHUNK;
    int64_t i1 = i0 + COPY_CHUNK;
    if (i1 > c->len) i1 = c->len;
    double *d = c->d + i0;
    const double *s = c->s + i0;
    #pragma omp simd
    for (int64_t i = 0; i < i1 - i0; i++) d[i] = s[i];
}

static void copy_bufs(double *dst, const double *src, int64_t len)
{
    CopyCtx c = { dst, src, len };
    run_rows((len + COPY_CHUNK - 1) / COPY_CHUNK, row_copy, &c);
}

static void zero_bufs(double *dst, int64_t len)
{
    CopyCtx c = { dst, NULL, len };
    int64_t nr = (len + COPY_CHUNK - 1) / COPY_CHUNK;
    if (g_nt > 1) {
        #pragma omp parallel for schedule(static) num_threads(g_nt)
        for (int64_t r = 0; r < nr; r++) {
            int64_t i0 = r * COPY_CHUNK, i1 = i0 + COPY_CHUNK;
            if (i1 > c.len) i1 = c.len;
            memset(dst + i0, 0, (size_t)(i1 - i0) * 8);
        }
    } else {
        memset(dst, 0, (size_t)len * 8);
    }
}

/* ---- driver ---- */

void mg_vcycle_fp64(const double *restrict f, double *restrict u, const int64_t N,
                    const int64_t ncycles, uint8_t *restrict workspace,
                    const int64_t workspace_size)
{
    build_coarse_P();
    if (ncycles <= 0) return;

    int64_t nlevels = 1;
    {
        int64_t e = N;
        while (e > NCOARSE) { e >>= 1; nlevels++; }
    }
    Lvl L[LMAX];
    int64_t pos = 0, e = N;
    for (int l = 0; l < nlevels; l++) {
        L[l].n = e;
        L[l].off = pos;
        pos += e * e * e;
        L[l].invh2 = (double)e * (double)e;
        {
            const double h2 = 1.0 / L[l].invh2;
            for (int d6 = 0; d6 < 4; d6++) {
                const double d = 6.0 + d6;
                L[l].af[d6] = OMEGA * h2 / d;
                L[l].ass[d6] = OMEGA / d;
                L[l].afk[d6] = OMEGA * h2 / (d + 1.0);
                L[l].ask[d6] = OMEGA / (d + 1.0);
            }
        }
        if (l + 1 < nlevels) {
            L[l].nc = e >> 1;
            L[l].offc = 0; /* filled after all offsets known */
        } else {
            L[l].nc = 0;
            L[l].offc = 0;
        }
        e >>= 1;
    }
    for (int l = 0; l + 1 < nlevels; l++) L[l].offc = L[l + 1].off;

    int64_t need = pos * 3 + 64;
    double *buf;
    if (workspace && workspace_size >= need * 8)
        buf = (double *)workspace;
    else {
        if (g_fb_n < need) {
            if (g_fb) free(g_fb);
            g_fb_n = (need + 7) & ~(int64_t)7;
            g_fb = (double *)aligned_alloc(64, (size_t)g_fb_n * 8);
            if (!g_fb) return;
        }
        buf = g_fb;
    }
    double *u1 = buf;
    double *fh = buf + pos;
    double *th = buf + 2 * pos;

    zero_bufs(u1, pos);
    copy_bufs(fh, f, N * N * N);

    const int64_t nl = nlevels - 1;
    {
        int mx = omp_get_max_threads();
        int k = (N >= 128) ? 12 : 1;
        if (k > mx) k = mx;
        g_nt = (k > 0) ? k : 1;
    }
    /* current u-buffer per level; smooth swaps with its partner each sweep */
    double *cur[LMAX];
    for (int l = 0; l < nlevels; l++) cur[l] = u1 + L[l].off;

    for (int c = 0; c < (int)ncycles; c++) {
        /* down */
        for (int l = 0; l < nl; l++) {
            int from_zero = (c == 0) || (l > 0); /* coarse levels restart at zero each cycle */
            double *other = (cur[l] == u1 + L[l].off) ? th + L[l].off : u1 + L[l].off;
            if (from_zero) {
                scale_f_lvl(other, fh + L[l].off, &L[l]); /* sweep 1 without reading u */
                cur[l] = other;
                other = (cur[l] == u1 + L[l].off) ? th + L[l].off : u1 + L[l].off;
            }
            smooth_lvl(&cur[l], fh + L[l].off, other, &L[l], from_zero ? NU1 - 1 : NU1);
            other = (cur[l] == u1 + L[l].off) ? th + L[l].off : u1 + L[l].off; /* now the free one */
            resid_lvl(cur[l], fh + L[l].off, other, &L[l]);
            restrict_lvl(other, fh + L[l].offc, &L[l]);
        }
        /* coarsest: u = P @ f (40 damped-Jacobi sweeps from zero), full overwrite */
        {
            const double *fc = fh + L[nl].off;
            double *uc = cur[nl];
            for (int i = 0; i < 64; i++) {
                const double *P = g_P + i * 64;
                double s = 0.0;
                #pragma omp simd
                for (int j = 0; j < 64; j++) s += P[j] * fc[j];
                uc[i] = s;
            }
        }
        /* up */
        for (int l = nl - 1; l >= 0; l--) {
            double *other = (cur[l] == u1 + L[l].off) ? th + L[l].off : u1 + L[l].off;
            prolong_lvl(cur[l], cur[l + 1], other, &L[l]); /* tmp = free level-l buffer */
            smooth_lvl(&cur[l], fh + L[l].off, other, &L[l], NU2); /* flips back */
        }
    }

    /* net swaps per cycle are even, so the fine solution sits in u1 */
    copy_bufs(u, u1, N * N * N);
}
