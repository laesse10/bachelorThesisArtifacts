/* Red-black Gauss-Seidel / SOR, packed color-major layout + fused half-sweeps.
 *
 * Packed arrays (row stride Kp = ceil(K/8)*8 doubles, 64B aligned rows):
 *   R[i][k] = u[i][2k + (i&1)]     (red, (i+j) even)
 *   B[i][k] = u[i][2k + 1 - (i&1)] (black)
 * Half-sweeps become pure stencils:
 *   red   R[i][k] <- (1-w)R[i][k] + w/4*(B[i-1][k]+B[i+1][k]+B[i][k-1+(i&1)]+B[i][k+(i&1)]+h2*fR[i][k])
 *   black B[i][k] <- (1-w)B[i][k] + w/4*(R[i-1][k]+R[i+1][k]+R[i][k-(i&1)]+R[i][k+1-(i&1)]+h2*fB[i][k])
 *
 * Each thread owns a slab of rows and, per step, walks it in L2-sized
 * sub-tiles doing red then immediately black (black reads the R values the
 * red pass of the same sub-tile just wrote, and the B lines it just read,
 * out of L2).  Sub-tile edge rows need neighbour-slab red values, so the
 * first/last black row of a slab run after a barrier.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

static void rb_generic(const double *restrict f, double *restrict u,
                       int64_t N, int64_t TSTEPS, double omega, double h2) {
    for (int64_t t = 0; t < TSTEPS; ++t) {
        for (int parity = 0; parity < 2; ++parity) {
            for (int64_t i = 1; i < N - 1; ++i) {
                double *restrict ui = u + i * N;
                for (int64_t j = 1 + ((parity + i + 1) & 1); j < N - 1; j += 2) {
                    double g = (ui[j - N] + ui[j + N] + ui[j - 1] + ui[j + 1]
                                + h2 * f[i * N + j]) * 0.25;
                    ui[j] = (1.0 - omega) * ui[j] + omega * g;
                }
            }
        }
    }
}

/* One red row: R[i][klo..khi] (own value loaded) */
static inline void red_row(double *restrict Rs, const double *restrict Bu,
                           const double *restrict Bd, const double *restrict Bl,
                           const double *restrict Br, const double *restrict Fs,
                           int64_t klo, int64_t khi, double c0, double c1) {
    for (int64_t k = klo; k <= khi; ++k) {
        double s = Bu[k] + Bd[k];
        s += Bl[k];
        s += Br[k];
        s += Fs[k];
        double g = 0.25 * s;
        Rs[k] = c0 * Rs[k] + c1 * g;
    }
}

/* One red row, omega == 1: own value never read */
static inline void red_row_w1(double *restrict Rs, const double *restrict Bu,
                             const double *restrict Bd, const double *restrict Bl,
                             const double *restrict Br, const double *restrict Fs,
                             int64_t klo, int64_t khi) {
    for (int64_t k = klo; k <= khi; ++k) {
        double s = Bu[k] + Bd[k];
        s += Bl[k];
        s += Br[k];
        s += Fs[k];
        Rs[k] = 0.25 * s;
    }
}

static inline void black_row(double *restrict Bs, const double *restrict Ru,
                             const double *restrict Rd, const double *restrict Rl,
                             const double *restrict Rr, const double *restrict Fs,
                             int64_t klo, int64_t khi, double c0, double c1) {
    for (int64_t k = klo; k <= khi; ++k) {
        double s = Ru[k] + Rd[k];
        s += Rl[k];
        s += Rr[k];
        s += Fs[k];
        double g = 0.25 * s;
        Bs[k] = c0 * Bs[k] + c1 * g;
    }
}

static inline void black_row_w1(double *restrict Bs, const double *restrict Ru,
                               const double *restrict Rd, const double *restrict Rl,
                               const double *restrict Rr, const double *restrict Fs,
                               int64_t klo, int64_t khi) {
    for (int64_t k = klo; k <= khi; ++k) {
        double s = Ru[k] + Rd[k];
        s += Rl[k];
        s += Rr[k];
        s += Fs[k];
        Bs[k] = 0.25 * s;
    }
}

