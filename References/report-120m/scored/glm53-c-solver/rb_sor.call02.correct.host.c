/* Red-black Gauss-Seidel / SOR on a 2-D Poisson grid.
 *
 * Within one colour every point reads only the other colour, so each half-sweep
 * is fully data parallel.  To make the memory access unit-stride we transpose
 * the grid ONCE into colour-packed arrays (ghost rows/columns hold the fixed
 * zero-Dirichlet boundary), run all TSTEPS of red-then-black relaxation on the
 * packed form, and write back at the end.  The sweeps themselves are untouched:
 * red for the whole grid, then black, TSTEPS times.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* Update every point of colour `color` in grid row i.
 * pc/wc: packed arrays of the colour being updated; po: packed array of the
 * other colour (read-only).  Grid row i lives at <base> + i*stride, elements
 * 1..M of each row are the real points, 0 and M+1 are zero ghosts, rows 0 and
 * N-1 of the packed arrays are zero ghost rows (the Dirichlet boundary).
 * The row stride is 7 mod 8 so that the data at +1 is 64-byte aligned. */
static inline void sweep_row(double *restrict pc, const double *restrict po,
                             const double *restrict wc, const int64_t i,
                             const int64_t M, const int64_t stride,
                             const double c1, const double om, const int color)
{
    double *restrict dst = pc + i * stride + 1;
    const double *restrict lr = po + i * stride;       /* 0 = ghost left  */
    const double *restrict up = po + (i - 1) * stride + 1; /* ghost row if i==1 */
    const double *restrict dn = po + (i + 1) * stride + 1; /* ghost row if i==N-2 */
    const double *restrict w  = wc + i * stride + 1;
    /* horizontal neighbours sit at lr[k+sh], lr[k+sh+1]; sh depends on the
     * parity of the row relative to the colour. */
    const int64_t sh = ((i & 1) == (int64_t)color) ? 1 : 0;
    if (c1 == 0.0) {
        /* plain red-black Gauss-Seidel: no self load */
        for (int64_t k = 0; k < M; k++) {
            const double g = (lr[k + sh] + lr[k + sh + 1] + up[k] + dn[k] + w[k]) * 0.25;
            dst[k] = om * g;
        }
    } else {
        for (int64_t k = 0; k < M; k++) {
            const double g = (lr[k + sh] + lr[k + sh + 1] + up[k] + dn[k] + w[k]) * 0.25;
            dst[k] = c1 * dst[k] + om * g;
        }
    }
}

#ifdef __AVX512F__
/* Same sweep with streaming (non-temporal) stores: for grids far larger than
 * any cache the written lines are never re-read before eviction, so skipping
 * the read-for-ownership saves a quarter of the memory traffic. */
