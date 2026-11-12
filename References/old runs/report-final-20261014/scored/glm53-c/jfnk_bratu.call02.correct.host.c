/* jfnk_bratu: faithful, fast port of the NumPy reference.
 *
 * The reference runs JFNK-Bratu: Newton (max 20 steps, stop at ||F|| <= 1e-14 f0)
 * around GMRES(restart=50, tol=1e-4) with a matrix-free finite-difference
 * Jacobian-vector product (eps = sqrt(macheps)*(1+||u||)/||v||) and modified
 * Gram-Schmidt orthogonalization.  With the loose inner tolerance the outer loop
 * converges only linearly, so the FINAL iterate depends on the whole trajectory:
 * the algorithm below reproduces the reference arithmetic (same eps, same FD
 * formula, same orthogonalization, same break conditions) while parallelizing and
 * fusing the data-parallel parts.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define MAXNEWTON 20
#define RESTART   50
#define INNER_TOL 1.0e-4
#define NEWTON_RTOL 1.0e-14
#define BREAKDOWN 1.0e-13

static double pdot(const double *restrict a, const double *restrict b, int64_t m)
{
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (int64_t i = 0; i < m; i++) s += a[i] * b[i];
    return s;
}

/* F = (4u - NSEW)/(h*h) - lam*exp(u) on the interior; u is the full N x N grid */
static void residual(const double *restrict u, double *restrict F,
                     int n, int N, double lam, double hh)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        double *Fr = F + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double s = 4.0 * ur[j] - ur[j - N] - ur[j + N] - ur[j - 1] - ur[j + 1];
            Fr[j] = s / hh - lam * exp(ur[j]);
        }
    }
}
/* up = u + eps*v (interior of the full-grid u);  ex = exp(up) */
static void jvp_up(const double *restrict u, int N, const double *restrict v, double eps,
                   double *restrict up, double *restrict ex, int n)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        const double *vr = v + (int64_t)i * n;
        double *upr = up + (int64_t)i * n;
        double *exr = ex + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double t = ur[j] + eps * vr[j];
            upr[j] = t;
            exr[j] = exp(t);
        }
    }
}
/* w = (Fp - F)/eps with Fp = (4*up - NSEW)/(h*h) - lam*ex */
static void jvp_w(const double *restrict up, const double *restrict ex,
                  const double *restrict F, double *restrict w,
                  int n, double lam, double eps, double hh)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = up + (int64_t)i * n;
        const double *er = ex + (int64_t)i * n;
        const double *Fr = F  + (int64_t)i * n;
        double *wr = w + (int64_t)i * n;
        for (int j = 0; j < n; j++) {
            double s = 4.0 * ur[j];
            if (i > 0)     s -= up[(int64_t)(i - 1) * n + j];
            if (i < n - 1) s -= up[(int64_t)(i + 1) * n + j];
            if (j > 0)     s -= ur[j - 1];
            if (j < n - 1) s -= ur[j + 1];
            wr[j] = (s / hh - lam * er[j] - Fr[j]) / eps;
        }
    }
}

/* one modified Gram-Schmidt pair: h = <Qp, w>; w -= h*Qp */
static double mgs_pair(const double *restrict Qp, double *restrict w, int64_t m)
{
    double h = pdot(Qp, w, m);
    if (h != 0.0) {
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < m; i++) w[i] -= h * Qp[i];
    }
    return h;
}

/* ||u||_2 over the interior of the full-grid u (boundary rows are 0) */
static double norm_u(const double *restrict u, int N, int n)
{
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (int i = 0; i < n; i++) {
        const double *ur = u + (int64_t)(i + 1) * N + 1;
        double ls = 0.0;
        for (int j = 0; j < n; j++) ls += ur[j] * ur[j];
        s += ls;
    }
    return sqrt(s);
}

/* GMRES(reSTART) with FD Jacobian-vector products: solves J du = -F.
 * Mirrors bratu_gmres of the reference exactly (same eps, same MGS, same
 * Givens rotations, same break conditions, same back substitution). */
