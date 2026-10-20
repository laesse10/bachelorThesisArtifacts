#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
static double hq_now(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
static int hq_dbg(void){const char*e=getenv("HQ_TIME");return e&&e[0];}
#include <omp.h>

typedef int lint;

/* ---------- local BLAS/LAPACK declarations (no headers needed) ---------- */
#define CBLAS_ROWMAJOR 101
#define CBLAS_NOTRANS  111
#define CBLAS_TRANS    112
#define CBLAS_UPPER    121
#define CBLAS_LOWER    122
#define CBLAS_NONUNIT  131
#define CBLAS_LEFT     141
#define CBLAS_RIGHT    142

extern void cblas_dgemm(int Order, int TransA, int TransB, int M, int N, int K,
                        double alpha, const double *a, int lda, const double *b, int ldb,
                        double beta, double *c, int ldc);
extern void cblas_dsyrk(int Order, int Uplo, int Trans, int N, int K,
                        double alpha, const double *a, int lda, double beta, double *c, int ldc);
extern void cblas_dtrmm(int Order, int Side, int Uplo, int TransA, int Diag, int M, int N,
                        double alpha, const double *a, int lda, double *b, int ldb);
extern void dtrtri_(const char *uplo, const char *diag, const lint *n, double *a,
                    const lint *lda, lint *info);
extern void openblas_set_num_threads(int);

#define MAXNB 64
#define MAXTHR 256

static int pick_nb(const int64_t N) {
    const char *e = getenv("HQ_NB");
    if (e && e[0]) return atoi(e);
    if (N >= 192) return 32;
    if (N >= 96) return 24;
    return 16;
}

/* ------------------------------------------------------------------ */
/*  Panel factorization of columns [jb, jb+nb) of the row-major M x N   */
/*  matrix P.  Reflector i (i = 0..nb-1, global column jb+i) is stored  */
/*  in column i of V (row-major M x nb, ld = nb).                       */
/* ------------------------------------------------------------------ */
static void panel_factor(double *P, double *V, double *tau,
                         const int64_t M, const int64_t N,
                         const int64_t jb, const int64_t nb) {
    const int64_t jend = jb + nb;
    double *sh_s = (double *)malloc((size_t)MAXTHR * sizeof(double));
    double *sh_d = (double *)malloc((size_t)MAXTHR * (size_t)MAXNB * sizeof(double));
    double *sh_row = (double *)malloc((size_t)MAXNB * sizeof(double));
    double (*shd)[MAXNB] = (double (*)[MAXNB])sh_d;

    /* zero the staircase region of V: rows jb..jend-1, columns jj > i-jb
       (reflector jj acts on rows jb+jj..M-1 only) */
    for (int64_t i = jb; i < jend; i++) {
        double *Vi = V + i * nb;
        for (int64_t jj = i - jb + 1; jj < nb; jj++) Vi[jj] = 0.0;
    }

    #pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int nth = omp_get_num_threads();
        const int64_t tot = M - jb;
        const int64_t r0 = jb + (tot * (int64_t)tid) / nth;
        const int64_t r1 = jb + (tot * (int64_t)(tid + 1)) / nth;

        for (int64_t j = jb; j < jend; j++) {
            const int64_t ncols = jend - j;      /* number of live columns */
            const int64_t jj = j - jb;           /* column index inside V */
            /* ---- pass 1: partial sums over rows > j ---- */
            double ps = 0.0;
            double pd[MAXNB];
            for (int64_t c = 1; c < ncols; c++) pd[c - 1] = 0.0;
            const int64_t i0 = (r0 > j + 1) ? r0 : (j + 1);
            for (int64_t i = i0; i < r1; i++) {
                const double v = P[i * N + j];
                ps += v * v;
                const double *Pi = P + i * N;
                for (int64_t c = j + 1; c < jend; c++) pd[c - j - 1] += v * Pi[c];
            }
            sh_s[tid] = ps;
            for (int64_t c = 1; c < ncols; c++) shd[tid][c - 1] = pd[c - 1];
            /* the owner of row j snapshots it (raced-free read: row j was last
               written before the previous barrier) */
            if (j >= r0 && j < r1) {
                const double *Pj = P + j * N;
                for (int64_t c = 0; c < ncols; c++) sh_row[c] = Pj[j + c];
            }
            #pragma omp barrier

            /* ---- reduction (every thread computes the same values) ---- */
            double s = 0.0;
            double dd[MAXNB];
            for (int64_t c = 1; c < ncols; c++) dd[c] = 0.0;
            for (int t = 0; t < nth; t++) {
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

            /* ---- pass 2: apply the reflector ---- */
            if (j >= r0 && j < r1) {
                double *Pj = P + j * N;
                V[j * nb + jj] = vj;
                for (int64_t c = 0; c < ncols; c++) Pj[j + c] -= vj * w[c];
            }
            const int64_t i1 = (r0 > j + 1) ? r0 : (j + 1);
            for (int64_t i = i1; i < r1; i++) {
                const double v = P[i * N + j];
                V[i * nb + jj] = v;
                double *Pi = P + i * N;
                for (int64_t c = j + 1; c < jend; c++) Pi[c] -= v * w[c - j];
            }
            #pragma omp barrier
        }
    }
    free(sh_s);
    free(sh_d);
    free(sh_row);
}

/* ------------------------------------------------------------------ */
void householder_qr_fp64(double *restrict A, double *restrict Q, double *restrict R,
                         const double *restrict b, double *restrict x,
                         const int64_t M, const int64_t N,
                         uint8_t *restrict workspace, const int64_t workspace_size) {
    if (M <= 0 || N <= 0) return;
#ifdef _OPENMP
    openblas_set_num_threads(omp_get_max_threads());
#endif
    const int64_t nb = pick_nb(N);
    const int64_t nb_eff = (nb < N) ? nb : N;
    double tp=0, tg=0;

    /* scratch: V (M*nb) + tau(N) + S,T (nb*nb) + W(nb*N) + Rinv(N*N) + g(N)
       + h (N) + per-thread h partials (MAXTHR*N) */
    int64_t need_d = M * nb_eff + N + 2 * nb_eff * nb_eff + nb_eff * N + N * N + N
                   + N + MAXTHR * N + 16;
    double *scr = NULL, *scr_heap = NULL;
    if (workspace && workspace_size >= need_d * (int64_t)sizeof(double)) {
        scr = (double *)workspace;
    } else {
        scr_heap = (double *)malloc((size_t)need_d * sizeof(double));
        scr = scr_heap;
    }
    double *V = scr;
    double *tau = V + M * nb_eff;
    double *S = tau + N;
    double *T = S + nb_eff * nb_eff;
    double *W = T + nb_eff * nb_eff;
    double *Rinv = W + nb_eff * N;
    double *g = Rinv + N * N;
    double *h = g + N;
    double *hp = h + N;

    double T0=hq_now();
    /* ---- copy A into Q and accumulate h = A^T b in the same pass ---- */
    {
        int nthr = 1;
        #pragma omp parallel
        {
            const int tid = omp_get_thread_num();
            const int nth = omp_get_num_threads();
            double *my = hp + (size_t)tid * N;
            for (int64_t j = 0; j < N; j++) my[j] = 0.0;
            const int64_t tot = M;
            const int64_t r0 = (tot * (int64_t)tid) / nth;
            const int64_t r1 = (tot * (int64_t)(tid + 1)) / nth;
            for (int64_t i = r0; i < r1; i++) {
                const double bi = b[i];
                const double *Ai = A + i * N;
                double *Qi = Q + i * N;
                for (int64_t j = 0; j < N; j++) {
                    Qi[j] = Ai[j];
                    my[j] += Ai[j] * bi;
                }
            }
            #pragma omp barrier
            if (tid == 0) {
                for (int64_t j = 0; j < N; j++) h[j] = 0.0;
                for (int t = 0; t < nth; t++)
                    for (int64_t j = 0; j < N; j++) h[j] += hp[(size_t)t * N + j];
            }
            if (tid == 0) nthr = nth;
        }
        (void)nthr;
    }

    double T1=hq_now();
    /* ---- blocked Householder QR of the copy held in Q ---- */
    for (int64_t jb = 0; jb < N; jb += nb_eff) {
        int64_t wdt = (N - jb < nb_eff) ? (N - jb) : nb_eff;
        double pa=hq_now(); panel_factor(Q, V, tau, M, N, jb, wdt); tp += hq_now()-pa;
        const int64_t jend = jb + wdt;
        if (jend >= N) break;
        const int64_t Mr = M - jb;
        const int64_t Nt = N - jend;
        const double *Vs = V + jb * nb_eff;
        double *At = Q + jb * N + jend;
        double ga=hq_now();
        /* S = Vs^T Vs (upper triangle) */
        cblas_dsyrk(CBLAS_ROWMAJOR, CBLAS_UPPER, CBLAS_TRANS, (int)wdt, (int)Mr,
                    1.0, Vs, (int)nb_eff, 0.0, S, (int)wdt);
        for (int64_t i = 0; i < wdt; i++)
            for (int64_t j2 = 0; j2 < i; j2++)
                S[i * wdt + j2] = S[j2 * wdt + i];   /* mirror to full symmetric */
        /* T (lower triangular in this convention) */
        for (int64_t i = 0; i < wdt; i++) {
            for (int64_t k = 0; k < i; k++) {
                double acc = 0.0;
                for (int64_t m = 0; m < i; m++) acc += S[m * wdt + i] * T[m * wdt + k];
                T[i * wdt + k] = -tau[i] * acc;
            }
            T[i * wdt + i] = tau[i];
        }
        /* W = T * (Vs^T * At) */
        cblas_dgemm(CBLAS_ROWMAJOR, CBLAS_TRANS, CBLAS_NOTRANS, (int)wdt, (int)Nt, (int)Mr,
                    1.0, Vs, (int)nb_eff, At, (int)N, 0.0, W, (int)Nt);
        cblas_dtrmm(CBLAS_ROWMAJOR, CBLAS_LEFT, CBLAS_LOWER, CBLAS_NOTRANS, CBLAS_NONUNIT,
                    (int)wdt, (int)Nt, 1.0, T, (int)wdt, W, (int)Nt);
        /* At -= Vs * W */
        cblas_dgemm(CBLAS_ROWMAJOR, CBLAS_NOTRANS, CBLAS_NOTRANS, (int)Mr, (int)Nt, (int)wdt,
                    -1.0, Vs, (int)nb_eff, W, (int)Nt, 1.0, At, (int)N);
        tg += hq_now()-ga;
    }

    double T2=hq_now();
    /* ---- R: upper triangle of Q, zeros below ---- */
    for (int64_t i = 0; i < N; i++) {
        double *Rr = R + i * N;
        const double *Qi = Q + i * N;
        for (int64_t j = 0; j < i; j++) Rr[j] = 0.0;
        for (int64_t j = i; j < N; j++) Rr[j] = Qi[j];
    }

    /* ---- Rinv = R^{-1} (row-major upper triangular) ---- */
    for (int64_t i = 0; i < N * N; i++) Rinv[i] = 0.0;
    for (int64_t i = 0; i < N; i++)
        for (int64_t j = i; j < N; j++) Rinv[i * N + j] = R[i * N + j];
    {
        lint nn = (lint)N, info = 0;
        dtrtri_("L", "N", &nn, Rinv, &nn, &info);
    }

    double T3=hq_now();
    /* ---- Q = A * Rinv ---- */
    cblas_dgemm(CBLAS_ROWMAJOR, CBLAS_NOTRANS, CBLAS_NOTRANS, (int)M, (int)N, (int)N,
                1.0, A, (int)N, Rinv, (int)N, 0.0, Q, (int)N);

    double T4=hq_now();
    /* ---- g = Q^T b = Rinv^T (A^T b) = Rinv^T h ---- */
    for (int64_t i = 0; i < N; i++) {
        double acc = 0.0;
        for (int64_t j = 0; j <= i; j++) acc += Rinv[j * N + i] * h[j];
        g[i] = acc;
    }

    double T5=hq_now();
    if (hq_dbg()) { /* g cross-check */
        double mx=0, m2=0;
        for (int64_t j = 0; j < N; j++) {
            double acc = 0.0;
            for (int64_t i = 0; i < M; i++) acc += Q[i*N+j]*b[i];
            double d = fabs(acc-g[j]); if (d>mx) mx=d;
            double d2 = fabs(acc); if (d2>m2) m2=d2;
        }
    }
    /* ---- overwrite A with the reference's in-place result: upper = R, below = 0 ---- */
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; i++) {
        double *Ai = A + i * N;
        const double *Ri = (i < N) ? (R + i * N) : NULL;
        for (int64_t j = 0; j < N; j++)
            Ai[j] = (i < N && j >= i) ? Ri[j] : 0.0;
    }

    /* ---- back substitution ---- */
    for (int64_t i = N - 1; i >= 0; i--) {
        double s = g[i];
        const double *Rr = R + i * N;
        for (int64_t j = i + 1; j < N; j++) s -= Rr[j] * x[j];
        x[i] = s / Rr[i];
    }
    double T6=hq_now();
    if (hq_dbg()) fprintf(stderr,"copy %.2f | panel %.2f | gemm %.2f | rextri %.2f | qgemm %.2f | gvec %.2f | awrite %.2f | bsub %.2f\n",(T1-T0)*1e3,tp*1e3,tg*1e3,(T2-T1-tp-tg)*1e3,(T3-T2)*1e3,(T4-T3)*1e3,(T5-T4)*1e3,(T6-T5)*1e3,(hq_now()-T6)*1e3);
    if (scr_heap) free(scr_heap);
    (void)workspace;
}