static inline void sweep_row_nt(double *restrict pc, const double *restrict po,
                                const double *restrict wc, const int64_t i,
                                const int64_t M, const int64_t stride,
                                const double c1, const double om, const int color)
{
    double *restrict dst = pc + i * stride + 1;    /* 64-byte aligned */
    const double *restrict lr = po + i * stride;
    const double *restrict up = po + (i - 1) * stride + 1;
    const double *restrict dn = po + (i + 1) * stride + 1;
    const double *restrict w  = wc + i * stride + 1;
    const int64_t sh = ((i & 1) == (int64_t)color) ? 1 : 0;
    const __m512d q = _mm512_set1_pd(0.25);
    const __m512d ov = _mm512_set1_pd(om);
    const __m512d cv = _mm512_set1_pd(c1);
    int64_t k = 0;
    if (c1 == 0.0) {
        for (; k + 8 <= M; k += 8) {
            const __m512d g = _mm512_mul_pd(_mm512_add_pd(
                _mm512_add_pd(_mm512_loadu_pd(lr + k + sh),
                              _mm512_loadu_pd(lr + k + sh + 1)),
                _mm512_add_pd(_mm512_add_pd(_mm512_loadu_pd(up + k),
                                             _mm512_loadu_pd(dn + k)),
                              _mm512_loadu_pd(w + k))), q);
            _mm512_stream_pd(dst + k, _mm512_mul_pd(ov, g));
        }
        for (; k < M; k++) {
            const double g = (lr[k + sh] + lr[k + sh + 1] + up[k] + dn[k] + w[k]) * 0.25;
            dst[k] = om * g;
        }
    } else {
        for (; k + 8 <= M; k += 8) {
            const __m512d g = _mm512_mul_pd(_mm512_add_pd(
                _mm512_add_pd(_mm512_loadu_pd(lr + k + sh),
                              _mm512_loadu_pd(lr + k + sh + 1)),
                _mm512_add_pd(_mm512_add_pd(_mm512_loadu_pd(up + k),
                                             _mm512_loadu_pd(dn + k)),
                              _mm512_loadu_pd(w + k))), q);
            _mm512_stream_pd(dst + k, _mm512_fmadd_pd(cv, _mm512_loadu_pd(dst + k),
                                                      _mm512_mul_pd(ov, g)));
        }
        for (; k < M; k++) {
            const double g = (lr[k + sh] + lr[k + sh + 1] + up[k] + dn[k] + w[k]) * 0.25;
            dst[k] = c1 * dst[k] + om * g;
        }
    }
}
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
    if (N < 4 || TSTEPS <= 0)
        return;
    if (N & 1) {
        /* the colouring is only defined for even N (see the initializer);
         * scalar fallback so any input still produces the reference answer */
        const double h = 1.0 / (double)(N - 1);
        const double h2 = h * h;
        for (int64_t t = 0; t < TSTEPS; t++) {
            for (int parity = 0; parity < 2; parity++) {
                #pragma omp parallel for schedule(static)
                for (int64_t i = 1; i < N - 1; i++) {
                    const double *fr = f + (size_t)i * (size_t)N;
                    double *ur = u + (size_t)i * (size_t)N;
                    const double *um = u + (size_t)(i - 1) * (size_t)N;
                    const double *up = u + (size_t)(i + 1) * (size_t)N;
                    for (int64_t j = 1; j < N - 1; j++) {
                        if (((i + j) & 1) == parity) {
                            const double g = (um[j] + up[j] + ur[j - 1] + ur[j + 1]
                                              + h2 * fr[j]) * 0.25;
                            ur[j] = (1.0 - omega) * ur[j] + omega * g;
                        }
                    }
                }
            }
        }
        return;
    }

    const int64_t M = (N - 2) / 2;        /* points of one colour per row   */
    const int64_t stride = (M + 9) & ~(int64_t)7;     /* >= M+2, multiple of 8 */
    const int64_t rows = N;               /* + top/bottom ghost rows         */

    const size_t need = (size_t)rows * (size_t)stride * 4 * sizeof(double) + 128;
    if (workspace == NULL || (size_t)workspace_size < need) {
        /* no scratch: parallel strided sweep straight on u */
        const double h = 1.0 / (double)(N - 1);
        const double h2 = h * h;
        for (int64_t t = 0; t < TSTEPS; t++) {
            for (int parity = 0; parity < 2; parity++) {
                #pragma omp parallel for schedule(static)
                for (int64_t i = 1; i < N - 1; i++) {
                    const double *fr = f + (size_t)i * (size_t)N;
                    double *ur = u + (size_t)i * (size_t)N;
                    const double *um = u + (size_t)(i - 1) * (size_t)N;
                    const double *up = u + (size_t)(i + 1) * (size_t)N;
                    for (int64_t j = 1 + (int64_t)(((i + 1) & 1) != parity); j < N - 1; j += 2) {
                        const double g = (um[j] + up[j] + ur[j - 1] + ur[j + 1]
                                          + h2 * fr[j]) * 0.25;
                        ur[j] = (1.0 - omega) * ur[j] + omega * g;
                    }
                }
            }
        }
        return;
    }

    /* ---- packed layout ---- */
    uintptr_t base = ((uintptr_t)workspace + 63u) & ~(uintptr_t)63u;
    const size_t asz = ((size_t)rows * (size_t)stride + 7u) & ~(size_t)7u;
    /* p0 sits 8 bytes below a 64-byte line so that <p0 + i*stride + 1> (the
     * data of every row) is 64-byte aligned for the streaming stores */
    double *p0 = (double *)base - 1;                   /* colour 0 u */
    double *p1 = p0 + asz;                              /* colour 1 u */
    double *w0 = p1 + asz;                              /* colour 0 h2*f */
    double *w1 = w0 + asz;                              /* colour 1 h2*f */

    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double c1 = 1.0 - omega;
    const double om = omega;

    /* streaming stores pay off once the two colour grids outgrow the caches */
