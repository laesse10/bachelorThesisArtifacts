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
static void rb_sor_blocked(const double *restrict f, double *restrict u,
                           const int64_t N, const int64_t TSTEPS, const double omega,
                           double *restrict Rg, double *restrict Bg,
                           double *restrict Rg2, double *restrict Bg2,
                           double *restrict buf,
                           const int64_t M, const int64_t rowlen,
                           const int64_t H, const int64_t S)
{
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om1 = 1.0 - omega;
    const int64_t nm2 = N - 2;
    const size_t rl = (size_t)rowlen;
    const int64_t nrb = H + 2 * S;          /* strip buffer rows */
    const int64_t total_hs = 2 * TSTEPS;
    const int64_t nstrips = (nm2 + H - 1) / H;

    /* pack both colours' initial values into Rg / Bg */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        const double *restrict ui = u + (size_t)i * (size_t)N;
        double *restrict Ri = Rg + (size_t)i * rl + 1;
        double *restrict Bi = Bg + (size_t)i * rl + 1;
        if (i & 1) {
            for (int64_t k = 0; k < M; k++) {
                Ri[k] = ui[2 * k + 1];
                Bi[k] = ui[2 * k + 2];
            }
        } else {
            for (int64_t k = 0; k < M; k++) {
                Ri[k] = ui[2 * k + 2];
                Bi[k] = ui[2 * k + 1];
            }
        }
    }

    double *BR  = buf;
    double *BB  = buf + (size_t)nrb * rl;
    double *BFr = buf + 2 * (size_t)nrb * rl;
    double *BFb = buf + 3 * (size_t)nrb * rl;

    for (int64_t s0 = 0, tile = 0; s0 < total_hs; s0 += S, tile++) {
    const int64_t St = (total_hs - s0 < S) ? (total_hs - s0) : S;
    const double *restrict srcR = (tile & 1) ? Rg2 : Rg;
    const double *restrict srcB = (tile & 1) ? Bg2 : Bg;
    double *restrict dstR = (tile & 1) ? Rg : Rg2;
    double *restrict dstB = (tile & 1) ? Bg : Bg2;
    const bool last_tile = (s0 + St >= total_hs);

    #pragma omp parallel
    {
        for (int64_t m = 0; m < nstrips; m++) {
            const int64_t a = 1 + m * H;
            const int64_t b = (a + H - 1 <= nm2) ? (a + H - 1) : nm2;
            const int64_t alo = (a - S >= 0) ? (a - S) : 0;
            const int64_t ahi = (b + S <= N - 1) ? (b + S) : (N - 1);

            /* ---- copy-in ---- */
            #pragma omp for schedule(static)
            for (int64_t g = alo; g <= ahi; g++) {
                const int64_t r = g - alo;
                double *restrict BRr = BR + (size_t)r * rl;
                double *restrict BBr = BB + (size_t)r * rl;
                double *restrict Frr = BFr + (size_t)r * rl;
                double *restrict Fbr = BFb + (size_t)r * rl;
                const double *restrict ug = u + (size_t)g * (size_t)N;
                BBr[0] = ug[0];
                BBr[M + 1] = ug[N - 1];
                BRr[0] = ug[0];
                BRr[M + 1] = ug[N - 1];
                {
                    const double *restrict Rgk = srcR + (size_t)g * rl + 1;
                    const double *restrict Bgk = srcB + (size_t)g * rl + 1;
                    const double *restrict fg = f + (size_t)g * (size_t)N;
                    if (g & 1) {
                        for (int64_t k = 0; k < M; k++) {
                            BRr[k + 1] = Rgk[k];
                            BBr[k + 1] = Bgk[k];
                            Frr[k] = h2 * fg[2 * k + 1];
                            Fbr[k] = h2 * fg[2 * k + 2];
                        }
                    } else {
                        for (int64_t k = 0; k < M; k++) {
                            BRr[k + 1] = Rgk[k];
                            BBr[k + 1] = Bgk[k];
                            Frr[k] = h2 * fg[2 * k + 2];
                            Fbr[k] = h2 * fg[2 * k + 1];
                        }
                    }
                }
            }

            /* ---- St half-steps of this time-tile (trapezoid) ---- */
            for (int64_t s = 0; s < St; s++) {
                const int64_t half = S - 1 - s;
                int64_t lo = a - half; if (lo < 1) lo = 1;
                int64_t hi = b + half; if (hi > nm2) hi = nm2;
                if ((s & 1) == 0) {
                    /* red: update BR from BB */
                    #pragma omp for schedule(static)
                    for (int64_t g = lo; g <= hi; g++) {
                        const int64_t r = g - alo;
                        const double *restrict Bu = BB + (size_t)(r - 1) * rl + 1;
                        const double *restrict Bd = BB + (size_t)(r + 1) * rl + 1;
                        const double *restrict Bc = BB + (size_t)r * rl + ((g & 1) ? 0 : 1);
                        const double *restrict Fc = BFr + (size_t)r * rl;
                        double *restrict Rt = BR + (size_t)r * rl + 1;
                        #pragma omp simd
                        for (int64_t k = 0; k < M; k++) {
                            const double g2 = (Bu[k] + Bd[k] + Bc[k] + Bc[k + 1] + Fc[k]) * 0.25;
                            Rt[k] = om1 * Rt[k] + omega * g2;
                        }
                    }
                } else {
                    /* black: update BB from BR */
                    #pragma omp for schedule(static)
                    for (int64_t g = lo; g <= hi; g++) {
                        const int64_t r = g - alo;
                        const double *restrict Ru = BR + (size_t)(r - 1) * rl + 1;
                        const double *restrict Rd = BR + (size_t)(r + 1) * rl + 1;
                        const double *restrict Rc = BR + (size_t)r * rl + ((g & 1) ? 1 : 0);
                        const double *restrict Fc = BFb + (size_t)r * rl;
                        double *restrict Bt = BB + (size_t)r * rl + 1;
                        #pragma omp simd
                        for (int64_t k = 0; k < M; k++) {
                            const double g2 = (Ru[k] + Rd[k] + Rc[k] + Rc[k + 1] + Fc[k]) * 0.25;
                            Bt[k] = om1 * Bt[k] + omega * g2;
                        }
                    }
                }
            }

            /* ---- copy-out ---- */
            #pragma omp for schedule(static)
            for (int64_t g = a; g <= b; g++) {
                const int64_t r = g - alo;
                const double *restrict Rr = BR + (size_t)r * rl + 1;
                const double *restrict Br = BB + (size_t)r * rl + 1;
                if (last_tile) {
                    /* one contiguous row of u per core row */
                    double *restrict ug = u + (size_t)g * (size_t)N;
                    if (g & 1) {
                        for (int64_t k = 0; k < M; k++) {
                            ug[2 * k + 1] = Rr[k];
                            ug[2 * k + 2] = Br[k];
                        }
                    } else {
                        for (int64_t k = 0; k < M; k++) {
                            ug[2 * k + 2] = Rr[k];
                            ug[2 * k + 1] = Br[k];
                        }
                    }
                } else {
                    /* carry the state over to the next tile's source arrays */
                    double *restrict dR = dstR + (size_t)g * rl + 1;
                    double *restrict dB = dstB + (size_t)g * rl + 1;
                    for (int64_t k = 0; k < M; k++) {
                        dR[k] = Rr[k];
                        dB[k] = Br[k];
                    }
                }
            }
        }
    }

    /* keep the static boundary rows valid in the state arrays the next
       tile will read (only core rows are written by copy-out) */
    if (!last_tile) {
        #pragma omp parallel for schedule(static)
        for (int64_t g = 0; g < N; g += (N > 1 ? N - 1 : 1)) {
            double *restrict dR = dstR + (size_t)g * rl + 1;
            double *restrict dB = dstB + (size_t)g * rl + 1;
            const double *restrict sR = srcR + (size_t)g * rl + 1;
            const double *restrict sB = srcB + (size_t)g * rl + 1;
            for (int64_t k = 0; k < M; k++) {
                dR[k] = sR[k];
                dB[k] = sB[k];
            }
        }
    }
    } /* time-tile loop */
}