void rb_sor_fp64(
    const double *restrict f,
    double *restrict u,
    const int64_t N,
    const int64_t TSTEPS,
    const double omega,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace;
    (void)workspace_size;
    if (N < 3 || TSTEPS <= 0) return;

    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;

    if (N & 1) {
        rb_generic(f, u, N, TSTEPS, omega, h2);
        return;
    }

    const int64_t K = N / 2;
    const int64_t Kp = (K + 7) & ~(int64_t)7;
    const double c0 = 1.0 - omega;
    const double c1 = omega;
    const int w1 = (omega == 1.0);

    const size_t bytes = (size_t)N * (size_t)Kp * sizeof(double);
    size_t ab = ((bytes + 63) / 64) * 64;
    double *R  = (double *)aligned_alloc(64, ab);
    double *B  = (double *)aligned_alloc(64, ab);
    double *FR = (double *)aligned_alloc(64, ab);
    double *FB = (double *)aligned_alloc(64, ab);
    if (!R || !B || !FR || !FB) { free(R); free(B); free(FR); free(FB); return; }

    /* ---- pack ---- */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t p = i & 1;
        const double *restrict ui = u + i * N;
        const double *restrict fi = f + i * N;
        double *restrict Ri = R + i * Kp;
        double *restrict Bi = B + i * Kp;
        double *restrict FRi = FR + i * Kp;
        double *restrict FBi = FB + i * Kp;
        for (int64_t k = 0; k < K; ++k) {
            Bi[k] = ui[2 * k + 1 - p];
            FBi[k] = h2 * fi[2 * k + 1 - p];
            FRi[k] = h2 * fi[2 * k + p];
        }
        if (!w1 || i == 0 || i == N - 1) {
            for (int64_t k = 0; k < K; ++k) Ri[k] = ui[2 * k + p];
        } else {
            /* red boundary columns are still read as neighbours */
            Ri[0] = ui[p];
            Ri[K - 1] = ui[2 * (K - 1) + p];
        }
    }

    /* ---- sweep ---- */
    #pragma omp parallel
    {
        const int nthr = omp_get_num_threads();
        const int tid = omp_get_thread_num();
        const int64_t nrows = N - 2;
        const int64_t a = 1 + (tid * nrows) / nthr;
        const int64_t b = 1 + ((tid + 1) * nrows) / nthr - 1;

        /* sub-tile height: keep 2*(S+2) packed rows (~= R,B halo) under 768KB */
        int64_t S = (int64_t)(786432.0 / (16.0 * (double)Kp)) - 2;
        {
            const char *e = getenv("RB_TILE");
            if (e) S = atoi(e);
        }
        if (S < 1) S = 1;
        if (S > 4096) S = 4096;

        for (int64_t t = 0; t < TSTEPS; ++t) {
            for (int64_t c = a; c <= b; c += S) {
                int64_t d = c + S - 1;
                if (d > b) d = b;
                for (int64_t i = c; i <= d; ++i) {
                    const int64_t p = i & 1;
                    if (w1)
                        red_row_w1(R + i * Kp, B + (i - 1) * Kp, B + (i + 1) * Kp,
                                   B + i * Kp + p - 1, B + i * Kp + p, FR + i * Kp,
                                   1 - p, K - 1 - p);
                    else
                        red_row(R + i * Kp, B + (i - 1) * Kp, B + (i + 1) * Kp,
                                B + i * Kp + p - 1, B + i * Kp + p, FR + i * Kp,
                                1 - p, K - 1 - p, c0, c1);
                }
                /* black rows whose red deps (i-1,i,i+1) are already done */
                int64_t b0 = (c == a) ? a + 1 : c - 1;
                for (int64_t i = b0; i <= d - 1; ++i) {
                    const int64_t p = i & 1;
                    if (w1)
                        black_row_w1(B + i * Kp, R + (i - 1) * Kp, R + (i + 1) * Kp,
                                     R + i * Kp - p, R + i * Kp + 1 - p, FB + i * Kp,
                                     p, K - 2 + p);
                    else
                        black_row(B + i * Kp, R + (i - 1) * Kp, R + (i + 1) * Kp,
                                  R + i * Kp - p, R + i * Kp + 1 - p, FB + i * Kp,
                                  p, K - 2 + p, c0, c1);
                }
            }
            /* barrier: all red rows of this step exist everywhere */
            #pragma omp barrier
            if (a <= b) {
                int64_t edge[2];
                int ne = 0;
                edge[ne++] = a;
                if (b != a) edge[ne++] = b;
                for (int e = 0; e < ne; ++e) {
                    const int64_t i = edge[e];
                    const int64_t p = i & 1;
                    if (w1)
                        black_row_w1(B + i * Kp, R + (i - 1) * Kp, R + (i + 1) * Kp,
                                     R + i * Kp - p, R + i * Kp + 1 - p, FB + i * Kp,
                                     p, K - 2 + p);
                    else
                        black_row(B + i * Kp, R + (i - 1) * Kp, R + (i + 1) * Kp,
                                  R + i * Kp - p, R + i * Kp + 1 - p, FB + i * Kp,
                                  p, K - 2 + p, c0, c1);
                }
            }
            if (t < TSTEPS - 1) {
                #pragma omp barrier
            }
        }
    }

    /* ---- unpack ---- */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t p = i & 1;
        double *restrict ui = u + i * N;
        const double *restrict Ri = R + i * Kp;
        const double *restrict Bi = B + i * Kp;
        for (int64_t k = 0; k < K; ++k) {
            ui[2 * k + p] = Ri[k];
            ui[2 * k + 1 - p] = Bi[k];
        }
    }

    free(R); free(B); free(FR); free(FB);
}
