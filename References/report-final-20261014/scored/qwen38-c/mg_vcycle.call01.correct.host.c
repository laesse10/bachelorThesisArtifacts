/* Geometric multigrid V-cycle, cell-centered 3-D grid (HPGMG-style).
 * Optimized C: OpenMP over (i-slice block, j-row block), vectorized over j.
 * Semantics match mg_vcycle_numpy.py (damped Jacobi w=6/7, odd-reflection ghosts,
 * full weighting, 27/9/3/1 trilinear prolongation, 40-sweep coarse solve).
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
#define BC 4    /* coarse tile rows over j in restrict */

typedef struct {
    int64_t n, off;          /* fine edge and offset */
    int64_t nc, offc;        /* coarse edge and offset (0 if coarsest) */
    double invh2;            /* 1/h2 */
    double dtab[2 * 512 + 2];/* d per combined k(0/1)/j(row 0 / n-1 / interior) */
} Lvl;

static double *g_fb = NULL;
static int g_nt = 1; /* thread count chosen per call */ /* fallback scratch */
static int64_t g_fb_n = 0;
static double g_P[64 * 64]; /* coarse 40-sweep affine map (from 0), built once */
static int g_P_built = 0;

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

/* ---- level kernels: block = BW j-rows x n k ----
 * Lane t uses row jr (clamped to [0, n-1]); missing neighbors are dropped from
 * the stencil sum and raise d by 1 (odd-reflection ghost). dtab[t] = 6 + (jr==0)
 * + (jr==n-1). k=0 and k=n-1 rows are patched separately (+1 to d, one fewer
 * k-neighbor). delta_i = 1 on i-boundary planes. */

static void smooth_block(double *restrict u, const double *restrict f, double *restrict ut,
                         const Lvl *L, int64_t i0, int64_t jb, double delta_i)
{
    const int64_t n = L->n;
    const int64_t row = n * n;
    const double invh2 = L->invh2, h2 = 1.0 / L->invh2;
    const int64_t j0 = jb * BW;
    const int has_im = (i0 > 0), has_ip = (i0 < n - 1);
    for (int64_t t = 0; t < BW; t++) {
        int64_t jr = j0 + t;
        if (jr < 0) jr = 0;
        if (jr >= n) jr = n - 1;
        const int has_jp = (jr < n - 1);
        const int has_jm = (jr > 0);
        const double *ur0 = u + i0 * row + jr * n;
        const double *ur1 = ur0 + n;
        const double *ul1 = u + (i0 - 1) * row + jr * n;
        const double *ul2 = u + (i0 + 1) * row + jr * n;
        const double *ur0m = ur0 - n;
        double *ut0 = ut + i0 * row + jr * n;
        const double *fr0 = f + i0 * row + jr * n;
        const double d = L->dtab[jr] + delta_i;
        if (has_im && has_ip && has_jp && has_jm) {
            #pragma omp simd
            for (int64_t k = 1; k < n - 1; k++) {
                double S = ur0[k - 1] + ur0[k + 1] + ur1[k] + ur0m[k] + ul1[k] + ul2[k];
                double au = (d * ur0[k] - S) * invh2;
                ut0[k] = ur0[k] + OMEGA * (fr0[k] - au) * h2 / d;
            }
        } else {
            for (int64_t k = 1; k < n - 1; k++) {
                double S = ur0[k - 1] + ur0[k + 1];
                if (has_jp) S += ur1[k];
                if (has_jm) S += ur0m[k];
                if (has_im) S += ul1[k];
                if (has_ip) S += ul2[k];
                double au = (d * ur0[k] - S) * invh2;
                ut0[k] = ur0[k] + OMEGA * (fr0[k] - au) * h2 / d;
            }
        }
        /* k = 0 and k = n-1 planes */
        {
            double S = ur0[1];
            if (has_jp) S += ur1[0];
            if (has_jm) S += ur0m[0];
            if (has_im) S += ul1[0];
            if (has_ip) S += ul2[0];
            double dk = d + 1.0;
            ut0[0] = ur0[0] + OMEGA * (fr0[0] - (dk * ur0[0] - S) * invh2) * h2 / dk;
            S = ur0[n - 2];
            if (has_jp) S += ur1[n - 1];
            if (has_jm) S += ur0m[n - 1];
            if (has_im) S += ul1[n - 1];
            if (has_ip) S += ul2[n - 1];
            ut0[n - 1] = ur0[n - 1] + OMEGA * (fr0[n - 1] - (dk * ur0[n - 1] - S) * invh2) * h2 / dk;
        }
    }
}

