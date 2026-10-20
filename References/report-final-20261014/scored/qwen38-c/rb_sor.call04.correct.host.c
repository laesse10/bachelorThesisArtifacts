#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>
#include <immintrin.h>

/* ------------------------------------------------------------------ */
/* Red-black SOR, split-parity layout, full-row vector updates:        */
/*   ue[i][k] = u[i][2k+2]  (k = 0..heu-1, heu = (N-1)/2)             */
/*   uo[i][k] = u[i][2k+1]  (k = 0..ho-1,  ho  = N/2)                 */
/* fe/fo are the same split of f. Rows are padded by PAD doubles so    */
/* that full 8-wide vector loads/stores never run off the allocation.  */
/* ------------------------------------------------------------------ */

#define PAD 8

static void *xalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (!p) {
        fprintf(stderr, "rb_sor: allocation failed\n");
        exit(1);
    }
    return p;
}

/* 8-wide update. Reference expression order is preserved exactly:
   g = (((up+dn)+left)+right)+h2*f ; g /= 4 ; u = (1-w)u + w*g        */
__attribute__((target("avx512f")))
static void vrow8(double *dst, const double *up, const double *dn,
                  const double *lft, const double *rgh, const double *fr,
                  double h2, double c1, double w)
{
    __m512d v = _mm512_add_pd(_mm512_loadu_pd(up), _mm512_loadu_pd(dn));
    v = _mm512_add_pd(v, _mm512_loadu_pd(lft));
    v = _mm512_add_pd(v, _mm512_loadu_pd(rgh));
    v = _mm512_add_pd(v, _mm512_mul_pd(_mm512_set1_pd(h2), _mm512_loadu_pd(fr)));
    v = _mm512_mul_pd(v, _mm512_set1_pd(0.25));
    _mm512_storeu_pd(dst, _mm512_fmadd_pd(_mm512_set1_pd(w), v,
                                          _mm512_mul_pd(_mm512_set1_pd(c1),
                                                        _mm512_loadu_pd(dst))));
}

/* de[k] = s[2k+2] for k < n. */
__attribute__((target("avx2")))
static void copy_even(double *de, const double *s, int64_t n)
{
    int64_t k = 0;
    for (; k + 8 <= n; k += 8) {
        const double *a = s + 2 * k + 2;
        _mm_storeu_pd(de + k, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a), 0x8)));
        _mm_storeu_pd(de + k + 2, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a + 4), 0x8)));
        _mm_storeu_pd(de + k + 4, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a + 8), 0x8)));
        _mm_storeu_pd(de + k + 6, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a + 12), 0x8)));
    }
    for (; k < n; k++) de[k] = s[2 * k + 2];
}

/* do_[k] = s[2k+1] for k < n. */
__attribute__((target("avx2")))
static void copy_odd(double *do_, const double *s, int64_t n)
{
    int64_t k = 0;
    for (; k + 8 <= n; k += 8) {
        const double *a = s + 2 * k + 1;
        _mm_storeu_pd(do_ + k, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a), 0x8)));
        _mm_storeu_pd(do_ + k + 2, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a + 4), 0x8)));
        _mm_storeu_pd(do_ + k + 4, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a + 8), 0x8)));
        _mm_storeu_pd(do_ + k + 6, _mm256_castpd256_pd128(_mm256_permute4x64_pd(_mm256_loadu_pd(a + 12), 0x8)));
    }
    for (; k < n; k++) do_[k] = s[2 * k + 1];
}

/* row[2k] = de[k-1] (k>=1), row[2k+1] = do_[k]; row[0] untouched. */
__attribute__((target("avx2")))
static void merge_row(double *row, const double *de, const double *do_,
                      int64_t he, int64_t ho)
{
    row[1] = do_[0];  /* the vector loop below only covers do_[k] with k >= 1 */
    int64_t k = 0;
    for (; k + 2 < he && k + 3 < ho; k += 2) {
        __m256d a = _mm256_castpd128_pd256(_mm_loadu_pd(de + k));
        a = _mm256_insertf128_pd(a, _mm_loadu_pd(do_ + k + 1), 1);
        _mm_storeu_pd(row + 2 * k + 2, _mm256_castpd256_pd128(_mm256_permute4x64_pd(a, 0x8)));
        _mm_storeu_pd(row + 2 * k + 4, _mm256_castpd256_pd128(_mm256_permute4x64_pd(a, 0xD)));
    }
    /* vector covered de up to k-1 and do_ up to k; de and do_ tails are
       independent so record the split point before either loop */
    const int64_t kd = k;
    for (; k < he; k++) row[2 * k + 2] = de[k];
    for (k = kd + 1; k < ho; k++) row[2 * k + 1] = do_[k];
}

/* Update w consecutive points with 8-wide vector blocks + scalar tail.
   All pointer sets are for the SAME relative index: point k uses
   up[k], dn[k], lft[k], rgh[k], fr[k], dst[k]. */