#ifdef __AVX512F__
    const int use_nt = (2.0 * (double)asz * 8.0) > (32.0 * 1024.0 * 1024.0);
#else
    const int use_nt = 0;
#endif

    /* pack: rows are independent */
    #pragma omp parallel
    {
        #pragma omp for schedule(static) nowait
        for (int64_t i = 1; i <= N - 2; i++) {
            const double *ur = u + (size_t)i * (size_t)N;
            const double *fr = f + (size_t)i * (size_t)N;
            double *d0 = p0 + i * stride + 1;
            double *d1 = p1 + i * stride + 1;
            double *e0 = w0 + i * stride + 1;
            double *e1 = w1 + i * stride + 1;
            if ((i & 1) == 0) {
                /* colour 0: even j (2,4,...); colour 1: odd j (1,3,...) */
                for (int64_t k = 0; k < M; k++) {
                    d0[k] = ur[2 * k + 2];
                    d1[k] = ur[2 * k + 1];
                    e0[k] = h2 * fr[2 * k + 2];
                    e1[k] = h2 * fr[2 * k + 1];
                }
            } else {
                /* colour 0: odd j; colour 1: even j */
                for (int64_t k = 0; k < M; k++) {
                    d0[k] = ur[2 * k + 1];
                    d1[k] = ur[2 * k + 2];
                    e0[k] = h2 * fr[2 * k + 1];
                    e1[k] = h2 * fr[2 * k + 2];
                }
            }
            /* ghosts */
            p0[i * stride] = 0.0; p0[i * stride + M + 1] = 0.0;
            p1[i * stride] = 0.0; p1[i * stride + M + 1] = 0.0;
        }
        /* ghost rows 0 and N-1 for both colours */
        #pragma omp single
        {
            memset(p0, 0, sizeof(double) * (size_t)stride);
            memset(p1, 0, sizeof(double) * (size_t)stride);
            memset(p0 + (N - 1) * stride, 0, sizeof(double) * (size_t)stride);
            memset(p1 + (N - 1) * stride, 0, sizeof(double) * (size_t)stride);
        }

        for (int64_t t = 0; t < TSTEPS; t++) {
            if (use_nt) {
                #pragma omp for schedule(static)
                for (int64_t i = 1; i <= N - 2; i++)
                    sweep_row_nt(p0, p1, w0, i, M, stride, c1, om, 0);
                #pragma omp for schedule(static)
                for (int64_t i = 1; i <= N - 2; i++)
                    sweep_row_nt(p1, p0, w1, i, M, stride, c1, om, 1);
            } else {
                #pragma omp for schedule(static)
                for (int64_t i = 1; i <= N - 2; i++)
                    sweep_row(p0, p1, w0, i, M, stride, c1, om, 0);
                #pragma omp for schedule(static)
                for (int64_t i = 1; i <= N - 2; i++)
                    sweep_row(p1, p0, w1, i, M, stride, c1, om, 1);
            }
        }

        /* unpack: write the interior back, boundary ring of u untouched */
        #pragma omp for schedule(static)
        for (int64_t i = 1; i <= N - 2; i++) {
            double *ur = u + (size_t)i * (size_t)N;
            const double *d0 = p0 + i * stride + 1;
            const double *d1 = p1 + i * stride + 1;
            if ((i & 1) == 0) {
                for (int64_t k = 0; k < M; k++) {
                    ur[2 * k + 2] = d0[k];
                    ur[2 * k + 1] = d1[k];
                }
            } else {
                for (int64_t k = 0; k < M; k++) {
                    ur[2 * k + 1] = d0[k];
                    ur[2 * k + 2] = d1[k];
                }
            }
        }
    }
}
