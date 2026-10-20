/* JFNK solve of the 2-D Bratu problem on the unit square.
 *
 *   -Laplacian(u) - lam*exp(u) = 0,  u = 0 on the boundary.
 *
 * OpenMP port of jfnk_bratu_numpy.py preserving the element-wise arithmetic
 * (expressions, evaluation order and row-order reductions) so the Newton/GMRES
 * iterate tracks the compiled reference bit-for-bit:
 *   - scaled finite-difference Jacobian (Pernice & Walker 1998)
 *   - GMRES(restart), modified Gram-Schmidt, incremental Givens rotations
 *   - Newton gate  fnow <= 1e-14 * f0,  max_newton steps
 *
 * Parallelism: one persistent team per call. Every thread owns a contiguous
 * block of rows (static partition) and works on it alone; explicit barriers
 * separate producer/consumer passes. Reductions store per-row partials, then
 * every thread combines them in plain row order, exactly as the serial
 * reference does. Control flow (Newton/GMRES break tests) only reads data
 * established at a barrier, so all threads agree on the path taken.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef RESTART
#define RESTART 50
#endif
#ifndef MAX_NEWTON
#define MAX_NEWTON 20
#endif
#ifndef INNER_TOL
#define INNER_TOL 1.0e-4
#endif

#define SQRT_MACHEPS 1.4901161193847656e-08 /* sqrt(2^-52) */

typedef struct {
    double *F, *du, *up, *w;        /* single grid buffers           */
    double *Q;                       /* (RESTART+1) grid Krylov basis */
    double *H;                       /* (RESTART+1) x RESTART         */
    double *cs, *sn;                 /* RESTART each                  */
    double *g;                       /* RESTART+1                     */
    double *y;                       /* RESTART                       */
    double *rows;                    /* N row partials for reductions */
} Work;

/* static row partition identical to schedule(static,1) */
static inline void row_range(int64_t tid, int64_t nt, int64_t N, int64_t *i0, int64_t *i1) {
    const int64_t base = N / nt;
    const int64_t rem = N % nt;
    *i0 = base * tid + (tid < rem ? tid : rem);
    *i1 = *i0 + base + (tid < rem ? 1 : 0);
}

/* per-row squares/ products into rows[]; caller must barrier before combine */
static inline void k_rowsq(const double *A, int64_t N, int64_t tid, int64_t nt, double *rows) {
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        double t = 0.0;
        const double *r = A + i * N;
        for (int64_t j = 0; j < N; j++) t += r[j] * r[j];
        rows[i] = t;
    }
}

static inline double k_combine(int64_t N, double *rows) {
    double s = 0.0;
    for (int64_t i = 0; i < N; i++) s += rows[i];
    return s;
}

static inline void k_rows2(const double *A, const double *B, int64_t N, int64_t tid, int64_t nt,
                           double *rows) {
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        double t = 0.0;
        const double *ra = A + i * N;
        const double *rb = B + i * N;
        for (int64_t j = 0; j < N; j++) t += ra[j] * rb[j];
        rows[i] = t;
    }
}

/* F = F(u): 5-point stencil interior rows; frame is copied afterwards. */
static inline void k_resid_chunk(const double *u, double *F, int64_t N, double lam,
                                 int64_t tid, int64_t nt) {
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    i0 = i0 > 1 ? i0 : 1;
    i1 = i1 < N - 1 ? i1 : N - 1;
    for (int64_t i = i0; i < i1; i++) {
        const double *uc = u + i * N;
        const double *un = uc - N;
        const double *ud = uc + N;
        double *Fc = F + i * N;
        for (int64_t j = 1; j < N - 1; j++)
            Fc[j] = (4.0 * uc[j] - un[j] - ud[j] - uc[j - 1] - uc[j + 1]) / h2
                    - lam * exp(uc[j]);
    }
}

static inline void k_resid_frame(const double *u, double *F, int64_t N) {
    for (int64_t j = 0; j < N; j++) { F[j] = u[j]; F[(N - 1) * N + j] = u[(N - 1) * N + j]; }
    for (int64_t i = 1; i < N - 1; i++) {
        F[i * N] = u[i * N];
        F[i * N + N - 1] = u[i * N + N - 1];
    }
}

static inline void k_axp_chunk(double *y, const double *x, double a, int64_t N, int64_t tid,
                               int64_t nt) {
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        double *yc = y + i * N;
        const double *xc = x + i * N;
        for (int64_t j = 0; j < N; j++) yc[j] = yc[j] + a * xc[j];
    }
}

static inline void k_scale_chunk(double *y, const double *x, double a, int64_t N, int64_t tid,
                                 int64_t nt) {
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        double *yc = y + i * N;
        const double *xc = x + i * N;
        for (int64_t j = 0; j < N; j++) yc[j] = xc[j] * a;
    }
}