static void full8(double *dst, const double *up, const double *dn,
                  const double *lft, const double *rgh, const double *fr,
                  int64_t w, double h2, double c1, double om, bool use512)
{
    int64_t k = 0;
    if (use512) {
        /* 2-way unrolled: two independent update chains in flight to
           overlap the load latency and the add-chain latency */
        for (; k + 16 <= w; k += 16) {
            vrow8(dst + k, up + k, dn + k, lft + k, rgh + k, fr + k,
                  h2, c1, om);
            vrow8(dst + k + 8, up + k + 8, dn + k + 8, lft + k + 8,
                  rgh + k + 8, fr + k + 8, h2, c1, om);
        }
        for (; k + 8 <= w; k += 8)
            vrow8(dst + k, up + k, dn + k, lft + k, rgh + k, fr + k,
                  h2, c1, om);
    }
    for (; k < w; k++) {
        const double g = (up[k] + dn[k] + lft[k] + rgh[k] + h2 * fr[k]) * 0.25;
        dst[k] = c1 * dst[k] + om * g;
    }
}

void rb_sor_fp64(const double *restrict f, double *restrict u,
                 const int64_t N, const int64_t TSTEPS, const double omega,
                 uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    if (N < 4 || TSTEPS < 1) return;

    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double c1 = 1.0 - omega;

    /* Small problems: direct scalar sweep on u (no split overhead). */
    if (N < 96 || TSTEPS < 3) {
        for (int64_t t = 0; t < TSTEPS; t++)
            for (int64_t parity = 0; parity < 2; parity++)
                #pragma omp parallel for schedule(static)
                for (int64_t i = 1; i < N - 1; i++) {
                    const int64_t j0 = (((i & 1) == parity) ? 2 : 1);
                    const double *up = u + (i - 1) * N;
                    const double *dn = u + (i + 1) * N;
                    double *row = u + i * N;
                    const double *fr = f + i * N;
                    for (int64_t j = j0; j < N - 1; j += 2) {
                        const double g = (up[j] + dn[j] + row[j - 1] + row[j + 1]
                                          + h2 * fr[j]) * 0.25;
                        row[j] = c1 * row[j] + omega * g;
                    }
                }
        return;
    }

    const int64_t heu = (N - 1) / 2;    /* ue columns: j = 2,4,...        */
    const int64_t ho = N / 2;           /* uo columns: j = 1,3,...        */
    /* row strides rounded up to whole 64-byte lines (+PAD) so that every
       8-wide vector access starts line-aligned; misaligned 512-bit loads
       would double the cache-line traffic */
    const int64_t heuP = ((heu + 7) & ~7LL) + PAD;
    const int64_t hoP = ((ho + 7) & ~7LL) + PAD;
    const int64_t cnt_e = (N - 2) / 2;  /* interior ue columns            */
    const int64_t cnt_o = (N - 1) / 2;  /* interior uo columns            */
    const bool use512 = __builtin_cpu_supports("avx512f");

    double *ue = (double *)xalloc((size_t)N * heuP * sizeof(double));
    double *uo = (double *)xalloc((size_t)N * hoP * sizeof(double));
    double *fe = (double *)xalloc((size_t)N * heuP * sizeof(double));
    double *fo = (double *)xalloc((size_t)N * hoP * sizeof(double));

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++) {
        const double *ur = u + i * N;
        const double *fr = f + i * N;
        copy_even(ue + i * heuP, ur, heu);
        copy_odd(uo + i * hoP, ur, ho);
        copy_even(fe + i * heuP, fr, heu);
        copy_odd(fo + i * hoP, fr, ho);
    }

    for (int64_t t = 0; t < TSTEPS; t++) {
        for (int64_t parity = 0; parity < 2; parity++) {
            #pragma omp parallel for schedule(static)
            for (int64_t i = 1; i < N - 1; i++) {
                if ((i & 1) == parity) {
                    /* row i even-parity colour lives in ue, full width */
                    double *dst = (double *)ue + i * heuP;
                    full8(dst,
                          (const double *)ue + (i - 1) * heuP,
                          (const double *)ue + (i + 1) * heuP,
                          (const double *)uo + i * hoP,
                          (const double *)uo + i * hoP + 1,
                          (const double *)fe + i * heuP,
                          cnt_e, h2, c1, omega, use512);
                } else {
                    /* row i colour lives in uo; k = 0 has the j = 0
                       boundary as left neighbour, so update it scalar
                       and vectorise k = 1 .. cnt_o-1 (all aligned at +1
                       offset pointers) */
                    double *drow = (double *)uo + i * hoP;
                    const double *upr = (const double *)uo + (i - 1) * hoP;
                    const double *dnr = (const double *)uo + (i + 1) * hoP;
                    const double *eer = (const double *)ue + i * heuP;
                    const double *forr = (const double *)fo + i * hoP;
                    const double g = (upr[0] + dnr[0] + u[i * N] + eer[0]
                                      + h2 * forr[0]) * 0.25;
                    drow[0] = c1 * drow[0] + omega * g;
                    full8(drow + 1, upr + 1, dnr + 1, eer, eer + 1,
                          forr + 1, cnt_o - 1, h2, c1, omega, use512);
                }
            }
        }
    }

    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < N; i++)
        merge_row(u + i * N, ue + i * heuP, uo + i * hoP, heu, ho);

    free(ue); free(uo); free(fe); free(fo);
}
