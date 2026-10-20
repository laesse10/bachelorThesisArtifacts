#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* JFNK Bratu: faithful replica of the reference iteration sequence.
 * (max_newton, inner_tol, gmres_restart) are fixed by the task configuration.
 *
 * One persistent OpenMP team; control flow runs redundantly on every thread
 * (deterministic, identical values), heavy kernels are `omp for` worksharing.
 * All cross-thread summations are done manually: every thread accumulates
 * into its own slot (fixed data chunk under schedule(static,1)), and thread 0
 * combines the slots sequentially in slot order. The team size is pinned at
 * entry, so the chunking -- and hence the summed value -- is reproducible
 * run-to-run (libgomp's built-in reduction tree is arrival-order dependent
 * and would otherwise change the result every region).
 */
#define MAX_NEWTON 20
#define INNER_TOL 1.0e-4
#define RESTART 50
#define PB 8 /* p-block width for the Krylov dot / axpy / du phases */
#define MAX_T 128

void jfnk_bratu_fp64(double *restrict u, const int64_t N, const double lam,
                     uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace_size;
    const int64_t n2 = N * N;
    double *F = (double *)workspace;
    double *du = (double *)(workspace + 8 * n2);
    double *w = (double *)(workspace + 16 * n2);
    double *Q = (double *)(workspace + 24 * n2); /* (RESTART+1) * n2 */
    double *H = Q + (int64_t)(RESTART + 1) * n2;
    double *cs = H + (int64_t)(RESTART + 1) * RESTART;
    double *sn = cs + RESTART;
    double *g = sn + RESTART;
    double *y = g + RESTART + 1;
    double *slot = y + 16; /* nthreads*(PB+1) doubles of per-thread scratch */

    int nt = omp_get_max_threads();
    if (nt > MAX_T)
        nt = MAX_T;
    omp_set_num_threads(nt);

    /* shared combination targets (thread 0 writes after a barrier) */
    double s1 = 0.0, s2 = 0.0, s3 = 0.0;
    double rh[PB];
    int kstop = 0, kstop_k = 0;

    const double macheps = 2.220446049250313e-16;
    const double newton_rtol = 1.0e-14;
    const double sqrt_macheps = sqrt(macheps);
    const double h = 1.0 / (double)(N - 1);
    const double hh = h * h;
    const int64_t nm1 = N - 1;

    #pragma omp parallel
    {
        const int mytid = omp_get_thread_num();
        double *mys = slot + (int64_t)mytid * (PB + 1);
        double f0 = 0.0, fnow = 0.0;

        /* F = residual(u); f0 = ||F|| (per-row partials) */
        if (mytid == 0)
            for (int t = 0; t < nt; t++)
                slot[(int64_t)t * (PB + 1)] = 0.0;
#pragma omp barrier
#pragma omp for schedule(static, 1)
        for (int64_t i = 0; i < N; i++) {
            double *Fr = F + i * N;
            const double *ur = u + i * N;
            double ss = 0.0;
            if (i == 0 || i == nm1) {
                for (int64_t j = 0; j < N; j++) {
                    Fr[j] = ur[j];
                    ss += ur[j] * ur[j];
                }
            } else {
                const double *un = u + (i - 1) * N;
                const double *ud = u + (i + 1) * N;
                Fr[0] = ur[0];
                Fr[nm1] = ur[nm1];
                ss += ur[0] * ur[0];
                for (int64_t j = 1; j < nm1; j++) {
                    double v = (4.0 * ur[j] - un[j] - ud[j] - ur[j - 1] - ur[j + 1]) /
                                   hh -
                               lam * exp(ur[j]);
                    Fr[j] = v;
                    ss += v * v;
                }
                ss += ur[nm1] * ur[nm1];
            }
            mys[0] += ss;
        }
        if (mytid == 0) {
            double tot = 0.0;
            for (int t = 0; t < nt; t++)
                tot += slot[(int64_t)t * (PB + 1)];
            s1 = tot;
        }
#pragma omp barrier
        f0 = sqrt(s1);
        fnow = f0;

        for (int step = 0; step < MAX_NEWTON; step++) {
            if (fnow <= newton_rtol * f0)
                break;
            /* ---- one GMRES(restart) cycle solving J du = -F ---- */
            const double beta = fnow;
            /* Q[0] = -F/beta; s2 = ||u||^2 (per-row partials) */
            if (mytid == 0)
                for (int t = 0; t < nt; t++)
                    slot[(int64_t)t * (PB + 1)] = 0.0;
#pragma omp barrier
#pragma omp for schedule(static, 1)
            for (int64_t i = 0; i < N; i++) {
                double *Qr = Q + i * N;
                const double *Fr = F + i * N;
                const double *ur = u + i * N;
                double ss = 0.0;
                for (int64_t j = 0; j < N; j++) {
                    Qr[j] = -Fr[j] / beta;
                    ss += ur[j] * ur[j];
                }
                mys[0] += ss;
            }
            if (mytid == 0) {
                double tot = 0.0;
                for (int t = 0; t < nt; t++)
                    tot += slot[(int64_t)t * (PB + 1)];
                s2 = tot;
                g[0] = beta;
            }
#pragma omp barrier
            const double nu = sqrt(s2);
            const double nuf = 1.0 + nu;
            for (int k = 0; k < RESTART; k++) {
                const double *Qk = Q + (int64_t)k * n2;
                /* nv = ||Q[k]|| (per-row partials) */
                if (mytid == 0)
                    for (int t = 0; t < nt; t++)
                        slot[(int64_t)t * (PB + 1)] = 0.0;
#pragma omp barrier
#pragma omp for schedule(static, 1)
                for (int64_t i = 0; i < N; i++) {
                    const double *r = Qk + i * N;
                    double ss = 0.0;
                    for (int64_t j = 0; j < N; j++)
                        ss += r[j] * r[j];
                    mys[0] += ss;
                }
                if (mytid == 0) {
                    double tot = 0.0;
                    for (int t = 0; t < nt; t++)
                        tot += slot[(int64_t)t * (PB + 1)];
                    s3 = tot;
                }
#pragma omp barrier
                const double nv = sqrt(s3);
                if (nv == 0.0) {
#pragma omp for schedule(static)
                    for (int64_t i = 0; i < n2; i++)
                        w[i] = 0.0;
                } else {
                    const double eps = sqrt_macheps * nuf / nv;
                    /* w = (F(u + eps*Qk) - F) / eps, one pass */
#pragma omp for schedule(static)
                    for (int64_t i = 0; i < N; i++) {
                        double *wr = w + i * N;
                        const double *ur = u + i * N;
                        const double *qr = Qk + i * N;
                        const double *fr = F + i * N;
                        if (i == 0 || i == nm1) {
                            for (int64_t j = 0; j < N; j++) {
                                double upv = ur[j] + eps * qr[j];
                                wr[j] = (upv - fr[j]) / eps;
                            }
                        } else {
                            const double *un = u + (i - 1) * N;
                            const double *qn = Qk + (i - 1) * N;
                            const double *ud = u + (i + 1) * N;
                            const double *qd = Qk + (i + 1) * N;
                            wr[0] = (ur[0] + eps * qr[0] - fr[0]) / eps;
                            wr[nm1] = (ur[nm1] + eps * qr[nm1] - fr[nm1]) / eps;
                            for (int64_t j = 1; j < nm1; j++) {
                                double tc = ur[j] + eps * qr[j];
                                double tn = un[j] + eps * qn[j];
                                double ts = ud[j] + eps * qd[j];
                                double tw = ur[j - 1] + eps * qr[j - 1];
                                double te = ur[j + 1] + eps * qr[j + 1];
                                double fp = (4.0 * tc - tn - ts - tw - te) / hh -
                                            lam * exp(tc);
                                wr[j] = (fp - fr[j]) / eps;
                            }
                        }
                    }
                }
                /* h[p] = Q[p].w (p = 0..k) with per-row partials;
                 * then w -= sum_p h[p] Q[p] */
                for (int64_t pb = 0; pb <= k; pb += PB) {
                    const int cnt = (int)((k + 1 - pb) < PB ? (k + 1 - pb) : PB);
                    if (mytid == 0)
                        for (int t = 0; t < nt; t++)
                            for (int p = 0; p < cnt; p++)
                                slot[(int64_t)t * (PB + 1) + 1 + p] = 0.0;
#pragma omp barrier
#pragma omp for schedule(static, 1)
                    for (int64_t i = 0; i < N; i++) {
                        const double *wr = w + i * N;
                        for (int p = 0; p < cnt; p++) {
                            const double *qp = Q + (pb + p) * n2 + i * N;
                            double ss = 0.0;
                            for (int64_t j = 0; j < N; j++)
                                ss += qp[j] * wr[j];
                            mys[1 + p] += ss;
                        }
                    }
                    if (mytid == 0) {
                        for (int p = 0; p < cnt; p++) {
                            double tot = 0.0;
                            for (int t = 0; t < nt; t++)
                                tot += slot[(int64_t)t * (PB + 1) + 1 + p];
                            rh[p] = tot;
                            H[(pb + p) * RESTART + k] = rh[p];
                        }
                    }
#pragma omp barrier
                    double hv[PB];
                    for (int p = 0; p < cnt; p++)
                        hv[p] = -rh[p];
#pragma omp for schedule(static)
                    for (int64_t i = 0; i < n2; i++) {
                        double acc = w[i];
                        for (int p = 0; p < cnt; p++)
                            acc += hv[p] * Q[(pb + p) * n2 + i];
                        w[i] = acc;
                    }
                }
                /* h_next = ||w||, then Givens (thread 0) */
                if (mytid == 0)
                    for (int t = 0; t < nt; t++)
                        slot[(int64_t)t * (PB + 1)] = 0.0;
#pragma omp barrier
#pragma omp for schedule(static, 1)
                for (int64_t i = 0; i < N; i++) {
                    const double *r = w + i * N;
                    double ss = 0.0;
                    for (int64_t j = 0; j < N; j++)
                        ss += r[j] * r[j];
                    mys[0] += ss;
                }
                if (mytid == 0) {
                    double tot = 0.0;
                    for (int t = 0; t < nt; t++)
                        tot += slot[(int64_t)t * (PB + 1)];
                    s3 = tot;
                }
#pragma omp barrier
                const double h_next = sqrt(s3);
                if (mytid == 0) {
                    H[(k + 1) * RESTART + k] = h_next;
                    for (int p = 0; p < k; p++) {
                        const double temp = cs[p] * H[p * RESTART + k] +
                                            sn[p] * H[(p + 1) * RESTART + k];
                        H[(p + 1) * RESTART + k] = -sn[p] * H[p * RESTART + k] +
                                                   cs[p] * H[(p + 1) * RESTART + k];
                        H[p * RESTART + k] = temp;
                    }
                    const double denom =
                        sqrt(H[k * RESTART + k] * H[k * RESTART + k] +
                             H[(k + 1) * RESTART + k] * H[(k + 1) * RESTART + k]);
                    cs[k] = H[k * RESTART + k] / denom;
                    sn[k] = H[(k + 1) * RESTART + k] / denom;
                    H[k * RESTART + k] = cs[k] * H[k * RESTART + k] +
                                         sn[k] * H[(k + 1) * RESTART + k];
                    H[(k + 1) * RESTART + k] = 0.0;
                    const double temp = cs[k] * g[k];
                    g[k + 1] = -sn[k] * g[k];
                    g[k] = temp;
                    const double rel = fabs(g[k + 1]) / beta;
                    kstop = (rel < INNER_TOL || h_next < 1.0e-13 || k == RESTART - 1) ? 1 : 0;
                    if (kstop)
                        kstop_k = k;
                }
#pragma omp barrier
                if (kstop)
                    break;
                /* Q[k+1] = w / h_next */
#pragma omp for schedule(static)
                for (int64_t i = 0; i < n2; i++)
                    Q[(int64_t)(k + 1) * n2 + i] = w[i] / h_next;
            }
            const int m_used = kstop ? kstop_k + 1 : RESTART;
            /* triangular back substitution for y (thread 0) */
            if (mytid == 0) {
                for (int row = 0; row < m_used; row++) {
                    const int rr = m_used - 1 - row;
                    double s = g[rr];
                    for (int col = rr + 1; col < m_used; col++)
                        s -= H[rr * RESTART + col] * y[col];
                    y[rr] = s / H[rr * RESTART + rr];
                }
            }
#pragma omp barrier
            /* du = sum_p y[p] Q[p] (per-element FMA chain, reference order) */
#pragma omp for schedule(static)
            for (int64_t i = 0; i < n2; i++)
                du[i] = 0.0;
            for (int64_t pb = 0; pb < m_used; pb += PB) {
                const int cnt = (int)((m_used - pb) < PB ? (m_used - pb) : PB);
                double yv[PB];
                for (int p = 0; p < cnt; p++)
                    yv[p] = y[pb + p];
#pragma omp for schedule(static)
                for (int64_t i = 0; i < n2; i++) {
                    double acc = du[i];
                    for (int p = 0; p < cnt; p++)
                        acc += yv[p] * Q[(pb + p) * n2 + i];
                    du[i] = acc;
                }
            }
            /* u += du; F = residual(u); fnow = ||F|| */
#pragma omp for schedule(static)
            for (int64_t i = 0; i < n2; i++)
                u[i] += du[i];
            if (mytid == 0)
                for (int t = 0; t < nt; t++)
                    slot[(int64_t)t * (PB + 1)] = 0.0;
#pragma omp barrier
#pragma omp for schedule(static, 1)
            for (int64_t i = 0; i < N; i++) {
                double *Fr = F + i * N;
                const double *ur = u + i * N;
                double ss = 0.0;
                if (i == 0 || i == nm1) {
                    for (int64_t j = 0; j < N; j++) {
                        Fr[j] = ur[j];
                        ss += ur[j] * ur[j];
                    }
                } else {
                    const double *un = u + (i - 1) * N;
                    const double *ud = u + (i + 1) * N;
                    Fr[0] = ur[0];
                    Fr[nm1] = ur[nm1];
                    ss += ur[0] * ur[0];
                    for (int64_t j = 1; j < nm1; j++) {
                        double v = (4.0 * ur[j] - un[j] - ud[j] - ur[j - 1] - ur[j + 1]) /
                                       hh -
                                   lam * exp(ur[j]);
                        Fr[j] = v;
                        ss += v * v;
                    }
                    ss += ur[nm1] * ur[nm1];
                }
                mys[0] += ss;
            }
            if (mytid == 0) {
                double tot = 0.0;
                for (int t = 0; t < nt; t++)
                    tot += slot[(int64_t)t * (PB + 1)];
                s1 = tot;
            }
#pragma omp barrier
            fnow = sqrt(s1);
        }
    }
}
