#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* ------------------------------------------------------------------ */
/* Fallback: natural-layout, stride-2 sweeps (handles any N).          */
/* ------------------------------------------------------------------ */
static void rb_sor_direct(const double *restrict f, double *restrict u,
                         const int64_t N, const int64_t TSTEPS, const double omega)
{
    if (N < 3 || TSTEPS <= 0) return;
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om1 = 1.0 - omega;
    const int64_t nm2 = N - 2;
    for (int64_t t = 0; t < TSTEPS; t++) {
        for (int parity = 0; parity < 2; parity++) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 1; i <= nm2; i++) {
                double *restrict ui = u + (size_t)i * (size_t)N;
                const double *restrict fim = f + (size_t)i * (size_t)N;
                const int64_t j0 = (((i + 1) & 1) == (parity & 1)) ? 1 : 2;
                for (int64_t j = j0; j <= nm2; j += 2) {
                    const double g = (ui[j - N] + ui[j + N] + ui[j - 1] + ui[j + 1]
                                     + h2 * fim[j]) * 0.25;
                    ui[j] = om1 * ui[j] + omega * g;
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* V1: color-compacted layout, one parallel-for per half-sweep.        */
/* Kept as the fallback path for degenerate blocking parameters.        */
/* ------------------------------------------------------------------ */
static void rb_sor_compact(const double *restrict f, double *restrict u,
                           const int64_t N, const int64_t TSTEPS, const double omega,
                           double *restrict R, double *restrict B,
                           double *restrict FCr, double *restrict FCb,
                           const int64_t M, const int64_t rowlen)
{
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om1 = 1.0 - omega;
    const int64_t nm2 = N - 2;
    const size_t rl = (size_t)rowlen;

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        const double *restrict fi = f + (size_t)i * (size_t)N;
        const double *restrict ui = u + (size_t)i * (size_t)N;
        double *restrict Ri = R + (size_t)i * rl;
        double *restrict Bi = B + (size_t)i * rl;
        double *restrict Fri = FCr + (size_t)i * rl;
        double *restrict Fbi = FCb + (size_t)i * rl;
        Ri[0] = ui[0];
        Ri[M + 1] = ui[N - 1];
        Bi[0] = ui[0];
        Bi[M + 1] = ui[N - 1];
        if (i & 1) {
            for (int64_t k = 0; k < M; k++) {
                Ri[k + 1] = ui[2 * k + 1];
                Bi[k + 1] = ui[2 * k + 2];
                Fri[k] = h2 * fi[2 * k + 1];
                Fbi[k] = h2 * fi[2 * k + 2];
            }
        } else {
            for (int64_t k = 0; k < M; k++) {
                Ri[k + 1] = ui[2 * k + 2];
                Bi[k + 1] = ui[2 * k + 1];
                Fri[k] = h2 * fi[2 * k + 2];
                Fbi[k] = h2 * fi[2 * k + 1];
            }
        }
    }

    #pragma omp parallel
    {
        for (int64_t t = 0; t < TSTEPS; t++) {
            #pragma omp for schedule(static)
            for (int64_t i = 1; i <= nm2; i++) {
                const double *restrict Bu = B + (size_t)(i - 1) * rl + 1;
                const double *restrict Bd = B + (size_t)(i + 1) * rl + 1;
                const double *restrict Bc = B + (size_t)i * rl + ((i & 1) ? 0 : 1);
                const double *restrict Fc = FCr + (size_t)i * rl;
                double *restrict Ri = R + (size_t)i * rl + 1;
                #pragma omp simd
                for (int64_t k = 0; k < M; k++) {
                    const double g = (Bu[k] + Bd[k] + Bc[k] + Bc[k + 1] + Fc[k]) * 0.25;
                    Ri[k] = om1 * Ri[k] + omega * g;
                }
            }
            #pragma omp for schedule(static)
            for (int64_t i = 1; i <= nm2; i++) {
                const double *restrict Ru = R + (size_t)(i - 1) * rl + 1;
                const double *restrict Rd = R + (size_t)(i + 1) * rl + 1;
                const double *restrict Rc = R + (size_t)i * rl + ((i & 1) ? 1 : 0);
                const double *restrict Fc = FCb + (size_t)i * rl;
                double *restrict Bi = B + (size_t)i * rl + 1;
                #pragma omp simd
                for (int64_t k = 0; k < M; k++) {
                    const double g = (Ru[k] + Rd[k] + Rc[k] + Rc[k + 1] + Fc[k]) * 0.25;
                    Bi[k] = om1 * Bi[k] + omega * g;
                }
            }
        }
    }

    #pragma omp parallel for schedule(static)
    for (int64_t i = 1; i <= nm2; i++) {
        double *restrict ui = u + (size_t)i * (size_t)N;
        const double *restrict Ri = R + (size_t)i * rl + 1;
        const double *restrict Bi = B + (size_t)i * rl + 1;
        if (i & 1) {
            for (int64_t k = 0; k < M; k++) {
                ui[2 * k + 1] = Ri[k];
                ui[2 * k + 2] = Bi[k];
            }
        } else {
            for (int64_t k = 0; k < M; k++) {
                ui[2 * k + 2] = Ri[k];
                ui[2 * k + 1] = Bi[k];
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* V2: temporally blocked red-black SOR.                                */
/*                                                                      */
/* In half-step units, point (row i, half-step s) depends only on rows  */
/* i-1,i,i+1 at half-step s-1, so influence spreads exactly one row per */
/* half-step.  A strip of H core rows therefore advances S half-steps   */
/* with a halo of S-1 rows whose active range shrinks by one row per    */
/* side per step (trapezoid).  Halo rows are recomputed redundantly in  */
/* a private strip buffer, so strips are independent; only core rows    */
/* are written back.                                                    */
/*                                                                      */
/*   * BOTH colours' initial values are read (red as the self term of   */
/*     the s=0 update, black as neighbour and self term), so both are   */
/*     packed once into global static arrays Rg/Bg (u itself gets      */
/*     progressively overwritten by earlier strips' copy-out)           */
/*   * copy-in: red/black from Rg/Bg, h2*f values from f, ghosts from u */
/*   * copy-out: interle copy-out: interleaved red/black write = one contiguous row of u  */

/* ------------------------------------------------------------------ */
/* V3: L2-tiled temporally blocked red-black SOR.                       */
/*                                                                      */
/* V2's strips were L3-resident: every half-step re-streamed the whole   */
/* strip buffer from L3, so the kernel was L3-bandwidth bound, and the   */
/* per-half-step barriers cost ~40% at 24 threads.  V3 blocks in both    */
/* dimensions: a tile = H core rows x C core columns (plus an S-1 row     */
/* halo and 1-column ghosts) is buffered privately so it fits in L2,     */
/* and the thread that owns it runs all S half-steps of the time-tile    */
/* straight out of L2.  Influence spreads one row per half-step, so a    */
/* tile only needs its own halo recomputed redundantly; tiles are fully  */
/* independent and one omp-for over all tiles per time-tile              */
/* replaces all strip/half-step barriers.                               */
/* ------------------------------------------------------------------ */
static void rb_sor_tiled(const double *restrict f, double *restrict u,
                        const int64_t N, const int64_t TSTEPS, const double omega,
                        double *restrict Rg, double *restrict Bg,
                        double *restrict Rg2, double *restrict Bg2,
                        double *restrict tbuf,
                        const int64_t M, const int64_t rowlen,
                        const int64_t H, const int64_t S, const int64_t C,
                        const int64_t nthr)
{
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om1 = 1.0 - omega;
    const int64_t nm2 = N - 2;
    const size_t rl = (size_t)rowlen;
    const int64_t trows = H + 2 * S;      /* tile buffer rows */
    const int64_t tcols = C + 2 * S;
    const int64_t nstrips = (nm2 + H - 1) / H;
    const int64_t ncolt = (M + C - 1) / C;
    const int64_t ntiles = nstrips * ncolt;
    const int64_t total_hs = 2 * TSTEPS;
    (void)nthr;

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *tBR = tbuf + (size_t)tid * 4 * (size_t)trows * (size_t)tcols;
        double *tBB = tBR + (size_t)trows * (size_t)tcols;
        double *tFR = tBB + (size_t)trows * (size_t)tcols;
        double *tFB = tFR + (size_t)trows * (size_t)tcols;

        /* ---- single time-tile: pack u into Rg/Bg first (tile 0 both reads
           and writes u, so it must read the packed copy to avoid a race) ---- */
        const int64_t ntt = (total_hs + S - 1) / S;
        if (ntt == 1) {
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                const double *restrict ui = u + (size_t)i * (size_t)N;
                double *restrict Ri = Rg + (size_t)i * rl;
                double *restrict Bi = Bg + (size_t)i * rl;
                Ri[0] = ui[0];
                Ri[1 + M] = ui[N - 1];
                Bi[0] = ui[0];
                Bi[1 + M] = ui[N - 1];
                #pragma omp simd
                for (int64_t k = 0; k < M; k++) {
                    Ri[k + 1] = (i & 1) ? ui[2 * k + 1] : ui[2 * k + 2];
                    Bi[k + 1] = (i & 1) ? ui[2 * k + 2] : ui[2 * k + 1];
                }
            }
        }

        /* ---- time-tiles ----
           No full-grid pack for multi-tile runs: tile 0 reads the initial
           state straight from u, and the fixed cells (boundary rows, ghost
           columns) are always read from u.  Interior tiles exchange core
           segments through the state arrays; the last tile writes u. */
        for (int64_t s0 = 0, tile = 0; s0 < total_hs; s0 += S, tile++) {
        const int64_t St = (total_hs - s0 < S) ? (total_hs - s0) : S;
        const bool first_tile = (tile == 0);
        const bool last_tile = (s0 + St >= total_hs);
        const double *restrict srcR = first_tile ? ((ntt == 1) ? Rg : NULL)
                                                 : ((tile & 1) ? Rg : Rg2);
        const double *restrict srcB = first_tile ? ((ntt == 1) ? Bg : NULL)
                                                 : ((tile & 1) ? Bg : Bg2);
        double *restrict dstR = last_tile ? NULL : ((tile & 1) ? Rg2 : Rg);
        double *restrict dstB = last_tile ? NULL : ((tile & 1) ? Bg2 : Bg);

        #pragma omp for schedule(static)
        for (int64_t tile_id = 0; tile_id < ntiles; tile_id++) {
            const int64_t m = tile_id / ncolt;
            const int64_t j = tile_id - m * ncolt;
            const int64_t a = 1 + m * H;
            const int64_t b = (a + H - 1 <= nm2) ? (a + H - 1) : nm2;
            const int64_t c0 = j * C;
            const int64_t c1 = (c0 + C - 1 <= M - 1) ? (c0 + C - 1) : (M - 1);
            const int64_t alo = (a - St >= 0) ? (a - St) : 0;
            const int64_t ahi = (b + St <= N - 1) ? (b + St) : (N - 1);
            /* column halo of St (clipped to the static ghost columns -1 / M) */
            const int64_t klo = (c0 - St >= -1) ? (c0 - St) : -1;
            const int64_t khi = (c1 + St <= M) ? (c1 + St) : M;
            const int64_t kb0 = (klo >= 0) ? klo : 0;   /* first real column */
            const int64_t kb1 = (khi <= M - 1) ? khi : (M - 1); /* last real column */

            /* ---- copy-in: rows [alo,ahi] x cols [klo,khi] ----
               u-sourced rows: tile 0 and the (constant) boundary rows.
               Ghost columns (k = -1 / M) are the constant boundary values. */
            for (int64_t g = alo; g <= ahi; g++) {
                if (g + 6 <= ahi && srcR) {
                    __builtin_prefetch(srcR + (size_t)(g + 6) * rl + 1 + klo, 0, 3);
                    __builtin_prefetch(srcB + (size_t)(g + 6) * rl + 1 + klo, 0, 3);
                }
                __builtin_prefetch(f + (size_t)(g + 6) * (size_t)N + 2 * (klo > 0 ? klo : 0), 0, 3);
                const int64_t r = g - alo;
                double *restrict tR = tBR + (size_t)r * (size_t)tcols;
                double *restrict tB = tBB + (size_t)r * (size_t)tcols;
                double *restrict tF_ = tFR + (size_t)r * (size_t)tcols;
                double *restrict tG_ = tFB + (size_t)r * (size_t)tcols;
                const double *restrict fg = f + (size_t)g * (size_t)N;
                const double *restrict ug = u + (size_t)g * (size_t)N;
                const int64_t W = khi - klo + 1;
                if (klo < 0) { tR[0] = ug[0]; tB[0] = ug[0]; }
                if (khi >= M) { tR[W - 1] = ug[N - 1]; tB[W - 1] = ug[N - 1]; }
                const int64_t kks = (klo < 0) ? 1 : 0;
                const int64_t kke = (khi >= M) ? W - 1 : W;
                if (!srcR || g == 0 || g == N - 1) {
                    /* initial state, straight from u (stride-2 deinterleave) */
                    const int64_t oR = (g & 1) ? 1 : 2;
                    const int64_t ooB = (g & 1) ? 2 : 1;
                    #pragma omp simd
                    for (int64_t kk = kks; kk < kke; kk++) {
                        const int64_t k = kk + klo;
                        tR[kk] = ug[2 * k + oR];
                        tB[kk] = ug[2 * k + ooB];
                    }
                } else {
                    const double *restrict sR = srcR + (size_t)g * rl + 1 + klo;
                    const double *restrict sB = srcB + (size_t)g * rl + 1 + klo;
                    #pragma omp simd
                    for (int64_t kk = kks; kk < kke; kk++) {
                        tR[kk] = sR[kk];
                        tB[kk] = sB[kk];
                    }
                }
                if (g & 1) {
                    #pragma omp simd
                    for (int64_t kk = kb0 - klo; kk <= kb1 - klo; kk++) {
                        const int64_t k = kk + klo;
                        tF_[kk] = h2 * fg[2 * k + 1];
                        tG_[kk] = h2 * fg[2 * k + 2];
                    }
                } else {
                    #pragma omp simd
                    for (int64_t kk = kb0 - klo; kk <= kb1 - klo; kk++) {
                        const int64_t k = kk + klo;
                        tF_[kk] = h2 * fg[2 * k + 2];
                        tG_[kk] = h2 * fg[2 * k + 1];
                    }
                }
            }

            /* ---- St half-steps of the 2-D trapezoid, all inside L2 ---- */
            for (int64_t s = 0; s < St; s++) {
                const int64_t half = St - 1 - s;
                int64_t lo = a - half; if (lo < 1) lo = 1;
                int64_t hi = b + half; if (hi > nm2) hi = nm2;
                int64_t k0 = c0 - half; if (k0 < 0) k0 = 0;
                int64_t k1 = c1 + half; if (k1 > M - 1) k1 = M - 1;
                const int64_t kk0 = k0 - klo;
                const int64_t kk1 = k1 - klo;
                if (((s0 + s) & 1) == 0) {
                    /* red: update tBR from tBB */
                    for (int64_t g = lo; g <= hi; g++) {
                        const int64_t r = g - alo;
                        const double *restrict Bu = tBB + (size_t)(r - 1) * (size_t)tcols;
                        const double *restrict Bd = tBB + (size_t)(r + 1) * (size_t)tcols;
                        const double *restrict Bc = tBB + (size_t)r * (size_t)tcols + ((g & 1) ? -1 : 0);
                        const double *restrict Fc = tFR + (size_t)r * (size_t)tcols;
                        double *restrict Rt = tBR + (size_t)r * (size_t)tcols;
                        #pragma omp simd
                        for (int64_t kk = kk0; kk <= kk1; kk++) {
                            const double g2 = (Bu[kk] + Bd[kk] + Bc[kk] + Bc[kk + 1] + Fc[kk]) * 0.25;
                            Rt[kk] = om1 * Rt[kk] + omega * g2;
                        }
                    }
                } else {
                    /* black: update tBB from tBR */
                    for (int64_t g = lo; g <= hi; g++) {
                        const int64_t r = g - alo;
                        const double *restrict Ru = tBR + (size_t)(r - 1) * (size_t)tcols;
                        const double *restrict Rd = tBR + (size_t)(r + 1) * (size_t)tcols;
                        const double *restrict Rc = tBR + (size_t)r * (size_t)tcols + ((g & 1) ? 0 : -1);
                        const double *restrict Fc = tFB + (size_t)r * (size_t)tcols;
                        double *restrict Bt = tBB + (size_t)r * (size_t)tcols;
                        #pragma omp simd
                        for (int64_t kk = kk0; kk <= kk1; kk++) {
                            const double g2 = (Ru[kk] + Rd[kk] + Rc[kk] + Rc[kk + 1] + Fc[kk]) * 0.25;
                            Bt[kk] = om1 * Bt[kk] + omega * g2;
                        }
                    }
                }
            }
            /* ---- copy-out: core rows [a,b] x core cols [c0,c1] ---- */
            for (int64_t g = a; g <= b; g++) {
                const int64_t r = g - alo;
                const double *restrict Rr = tBR + (size_t)r * (size_t)tcols + (c0 - klo);
                const double *restrict Br = tBB + (size_t)r * (size_t)tcols + (c0 - klo);
                if (last_tile) {
                    double *restrict ug = u + (size_t)g * (size_t)N;
                    if (g & 1) {
                        for (int64_t k = c0; k <= c1; k++) {
                            ug[2 * k + 1] = Rr[k - c0];
                            ug[2 * k + 2] = Br[k - c0];
                        }
                    } else {
                        for (int64_t k = c0; k <= c1; k++) {
                            ug[2 * k + 2] = Rr[k - c0];
                            ug[2 * k + 1] = Br[k - c0];
                        }
                    }
                } else {
                    double *restrict dR = dstR + (size_t)g * rl + 1 + c0;
                    double *restrict dB = dstB + (size_t)g * rl + 1 + c0;
                    for (int64_t k = c0; k <= c1; k++) {
                        dR[k - c0] = Rr[k - c0];
                        dB[k - c0] = Br[k - c0];
                    }
                }
            }
        }
        } /* time-tile loop */
    }     /* parallel */
}

static void rb_sor_tiled_om1(const double *restrict f, double *restrict u,
                        const int64_t N, const int64_t TSTEPS, const double omega,
                        double *restrict Rg, double *restrict Bg,
                        double *restrict Rg2, double *restrict Bg2,
                        double *restrict tbuf,
                        const int64_t M, const int64_t rowlen,
                        const int64_t H, const int64_t S, const int64_t C,
                        const int64_t nthr)
{
    (void) omega; /* this specialization is omega == 1 */
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const int64_t nm2 = N - 2;
    const size_t rl = (size_t)rowlen;
    const int64_t trows = H + 2 * S;      /* tile buffer rows */
    const int64_t tcols = C + 2 * S;
    const int64_t nstrips = (nm2 + H - 1) / H;
    const int64_t ncolt = (M + C - 1) / C;
    const int64_t ntiles = nstrips * ncolt;
    const int64_t total_hs = 2 * TSTEPS;
    (void)nthr;

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        double *tBR = tbuf + (size_t)tid * 4 * (size_t)trows * (size_t)tcols;
        double *tBB = tBR + (size_t)trows * (size_t)tcols;
        double *tFR = tBB + (size_t)trows * (size_t)tcols;
        double *tFB = tFR + (size_t)trows * (size_t)tcols;

        /* ---- single time-tile: pack u into Rg/Bg first (tile 0 both reads
           and writes u, so it must read the packed copy to avoid a race) ---- */
        const int64_t ntt = (total_hs + S - 1) / S;
        if (ntt == 1) {
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < N; i++) {
                const double *restrict ui = u + (size_t)i * (size_t)N;
                double *restrict Ri = Rg + (size_t)i * rl;
                double *restrict Bi = Bg + (size_t)i * rl;
                Ri[0] = ui[0];
                Ri[1 + M] = ui[N - 1];
                Bi[0] = ui[0];
                Bi[1 + M] = ui[N - 1];
                #pragma omp simd
                for (int64_t k = 0; k < M; k++) {
                    Ri[k + 1] = (i & 1) ? ui[2 * k + 1] : ui[2 * k + 2];
                    Bi[k + 1] = (i & 1) ? ui[2 * k + 2] : ui[2 * k + 1];
                }
            }
        }

        /* ---- time-tiles ----
           No full-grid pack for multi-tile runs: tile 0 reads the initial
           state straight from u, and the fixed cells (boundary rows, ghost
           columns) are always read from u.  Interior tiles exchange core
           segments through the state arrays; the last tile writes u. */
        for (int64_t s0 = 0, tile = 0; s0 < total_hs; s0 += S, tile++) {
        const int64_t St = (total_hs - s0 < S) ? (total_hs - s0) : S;
        const bool first_tile = (tile == 0);
        const bool last_tile = (s0 + St >= total_hs);
        const double *restrict srcR = first_tile ? ((ntt == 1) ? Rg : NULL)
                                                 : ((tile & 1) ? Rg : Rg2);
        const double *restrict srcB = first_tile ? ((ntt == 1) ? Bg : NULL)
                                                 : ((tile & 1) ? Bg : Bg2);
        double *restrict dstR = last_tile ? NULL : ((tile & 1) ? Rg2 : Rg);
        double *restrict dstB = last_tile ? NULL : ((tile & 1) ? Bg2 : Bg);

        #pragma omp for schedule(static)
        for (int64_t tile_id = 0; tile_id < ntiles; tile_id++) {
            const int64_t m = tile_id / ncolt;
            const int64_t j = tile_id - m * ncolt;
            const int64_t a = 1 + m * H;
            const int64_t b = (a + H - 1 <= nm2) ? (a + H - 1) : nm2;
            const int64_t c0 = j * C;
            const int64_t c1 = (c0 + C - 1 <= M - 1) ? (c0 + C - 1) : (M - 1);
            const int64_t alo = (a - St >= 0) ? (a - St) : 0;
            const int64_t ahi = (b + St <= N - 1) ? (b + St) : (N - 1);
            /* column halo of St (clipped to the static ghost columns -1 / M) */
            const int64_t klo = (c0 - St >= -1) ? (c0 - St) : -1;
            const int64_t khi = (c1 + St <= M) ? (c1 + St) : M;
            const int64_t kb0 = (klo >= 0) ? klo : 0;   /* first real column */
            const int64_t kb1 = (khi <= M - 1) ? khi : (M - 1); /* last real column */

            /* ---- copy-in: rows [alo,ahi] x cols [klo,khi] ----
               u-sourced rows: tile 0 and the (constant) boundary rows.
               Ghost columns (k = -1 / M) are the constant boundary values. */
            for (int64_t g = alo; g <= ahi; g++) {
                if (g + 6 <= ahi && srcR) {
                    __builtin_prefetch(srcR + (size_t)(g + 6) * rl + 1 + klo, 0, 3);
                    __builtin_prefetch(srcB + (size_t)(g + 6) * rl + 1 + klo, 0, 3);
                }
                __builtin_prefetch(f + (size_t)(g + 6) * (size_t)N + 2 * (klo > 0 ? klo : 0), 0, 3);
                const int64_t r = g - alo;
                double *restrict tR = tBR + (size_t)r * (size_t)tcols;
                double *restrict tB = tBB + (size_t)r * (size_t)tcols;
                double *restrict tF_ = tFR + (size_t)r * (size_t)tcols;
                double *restrict tG_ = tFB + (size_t)r * (size_t)tcols;
                const double *restrict fg = f + (size_t)g * (size_t)N;
                const double *restrict ug = u + (size_t)g * (size_t)N;
                const int64_t W = khi - klo + 1;
                if (klo < 0) { tR[0] = ug[0]; tB[0] = ug[0]; }
                if (khi >= M) { tR[W - 1] = ug[N - 1]; tB[W - 1] = ug[N - 1]; }
                const int64_t kks = (klo < 0) ? 1 : 0;
                const int64_t kke = (khi >= M) ? W - 1 : W;
                if (!srcR || g == 0 || g == N - 1) {
                    /* initial state, straight from u (stride-2 deinterleave) */
                    const int64_t oR = (g & 1) ? 1 : 2;
                    const int64_t ooB = (g & 1) ? 2 : 1;
                    #pragma omp simd
                    for (int64_t kk = kks; kk < kke; kk++) {
                        const int64_t k = kk + klo;
                        tR[kk] = ug[2 * k + oR];
                        tB[kk] = ug[2 * k + ooB];
                    }
                } else {
                    const double *restrict sR = srcR + (size_t)g * rl + 1 + klo;
                    const double *restrict sB = srcB + (size_t)g * rl + 1 + klo;
                    #pragma omp simd
                    for (int64_t kk = kks; kk < kke; kk++) {
                        tR[kk] = sR[kk];
                        tB[kk] = sB[kk];
                    }
                }
                if (g & 1) {
                    #pragma omp simd
                    for (int64_t kk = kb0 - klo; kk <= kb1 - klo; kk++) {
                        const int64_t k = kk + klo;
                        tF_[kk] = h2 * fg[2 * k + 1];
                        tG_[kk] = h2 * fg[2 * k + 2];
                    }
                } else {
                    #pragma omp simd
                    for (int64_t kk = kb0 - klo; kk <= kb1 - klo; kk++) {
                        const int64_t k = kk + klo;
                        tF_[kk] = h2 * fg[2 * k + 2];
                        tG_[kk] = h2 * fg[2 * k + 1];
                    }
                }
            }

            /* ---- St half-steps of the 2-D trapezoid, all inside L2 ---- */
            for (int64_t s = 0; s < St; s++) {
                const int64_t half = St - 1 - s;
                int64_t lo = a - half; if (lo < 1) lo = 1;
                int64_t hi = b + half; if (hi > nm2) hi = nm2;
                int64_t k0 = c0 - half; if (k0 < 0) k0 = 0;
                int64_t k1 = c1 + half; if (k1 > M - 1) k1 = M - 1;
                const int64_t kk0 = k0 - klo;
                const int64_t kk1 = k1 - klo;
                if (((s0 + s) & 1) == 0) {
                    /* red: update tBR from tBB */
                    for (int64_t g = lo; g <= hi; g++) {
                        const int64_t r = g - alo;
                        const double *restrict Bu = tBB + (size_t)(r - 1) * (size_t)tcols;
                        const double *restrict Bd = tBB + (size_t)(r + 1) * (size_t)tcols;
                        const double *restrict Bc = tBB + (size_t)r * (size_t)tcols + ((g & 1) ? -1 : 0);
                        const double *restrict Fc = tFR + (size_t)r * (size_t)tcols;
                        double *restrict Rt = tBR + (size_t)r * (size_t)tcols;
                        #pragma omp simd
                        for (int64_t kk = kk0; kk <= kk1; kk++) {
                            const double g2 = (Bu[kk] + Bd[kk] + Bc[kk] + Bc[kk + 1] + Fc[kk]) * 0.25;
                            Rt[kk] = g2;
                        }
                    }
                } else {
                    /* black: update tBB from tBR */
                    for (int64_t g = lo; g <= hi; g++) {
                        const int64_t r = g - alo;
                        const double *restrict Ru = tBR + (size_t)(r - 1) * (size_t)tcols;
                        const double *restrict Rd = tBR + (size_t)(r + 1) * (size_t)tcols;
                        const double *restrict Rc = tBR + (size_t)r * (size_t)tcols + ((g & 1) ? 0 : -1);
                        const double *restrict Fc = tFB + (size_t)r * (size_t)tcols;
                        double *restrict Bt = tBB + (size_t)r * (size_t)tcols;
                        #pragma omp simd
                        for (int64_t kk = kk0; kk <= kk1; kk++) {
                            const double g2 = (Ru[kk] + Rd[kk] + Rc[kk] + Rc[kk + 1] + Fc[kk]) * 0.25;
                            Bt[kk] = g2;
                        }
                    }
                }
            }
            /* ---- copy-out: core rows [a,b] x core cols [c0,c1] ---- */
            for (int64_t g = a; g <= b; g++) {
                const int64_t r = g - alo;
                const double *restrict Rr = tBR + (size_t)r * (size_t)tcols + (c0 - klo);
                const double *restrict Br = tBB + (size_t)r * (size_t)tcols + (c0 - klo);
                if (last_tile) {
                    double *restrict ug = u + (size_t)g * (size_t)N;
                    if (g & 1) {
                        for (int64_t k = c0; k <= c1; k++) {
                            ug[2 * k + 1] = Rr[k - c0];
                            ug[2 * k + 2] = Br[k - c0];
                        }
                    } else {
                        for (int64_t k = c0; k <= c1; k++) {
                            ug[2 * k + 2] = Rr[k - c0];
                            ug[2 * k + 1] = Br[k - c0];
                        }
                    }
                } else {
                    double *restrict dR = dstR + (size_t)g * rl + 1 + c0;
                    double *restrict dB = dstB + (size_t)g * rl + 1 + c0;
                    for (int64_t k = c0; k <= c1; k++) {
                        dR[k - c0] = Rr[k - c0];
                        dB[k - c0] = Br[k - c0];
                    }
                }
            }
        }
        } /* time-tile loop */
    }     /* parallel */
}


/* ------------------------------------------------------------------ */
/* Dispatcher                                                           */
/* ------------------------------------------------------------------ */
#ifndef RB_L2_BUDGET
#define RB_L2_BUDGET (704u << 10)   /* target L2 bytes of one tile's 4 buffers */
#endif
#ifndef RB_MAX_S
#define RB_MAX_S 22                  /* half-steps per time-tile (even) */
#endif
#ifndef RB_SMALL_GRID
#define RB_SMALL_GRID ((size_t)8 << 20)
#endif

void rb_sor_fp64(
    const double *restrict f,
    double *restrict u,
    const int64_t N,
    const int64_t TSTEPS,
    const double omega,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    if (TSTEPS <= 0) return;
    if (N < 4 || (N & 1)) { rb_sor_direct(f, u, N, TSTEPS, omega); return; }

    const int64_t M = (N - 2) / 2;
    const int64_t rowlen = (M + 2 + 7) & ~(int64_t)7;
    const size_t rl = (size_t)rowlen;
    const int64_t nm2 = N - 2;

    /* small grids: the whole compacted problem fits in L3, use V1 */
    if ((size_t)4 * (size_t)N * rl * sizeof(double) <= RB_SMALL_GRID) {
        const size_t need = (size_t)4 * (size_t)N * rl * sizeof(double);
        double *R = (double *)aligned_alloc(64, (need + 63) & ~(size_t)63);
        if (R) {
            rb_sor_compact(f, u, N, TSTEPS, omega, R,
                           R + (size_t)N * rl, R + 2 * (size_t)N * rl,
                           R + 3 * (size_t)N * rl, M, rowlen);
            free(R);
        } else {
            rb_sor_direct(f, u, N, TSTEPS, omega);
        }
        return;
    }

    /* ---- V3: 2-D trapezoid tiles, L2-resident ----
     * Tile = H core rows x C core columns with an (S-1)-deep temporal
     * halo on all FOUR sides (influence spreads one row AND one column
     * per half-step), so a tile advances S half-steps entirely in L2.
     * (H+2S)(C+2S) doubles per colour array ~= RB_L2_BUDGET.
     */
    /* Balanced time-tiling: split the 2*TSTEPS half-steps over the minimum
       number of time-tiles of at most RB_MAX_S half-steps each, as evenly as
       possible (the last tile's halo is sized by its own step count, and the
       red/black phase is tracked so odd step counts are allowed). */
    const int64_t total_hs = 2 * TSTEPS;
    int64_t ntt = (total_hs + RB_MAX_S - 1) / RB_MAX_S;
    if (ntt < 1) ntt = 1;
    int64_t S = (total_hs + ntt - 1) / ntt;
    if (S < 2) S = 2;

    const int64_t area = (int64_t)(RB_L2_BUDGET / (4u * sizeof(double)));
    int64_t side = (int64_t)sqrt((double)area);
    if (side < 2 * S + 16) side = 2 * S + 16;
    int64_t H = side - 2 * S;
    if (H > nm2) H = nm2;
    int64_t C = area / (H + 2 * S) - 2 * S;
    if (C > M) C = M;
    if (C < 16) C = M;   /* single column tile */

    const int64_t ncolt = (M + C - 1) / C;
    int nthr = omp_get_max_threads();
    if (nthr < 1) nthr = 1;
    /* keep at least ~4 tiles per thread busy */
    {
        int64_t want_strips = (int64_t)((4 * nthr + ncolt - 1) / ncolt);
        if (want_strips < 1) want_strips = 1;
        int64_t nstrips = (nm2 + H - 1) / H;
        if (nstrips < want_strips) {
            int64_t Hn = (nm2 + want_strips - 1) / want_strips;
            if (Hn < 32) Hn = 32;
            if (Hn < H) H = Hn;
        }
    }

    if (H < 16) {
        /* degenerate: fall back to V1 */
        const size_t need = (size_t)4 * (size_t)N * rl * sizeof(double);
        double *R = (double *)aligned_alloc(64, (need + 63) & ~(size_t)63);
        if (R) {
            rb_sor_compact(f, u, N, TSTEPS, omega, R,
                           R + (size_t)N * rl, R + 2 * (size_t)N * rl,
                           R + 3 * (size_t)N * rl, M, rowlen);
            free(R);
        } else {
            rb_sor_direct(f, u, N, TSTEPS, omega);
        }
        return;
    }

    const int64_t n_tiles = (2 * TSTEPS + S - 1) / S;
    const size_t one_g = (size_t)N * rl * sizeof(double);
    /* state arrays: n_tiles==1 -> pack source; n_tiles==2 -> one buffer;
       n_tiles>2 -> double buffer. */
    const size_t need_g = (n_tiles > 2) ? 4 * one_g : 2 * one_g;
    const int64_t trows = H + 2 * S;
    const int64_t tcols = C + 2 * S;
    const size_t need_buf = (size_t)nthr * (size_t)4
                          * (size_t)trows * (size_t)tcols * sizeof(double);
    const size_t total = ((need_g + 63) & ~(size_t)63) + ((need_buf + 63) & ~(size_t)63);
    /* The multi-hundred-MB scratch (state arrays + per-thread tiles) is
       cached across calls: re-faulting it every call would cost far more
       than the solver itself.  Grows monotonically, never freed. */
    static uint8_t *mem;
    static size_t mem_size;
    if (mem_size < total) {
        if (mem) free(mem);
        mem = (uint8_t *)aligned_alloc(64, (total + 63) & ~(size_t)63);
        if (!mem) { mem_size = 0; rb_sor_direct(f, u, N, TSTEPS, omega); return; }
        mem_size = total;
    }
    double *Rg  = (double *)mem;
    double *Bg  = (double *)(mem + one_g);
    double *Rg2 = (n_tiles > 2) ? (double *)(mem + 2 * one_g) : NULL;
    double *Bg2 = (n_tiles > 2) ? (double *)(mem + 3 * one_g) : NULL;
    double *tbuf = (double *)(mem + ((need_g + 63) & ~(size_t)63));

    if (omega == 1.0)
        rb_sor_tiled_om1(f, u, N, TSTEPS, omega, Rg, Bg, Rg2, Bg2, tbuf,
                         M, rowlen, H, S, C, nthr);
    else
        rb_sor_tiled(f, u, N, TSTEPS, omega, Rg, Bg, Rg2, Bg2, tbuf,
                     M, rowlen, H, S, C, nthr);
}