static void resid_block(const double *restrict u, const double *restrict f,
                        double *restrict r, const Lvl *L, int64_t i0, int64_t jb, double delta_i)
{
    const int64_t n = L->n;
    const int64_t row = n * n;
    const double invh2 = L->invh2;
    const int64_t j0 = jb * BW;
    const int has_im = (i0 > 0), has_ip = (i0 < n - 1);
    for (int64_t t = 0; t < BW; t++) {
        int64_t jr = j0 + t;
        if (jr < 0) jr = 0;
        if (jr >= n) jr = n - 1;
        const int has_jp = (jr < n - 1);
        const int has_jm = (jr > 0);
        const double *ur0 = u + i0 * row + jr * n;
        const double *ur1 = ur0 + n;
        const double *ul1 = u + (i0 - 1) * row + jr * n;
        const double *ul2 = u + (i0 + 1) * row + jr * n;
        const double *ur0m = ur0 - n;
        double *r0 = r + i0 * row + jr * n;
        const double *fr0 = f + i0 * row + jr * n;
        const double d = L->dtab[jr] + delta_i;
        if (has_im && has_ip && has_jp && has_jm) {
            #pragma omp simd
            for (int64_t k = 1; k < n - 1; k++) {
                double S = ur0[k - 1] + ur0[k + 1] + ur1[k] + ur0m[k] + ul1[k] + ul2[k];
                r0[k] = fr0[k] - (d * ur0[k] - S) * invh2;
            }
        } else {
            for (int64_t k = 1; k < n - 1; k++) {
                double S = ur0[k - 1] + ur0[k + 1];
                if (has_jp) S += ur1[k];
                if (has_jm) S += ur0m[k];
                if (has_im) S += ul1[k];
                if (has_ip) S += ul2[k];
                r0[k] = fr0[k] - (d * ur0[k] - S) * invh2;
            }
        }
        {
            double S = ur0[1];
            if (has_jp) S += ur1[0];
            if (has_jm) S += ur0m[0];
            if (has_im) S += ul1[0];
            if (has_ip) S += ul2[0];
            double dk = d + 1.0;
            r0[0] = fr0[0] - (dk * ur0[0] - S) * invh2;
            S = ur0[n - 2];
            if (has_jp) S += ur1[n - 1];
            if (has_jm) S += ur0m[n - 1];
            if (has_im) S += ul1[n - 1];
            if (has_ip) S += ul2[n - 1];
            r0[n - 1] = fr0[n - 1] - (dk * ur0[n - 1] - S) * invh2;
        }
    }
}

/* ---- restrict: full weighting = uniform average of the 8 children ---- */
static void restrict_lvl(const double *restrict rf, double *restrict fc, const Lvl *L)
{
    const int64_t nc = L->nc, nf = L->n, n2 = nf * nf;
    const double *r = rf;
    double *o = fc;
    #pragma omp parallel for collapse(2) schedule(static) num_threads(g_nt)
    for (int64_t i = 0; i < nc; i++)
        for (int64_t j = 0; j < nc; j++) {
            const double *a = r + (2 * i) * n2 + (2 * j) * nf;
            const double *b = a + nf;
            const double *c = a + n2;
            const double *d = b + n2;
            double *o_ = o + (i * nc + j) * nc;
            for (int64_t k = 0; k < nc; k++)
                o_[k] = 0.125 * (a[2 * k] + a[2 * k + 1] + b[2 * k] + b[2 * k + 1]
                                 + c[2 * k] + c[2 * k + 1] + d[2 * k] + d[2 * k + 1]);
        }
}

