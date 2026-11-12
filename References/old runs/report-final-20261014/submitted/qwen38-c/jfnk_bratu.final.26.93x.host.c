#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

/* JFNK Bratu: faithful replica of the reference iteration sequence.
 * (max_newton, inner_tol, gmres_restart) are fixed by the task configuration.
 *
 * One persistent OpenMP team; control flow runs redundantly on every thread
 * (deterministic, identical values). Cross-thread summations are manual:
 * each thread accumulates its own chunk partials into a private slot in
 * shared scratch, and after each region every thread adds the slots itself
 * in fixed slot order; a barrier follows each combine so that the next
 * region's self-zero cannot race a previous combine. Elementwise
 * accumulations use explicit 8-lane (a0..a7) accumulators so the compiler
 * keeps them as 512-bit registers (one horizontal reduction at the end),
 * never a per-chunk scalar reduction.
 */
#define MAX_NEWTON 20
#define INNER_TOL 1.0e-4
#define RESTART 50
#define PB 8 /* p-block width for the Krylov dot / axpy / du phases */
#define MAX_T 128

/* Horizontal sum of an AVX-512 accumulator (F/DQ-free: store + 8 adds). */
static inline double r512(__m512d v)
{
    double r[8];
    _mm512_storeu_pd(r, v);
    return ((r[0] + r[1] + r[2] + r[3]) + (r[4] + r[5] + r[6] + r[7]));
}

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
    double *slot = y + 16; /* nt*(PB+1) doubles of per-thread scratch */

    int nt = omp_get_max_threads();
    if (nt > MAX_T)
        nt = MAX_T;
    omp_set_num_threads(nt);

    int kstop = 0, kstop_k = 0;

    const double macheps = 2.220446049250313e-16;
    const double newton_rtol = 1.0e-14;
    const double sqrt_macheps = sqrt(macheps);
    const double h = 1.0 / (double)(N - 1);
    const double hh = h * h;
    const int64_t nm1 = N - 1;
    const int64_t n8 = n2 >> 3;
    const int64_t n8p = (n2 + 7) >> 3;
    const int64_t itail = 8 * n8;

    #pragma omp parallel
    {
        const int mytid = omp_get_thread_num();
        const int64_t rowbase = (int64_t)mytid * (PB + 1);
        double *mys = slot + rowbase;
        double s1 = 0.0, s2 = 0.0, s3 = 0.0;
        double rh[PB];
        double f0 = 0.0, fnow = 0.0;

        /* F = residual(u); f0 = ||F|| (per-row partials) */
        mys[0] = 0.0;
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
        s1 = 0.0;
        for (int t = 0; t < nt; t++)
            s1 += slot[(int64_t)t * (PB + 1)];
#pragma omp barrier
        f0 = sqrt(s1);
        fnow = f0;

        for (int step = 0; step < MAX_NEWTON; step++) {
            if (fnow <= newton_rtol * f0)
                break;
            /* ---- one GMRES(restart) cycle solving J du = -F ---- */
            const double beta = fnow;
            /* Q[0] = -F/beta; s2 = ||u||^2 (AVX-512) */
            mys[0] = 0.0;
            {
                const __m512d b8 = _mm512_set1_pd(beta);
                const __m512d z8 = _mm512_setzero_pd();
                __m512d va = _mm512_setzero_pd();
#pragma omp for schedule(static)
                for (int64_t c = 0; c < n8; c++) {
                    const int64_t i = 8 * c;
                    const __m512d F8 = _mm512_loadu_pd(F + i);
                    const __m512d u8 = _mm512_loadu_pd(u + i);
                    _mm512_storeu_pd(Q + i, _mm512_sub_pd(z8, _mm512_div_pd(F8, b8)));
                    va = _mm512_fmadd_pd(u8, u8, va);
                }
                if (n8p > n8) {
                    const __mmask8 mk = (__mmask8)((1u << (int)(n2 - itail)) - 1);
                    const int64_t i = itail;
#pragma omp for schedule(static)
                    for (int64_t c = n8; c < n8p; c++) {
                        const __m512d F8 = _mm512_maskz_loadu_pd(mk, F + i);
                        const __m512d u8 = _mm512_maskz_loadu_pd(mk, u + i);
                        _mm512_mask_storeu_pd(Q + i, mk, _mm512_sub_pd(z8, _mm512_div_pd(F8, b8)));
                        va = _mm512_fmadd_pd(u8, u8, va);
                    }
                }
                mys[0] += r512(va);
#pragma omp barrier
            }
            s2 = 0.0;
            for (int t = 0; t < nt; t++)
                s2 += slot[(int64_t)t * (PB + 1)];
#pragma omp barrier
            if (mytid == 0)
                g[0] = beta;
            const double nu = sqrt(s2);
            const double nuf = 1.0 + nu;
            for (int k = 0; k < RESTART; k++) {
                const double *Qk = Q + (int64_t)k * n2;
                /* nv = ||Q[k]|| (AVX-512) */
                mys[0] = 0.0;
                {
                    __m512d va = _mm512_setzero_pd();
#pragma omp for schedule(static)
                    for (int64_t c = 0; c < n8; c++)
                        va = _mm512_fmadd_pd(_mm512_loadu_pd(Qk + 8 * c),
                                             _mm512_loadu_pd(Qk + 8 * c), va);
                    if (n8p > n8) {
                        const __mmask8 mk = (__mmask8)((1u << (int)(n2 - itail)) - 1);
                        const int64_t i = itail;
#pragma omp for schedule(static)
                        for (int64_t c = n8; c < n8p; c++)
                            va = _mm512_fmadd_pd(_mm512_maskz_loadu_pd(mk, Qk + i),
                                                 _mm512_maskz_loadu_pd(mk, Qk + i), va);
                    }
                    mys[0] += r512(va);
#pragma omp barrier
                }
                s3 = 0.0;
                for (int t = 0; t < nt; t++)
                    s3 += slot[(int64_t)t * (PB + 1)];
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
                /* h[p] = Q[p].w (p = 0..k) with 8-lane accumulators;
                 * then w -= sum_p h[p] Q[p] */
                for (int64_t pb = 0; pb <= k; pb += PB) {
                    const int cnt = (int)((k + 1 - pb) < PB ? (k + 1 - pb) : PB);
                    for (int p = 0; p < cnt; p++)
                        mys[1 + p] = 0.0;
                    for (int p = 0; p < cnt; p++) {
                        const double *qp = Q + (int64_t)(pb + p) * n2;
                        __m512d va = _mm512_setzero_pd();
#pragma omp for schedule(static)
                        for (int64_t c = 0; c < n8; c++)
                            va = _mm512_fmadd_pd(_mm512_loadu_pd(w + 8 * c),
                                                 _mm512_loadu_pd(qp + 8 * c), va);
                        if (n8p > n8) {
                            const __mmask8 mk = (__mmask8)((1u << (int)(n2 - itail)) - 1);
                            const int64_t i = itail;
#pragma omp for schedule(static)
                            for (int64_t c = n8; c < n8p; c++)
                                va = _mm512_fmadd_pd(_mm512_maskz_loadu_pd(mk, w + i),
                                                     _mm512_maskz_loadu_pd(mk, qp + i),
                                                     va);
                        }
                        mys[1 + p] += r512(va);
#pragma omp barrier
                    }
                    for (int p = 0; p < cnt; p++) {
                        double tot = 0.0;
                        for (int t = 0; t < nt; t++)
                            tot += slot[(int64_t)t * (PB + 1) + 1 + p];
                        rh[p] = tot;
                    }
#pragma omp barrier
                    if (mytid == 0)
                        for (int p = 0; p < cnt; p++)
                            H[(pb + p) * RESTART + k] = rh[p];
                    double hv[PB];
                    for (int p = 0; p < cnt; p++)
                        hv[p] = -rh[p];
                    /* w -= sum_p hv[p] Q[p], 8-wide */
#pragma omp for schedule(static)
                    for (int64_t c = 0; c < n8; c++) {
                        const int64_t i = 8 * c;
                        double a0 = w[i + 0], a1 = w[i + 1], a2 = w[i + 2],
                               a3 = w[i + 3], a4 = w[i + 4], a5 = w[i + 5],
                               a6 = w[i + 6], a7 = w[i + 7];
                        for (int p = 0; p < cnt; p++) {
                            const double *qp = Q + (int64_t)(pb + p) * n2;
                            a0 += hv[p] * qp[i + 0];
                            a1 += hv[p] * qp[i + 1];
                            a2 += hv[p] * qp[i + 2];
                            a3 += hv[p] * qp[i + 3];
                            a4 += hv[p] * qp[i + 4];
                            a5 += hv[p] * qp[i + 5];
                            a6 += hv[p] * qp[i + 6];
                            a7 += hv[p] * qp[i + 7];
                        }
                        w[i + 0] = a0;
                        w[i + 1] = a1;
                        w[i + 2] = a2;
                        w[i + 3] = a3;
                        w[i + 4] = a4;
                        w[i + 5] = a5;
                        w[i + 6] = a6;
                        w[i + 7] = a7;
                    }
                    for (int64_t i = itail; i < n2; i++) {
                        double acc = w[i];
                        for (int p = 0; p < cnt; p++)
                            acc += hv[p] * Q[(int64_t)(pb + p) * n2 + i];
                        w[i] = acc;
                    }
                }
                /* h_next = ||w||, then Givens (thread 0) (AVX-512) */
                mys[0] = 0.0;
                {
                    __m512d va = _mm512_setzero_pd();
#pragma omp for schedule(static)
                    for (int64_t c = 0; c < n8; c++)
                        va = _mm512_fmadd_pd(_mm512_loadu_pd(w + 8 * c),
                                             _mm512_loadu_pd(w + 8 * c), va);
                    if (n8p > n8) {
                        const __mmask8 mk = (__mmask8)((1u << (int)(n2 - itail)) - 1);
                        const int64_t i = itail;
#pragma omp for schedule(static)
                        for (int64_t c = n8; c < n8p; c++)
                            va = _mm512_fmadd_pd(_mm512_maskz_loadu_pd(mk, w + i),
                                                 _mm512_maskz_loadu_pd(mk, w + i), va);
                    }
                    mys[0] += r512(va);
#pragma omp barrier
                }
                s3 = 0.0;
                for (int t = 0; t < nt; t++)
                    s3 += slot[(int64_t)t * (PB + 1)];
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
                for (int64_t c = 0; c < n8; c++) {
                    const int64_t i = 8 * c;
                    double a0 = du[i + 0], a1 = du[i + 1], a2 = du[i + 2],
                           a3 = du[i + 3], a4 = du[i + 4], a5 = du[i + 5],
                           a6 = du[i + 6], a7 = du[i + 7];
                    for (int p = 0; p < cnt; p++) {
                        const double *qp = Q + (int64_t)(pb + p) * n2;
                        a0 += yv[p] * qp[i + 0];
                        a1 += yv[p] * qp[i + 1];
                        a2 += yv[p] * qp[i + 2];
                        a3 += yv[p] * qp[i + 3];
                        a4 += yv[p] * qp[i + 4];
                        a5 += yv[p] * qp[i + 5];
                        a6 += yv[p] * qp[i + 6];
                        a7 += yv[p] * qp[i + 7];
                    }
                    du[i + 0] = a0;
                    du[i + 1] = a1;
                    du[i + 2] = a2;
                    du[i + 3] = a3;
                    du[i + 4] = a4;
                    du[i + 5] = a5;
                    du[i + 6] = a6;
                    du[i + 7] = a7;
                }
                for (int64_t i = itail; i < n2; i++) {
                    double acc = du[i];
                    for (int p = 0; p < cnt; p++)
                        acc += yv[p] * Q[(int64_t)(pb + p) * n2 + i];
                    du[i] = acc;
                }
            }
            /* u += du; F = residual(u); fnow = ||F|| */
#pragma omp for schedule(static)
            for (int64_t i = 0; i < n2; i++)
                u[i] += du[i];
            mys[0] = 0.0;
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
            s1 = 0.0;
            for (int t = 0; t < nt; t++)
                s1 += slot[(int64_t)t * (PB + 1)];
#pragma omp barrier
            fnow = sqrt(s1);
        }
    }
}
