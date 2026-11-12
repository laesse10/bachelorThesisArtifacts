#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

/* ---- CPU affinity (glibc ABI: cpu_set_t = 1024-bit mask) ---- */
#define HQ_CPUWORDS 1024
typedef struct { unsigned long bits[HQ_CPUWORDS / (8 * sizeof(unsigned long))]; } hq_cpuset;
extern int sched_getaffinity(int pid, unsigned long size, hq_cpuset *set);
extern int sched_setaffinity(int pid, unsigned long size, const hq_cpuset *set);

static void hq_cpus_zero(hq_cpuset *s) {
    for (int i = 0; i < HQ_CPUWORDS / 64; i++) s->bits[i] = 0UL;
}
static int hq_cpus_is(const hq_cpuset *s, int cpu) {
    return (cpu >= 0 && cpu < HQ_CPUWORDS) && ((s->bits[cpu >> 6] >> (cpu & 63)) & 1UL);
}
static void hq_cpus_set(hq_cpuset *s, int cpu) {
    if (cpu >= 0 && cpu < HQ_CPUWORDS) s->bits[cpu >> 6] |= 1UL << (cpu & 63);
}
static int hq_cpus_count(const hq_cpuset *s) {
    int n = 0;
    for (int i = 0; i < HQ_CPUWORDS / 64; i++) n += __builtin_popcountl(s->bits[i]);
    return n;
}

/* Pin the calling thread to the idx-th CPU of the process affinity mask, so
   that first-touch page placement stays valid across the kernel's phases. */
static void hq_pin_thread(const int idx) {
    hq_cpuset allowed, one;
    hq_cpus_zero(&allowed);
    if (sched_getaffinity(0, sizeof(allowed), &allowed) != 0) return;
    const int cnt = hq_cpus_count(&allowed);
    if (cnt <= 0) return;
    int want = idx % cnt, seen = 0;
    for (int cpu = 0; cpu < HQ_CPUWORDS; cpu++) {
        if (!hq_cpus_is(&allowed, cpu)) continue;
        if (seen == want) {
            hq_cpus_zero(&one);
            hq_cpus_set(&one, cpu);
            sched_setaffinity(0, sizeof(one), &one);
            return;
        }
        seen++;
    }
}

typedef int lint;

/* ---------- local BLAS/LAPACK declarations ---------- */
#define CBLAS_ROWMAJOR 101
#define CBLAS_NOTRANS  111
#define CBLAS_TRANS    112
#define CBLAS_UPPER    121
#define CBLAS_LOWER    122
#define CBLAS_NONUNIT  131

extern void cblas_dgemm(int Order, int TransA, int TransB, int M, int N, int K,
                        double alpha, const double *a, int lda, const double *b, int ldb,
                        double beta, double *c, int ldc);
extern void cblas_dsyrk(int Order, int Uplo, int Trans, int N, int K,
                        double alpha, const double *a, int lda, double beta, double *c, int ldc);
extern void dtrtri_(const char *uplo, const char *diag, const lint *n, double *a,
                    const lint *lda, lint *info);
extern void openblas_set_num_threads(int);

#define MAXNB 64

static double hq_now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
static int hq_dbg(void){return 1;}

static int pick_nb(const int64_t N) {
    const char *e = getenv("HQ_NB");
    if (e && e[0]) return atoi(e);
    return 32;
}