/* w = (F(u + eps*v) - Fu) / eps; the F(up) stencil is evaluated in place,
 * so no separate Fp buffer is needed (element values are identical). */
static inline void k_jvp_chunk(const double *u, const double *v, const double *Fu, double *w,
                               double *up, int64_t N, double lam, double eps, int64_t tid,
                               int64_t nt) {
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        const double *uc = u + i * N;
        const double *vc = v + i * N;
        double *upc = up + i * N;
        for (int64_t j = 0; j < N; j++) upc[j] = uc[j] + eps * vc[j];
    }
    for (int64_t i = i0; i < i1; i++) {
        const double *upc = up + i * N;
        const double *upn = upc - N;
        const double *upd = upc + N;
        const double *fc = Fu + i * N;
        double *wc = w + i * N;
        if (i == 0 || i == N - 1) {
            for (int64_t j = 0; j < N; j++) wc[j] = (upc[j] - fc[j]) / eps;
        } else {
            for (int64_t j = 1; j < N - 1; j++) {
                const double fpi =
                    (4.0 * upc[j] - upn[j] - upd[j] - upc[j - 1] - upc[j + 1]) / h2
                    - lam * exp(upc[j]);
                wc[j] = (fpi - fc[j]) / eps;
            }
            wc[0] = (upc[0] - fc[0]) / eps;
            wc[N - 1] = (upc[N - 1] - fc[N - 1]) / eps;
        }
    }
}

/* up pass only (split so the halo reads of the stencil pass are safe) */
static inline void k_up_chunk(const double *u, const double *v, double *up, double eps,
                              int64_t N, int64_t tid, int64_t nt) {
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        const double *uc = u + i * N;
        const double *vc = v + i * N;
        double *upc = up + i * N;
        for (int64_t j = 0; j < N; j++) upc[j] = uc[j] + eps * vc[j];
    }
}

static inline void k_jv_stencil_chunk(const double *Fu, double *w, double *up, int64_t N,
                                      double lam, double eps, int64_t tid, int64_t nt) {
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    int64_t i0, i1;
    row_range(tid, nt, N, &i0, &i1);
    for (int64_t i = i0; i < i1; i++) {
        const double *upc = up + i * N;
        const double *upn = upc - N;
        const double *upd = upc + N;
        const double *fc = Fu + i * N;
        double *wc = w + i * N;
        if (i == 0 || i == N - 1) {
            for (int64_t j = 0; j < N; j++) wc[j] = (upc[j] - fc[j]) / eps;
        } else {
            for (int64_t j = 1; j < N - 1; j++) {
                const double fpi =
                    (4.0 * upc[j] - upn[j] - upd[j] - upc[j - 1] - upc[j + 1]) / h2
                    - lam * exp(upc[j]);
                wc[j] = (fpi - fc[j]) / eps;
            }
            wc[0] = (upc[0] - fc[0]) / eps;
            wc[N - 1] = (upc[N - 1] - fc[N - 1]) / eps;
        }
    }
}

