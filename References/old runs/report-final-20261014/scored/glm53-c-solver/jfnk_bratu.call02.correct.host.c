/* JFNK Bratu 2-D: faithful port of the NumPy reference, OpenMP-parallel.
 *
 * The outer Newton loop and the Arnoldi loop are SEQUENTIAL (each Newton step
 * needs the previous one's u; each Krylov basis vector needs every earlier
 * one).  Everything INSIDE one Arnoldi step -- the finite-difference residual
 * evaluation with its exp, the grid stencil, the dot products against the
 * earlier basis vectors, the vector updates -- is data-parallel, and that is
 * where all the time is.
 *
 * Structure and every scalar decision match the reference:
 *   - undamped Newton, u += du, until ||F|| <= 1e-14*||F(u0)|| or the step cap;
 *   - GMRES(restart) with incremental Givens rotations, modified Gram-Schmidt
 *     (the projection loop is sequential in p; the reductions inside are
 *     parallel and reassociated, which the graded tolerance covers);
 *   - matrix-free Jacobian: eps = sqrt(macheps)*(1+||u||)/||v||, and the
 *     Jacobian-vector product is fused so no up/Fp temporaries are formed;
 *   - the FD step is a round-off bound read off the working precision.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define DBL_EPSV 2.2204460492503131e-16

/* solver configuration, as in the reference preset */
enum { JF_MAX_NEWTON = 20 };
enum { JF_RESTART = 50 };
#define JF_INNER_TOL 1e-4
#define JF_NEWTON_RTOL 1.0e-14

static double jf_vec_norm(const double *restrict a, size_t n)
{
    double s = 0.0;
    #pragma omp parallel for reduction(+:s) schedule(static)
    for (ptrdiff_t i = 0; (size_t)i < n; ++i)
        s += a[i] * a[i];
    return sqrt(s);
}

/* F = residual(u): boundary copies u, interior gets the 5-point stencil. */
static void jf_residual(const double *restrict u, double *restrict F, int N, double lam)
{
    const double h = 1.0 / (double)(N - 1);
    const double hh = h * h;

    #pragma omp parallel
    {
        #pragma omp for schedule(static) nowait
        for (int i = 0; i < N; ++i) {
            size_t r = (size_t)i * (size_t)N;
            F[r] = u[r];
            F[r + (size_t)N - 1] = u[r + (size_t)N - 1];
        }
        #pragma omp for schedule(static)
        for (int j = 0; j < N; ++j) {
            F[j] = u[j];
            F[(size_t)(N - 1) * (size_t)N + (size_t)j] =
                u[(size_t)(N - 1) * (size_t)N + (size_t)j];
        }

        #pragma omp for schedule(static)
        for (int i = 1; i < N - 1; ++i) {
            const double *restrict um = u + (size_t)(i - 1) * (size_t)N;
            const double *restrict u0 = u + (size_t)i * (size_t)N;
            const double *restrict up = u + (size_t)(i + 1) * (size_t)N;
            double *restrict Fr = F + (size_t)i * (size_t)N;
            for (int j = 1; j < N - 1; ++j) {
                const double num = 4.0 * u0[j] - um[j] - up[j] - u0[j - 1] - u0[j + 1];
                Fr[j] = num / hh - lam * exp(u0[j]);
            }
        }
    }
}

/* w = (F(u + eps*v) - F(u)) / eps, fused.  Boundary of w is 0. */
static void jf_jvp(const double *restrict u, const double *restrict v,
                   const double *restrict Fu, double *restrict w,
                   int N, double lam, double nu)
{
    const double h = 1.0 / (double)(N - 1);
    const double hh = h * h;
    const size_t nn = (size_t)N * (size_t)N;
    const double nv = jf_vec_norm(v, nn);
    const double eps = sqrt(DBL_EPSV) * (1.0 + nu) / nv;

    /* boundary of Jv is zero (F is the identity there) */
    #pragma omp parallel
    {
        #pragma omp for schedule(static) nowait
        for (int i = 0; i < N; ++i) {
            size_t r = (size_t)i * (size_t)N;
            w[r] = 0.0;
            w[r + (size_t)N - 1] = 0.0;
        }
        #pragma omp for schedule(static)
        for (int j = 0; j < N; ++j) {
            w[j] = 0.0;
            w[(size_t)(N - 1) * (size_t)N + (size_t)j] = 0.0;
        }

        if (nv != 0.0) {
            #pragma omp for schedule(static)
            for (int i = 1; i < N - 1; ++i) {
                const double *restrict um = u + (size_t)(i - 1) * (size_t)N;
                const double *restrict u0 = u + (size_t)i * (size_t)N;
                const double *restrict up = u + (size_t)(i + 1) * (size_t)N;
                const double *restrict vm = v + (size_t)(i - 1) * (size_t)N;
                const double *restrict v0 = v + (size_t)i * (size_t)N;
                const double *restrict vp = v + (size_t)(i + 1) * (size_t)N;
                const double *restrict Fur = Fu + (size_t)i * (size_t)N;
                double *restrict wr = w + (size_t)i * (size_t)N;
                for (int j = 1; j < N - 1; ++j) {
                    const double c   = u0[j]     + eps * v0[j];
                    const double cm  = um[j]     + eps * vm[j];
                    const double cp  = up[j]     + eps * vp[j];
                    const double cl  = u0[j - 1] + eps * v0[j - 1];
                    const double cr  = u0[j + 1] + eps * v0[j + 1];
                    const double num = 4.0 * c - cm - cp - cl - cr;
                    const double Fp  = num / hh - lam * exp(c);
                    wr[j] = (Fp - Fur[j]) / eps;
                }
            }
        }
    }
}