void householder_qr_fp64(double *restrict A, double *restrict Q, double *restrict R,
                         const double *restrict b, double *restrict x,
                         const int64_t M, const int64_t N,
                         uint8_t *restrict workspace, const int64_t workspace_size) {
    if (M <= 0 || N <= 0) return;
#ifdef _OPENMP
    openblas_set_num_threads(1);
#endif
    int64_t nth = 1;
#ifdef _OPENMP
    nth = omp_get_max_threads();
#endif
    if (nth < 1) nth = 1;
    const int64_t nb_in = pick_nb(N);
    const int64_t nb = (nb_in < N) ? nb_in : N;

    const int64_t need = M * nb + N
                 + nth + nth * MAXNB + MAXNB
                 + nth * MAXNB * MAXNB + 2 * MAXNB * MAXNB
                 + nth * MAXNB * N + MAXNB * N
                 + N * N + 2 * N + nth * N;
    double *scr = NULL, *scr_heap = NULL;
    if (workspace && workspace_size >= need * (int64_t)sizeof(double)) {
        scr = (double *)workspace;
    } else {
        scr_heap = (double *)malloc((size_t)need * sizeof(double));
        scr = scr_heap;
    }
    double *V    = scr;
    double *tau  = V + M * nb;
    double *sh_s = tau + N;
    double *sh_d = sh_s + nth;
    double *sh_row = sh_d + nth * MAXNB;
    double *Sp  = sh_row + MAXNB;
    double *S   = Sp + nth * MAXNB * MAXNB;
    double *T   = S + MAXNB * MAXNB;
    double *Yp  = T + MAXNB * MAXNB;
    double *W   = Yp + nth * MAXNB * N;
    double *Rinv = W + MAXNB * N;
    double *g    = Rinv + N * N;
    double *h    = g + N;
    double *hp   = h + N;

    double tm_copy = 0, tm_pan = 0, tm_trail = 0, tm_post = 0, tm_qgemm = 0, tm_fin = 0;
    double tm_st = 0, tm_y = 0, tm_w = 0, tm_u = 0;
    double T0 = hq_now();

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int nthr = omp_get_num_threads();
        hq_pin_thread(tid);
        double *my_h = hp + (size_t)tid * N;
        double (*shd)[MAXNB] = (double (*)[MAXNB])sh_d;

        /* ---------- phase 1: Q <- A, h = A^T b ---------- */
        {
            const int64_t tot = M;
            const int64_t r0 = (tot * (int64_t)tid) / nthr;
            const int64_t r1 = (tot * (int64_t)(tid + 1)) / nthr;
            for (int64_t j = 0; j < N; j++) my_h[j] = 0.0;
            for (int64_t i = r0; i < r1; i++) {
                const double bi = b[i];
                const double *Ai = A + i * N;
                double *Qi = Q + i * N;
                for (int64_t j = 0; j < N; j++) {
                    Qi[j] = Ai[j];
                    my_h[j] += Ai[j] * bi;
                }
            }
        }
        #pragma omp barrier
        if (tid == 0) {
            tm_copy = hq_now() - T0;
            for (int64_t j = 0; j < N; j++) h[j] = 0.0;
            for (int t = 0; t < nthr; t++)
                for (int64_t j = 0; j < N; j++) h[j] += hp[(size_t)t * N + j];
        }

        /* ---------- phase 2: blocked Householder factorization ---------- */
        for (int64_t jb = 0; jb < N; jb += nb) {
            const int64_t wdt = (N - jb < nb) ? (N - jb) : nb;
            const int64_t jend = jb + wdt;
            const int64_t Nt = N - jend;
            const int64_t tot = M - jb;
            const int64_t r0 = jb + (tot * (int64_t)tid) / nthr;
            const int64_t r1 = jb + (tot * (int64_t)(tid + 1)) / nthr;
            double *Sp_t = Sp + (size_t)tid * MAXNB * MAXNB;
            double tmark = 0.0;

            if (tid == 0 && jb % (4 * nb) == 0) tmark = hq_now();

            /* staircase zero of V */
            for (int64_t i = r0; (i < r1) && (i < jend); ++i) {
                double *Vi = V + i * nb;
                for (int64_t jj = i - jb + 1; jj < wdt; jj++) Vi[jj] = 0.0;
            }

            /* ---- reflector loop ---- */
            for (int64_t j = jb; j < jend; j++) {
                const int64_t ncols = jend - j;
                const int64_t jj = j - jb;
                double ps = 0.0;
                double pd[MAXNB];
                for (int64_t c = 1; c < ncols; c++) pd[c - 1] = 0.0;
                const int64_t i0 = (r0 > j + 1) ? r0 : (j + 1);
                for (int64_t i = i0; i < r1; i++) {
                    const double v = Q[i * N + j];
                    ps += v * v;
                    const double *Pi = Q + i * N;
                    for (int64_t c = j + 1; c < jend; c++) pd[c - j - 1] += v * Pi[c];
                }
                sh_s[tid] = ps;
                for (int64_t c = 1; c < ncols; c++) shd[tid][c - 1] = pd[c - 1];
                if (j >= r0 && j < r1) {
                    const double *Pj = Q + j * N;
                    for (int64_t c = 0; c < ncols; c++) sh_row[c] = Pj[j + c];
                }
                #pragma omp barrier
                double s = 0.0;
                double dd[MAXNB];
                for (int64_t c = 1; c < ncols; c++) dd[c] = 0.0;
                for (int t = 0; t < nthr; t++) {
                    s += sh_s[t];
                    for (int64_t c = 1; c < ncols; c++) dd[c] += shd[t][c - 1];
                }
                const double alpha = sh_row[0];
                const double norm = sqrt(alpha * alpha + s);
                const double sgn = (alpha >= 0.0) ? 1.0 : -1.0;
                const double vj = alpha + sgn * norm;
                const double vv = s + vj * vj;
                const double beta = (vv > 0.0) ? (2.0 / vv) : 0.0;
                double w[MAXNB];
                w[0] = beta * (vj * sh_row[0] + s);
                for (int64_t c = 1; c < ncols; c++)
                    w[c] = beta * (vj * sh_row[c] + dd[c]);

                if (tid == 0) tau[jj] = beta;
                if (j >= r0 && j < r1) {
                    double *Pj = Q + j * N;
                    V[j * nb + jj] = vj;
                    for (int64_t c = 0; c < ncols; c++) Pj[j + c] -= vj * w[c];
                }
                const int64_t i1 = (r0 > j + 1) ? r0 : (j + 1);
                for (int64_t i = i1; i < r1; i++) {
                    const double v = Q[i * N + j];
                    V[i * nb + jj] = v;
                    double *Pi = Q + i * N;
                    for (int64_t c = j + 1; c < jend; c++) Pi[c] -= v * w[c - j];
                }
                #pragma omp barrier
            }

            if (tid == 0) {
                if (jb % (4 * nb) == 0) tm_pan += hq_now() - tmark;
                tmark = hq_now();
            }

            /* ---- S = V^T V partials over row chunks (hand-rolled) ---- */
            if (tid == 0) tm_st -= hq_now();
            {
                for (int64_t a = 0; a < wdt; a++)
                    for (int64_t b2 = a; b2 < wdt; b2++) Sp_t[a * MAXNB + b2] = 0.0;
                for (int64_t i = r0; i < r1; i++) {
                    const double *Vi = V + i * nb;
                    for (int64_t a = 0; a < wdt; a++) {
                        const double va = Vi[a];
                        double *Sa = Sp_t + a * MAXNB;
                        for (int64_t b2 = a; b2 < wdt; b2++) Sa[b2] += va * Vi[b2];
                    }
                }
            }
            #pragma omp barrier

            /* ---- S reduce + mirror, T recurrence (thread 0 only) ---- */
            if (tid == 0) {
            /* ---- S reduce + mirror, T recurrence (redundant) ---- */
            for (int64_t a = 0; a < wdt; a++)
                for (int64_t b2 = a; b2 < wdt; b2++) {
                    double acc = 0.0;
                    for (int t = 0; t < nthr; t++)
                        acc += Sp[(size_t)t * MAXNB * MAXNB + a * MAXNB + b2];
                    S[a * MAXNB + b2] = acc;
                    S[b2 * MAXNB + a] = acc;
                }
            for (int64_t i = 0; i < wdt; i++) {
                for (int64_t k = 0; k < i; k++) {
                    double acc = 0.0;
                    for (int64_t m = 0; m < i; m++) acc += S[m * MAXNB + i] * T[m * MAXNB + k];
                    T[i * MAXNB + k] = -tau[i] * acc;
                }
                T[i * MAXNB + i] = tau[i];
            }

            }
            /* ---- trailing update of columns [jend, N) ---- */
            if (tid == 0) { tm_st += hq_now(); tm_u -= hq_now(); }
            if (Nt > 0) {
                if (tid == 0) tm_y -= hq_now();
                double *Yp_t = Yp + (size_t)tid * MAXNB * N;
                if (r1 > r0) {
                    cblas_dgemm(CBLAS_ROWMAJOR, CBLAS_TRANS, CBLAS_NOTRANS, (int)wdt, (int)Nt,
                                (int)(r1 - r0), 1.0, V + r0 * nb, (int)nb,
                                Q + r0 * N + jend, (int)N, 0.0, Yp_t, (int)N);
                } else {
                    for (int64_t a = 0; a < wdt; a++)
                        for (int64_t c = 0; c < Nt; c++) Yp_t[a * N + c] = 0.0;
                }
                #pragma omp barrier
                if (tid == 0) { tm_y += hq_now(); tm_w -= hq_now(); }
                /* W = T * (sum of Y parts), split over columns */
                {
                    const int64_t w0 = (Nt * (int64_t)tid) / nthr;
                    const int64_t w1 = (Nt * (int64_t)(tid + 1)) / nthr;
                    for (int64_t c = w0; c < w1; c++) {
                        double ycol[MAXNB];
                        for (int64_t k = 0; k < wdt; k++) {
                            double acc = 0.0;
                            for (int t = 0; t < nthr; t++)
                                acc += Yp[(size_t)t * MAXNB * N + k * N + c];
                            ycol[k] = acc;
                        }
                        for (int64_t i = 0; i < wdt; i++) {
                            double wv = 0.0;
                            for (int64_t k = 0; k < i; k++) wv += T[i * MAXNB + k] * ycol[k];
                            W[i * N + c] = wv + T[i * MAXNB + i] * ycol[i];
                        }
                    }
                }
                #pragma omp barrier
                if (tid == 0) { tm_w += hq_now(); }
                if (r1 > r0) {
                    cblas_dgemm(CBLAS_ROWMAJOR, CBLAS_NOTRANS, CBLAS_NOTRANS, (int)(r1 - r0), (int)Nt,
                                (int)wdt, -1.0, V + r0 * nb, (int)nb, W, (int)N, 1.0,
                                Q + r0 * N + jend, (int)N);
                }
            }
            if (tid == 0) { double _tu = hq_now(); tm_u += _tu; }
            #pragma omp barrier
            if (tid == 0) { tm_trail += hq_now() - tmark; }
        }

        /* ---------- phase 3: R extract + Rinv (thread 0) ---------- */
        if (tid == 0) {
            tm_post = hq_now();
            for (int64_t i = 0; i < N; i++) {
                double *Rr = R + i * N;
                const double *Qi = Q + i * N;
                for (int64_t j = 0; j < i; j++) Rr[j] = 0.0;
                for (int64_t j = i; j < N; j++) Rr[j] = Qi[j];
            }
            for (int64_t i = 0; i < N * N; i++) Rinv[i] = 0.0;
            for (int64_t i = 0; i < N; i++)
                for (int64_t j = i; j < N; j++) Rinv[i * N + j] = R[i * N + j];
            {
                lint nn = (lint)N, info = 0;
                dtrtri_("L", "N", &nn, Rinv, &nn, &info);
            }
            tm_post = hq_now() - tm_post;
        }
        #pragma omp barrier
        if (tid == 0) tm_qgemm = hq_now();

        /* ---- phase 4: Q = A * Rinv row-split GEMMs, then A overwrite ---- */
        {
            const int64_t r0 = (M * (int64_t)tid) / nthr;
            const int64_t r1 = (M * (int64_t)(tid + 1)) / nthr;
            if (r1 > r0)
                cblas_dgemm(CBLAS_ROWMAJOR, CBLAS_NOTRANS, CBLAS_NOTRANS, (int)(r1 - r0), (int)N, (int)N,
                            1.0, A + r0 * N, (int)N, Rinv, (int)N, 0.0, Q + r0 * N, (int)N);
            #pragma omp barrier
            if (tid == 0) { tm_qgemm = hq_now() - tm_qgemm; tm_fin = hq_now(); }
            for (int64_t i = r0; i < r1; i++) {
                double *Ai = A + i * N;
                const double *Ri = (i < N) ? (R + i * N) : NULL;
                for (int64_t j = 0; j < N; j++)
                    Ai[j] = (i < N && j >= i) ? Ri[j] : 0.0;
            }
            if (tid == 0) {
                for (int64_t i = 0; i < N; i++) {
                    double acc = 0.0;
                    for (int64_t j = 0; j <= i; j++) acc += Rinv[j * N + i] * h[j];
                    g[i] = acc;
                }
                for (int64_t i = N - 1; i >= 0; i--) {
                    double sv = g[i];
                    const double *Rr = R + i * N;
                    for (int64_t j = i + 1; j < N; j++) sv -= Rr[j] * x[j];
                    x[i] = sv / Rr[i];
                }
                tm_fin = hq_now() - tm_fin;
            }
        }
    }

    if (hq_dbg())
        printf("copy %.2f | panel %.2f | trail %.2f [st %.2f y %.2f w %.2f u %.2f] | post %.2f | qgemm %.2f | fin %.2f | nth=%d nb=%d M=%d N=%d\n",
               tm_copy * 1e3, tm_pan * 1e3, tm_trail * 1e3, tm_st * 1e3, tm_y * 1e3, tm_w * 1e3, tm_u * 1e3,
               tm_post * 1e3, tm_qgemm * 1e3, tm_fin * 1e3, (int)nth, (int)nb, (int)M, (int)N);

    if (scr_heap) free(scr_heap);
    (void)workspace;
}