static void bratu_gmres(const double *restrict u, const double *restrict F,
                        double *restrict du, int n, int N, double lam, double hh,
                        double nu, double *restrict V, double *restrict up,
                        double *restrict ex, double *restrict w,
                        double H[RESTART + 2][RESTART], double *cs, double *sn,
                        double *g, double *y)
{
    const int64_t nn = (int64_t)n * n;
    const double beta = sqrt(pdot(F, F, nn));
    if (beta == 0.0) {
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < nn; i++) du[i] = 0.0;
        return;
    }
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < nn; i++) V[i] = -F[i] / beta;
    g[0] = beta;
    int m_used = RESTART;
    int k;
    for (k = 0; k < RESTART; k++) {
        /* J v  via the scaled finite-difference formula */
        const double nv = sqrt(pdot(V + (int64_t)k * nn, V + (int64_t)k * nn, nn));
        if (nv == 0.0) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 0; i < nn; i++) w[i] = 0.0;
        } else {
            const double eps = 1.4901161193847656e-08 * (1.0 + nu) / nv;
            jvp_up(u, N, V + (int64_t)k * nn, eps, up, ex, n);
            jvp_w(up, ex, F, w, n, lam, eps, hh);
        }

        for (int p = 0; p <= k; p++)
            H[p][k] = mgs_pair(V + (int64_t)p * nn, w, nn);
        const double hnext = sqrt(pdot(w, w, nn));
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
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < nn; i++)
            V[(int64_t)(k + 1) * nn + i] = w[i] / hnext;
    }

    (void)k;
    for (int row = 0; row < m_used; row++) {
        const int rr = m_used - 1 - row;
        double s = g[rr];
        for (int col = rr + 1; col < m_used; col++) s -= H[rr][col] * y[col];
        y[rr] = s / H[rr][rr];
    }
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < nn; i++) du[i] = 0.0;
    for (int p = 0; p < m_used; p++) {
        const double yp = y[p];
        if (yp == 0.0) continue;
        const double *Vp = V + (int64_t)p * nn;
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < nn; i++) du[i] += yp * Vp[i];
    }
}

/* ------------------------------------------------------------------ */

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
    const double hh = h * h;
    const int64_t nn = (int64_t)n * n;

    double *base;
    int used_ws = 0;
    const size_t total = (size_t)(RESTART + 6) * nn + 64;
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
    double *F   = ptr; ptr += nn;
    double *du  = ptr; ptr += nn;
    double *up  = ptr; ptr += nn;
    double *ex  = ptr; ptr += nn;
    double *w   = ptr; ptr += nn;
    double *V   = ptr; ptr += (size_t)RESTART * nn;   /* V[0..RESTART] */
    double (*H)[RESTART] = (double (*)[RESTART])ptr; ptr += (RESTART + 2) * RESTART;
    double *cs = ptr; ptr += RESTART + 2;
    double *sn = ptr; ptr += RESTART + 2;
    double *g  = ptr; ptr += RESTART + 3;
    double *y  = ptr;

    /* Newton loop, exactly as in the reference */
    residual(u, F, n, (int)N, lam, hh);
    const double f0 = sqrt(pdot(F, F, nn));
    for (int step = 0; step < MAXNEWTON; step++) {
        const double fnow = sqrt(pdot(F, F, nn));
        if (fnow <= NEWTON_RTOL * f0) break;
        const double nu = norm_u(u, (int)N, n);
        bratu_gmres(u, F, du, n, (int)N, lam, hh, nu, V, up, ex, w,
                    H, cs, sn, g, y);
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < n; i++) {
            double *ur = u + (int64_t)(i + 1) * (int)N + 1;
            const double *dr = du + (int64_t)i * n;
            for (int j = 0; j < n; j++) ur[j] += dr[j];
        }
        residual(u, F, n, (int)N, lam, hh);
    }
    if (!used_ws) free(base);
}