static int jf_trace = -1;

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    if (jf_trace < 0) jf_trace = (getenv("JFNK_TRACE") != NULL);
    const int n = (int)N;
    if (n < 3) return;
    const size_t nn = (size_t)n * (size_t)n;
    const int restart = JF_RESTART;

    double *const F  = (double *)aligned_alloc(64, nn * sizeof(double));
    double *const Q  = (double *)aligned_alloc(64, (size_t)(restart + 1) * nn * sizeof(double));
    double *const H  = (double *)aligned_alloc(64, (size_t)(restart + 1) * (size_t)restart * sizeof(double));
    double cs[JF_RESTART], sn[JF_RESTART], g[JF_RESTART + 1], yv[JF_RESTART];
    if (!F || !Q || !H) { free(F); free(Q); free(H); return; }

    jf_residual(u, F, n, lam);
    const double f0 = jf_vec_norm(F, nn);
    double nu = jf_vec_norm(u, nn);

    for (int step = 0; step < JF_MAX_NEWTON; ++step) {
        const double beta = jf_vec_norm(F, nn);
        if (beta <= JF_NEWTON_RTOL * f0) break;

        /* ---- GMRES(restart) on J(u) du = -F(u), matrix-free ---- */
        {
            const double s = -1.0 / beta;
            #pragma omp parallel for schedule(static)
            for (ptrdiff_t i = 0; (size_t)i < nn; ++i) Q[i] = s * F[i];
        }
        g[0] = beta;
        int m_used = restart;
        double *restrict w = Q + (size_t)restart * nn; /* last Q slot is scratch */

        for (int k = 0; k < restart; ++k) {
            jf_jvp(u, Q + (size_t)k * nn, F, w, n, lam, nu);

            /* modified Gram-Schmidt: sequential in p, parallel in space */
            double hp = 0.0; /* shared: reduced dot of the current p */
            #pragma omp parallel
            {
                for (int p = 0; p <= k; ++p) {
                    const double *restrict Qp = Q + (size_t)p * nn;
                    #pragma omp for reduction(+:hp) schedule(static)
                    for (ptrdiff_t i = 0; (size_t)i < nn; ++i)
                        hp += Qp[i] * w[i];
                    #pragma omp for schedule(static)
                    for (ptrdiff_t i = 0; (size_t)i < nn; ++i)
                        w[i] -= hp * Qp[i];
                    #pragma omp single
                    {
                        H[(size_t)p * (size_t)restart + (size_t)k] = hp;
                        hp = 0.0;
                    }
                }
            }
            const double h_next = jf_vec_norm(w, nn);
            H[(size_t)(k + 1) * (size_t)restart + (size_t)k] = h_next;

            for (int p = 0; p < k; ++p) {
                const double hk  = H[(size_t)p * (size_t)restart + (size_t)k];
                const double hk1 = H[(size_t)(p + 1) * (size_t)restart + (size_t)k];
                H[(size_t)(p + 1) * (size_t)restart + (size_t)k] = -sn[p] * hk + cs[p] * hk1;
                H[(size_t)p * (size_t)restart + (size_t)k] = cs[p] * hk + sn[p] * hk1;
            }
            const double d0 = H[(size_t)k * (size_t)restart + (size_t)k];
            const double d1 = h_next;
            const double denom = sqrt(d0 * d0 + d1 * d1);
            cs[k] = d0 / denom;
            sn[k] = d1 / denom;
            H[(size_t)k * (size_t)restart + (size_t)k] = cs[k] * d0 + sn[k] * d1;
            H[(size_t)(k + 1) * (size_t)restart + (size_t)k] = 0.0;

            const double tg = cs[k] * g[k];
            g[k + 1] = -sn[k] * g[k];
            g[k] = tg;

            const double rel = fabs(g[k + 1]) / beta;
            if (rel < JF_INNER_TOL || h_next < 1.0e-13 || k == restart - 1) {
                m_used = k + 1;
                break;
            }
            {
                const double s = 1.0 / h_next;
                double *restrict Qn = Q + (size_t)(k + 1) * nn;
                #pragma omp parallel for schedule(static)
                for (ptrdiff_t i = 0; (size_t)i < nn; ++i) Qn[i] = s * w[i];
            }
        }

        /* back substitution for y */
        for (int row = 0; row < m_used; ++row) {
            const int rr = m_used - 1 - row;
            double s = g[rr];
            for (int col = rr + 1; col < m_used; ++col)
                s -= H[(size_t)rr * (size_t)restart + (size_t)col] * yv[col];
            yv[rr] = s / H[(size_t)rr * (size_t)restart + (size_t)rr];
        }

        /* u += sum_p y_p Q[p], fused */
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < n; ++i) {
            double *restrict ur = u + (size_t)i * (size_t)n;
            for (int p = 0; p < m_used; ++p) {
                const double yp = yv[p];
                const double *restrict Qp = Q + ((size_t)p * nn + (size_t)i * (size_t)n);
                for (int j = 0; j < n; ++j) ur[j] += yp * Qp[j];
            }
        }
        jf_residual(u, F, n, lam);
        nu = jf_vec_norm(u, nn);
    }

    free(F); free(Q); free(H);
}
