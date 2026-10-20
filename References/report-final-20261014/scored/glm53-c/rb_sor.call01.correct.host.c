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
/* Color-compacted layout.                                             */
/*                                                                     */
/* For row i, "red" points are interior columns j with (i+j)%2==0      */
/* (j == i%2 pattern: even i -> even cols, odd i -> odd cols);          */
/* black points are the other interior columns.  Both colours are      */
/* stored contiguously, M = (N-2)/2 values per row:                    */
/*   Rp[i][k] = u[i][colR(i,k)],  colR = (i even) ? 2k+2 : 2k+1        */
/*   Bp[i][k] = u[i][colB(i,k)],  colB = (i even) ? 2k+1 : 2k+2        */
/* Ghost slots Rp[i][-1] = Bp[i][-1] = u[i][0], Rp[i][M] = Bp[i][M] =   */
/* u[i][N-1] hold the fixed boundary ring.                             */
/*                                                                     */
/* For a red point (i,colR): up/down neighbours are black points of     */
/* rows i-1,i+1 at the SAME index k, and the left/right neighbours are  */
/* the black points at indices (i even ? k,k+1 : k-1,k) of row i.       */
/* So with a parity-dependent offset the whole update is:               */
/*   g = (B[i-1][k] + B[i+1][k] + Bs[k] + Bs[k+1] + h2*f) * 0.25        */
/* everything contiguous and unit-stride.  Black sweep is symmetric.     */
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

    /* ---- pack u and f into the compacted layout ---- */
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

    /* ---- relax ---- */
    #pragma omp parallel
    {
        for (int64_t t = 0; t < TSTEPS; t++) {
            /* red half-sweep: update R from B */
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
            /* black half-sweep: update B from R */
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

    /* ---- unpack ---- */
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

void rb_sor_fp64(
    const double *restrict f,
    double *restrict u,
    const int64_t N,
    const int64_t TSTEPS,
    const double omega,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    if (TSTEPS <= 0) return;
    if (N < 4 || (N & 1)) { rb_sor_direct(f, u, N, TSTEPS, omega); return; }

    const int64_t M = (N - 2) / 2;
    const int64_t rowlen = (M + 2 + 7) & ~(int64_t)7;
    const size_t need = (size_t)4 * (size_t)N * (size_t)rowlen * sizeof(double);

    double *R = NULL;
    void *heap = NULL;
    if (workspace && workspace_size > 0) {
        const uintptr_t base = (uintptr_t)workspace;
        const uintptr_t abase = (base + 63) & ~(uintptr_t)63;
        const size_t avail = (size_t)workspace_size - (size_t)(abase - base);
        if (avail >= need) R = (double *)abase;
    }
    if (!R) {
        heap = aligned_alloc(64, (need + 63) & ~(size_t)63);
        if (!heap) { rb_sor_direct(f, u, N, TSTEPS, omega); return; }
        R = (double *)heap;
    }
    double *B   = R + (size_t)N * (size_t)rowlen;
    double *FCr = B + (size_t)N * (size_t)rowlen;
    double *FCb = FCr + (size_t)N * (size_t)rowlen;

    rb_sor_compact(f, u, N, TSTEPS, omega, R, B, FCr, FCb, M, rowlen);
    free(heap);
}
