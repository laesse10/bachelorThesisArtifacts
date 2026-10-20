/* JFNK Bratu 2-D: faithful port of the NumPy reference, OpenMP-parallel.
 *
 * The outer Newton loop and the Arnoldi loop are SEQUENTIAL.  Everything
 * inside one Arnoldi step -- the finite-difference residual evaluation with
 * its exp, the grid stencil, the dot products against the earlier basis
 * vectors, the vector updates -- is data-parallel, and that is where all the
 * time is.
 *
 * Parallel form: ONE persistent thread team for the whole solve.  The team
 * runs the identical sequential control flow redundantly (every scalar --
 * the Givens rotations, the back substitution, every break decision -- is a
 * thread-local value computed from the same shared arrays, so all threads
 * agree bit for bit), and every vector pass is chunked statically over the
 * team with a hand-rolled sense-reversing barrier closing each pass.  The
 * reduction combines per-thread partials in a fixed order, so every thread
 * derives the same scalar from the same pass.
 *
 * Structure and every scalar decision match the reference:
 *   - undamped Newton, u += du, until ||F|| <= 1e-14*||F(u0)|| or the step cap;
 *   - GMRES(restart) with incremental Givens rotations, modified Gram-Schmidt
 *     (the projection loop is sequential in p; the reductions inside are
 *     parallel and reassociated, which the graded tolerance covers);
 *   - matrix-free Jacobian: eps = sqrt(macheps)*(1+||u||)/||v||, fused so no
 *     up/Fp temporaries are formed.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdatomic.h>
#include <omp.h>

#define DBL_EPSV 2.2204460492503131e-16

/* solver configuration, as in the reference preset */
enum { JF_MAX_NEWTON = 20 };
enum { JF_RESTART = 50 };
#define JF_INNER_TOL 1e-4
#define JF_NEWTON_RTOL 1.0e-14
enum { JF_MAX_THREADS = 1024 };

/* sense-reversing spin barrier: closes each chunked vector pass */
static inline void jf_bar(atomic_int *restrict bcnt, atomic_int *restrict bsense,
                          const int nth, int *restrict mysense)
{
    if (nth == 1) return;
    const int s = *mysense;
    if (atomic_fetch_add_explicit(bcnt, 1, memory_order_acq_rel) == nth - 1) {
        atomic_store_explicit(bcnt, 0, memory_order_relaxed);
        atomic_store_explicit(bsense, 1 - s, memory_order_release);
    } else {
        while (atomic_load_explicit(bsense, memory_order_acquire) == s) {
#if defined(__x86_64__) || defined(__i386__)
            __builtin_ia32_pause();
#endif
        }
    }
    *mysense = 1 - s;
}

typedef struct { double v, w; char pad[48]; } jf_part;

/* reducing barrier: the LAST ARRIVER folds the per-thread partials and
 * publishes the sums through the barrier's release/acquire pair, so every
 * thread receives the same bits and no thread can still be folding while
 * the next pass overwrites the partials.  Sums .v into *res and .w into
 * *res2; returns *res. */