/* ---- entry point ------------------------------------------------------- */

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size) {
    const int64_t n = N * N;
    /* grids: Q (RESTART+1) + F,du,up,w (4) ; small: H,cs,sn,g,y,rows */
    const size_t need =
        (size_t)(RESTART + 5) * n * sizeof(double)
        + ((size_t)(RESTART + 1) * RESTART + 3 * RESTART + (RESTART + 1) + N) * sizeof(double);

    void *own = NULL;
    uint8_t *ws = workspace;
    if ((size_t)workspace_size < need) {
        own = malloc(need);
        if (!own) return;
        ws = (uint8_t *)own;
    }
    size_t mis = (size_t)ws % 64;
    double *p = mis ? (double *)((char *)ws + (64 - mis)) : (double *)ws;

    Work W;
    W.F  = p;
    W.du = p + n;
    W.up = p + 2 * n;
    W.w  = p + 3 * n;
    W.Q  = p + 4 * n;
    p += (size_t)(RESTART + 5) * n;
    W.H  = p; p += (size_t)(RESTART + 1) * RESTART;
    W.cs = p; p += RESTART;
    W.sn = p; p += RESTART;
    W.g  = p; p += RESTART + 1;
    W.y  = p; p += RESTART;
    W.rows = p;

    double *F = W.F;

    #pragma omp parallel
    {
        const int64_t tid = omp_get_thread_num();
        const int64_t nt  = omp_get_num_threads();
        double *H = W.H, *cs = W.cs, *sn = W.sn, *g = W.g, *y = W.y;
        double *Q = W.Q, *w = W.w, *up = W.up, *du = W.du, *rows = W.rows;

        k_resid_chunk(u, F, N, lam, tid, nt);
        #pragma omp barrier
        k_resid_frame(u, F, N);
        #pragma omp barrier
        k_rowsq(F, N, tid, nt, rows);
        #pragma omp barrier
        const double f0 = sqrt(k_combine(N, rows));

        for (int step = 0; step < MAX_NEWTON; step++) {
            k_rowsq(F, N, tid, nt, rows);
            #pragma omp barrier
            const double fnow = sqrt(k_combine(N, rows));
            if (fnow <= 1.0e-14 * f0) break;

            /* ---- GMRES on J(u) du = -F(u) ---- */
            k_rowsq(F, N, tid, nt, rows);
            #pragma omp barrier
            const double beta = sqrt(k_combine(N, rows));

            #pragma omp barrier
            k_scale_chunk(Q, F, -1.0 / beta, N, tid, nt);   /* Q[0] = -Fu / beta */
            #pragma omp barrier
            g[0] = beta;

            k_rowsq(u, N, tid, nt, rows);
            #pragma omp barrier
            const double nu = sqrt(k_combine(N, rows));

            int m_used = RESTART;
            for (int k = 0; k < RESTART; k++) {
                double *Qk = Q + (int64_t)k * n;

                /* w = J_fd(u) Qk */
                k_rowsq(Qk, N, tid, nt, rows);
                #pragma omp barrier
                const double nv = sqrt(k_combine(N, rows));
                if (nv == 0.0) {
                    k_scale_chunk(w, Qk, 0.0, N, tid, nt);
                    #pragma omp barrier
                } else {
                    const double eps = SQRT_MACHEPS * (1.0 + nu) / nv;
                    k_up_chunk(u, Qk, up, eps, N, tid, nt);
                    #pragma omp barrier
                    k_jv_stencil_chunk(F, w, up, N, lam, eps, tid, nt);
                    #pragma omp barrier
                }

                for (int p = 0; p <= k; p++) {
                    double *Qp = Q + (int64_t)p * n;
                    k_rows2(Qp, w, N, tid, nt, rows);
                    #pragma omp barrier
                    const double hpk = k_combine(N, rows);
                    H[(int64_t)p * RESTART + k] = hpk;
                    k_axp_chunk(w, Qp, -hpk, N, tid, nt);
                    #pragma omp barrier
                }

                k_rowsq(w, N, tid, nt, rows);
                #pragma omp barrier
                const double hnext = sqrt(k_combine(N, rows));
                H[(int64_t)(k + 1) * RESTART + k] = hnext;

                for (int p = 0; p < k; p++) {
                    const double a = H[(int64_t)p * RESTART + k];
                    const double b = H[(int64_t)(p + 1) * RESTART + k];
                    H[(int64_t)(p + 1) * RESTART + k] = -sn[p] * a + cs[p] * b;
                    H[(int64_t)p * RESTART + k] = cs[p] * a + sn[p] * b;
                }
                const double hkk = H[(int64_t)k * RESTART + k];
                const double denom = sqrt(hkk * hkk + hnext * hnext);
                cs[k] = hkk / denom;
                sn[k] = hnext / denom;
                H[(int64_t)k * RESTART + k] = cs[k] * hkk + sn[k] * hnext;
                H[(int64_t)(k + 1) * RESTART + k] = 0.0;

                const double gt = cs[k] * g[k];
                g[k + 1] = -sn[k] * g[k];
                g[k] = gt;

                const double rel = fabs(g[k + 1]) / beta;
                if (rel < INNER_TOL || hnext < 1.0e-13 || k == RESTART - 1) {
                    m_used = k + 1;
                    break;
                }
                k_scale_chunk(Q + (int64_t)(k + 1) * n, w, 1.0 / hnext, N, tid, nt);
                #pragma omp barrier
            }

            for (int row = 0; row < m_used; row++) {
                const int rr = m_used - 1 - row;
                double s = g[rr];
                for (int col = rr + 1; col < m_used; col++)
                    s -= H[(int64_t)rr * RESTART + col] * y[col];
                y[rr] = s / H[(int64_t)rr * RESTART + rr];
            }

            k_scale_chunk(du, Q, 0.0, N, tid, nt);          /* du = 0 */
            #pragma omp barrier
            for (int p = 0; p < m_used; p++) {
                k_axp_chunk(du, Q + (int64_t)p * n, y[p], N, tid, nt);
                #pragma omp barrier
            }

            k_axp_chunk(u, du, 1.0, N, tid, nt);            /* u = u + du */
            #pragma omp barrier
            k_resid_chunk(u, F, N, lam, tid, nt);
            #pragma omp barrier
            k_resid_frame(u, F, N);
            #pragma omp barrier
        }
    }

    if (own) free(own);
}