/* ------------------------------------------------------------------ */
/* Dispatcher                                                           */
/* ------------------------------------------------------------------ */
#ifndef RB_L3_BUDGET
#define RB_L3_BUDGET (192u << 20)   /* target bytes of the strip hot set */
#endif
#ifndef RB_SMALL_GRID
#define RB_SMALL_GRID ((size_t)48 << 20)
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

    /* temporally blocked path */
    const int64_t W = (int64_t)((double)RB_L3_BUDGET / (4.0 * (double)rl * 8.0));
    int64_t S = 2 * TSTEPS;
    if (S > 64) S = 64;
    if (S < 2) S = 2;
    {
        const int64_t smax = (nm2 / 2 > 2) ? nm2 / 2 : 2;
        if (S > smax) S = smax;
        const int64_t scap = (W / 8 > 2) ? W / 8 : 2;
        if (S > scap) S = scap;
    }
    if (S & 1) S--;                 /* tiles must end on a completed timestep */
    if (S < 2) S = 2;
    int64_t H = W - 2 * S;
    if (H > nm2) H = nm2;
    if (H < 32) {
        /* degenerate: grid too wide for the L3 budget, fall back to V1 */
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
    const size_t need_g = (size_t)(2 * n_tiles) * (size_t)N * rl * sizeof(double);
    const size_t need_buf = (size_t)4 * (size_t)(H + 2 * S) * rl * sizeof(double);
    const size_t total = ((need_g + 63) & ~(size_t)63) + ((need_buf + 63) & ~(size_t)63);
    uint8_t *mem = (uint8_t *)aligned_alloc(64, (total + 63) & ~(size_t)63);
    if (!mem) { rb_sor_direct(f, u, N, TSTEPS, omega); return; }

    const size_t one_g = (size_t)N * rl * sizeof(double);
    double *Rg  = (double *)mem;
    double *Bg  = (double *)(mem + one_g);
    double *Rg2 = (n_tiles > 1) ? (double *)(mem + 2 * one_g) : Rg;
    double *Bg2 = (n_tiles > 1) ? (double *)(mem + 3 * one_g) : Bg;
    double *buf = (double *)(mem + ((need_g + 63) & ~(size_t)63));

    rb_sor_blocked(f, u, N, TSTEPS, omega, Rg, Bg, Rg2, Bg2, buf, M, rowlen, H, S);
    free(mem);
}