/* ---- prolongation: 3-pass factorized trilinear (27/9/3/1, odd-reflection) ----
 * 1-D factor (exact): x[0] = 0.5*u[0]; x[n-1] = 0.5*u[nc-1];
 * x[2m] = 0.75*u[m] + 0.25*u[m-1]; x[2m+1] = 0.75*u[m] + 0.25*u[m+1].
 * Product over the 3 dims = w/64 of the reference (edge: (3-1)/4 = 1/2).
 * Passes: k: tmp[k] = filt(uf row); j: in-place descending; i: uf += filt(tmp).
 */
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
    for (int64_t k = 0; k < n; k++) {
        double v = 0.75 * A[k] + 0.25 * B[k];
        out[k] = add ? out[k] + v : v;
    }
}

static void prolong_lvl(double *restrict uf, const double *restrict uc, double *restrict tmp,
                        const Lvl *L)
{
    const int64_t n = L->n, nc = n / 2;
    const int64_t r = n * n; /* per-i slice: n rows of n doubles */
    /* pass k: P_k over each coarse row -> T1[i][j] fine row (len n) at (i*n+j)*n,
       so each i slice owns a private n*n block (the in-place j pass must not alias
       another slice's T1 rows) */
    /* pass k then pass j, per i slice (one region; the j pass only reads rows
       of its own slice, which the k pass just finished) */
    #pragma omp parallel for schedule(static) num_threads(g_nt)
    for (int64_t i = 0; i < nc; i++) {
        double *base = tmp + i * r;
        for (int64_t j = 0; j < nc; j++) filt_row(base + j * n, uc + (i * nc + j) * nc, n, 0);
        for (int64_t k = 0; k < n; k++) base[(n - 1) * n + k] = 0.5 * base[(nc - 1) * n + k]; /* j = n-1 */
        for (int64_t j = n - 2; j >= 2; j--) {
            if (j & 1) row2tap(base + j * n, base + (j / 2) * n, base + (j / 2 + 1) * n, n, 0);
            else row2tap(base + j * n, base + (j / 2) * n, base + (j / 2 - 1) * n, n, 0);
        }
        for (int64_t k = 0; k < n; k++) base[n + k] = 0.75 * base[k] + 0.25 * base[n + k]; /* j = 1 */
        for (int64_t k = 0; k < n; k++) base[k] = 0.5 * base[k]; /* j = 0 */
    }
    /* pass i: uf slice i (len n*n) += P_i over T2 slices (stride r) */
    #pragma omp parallel for schedule(static) num_threads(g_nt)
    for (int64_t i = 0; i < n; i++) {
        double *o = uf + i * n * n;
        if (i == 0) {
            const double *A = tmp;
            for (int64_t k = 0; k < n * n; k++) o[k] += 0.5 * A[k];
        } else if (i == n - 1) {
            const double *A = tmp + (nc - 1) * r;
            for (int64_t k = 0; k < n * n; k++) o[k] += 0.5 * A[k];
        } else if (i & 1)
            row2tap(o, tmp + (i / 2) * r, tmp + (i / 2 + 1) * r, n * n, 1);
        else
            row2tap(o, tmp + (i / 2) * r, tmp + (i / 2 - 1) * r, n * n, 1);
    }
}

/* ---- level drivers ---- */

static int slice_step(int64_t n)
{
    if (n >= 128) return 8;
    if (n >= 64) return 2;
    return 1;
}

/* Jacobi sweep: reads *u, writes ut, then swaps -- no n**3 copy-back.
   On return *u points at the freshest solution; the other buffer is scratch. */
/* First Jacobi sweep from a zero field: ut = OMEGA * h2 / d * f elementwise.
   Coarse levels are zeroed before every down phase and the fine level starts
   at zero on cycle 0, so this replaces one full sweep (7 reads + 1 write per
   element) with one read + one write, and removes the zeroing pass. */