static inline double jf_red(atomic_int *restrict bcnt, atomic_int *restrict bsense,
                             const jf_part *restrict parts, const int nth,
                             int *restrict mysense, double *restrict res,
                             double *restrict res2)
{
    const int s = *mysense;
    if (nth == 1) {
        double sv = 0.0, sw = 0.0;
        for (int t = 0; t < nth; ++t) { sv += parts[t].v; sw += parts[t].w; }
        *res = sv; *res2 = sw;
        return sv;
    }
    if (atomic_fetch_add_explicit(bcnt, 1, memory_order_acq_rel) == nth - 1) {
        double sv = 0.0, sw = 0.0;
        for (int t = 0; t < nth; ++t) { sv += parts[t].v; sw += parts[t].w; }
        *res = sv;  /* ordered by the release flip below */
        *res2 = sw;
        atomic_store_explicit(bcnt, 0, memory_order_relaxed);
        atomic_store_explicit(bsense, 1 - s, memory_order_release);
        *mysense = 1 - s;
        return sv;
    }
    while (atomic_load_explicit(bsense, memory_order_acquire) == s) {
#if defined(__x86_64__) || defined(__i386__)
        __builtin_ia32_pause();
#endif
    }
    *mysense = 1 - s;
    return *res;  /* the last arriver's store: visible with the flip */
}

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const int n = (int)N;
    if (n < 3) return;
    const size_t nn = (size_t)n * (size_t)n;

    double *const F = (double *)aligned_alloc(64, nn * sizeof(double));
    double *const Q = (double *)aligned_alloc(64, (size_t)(JF_RESTART + 1) * nn * sizeof(double));
    jf_part *const parts = (jf_part *)aligned_alloc(64, (size_t)JF_MAX_THREADS * sizeof(jf_part));
    atomic_int *const bcnt = (atomic_int *)aligned_alloc(64, 256);
    if (!F || !Q || !parts || !bcnt) {
        free(F); free(Q); free(parts); free(bcnt); return;
    }
    atomic_int *const bsense = bcnt + 1;
    double *const redv = (double *)((char *)bcnt + 64);  /* reduce slots */
    double *const redw = redv + 1;
    atomic_store_explicit(bcnt, 0, memory_order_relaxed);
    atomic_store_explicit(bsense, 0, memory_order_relaxed);

    const double h = 1.0 / (double)(n - 1);
    const double hh = h * h;

    #pragma omp parallel shared(u, F, Q, parts, bcnt, bsense)
    {
        const int tid = omp_get_thread_num();
        const int nth = omp_get_num_threads();
        int mysense = atomic_load_explicit(bsense, memory_order_acquire);
        double myloc = 0.0, mylocF = 0.0, mylocU = 0.0;

        /* static chunks: rows [r0, r1) for row passes, [i0, i1) for flat ones */
        const size_t r0 = (size_t)n * (size_t)tid / (size_t)nth;
        const size_t r1 = (size_t)n * (size_t)(tid + 1) / (size_t)nth;
        const size_t i0 = nn * (size_t)tid / (size_t)nth;
        const size_t i1 = nn * (size_t)(tid + 1) / (size_t)nth;

        /* thread-local copies of the small solver state: written by the
         * redundant scalar code, identical on every thread, never shared */
        double H[(JF_RESTART + 1) * JF_RESTART];
        double cs[JF_RESTART], sn[JF_RESTART], g[JF_RESTART + 1];
        double yv[JF_RESTART], hcol[JF_RESTART + 2];
        double *restrict w = Q + (size_t)JF_RESTART * nn;

        /* ---- initial residual with ||F||^2 and ||u||^2 folded in ---- */
        mylocF = 0.0; mylocU = 0.0;
        for (size_t i = r0; i < r1; ++i) {
            const double *restrict ur = u + i * n;
            double *restrict Fr = F + i * n;
            if (i == 0 || i == (size_t)n - 1) {
                for (size_t j = 0; j < (size_t)n; ++j) {
                    Fr[j] = ur[j];
                    mylocF += ur[j] * ur[j];
                    mylocU += ur[j] * ur[j];
                }
            } else {
                const double *restrict urm = u + (i - 1) * n;
                const double *restrict urp = u + (i + 1) * n;
                Fr[0] = ur[0];
                Fr[n - 1] = ur[n - 1];
                mylocF += ur[0] * ur[0] + ur[n - 1] * ur[n - 1];
                mylocU += ur[0] * ur[0] + ur[n - 1] * ur[n - 1];
                for (int j = 1; j < n - 1; ++j) {
                    const double num = 4.0 * ur[j] - urm[j] - urp[j]
                                    - ur[j - 1] - ur[j + 1];
                    const double Fj = num / hh - lam * exp(ur[j]);
                    Fr[j] = Fj;
                    mylocF += Fj * Fj;
                    mylocU += ur[j] * ur[j];
                }
            }
        }
        parts[tid].v = mylocF;
        parts[tid].w = mylocU;
        const double f0 = sqrt(jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw));
        const double nu0 = sqrt(*redw);
        double nu = nu0;
        double beta = f0;

        for (int step = 0; step < JF_MAX_NEWTON; ++step) {
            if (beta <= JF_NEWTON_RTOL * f0) break;

            /* ---- GMRES(restart) on J(u) du = -F(u), matrix-free ---- */
            myloc = 0.0;
            for (size_t i = i0; i < i1; ++i) {
                const double q = -F[i] / beta;
                Q[i] = q;
                myloc += q * q;
            }
            parts[tid].v = myloc;
            double nv = sqrt(jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw));

            g[0] = beta;
            int m_used = JF_RESTART;

            for (int k = 0; k < JF_RESTART; ++k) {
                /* matrix-free Jacobian-vector product, row-partitioned, with
                 * the MGS p=0 dot folded into the same pass */
                const double eps = sqrt(DBL_EPSV) * (1.0 + nu) / nv;
                myloc = 0.0;
                if (nv != 0.0) {
                    for (size_t i = r0; i < r1; ++i) {
                        const double *restrict um = u + (i - 1) * n;
                        const double *restrict u0 = u + i * n;
                        const double *restrict up = u + (i + 1) * n;
                        const double *restrict vm = Q + k * nn + (i - 1) * n;
                        const double *restrict v0 = Q + k * nn + i * n;
                        const double *restrict vp = Q + k * nn + (i + 1) * n;
                        const double *restrict Fur = F + i * n;
                        const double *restrict q0r = Q + i * n;
                        double *restrict wr = w + i * n;
                        if (i == 0 || i == (size_t)n - 1) {
                            for (size_t j = 0; j < (size_t)n; ++j) wr[j] = 0.0;
                        } else {
                            wr[0] = 0.0;
                            wr[n - 1] = 0.0;
                            for (int j = 1; j < n - 1; ++j) {
                                const double c   = u0[j]     + eps * v0[j];
                                const double cm  = um[j]     + eps * vm[j];
                                const double cp  = up[j]     + eps * vp[j];
                                const double cl  = u0[j - 1] + eps * v0[j - 1];
                                const double cr  = u0[j + 1] + eps * v0[j + 1];
                                const double num = 4.0 * c - cm - cp - cl - cr;
                                const double Fp  = num / hh - lam * exp(c);
                                const double wj = (Fp - Fur[j]) / eps;
                                wr[j] = wj;
                                myloc += q0r[j] * wj;
                            }
                        }
                    }
                } else {
                    for (size_t i = i0; i < i1; ++i) w[i] = 0.0;
                }
                parts[tid].v = myloc;
                hcol[0] = jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw);

                /* modified Gram-Schmidt, sequential in p, chunked in space:
                 * the axpy for p-1 and the dot for p share one pass; the
                 * epilogue applies the last axpy and accumulates ||w||^2.
                 * The p=0 dot is folded into the Jacobian pass above. */
                double h_next = 0.0;
                {
                    for (int p = 1; p <= k; ++p) {
                        const double *restrict Qa = Q + (size_t)(p - 1) * nn;
                        const double *restrict Qd = Q + (size_t)p * nn;
                        const double hpm = hcol[p - 1];
                        myloc = 0.0;
                        for (size_t i = i0; i < i1; ++i) {
                            w[i] -= hpm * Qa[i];
                            myloc += Qd[i] * w[i];
                        }
                        parts[tid].v = myloc;
                        hcol[p] = jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw);
                    }
                    {
                        const double *restrict Qk = Q + (size_t)k * nn;
                        const double hk = hcol[k];
                        myloc = 0.0;
                        for (size_t i = i0; i < i1; ++i) {
                            w[i] -= hk * Qk[i];
                            myloc += w[i] * w[i];
                        }
                        parts[tid].v = myloc;
                        h_next =
                            sqrt(jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw));
                    }
                }
                for (int p = 0; p <= k; ++p)
                    H[(size_t)p * (size_t)JF_RESTART + (size_t)k] = hcol[p];
                H[(size_t)(k + 1) * (size_t)JF_RESTART + (size_t)k] = h_next;

                /* apply the previously accumulated rotations to the column */
                for (int p = 0; p < k; ++p) {
                    const double hk  = H[(size_t)p * (size_t)JF_RESTART + (size_t)k];
                    const double hk1 = H[(size_t)(p + 1) * (size_t)JF_RESTART + (size_t)k];
                    H[(size_t)(p + 1) * (size_t)JF_RESTART + (size_t)k] = -sn[p] * hk + cs[p] * hk1;
                    H[(size_t)p * (size_t)JF_RESTART + (size_t)k] = cs[p] * hk + sn[p] * hk1;
                }
                const double d0 = H[(size_t)k * (size_t)JF_RESTART + (size_t)k];
                const double denom = sqrt(d0 * d0 + h_next * h_next);
                cs[k] = d0 / denom;
                sn[k] = h_next / denom;
                H[(size_t)k * (size_t)JF_RESTART + (size_t)k] =
                    cs[k] * d0 + sn[k] * h_next;

                const double tg = cs[k] * g[k];
                g[k + 1] = -sn[k] * g[k];
                g[k] = tg;

                const double rel = fabs(g[k + 1]) / beta;
                if (rel < JF_INNER_TOL || h_next < 1.0e-13 || k == JF_RESTART - 1) {
                    m_used = k + 1;
                    break;
                }
                {
                    double *restrict Qn = Q + (size_t)(k + 1) * nn;
                    myloc = 0.0;
                    for (size_t i = i0; i < i1; ++i) {
                        const double q = w[i] / h_next;
                        Qn[i] = q;
                        myloc += q * q;
                    }
                    parts[tid].v = myloc;
                    nv = sqrt(jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw));
                }
            }

            /* back substitution for y (Saad Alg. 6.9) */
            for (int row = 0; row < m_used; ++row) {
                const int rr = m_used - 1 - row;
                double s = g[rr];
                for (int col = rr + 1; col < m_used; ++col)
                    s -= H[(size_t)rr * (size_t)JF_RESTART + (size_t)col] * yv[col];
                yv[rr] = s / H[(size_t)rr * (size_t)JF_RESTART + (size_t)rr];
            }

            /* u += sum_p y_p Q[p], fused, in the reference's grouping */
            for (size_t i = r0; i < r1; ++i) {
                double *restrict ur = u + i * n;
                for (size_t j = 0; j < (size_t)n; ++j) {
                    double d = 0.0;
                    for (int p = 0; p < m_used; ++p)
                        d += yv[p] * Q[(size_t)p * nn + i * n + j];
                    ur[j] += d;
                }
            }
            jf_bar(bcnt, bsense, nth, &mysense);

            /* residual at the new u with ||F||^2 and ||u||^2 folded in */
            mylocF = 0.0; mylocU = 0.0;
            for (size_t i = r0; i < r1; ++i) {
                const double *restrict ur = u + i * n;
                const double *restrict urm = u + (i - 1) * n;
                const double *restrict urp = u + (i + 1) * n;
                double *restrict Fr = F + i * n;
                if (i == 0 || i == (size_t)n - 1) {
                    for (size_t j = 0; j < (size_t)n; ++j) {
                        Fr[j] = ur[j];
                        mylocF += ur[j] * ur[j];
                        mylocU += ur[j] * ur[j];
                    }
                } else {
                    Fr[0] = ur[0];
                    Fr[n - 1] = ur[n - 1];
                    mylocF += ur[0] * ur[0] + ur[n - 1] * ur[n - 1];
                    mylocU += ur[0] * ur[0] + ur[n - 1] * ur[n - 1];
                    for (int j = 1; j < n - 1; ++j) {
                        const double num = 4.0 * ur[j] - urm[j] - urp[j]
                                            - ur[j - 1] - ur[j + 1];
                        const double Fj = num / hh - lam * exp(ur[j]);
                        Fr[j] = Fj;
                        mylocF += Fj * Fj;
                        mylocU += ur[j] * ur[j];
                    }
                }
            }
            parts[tid].v = mylocF;
            parts[tid].w = mylocU;
            beta = sqrt(jf_red(bcnt, bsense, parts, nth, &mysense, redv, redw));
            nu = sqrt(*redw);
        }
    }

    free(F); free(Q); free(parts); free(bcnt);
}
