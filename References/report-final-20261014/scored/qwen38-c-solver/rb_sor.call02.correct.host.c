#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

void rb_sor_fp64(
    const double *restrict f,
    double *restrict u,
    const int64_t N,
    const int64_t TSTEPS,
    const double omega,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    const double h = 1.0 / (double)(N - 1);
    const double h2 = h * h;
    const double om = omega;
    const double som = 1.0 - omega;
    const int64_t N2 = N - 2;

    const __m512d vh2  = _mm512_set1_pd(h2);
    const __m512d vom  = _mm512_set1_pd(om);
    const __m512d vsom = _mm512_set1_pd(som);
    const __m512d vq   = _mm512_set1_pd(0.25);
    const __m512i EV8  = _mm512_setr_epi64(0,2,4,6,8,10,12,14);
    const __m512i OD8  = _mm512_setr_epi64(1,3,5,7,9,11,13,15);
    const __m512i EXP  = _mm512_setr_epi64(0,0,1,1,2,2,3,3);
    const __m512i SH4  = _mm512_setr_epi64(4,5,6,7,0,1,2,3);

    #pragma omp parallel
    {
        const int nt = omp_get_num_threads();
        const int tid = omp_get_thread_num();
        const int64_t rows = N - 2;
        int64_t r0 = 1 + (rows * tid) / nt;
        int64_t r1 = 1 + (rows * (tid + 1)) / nt;

        for (int64_t t = 0; t < TSTEPS; ++t) {
            for (int parity = 0; parity < 2; ++parity) {
                for (int64_t i = r0; i < r1; ++i) {
                    const double *up   = u + (i - 1) * N;
                    const double *dn   = u + (i + 1) * N;
                    double        *row = u + i * N;
                    const double *frow = f + i * N;
                    int j0 = ( (((parity ^ (int)(i & 1))) & 1) ) ? 1 : 2;

                    int64_t j = j0;
                    for (; j + 14 <= N2; j += 16) {
                        __m512d A, B;
                        A = _mm512_loadu_pd(&up[j]);   B = _mm512_loadu_pd(&up[j+8]);
                        __m512d vup   = _mm512_permutex2var_pd(A, EV8, B);
                        A = _mm512_loadu_pd(&dn[j]);   B = _mm512_loadu_pd(&dn[j+8]);
                        __m512d vdn   = _mm512_permutex2var_pd(A, EV8, B);
                        A = _mm512_loadu_pd(&frow[j]); B = _mm512_loadu_pd(&frow[j+8]);
                        __m512d vf    = _mm512_permutex2var_pd(A, EV8, B);
                        A = _mm512_loadu_pd(&row[j-1]); B = _mm512_loadu_pd(&row[j+7]);
                        __m512d vleft = _mm512_permutex2var_pd(A, EV8, B);
                        __m512d vself = _mm512_permutex2var_pd(A, OD8, B);
                        A = _mm512_loadu_pd(&row[j]);   B = _mm512_loadu_pd(&row[j+8]);
                        __m512d vright= _mm512_permutex2var_pd(A, OD8, B);

                        __m512d s = vup + vdn;
                        s = _mm512_add_pd(s, vleft);
                        s = _mm512_add_pd(s, vright);
                        s = _mm512_fmadd_pd(vh2, vf, s);
                        __m512d g = _mm512_mul_pd(s, vq);
                        __m512d vnew = _mm512_fmadd_pd(vom, g, _mm512_mul_pd(vsom, vself));

                        __m512d W0 = _mm512_permutexvar_pd(EXP, vnew);
                        _mm512_mask_storeu_pd(&row[j-1], 0xAA, W0);
                        __m512d v2 = _mm512_permutexvar_pd(SH4, vnew);
                        __m512d W1 = _mm512_permutexvar_pd(EXP, v2);
                        _mm512_mask_storeu_pd(&row[j+7], 0xAA, W1);
                    }
                    for (; j <= N2; j += 2) {
                        const double g = (up[j] + dn[j] + row[j-1] + row[j+1] + h2*frow[j]) * 0.25;
                        row[j] = som * row[j] + om * g;
                    }
                }
                #pragma omp barrier
            }
        }
    }
}
