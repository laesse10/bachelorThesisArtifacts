/* Red-black Gauss-Seidel / SOR, packed color-major layout.
 *
 * The grid is split into two packed arrays:
 *   R[i][k] = u[i][2k + (i&1)]    (red points, (i+j) even)
 *   B[i][k] = u[i][2k + 1 - (i&1)] (black points, (i+j) odd)
 * each of shape N x (N/2).  Within one color's half-sweep every point reads
 * only the *other* color, so the sweep is a pure stencil on the packed arrays:
 *   red   point R[i][k]: reads B[i-1][k], B[i+1][k], B[i][k-1+(i&1)], B[i][k+(i&1)]
 *   black point B[i][k]: reads R[i-1][k], R[i+1][k], R[i][k-(i&1)],   R[i][k+1-(i&1)]
 * which is fully contiguous and vectorizable with no wasted lanes.
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
    /* Fallback for odd N (the generator only produces even N). */
    for (int64_t t = 0; t < TSTEPS; ++t) {
        for (int parity = 0; parity < 2; ++parity) {
            for (int64_t i = 1; i < N - 1; ++i) {
                double *restrict ui = u + i * N;
                const double *restrict fim = f + (i - 1) * N;
                const double *restrict fip = f + (i + 1) * N;
                for (int64_t j = 1 + ((parity + i + 1) & 1); j < N - 1; j += 2) {
                    double g = (ui[j - N] + ui[j + N] + ui[j - 1] + ui[j + 1]
                                + h2 * f[i * N + j]) * 0.25;
                    ui[j] = (1.0 - omega) * ui[j] + omega * g;
                }
                (void)fim; (void)fip;
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
    const double c0 = 1.0 - omega;
    const double c1 = omega;

    /* R, B: packed u.  FR, FB: h2 * f packed by color. */
    const size_t bytes = (size_t)N * (size_t)K * sizeof(double);
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
        double *restrict Ri = R + i * K;
        double *restrict Bi = B + i * K;
        double *restrict FRi = FR + i * K;
        double *restrict FBi = FB + i * K;
        for (int64_t k = 0; k < K; ++k) {
            const double a = ui[2 * k + p];
            const double b = ui[2 * k + 1 - p];
            const double x = fi[2 * k + p];
            const double y = fi[2 * k + 1 - p];
            Ri[k] = a;
            Bi[k] = b;
            FRi[k] = h2 * x;
            FBi[k] = h2 * y;
        }
    }

    /* ---- sweep ---- */
    #pragma omp parallel
    {
        for (int64_t t = 0; t < TSTEPS; ++t) {
            /* red half-sweep */
            #pragma omp for schedule(static)
            for (int64_t i = 1; i < N - 1; ++i) {
                const int64_t p = i & 1;
                const int64_t klo = 1 - p;
                const int64_t khi = K - 1 - p;
                const double *restrict Bu = B + (i - 1) * K;
                const double *restrict Bd = B + (i + 1) * K;
                const double *restrict Bl = B + i * K + p - 1;
                const double *restrict Br = B + i * K + p;
                const double *restrict Fs = FR + i * K;
                double *restrict Rs = R + i * K;
                for (int64_t k = klo; k <= khi; ++k) {
                    double s = Bu[k] + Bd[k];
                    s += Bl[k];
                    s += Br[k];
                    s += Fs[k];
                    const double g = 0.25 * s;
                    Rs[k] = c0 * Rs[k] + c1 * g;
                }
            }
            /* black half-sweep */
            #pragma omp for schedule(static)
            for (int64_t i = 1; i < N - 1; ++i) {
                const int64_t p = i & 1;
                const int64_t klo = p;
                const int64_t khi = K - 2 + p;
                const double *restrict Ru = R + (i - 1) * K;
                const double *restrict Rd = R + (i + 1) * K;
                const double *restrict Rl = R + i * K - p;
                const double *restrict Rr = R + i * K + 1 - p;
                const double *restrict Fs = FB + i * K;
                double *restrict Bs = B + i * K;
                for (int64_t k = klo; k <= khi; ++k) {
                    double s = Ru[k] + Rd[k];
                    s += Rl[k];
                    s += Rr[k];
                    s += Fs[k];
                    const double g = 0.25 * s;
                    Bs[k] = c0 * Bs[k] + c1 * g;
                }
            }
        }
    }

    /* ---- unpack ---- */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        const int64_t p = i & 1;
        double *restrict ui = u + i * N;
        const double *restrict Ri = R + i * K;
        const double *restrict Bi = B + i * K;
        for (int64_t k = 0; k < K; ++k) {
            ui[2 * k + p] = Ri[k];
            ui[2 * k + 1 - p] = Bi[k];
        }
    }

    free(R); free(B); free(FR); free(FB);
}