static void scale_f_lvl(double *restrict ut, const double *restrict fh, Lvl *L)
{
    const int64_t n = L->n;
    const int64_t nblk = (n + BW - 1) / BW;
    const int S = slice_step(n);
    const int64_t nsl = (n + S - 1) / S;
    const double h2 = 1.0 / L->invh2;
    #pragma omp parallel for collapse(2) schedule(static) num_threads(g_nt)
    for (int64_t is = 0; is < nsl; is++)
        for (int64_t jb = 0; jb < nblk; jb++) {
            int64_t i1 = (is + 1) * S;
            if (i1 > n) i1 = n;
            for (int64_t i = is * S; i < i1; i++) {
                const double di = (i > 0 && i < n - 1) ? 0.0 : 1.0;
                for (int64_t t = 0; t < BW; t++) {
                    int64_t jr = jb * BW + t;
                    if (jr >= n) jr = n - 1;
                    const double d = L->dtab[jr] + di;
                    const double c = OMEGA * h2 / d, c0 = OMEGA * h2 / (d + 1.0);
                    const double *f0 = fh + i * n * n + jr * n;
                    double *o = ut + i * n * n + jr * n;
                    #pragma omp simd
                    for (int64_t k = 1; k < n - 1; k++) o[k] = f0[k] * c;
                    o[0] = f0[0] * c0;
                    o[n - 1] = f0[n - 1] * c0;
                }
            }
        }
}

static void smooth_lvl(double **u, const double *fh, double *ut, Lvl *L, int nsweeps)
{
    const int64_t n = L->n;
    const int64_t nblk = (n + BW - 1) / BW;
    const int S = slice_step(n);
    const int64_t nsl = (n + S - 1) / S;
    for (int s = 0; s < nsweeps; s++) {
        #pragma omp parallel for collapse(2) schedule(static) num_threads(g_nt)
        for (int64_t is = 0; is < nsl; is++)
            for (int64_t jb = 0; jb < nblk; jb++) {
                int64_t i1 = (is + 1) * S;
                if (i1 > n) i1 = n;
                for (int64_t i = is * S; i < i1; i++) {
                    double di = (i > 0 && i < n - 1) ? 0.0 : 1.0;
                    smooth_block(*u, fh, ut, L, i, jb, di);
                }
            }
        double *t = *u;
        *u = ut;
        ut = t;
    }
}

static void resid_lvl(const double *u, const double *fh, double *r, Lvl *L)
{
    const int64_t n = L->n;
    const int64_t nblk = (n + BW - 1) / BW;
    const int S = slice_step(n);
    const int64_t nsl = (n + S - 1) / S;
    #pragma omp parallel for collapse(2) schedule(static) num_threads(g_nt)
    for (int64_t is = 0; is < nsl; is++)
        for (int64_t jb = 0; jb < nblk; jb++) {
            int64_t i1 = (is + 1) * S;
            if (i1 > n) i1 = n;
            for (int64_t i = is * S; i < i1; i++) {
                double di = (i > 0 && i < n - 1) ? 0.0 : 1.0;
                resid_block(u, fh, r, L, i, jb, di);
            }
        }
}

/* ---- main kernel ---- */

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

    for (int64_t l = 0; l < nlevels; l++) {
        int64_t n = L[l].n;
        for (int64_t jr = 0; jr < n; jr++)
            L[l].dtab[jr] = 6.0 + (jr == 0) + (jr == n - 1); /* indexed by row jr */
    }

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

    #pragma omp parallel for schedule(static) num_threads(g_nt)
    for (int64_t i = 0; i < pos; i++) u1[i] = 0.0;
    #pragma omp parallel for schedule(static) num_threads(g_nt)
    for (int64_t i = 0; i < N * N * N; i++) fh[i] = f[i];

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
    #pragma omp parallel for schedule(static) num_threads(g_nt)
    for (int64_t i = 0; i < N * N * N; i++) u[i] = u1[i];
}
