#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Blocked Householder QR + least squares, matching the reference semantics.
 *
 * workspace (in doubles):
 *   Vp    : M*blk        current panel Householder vectors (M rows x blk cols)
 *   WA    : blk*N        right-hand W for A2
 *   WQ    : blk*N        right-hand W for Q2
 *   beta  : blk
 *   wpart : P*blk        per-thread panel-w partials
 *   sbuf  : P            per-thread S partials
 *   gpart : P*N          per-thread g partials
 *   wp2   : P*blk*JCB*2  per-thread W partials for right-hand
 */
#define JCB 32

void householder_qr_fp64(
    double *restrict A,
    double *restrict Q,
    double *restrict R,
    const double *restrict b,
    double *restrict x,
    const int64_t M,
    const int64_t N,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace_size;
    const long P = omp_get_max_threads();
    long blk = (N >= 192) ? 32 : (N >= 64 ? 16 : N);
    if (blk > 32) blk = 32;
    if (blk < 1) blk = 1;

    double *restrict Vp    = (double *)workspace;
    double *restrict WA    = Vp    + M * blk;
    double *restrict WQ    = WA    + blk * N;
    double *restrict beta  = WQ    + blk * N;
    double *restrict wpart = beta  + blk;
    double *restrict sbuf  = wpart + P * blk;
    double *restrict gpart = sbuf  + P;
    double *restrict wp2   = gpart + P * N;

    memset(Q, 0, (size_t)M * N * sizeof(double));

    for (long k0 = 0; k0 < N; k0 += blk) {
        const long bl = (blk < N - k0) ? blk : N - k0;   /* panel width */
        const long m0 = M - k0;
        const long n2 = N - k0 - bl;                     /* right-hand width */

        /* Vp rows 0..l-1 of column l are never written (v_l lives in rows l..m0-1) but ARE
         * read by the right-hand and in-panel-Q passes: zero them (workspace is not zeroed). */
        for (long l = 0; l < bl; l++)
            for (long i = 0; i < l; i++) Vp[i * blk + l] = 0.0;

        /* ---------- panel factorization (sequential in l) ---------- */
        for (long l = 0; l < bl; l++) {
            const long k  = k0 + l;
            const long m  = m0 - l;                      /* active length */
            const long aw = bl - l;                       /* active width */
            double *restrict vcol = Vp + (size_t)l * blk + l;
            const double *restrict acol = A + k * N + k;
            double S = 0.0;

            #pragma omp parallel
            {
                const int tid = omp_get_thread_num();
                double *restrict myw = wpart + (size_t)tid * blk;
                double mys = 0.0;
                for (long j = 0; j < aw; j++) myw[j] = 0.0;

                #pragma omp for schedule(static)
                for (long r = 0; r < m; r++) {
                    const double a = acol[r * N];
                    if (r == 0) {
                        mys += a * a;
                    } else {
                        vcol[r * blk] = a;
                        mys += a * a;
                        const double *restrict arow = A + (k + r) * N + k;
                        for (long j = 0; j < aw; j++) myw[j] += a * arow[j];
                    }
                }
                sbuf[tid] = mys;

                #pragma omp barrier
                if (tid == 0) {
                    S = sbuf[0];
                    for (int t = 1; t < P; t++) S += sbuf[t];
                    const double c0 = acol[0];
                    const double normx = sqrt(S);
                    const double v0 = c0 + (c0 >= 0.0 ? 1.0 : -1.0) * normx;
                    const double vv = v0 * v0 + (S - c0 * c0);
                    const double bk = (vv > 0.0) ? 2.0 / vv : 0.0;
                    vcol[0] = v0;
                    beta[l] = bk;
                    const double *arow0 = A + k * N + k;
                    for (long j = 0; j < aw; j++) {
                        double sj = 0.0;
                        for (int t = 0; t < P; t++) sj += wpart[(size_t)t * blk + j];
                        wpart[j] = bk * (sj + v0 * arow0[j]);
                    }
                }
                #pragma omp barrier

                #pragma omp for schedule(static)
                for (long r = 0; r < m; r++) {
                    const double vi = vcol[r * blk];
                    double *restrict arow = A + (k + r) * N + k;
                    for (long j = 0; j < aw; j++)
                        arow[j] -= vi * wpart[j];
                }
            }
            for (long j = 0; j < aw; j++) R[k * N + k + j] = A[k * N + k + j];
        }

        /* ---------- right-hand combined pass (A2 and Q2) ---------- */
        if (n2 > 0) {
            for (long c = 0; c < n2; c += JCB) {
                const long nc2 = (JCB < n2 - c) ? JCB : n2 - c;
                double *restrict WAc = WA + c;
                double *restrict WQc = WQ + c;

                /* W sub-pass: W[l,j] = sum_i Vp[i,l] * X2[i,j]  for X2 in {A2, Q2} */
                #pragma omp parallel
                {
                    const int tid = omp_get_thread_num();
                    double *restrict pA = wp2 + (size_t)tid * blk * JCB;
                    double *restrict pQ = pA + blk * JCB;
                    for (long l = 0; l < bl; l++)
                        for (long j = 0; j < nc2; j++) { pA[l * JCB + j] = 0.0; pQ[l * JCB + j] = 0.0; }

                    #pragma omp for schedule(static)
                    for (long i = 0; i < m0; i++) {
                        const double *restrict vrow = Vp + (size_t)i * blk;
                        const double *restrict a2row = A + (size_t)(k0 + i) * N + k0 + bl + c;
                        const double *restrict q2row = Q + (size_t)(k0 + i) * N + k0 + bl + c;
                        for (long l = 0; l < bl; l++) {
                            const double v = vrow[l];
                            double *ra = pA + l * JCB;
                            double *rq = pQ + l * JCB;
                            for (long j = 0; j < nc2; j++) {
                                ra[j] += v * a2row[j];
                                rq[j] += v * q2row[j];
                            }
                        }
                    }
                }
                for (long l = 0; l < bl; l++) {
                    const double blr = beta[l];
                    for (long j = 0; j < nc2; j++) {
                        double sa = 0.0, sq = 0.0;
                        for (int t = 0; t < P; t++) {
                            const double *p = wp2 + (size_t)t * blk * JCB;
                            sa += p[l * JCB + j];
                            sq += p[blk * JCB + l * JCB + j];
                        }
                        WAc[l * N + j] = blr * sa;
                        WQc[l * N + j] = blr * sq;
                    }
                }

                /* update sub-pass: X2[i,j] -= sum_l Vp[i,l] W[l,j] */
                #pragma omp parallel for schedule(static)
                for (long i = 0; i < m0; i++) {
                    const double *restrict vrow = Vp + (size_t)i * blk;
                    double *restrict a2row = A + (size_t)(k0 + i) * N + k0 + bl + c;
                    double *restrict q2row = Q + (size_t)(k0 + i) * N + k0 + bl + c;
                    for (long j = 0; j < nc2; j++) {
                        double sa = 0.0, sq = 0.0;
                        for (long l = 0; l < bl; l++) {
                            const double v = vrow[l];
                            sa += v * WAc[l * N + j];
                            sq += v * WQc[l * N + j];
                        }
                        a2row[j] -= sa;
                        q2row[j] -= sq;
                    }
                }
            }
            for (long l = 0; l < bl; l++)
                for (long j = 0; j < n2; j++)
                    R[(k0 + l) * N + k0 + bl + j] = A[(k0 + l) * N + k0 + bl + j];
        }

        /* ---------- in-panel Q columns ----------
         * Q[:, k0+l0] (rows k0:M) = e_l0 - sum_{l<l0} beta_l * Vp[l0,l] * Vp[:, l]
         */
        for (long l0 = 0; l0 < bl; l0++) {
            double *restrict qcol = Q + (size_t)k0 * N + k0 + l0;   /* col k0+l0, rows k0..M-1 */
            qcol[l0 * N] = 1.0;                                       /* diagonal */
            if (l0 == 0) continue;
            double csc[32];
            for (long l = 0; l < l0; l++) csc[l] = beta[l] * Vp[(size_t)l0 * blk + l];
            #pragma omp parallel for schedule(static)
            for (long i = 0; i < m0; i++) {
                const double *restrict vrow = Vp + (size_t)i * blk;
                double s = 0.0;
                for (long l = 0; l < l0; l++) s += csc[l] * vrow[l];
                qcol[i * N] -= s;
            }
        }
    }

    /* ---------- g = Q^T b (stored into x, overwritten by back substitution) ---------- */
    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *restrict myg = gpart + (size_t)tid * N;
        for (long j = 0; j < N; j++) myg[j] = 0.0;
        #pragma omp for schedule(static)
        for (long i = 0; i < M; i++) {
            const double bi = b[i];
            const double *restrict qrow = Q + (size_t)i * N;
            for (long j = 0; j < N; j++) myg[j] += qrow[j] * bi;
        }
    }
    for (long j = 0; j < N; j++) {
        double s = gpart[j];
        for (int t = 1; t < P; t++) s += gpart[(size_t)t * N + j];
        x[j] = s;
    }
    for (long i = N - 1; i >= 0; i--) {
        double s = x[i];
        for (long j = i + 1; j < N; j++) s -= R[i * N + j] * x[j];
        x[i] = s / R[i * N + i];
    }
}
