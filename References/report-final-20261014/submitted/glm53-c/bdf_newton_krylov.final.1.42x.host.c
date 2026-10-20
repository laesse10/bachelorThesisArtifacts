/* bdf_newton_krylov - exact, optimized port of the NumPy reference.
 *
 * Correctness contract: outputs must match the numba/numpy reference
 * bit-for-bit (the algorithm's acceptance thresholds are chaotic w.r.t.
 * rounding at N >= 61).  Therefore:
 *   - every row dot product uses an exact replica of OpenBLAS 0.3.30's
 *     dot_compute (the kernel numba's `a @ b` calls on this host),
 *   - row-dot results are folded left, in row order, as the reference,
 *   - FMA contraction is disabled in every arithmetic kernel (numba does
 *     not contract),
 *   - every expression keeps the reference's evaluation order.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>

#define NOFMA __attribute__((optimize("fp-contract=off")))

#define H0 1.0e-3
#define CONV_DEGRADE 0.3
#define SAFETY 0.9
#define MIN_FACTOR 0.2
#define MAX_FACTOR 6.0

#define RTOL 1.0e-6
#define ABSTOL 1.0e-8
#define NEWTON_RTOL 1.0e-12
#define MAX_ORDER 5
#define MAX_NEWTON 12
#define GMRES_RESTART 20
#define GMRES_TOL 0.1

#define MAXHIST (MAX_ORDER + 1)      /* 6 */
#define MAXQ (GMRES_RESTART + 1)     /* 21 */

/* ------------- exact replica of OpenBLAS dot_compute (inc_x=inc_y=1) ------- */

static inline double ddot_rep(int n, const double *x, const double *y) {
    if (n <= 0) return 0.0;
    if (n < 16) {
        __m128d acc = _mm_setzero_pd();
        int i = 0;
        if (n >= 8) {
            __m512d X = _mm512_loadu_pd(x), Y = _mm512_loadu_pd(y);
            __m256d p = _mm256_mul_pd(_mm512_castpd512_pd256(X),
                                      _mm512_castpd512_pd256(Y));
            __m256d q = _mm256_mul_pd(_mm512_extractf64x4_pd(X, 1),
                                      _mm512_extractf64x4_pd(Y, 1));
            double pv[4] __attribute__((aligned(32))), qv[4] __attribute__((aligned(32)));
            _mm256_store_pd(pv, p);
            _mm256_store_pd(qv, q);
            for (int j = 0; j < 4; j++) acc = _mm_add_sd(acc, _mm_set_sd(pv[j]));
            for (int j = 0; j < 4; j++) acc = _mm_add_sd(acc, _mm_set_sd(qv[j]));
            i = 8;
        }
        for (; i < n; i++)
            acc = _mm_add_sd(acc, _mm_mul_sd(_mm_load_sd(x + i), _mm_load_sd(y + i)));
        return _mm_cvtsd_f64(acc);
    }
    int m32 = n & ~31;
    __m512d a0 = _mm512_setzero_pd(), a1 = _mm512_setzero_pd();
    __m512d a2 = _mm512_setzero_pd(), a3 = _mm512_setzero_pd();
    for (int i = 0; i < m32; i += 32) {
        a0 = _mm512_fmadd_pd(_mm512_loadu_pd(x + i),      _mm512_loadu_pd(y + i),      a0);
        a1 = _mm512_fmadd_pd(_mm512_loadu_pd(x + i + 8),  _mm512_loadu_pd(y + i + 8),  a1);
        a2 = _mm512_fmadd_pd(_mm512_loadu_pd(x + i + 16), _mm512_loadu_pd(y + i + 16), a2);
        a3 = _mm512_fmadd_pd(_mm512_loadu_pd(x + i + 24), _mm512_loadu_pd(y + i + 24), a3);
    }
    __m256d A = _mm256_add_pd(_mm512_castpd512_pd256(a0), _mm512_extractf64x4_pd(a0, 1));
    __m256d B = _mm256_add_pd(_mm512_castpd512_pd256(a1), _mm512_extractf64x4_pd(a1, 1));
    __m256d C = _mm256_add_pd(_mm512_castpd512_pd256(a2), _mm512_extractf64x4_pd(a2, 1));
    __m256d D = _mm256_add_pd(_mm512_castpd512_pd256(a3), _mm512_extractf64x4_pd(a3, 1));
    if ((n & 31) >= 16) {
        A = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32),      _mm256_loadu_pd(y + m32),      A);
        B = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32 + 4),  _mm256_loadu_pd(y + m32 + 4),  B);
        C = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32 + 8),  _mm256_loadu_pd(y + m32 + 8),  C);
        D = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32 + 12), _mm256_loadu_pd(y + m32 + 12), D);
    }
    __m256d S = _mm256_add_pd(A, B);
    S = _mm256_add_pd(S, C);
    S = _mm256_add_pd(S, D);
    __m128d lo = _mm256_castpd256_pd128(S);
    __m128d t = _mm_add_pd(lo, _mm256_extractf128_pd(S, 1));
    double res = _mm_cvtsd_f64(_mm_hadd_pd(t, t));
    __m128d acc = _mm_set_sd(res);
    for (int i = (n & ~15); i < n; i++)
        acc = _mm_add_sd(acc, _mm_mul_sd(_mm_load_sd(x + i), _mm_load_sd(y + i)));
    return _mm_cvtsd_f64(acc);
}

/* left fold over the per-row dots: s = (s + du_i) + dv_i, row order */
static inline double fold_dots(const double *dv, int nrows) {
    double s = 0.0;
    for (int i = 0; i < nrows; i++) {
        s = s + dv[2*i];
        s = s + dv[2*i + 1];
    }
    return s;
}

/* Batched row-dots, bit-identical to per-row ddot_rep:
 * dvals[2i]   = ddot_rep(N, xu_row_i, yu_row_i)
 * dvals[2i+1] = ddot_rep(N, xv_row_i, yv_row_i)
 * (batches 2 grid-rows at a time: 4 independent dot chains in registers) */

/* finish one 53-style row dot from its 4 zmm accumulators + optional 16-block + scalar tail */
static inline double rd_finish(const double *x, const double *y, int N,
                               __m512d a0, __m512d a1, __m512d a2, __m512d a3) {
    int m32 = N & ~31;
    __m256d A = _mm256_add_pd(_mm512_castpd512_pd256(a0), _mm512_extractf64x4_pd(a0, 1));
    __m256d B = _mm256_add_pd(_mm512_castpd512_pd256(a1), _mm512_extractf64x4_pd(a1, 1));
    __m256d C = _mm256_add_pd(_mm512_castpd512_pd256(a2), _mm512_extractf64x4_pd(a2, 1));
    __m256d D = _mm256_add_pd(_mm512_castpd512_pd256(a3), _mm512_extractf64x4_pd(a3, 1));
    if ((N & 31) >= 16) {
        A = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32),     _mm256_loadu_pd(y + m32),     A);
        B = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32 + 4), _mm256_loadu_pd(y + m32 + 4), B);
        C = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32 + 8), _mm256_loadu_pd(y + m32 + 8), C);
        D = _mm256_fmadd_pd(_mm256_loadu_pd(x + m32 + 12), _mm256_loadu_pd(y + m32 + 12), D);
    }
    __m256d S = _mm256_add_pd(A, B);
    S = _mm256_add_pd(S, C);
    S = _mm256_add_pd(S, D);
    __m128d lo = _mm256_castpd256_pd128(S);
    __m128d t = _mm_add_pd(lo, _mm256_extractf128_pd(S, 1));
    double res = _mm_cvtsd_f64(_mm_hadd_pd(t, t));
    __m128d acc = _mm_set_sd(res);
    for (int i = N & ~15; i < N; i++)
        acc = _mm_add_sd(acc, _mm_mul_sd(_mm_load_sd(x + i), _mm_load_sd(y + i)));
    return _mm_cvtsd_f64(acc);
}

#define RD_BLK(X, Y, A0, A1, A2, A3) do { \
    A0 = _mm512_fmadd_pd(_mm512_loadu_pd(X + i),      _mm512_loadu_pd(Y + i),      A0); \
    A1 = _mm512_fmadd_pd(_mm512_loadu_pd(X + i + 8),  _mm512_loadu_pd(Y + i + 8),  A1); \
    A2 = _mm512_fmadd_pd(_mm512_loadu_pd(X + i + 16), _mm512_loadu_pd(Y + i + 16), A2); \
    A3 = _mm512_fmadd_pd(_mm512_loadu_pd(X + i + 24), _mm512_loadu_pd(Y + i + 24), A3); \
} while (0)

NOFMA static void row_dots(const double *xu, const double *xv,
                           const double *yu, const double *yv,
                           int N, double *dvals) {
    int m32 = N & ~31;
    size_t RN = (size_t)N;
    for (int r = 0; r < N; r += 2) {
        if (r + 1 < N) {
            const double *X0 = xu + r*RN,       *Y0 = yu + r*RN;
            const double *X1 = xv + r*RN,       *Y1 = yv + r*RN;
            const double *X2 = xu + (r+1)*RN,   *Y2 = yu + (r+1)*RN;
            const double *X3 = xv + (r+1)*RN,   *Y3 = yv + (r+1)*RN;
            __m512d a00=_mm512_setzero_pd(), a01=_mm512_setzero_pd(),
                    a02=_mm512_setzero_pd(), a03=_mm512_setzero_pd();
            __m512d a10=_mm512_setzero_pd(), a11=_mm512_setzero_pd(),
                    a12=_mm512_setzero_pd(), a13=_mm512_setzero_pd();
            __m512d a20=_mm512_setzero_pd(), a21=_mm512_setzero_pd(),
                    a22=_mm512_setzero_pd(), a23=_mm512_setzero_pd();
            __m512d a30=_mm512_setzero_pd(), a31=_mm512_setzero_pd(),
                    a32=_mm512_setzero_pd(), a33=_mm512_setzero_pd();
            for (int i = 0; i < m32; i += 32) {
                RD_BLK(X0, Y0, a00, a01, a02, a03);
                RD_BLK(X1, Y1, a10, a11, a12, a13);
                RD_BLK(X2, Y2, a20, a21, a22, a23);
                RD_BLK(X3, Y3, a30, a31, a32, a33);
            }
            dvals[2*r]     = rd_finish(X0, Y0, N, a00, a01, a02, a03);
            dvals[2*r+1]   = rd_finish(X1, Y1, N, a10, a11, a12, a13);
            dvals[2*r+2]   = rd_finish(X2, Y2, N, a20, a21, a22, a23);
            dvals[2*r+3]   = rd_finish(X3, Y3, N, a30, a31, a32, a33);
        } else {
            const double *X0 = xu + r*RN, *Y0 = yu + r*RN;
            const double *X1 = xv + r*RN, *Y1 = yv + r*RN;
            __m512d a00=_mm512_setzero_pd(), a01=_mm512_setzero_pd(),
                    a02=_mm512_setzero_pd(), a03=_mm512_setzero_pd();
            __m512d a10=_mm512_setzero_pd(), a11=_mm512_setzero_pd(),
                    a12=_mm512_setzero_pd(), a13=_mm512_setzero_pd();
            for (int i = 0; i < m32; i += 32) {
                RD_BLK(X0, Y0, a00, a01, a02, a03);
                RD_BLK(X1, Y1, a10, a11, a12, a13);
            }
            dvals[2*r]   = rd_finish(X0, Y0, N, a00, a01, a02, a03);
            dvals[2*r+1] = rd_finish(X1, Y1, N, a10, a11, a12, a13);
        }
    }
}

typedef struct {
    int N;
    double hgrid;
    double *hu[MAXHIST], *hv[MAXHIST];
    double *qu[MAXQ], *qv[MAXQ];
    double *wu, *wv;
    double *lap_u, *lap_v;
    double *cu, *cuv;      /* frozen jacobian coefficients */
    double *cu2, *cu2v;
    double *pred_u, *pred_v;
    double *trial_u, *trial_v;
    double *rhs_u, *rhs_v;
    double *fu, *fv;
    double *res_u, *res_v;
    double *step_u, *step_v;
    double *trow_u, *trow_v;
    double *dvals;         /* 2N row-dot values */
    double hess[MAXQ][GMRES_RESTART];
    double cs[GMRES_RESTART], sn[GMRES_RESTART], g[MAXQ], y[GMRES_RESTART];
} Ctx;

/* 5-point Neumann laplacian with the reference's exact rounding:
 * interior + top/bottom rows: ((up+down)+left)+right)-4c
 * left/right columns + corners: (up+down) + (nbr+nbr) - 4c   (nbr+nbr == 2*nbr exactly)
 */
NOFMA static void laplacian(const double *A, double *L, int N) {
    const double *restrict Ap = A;
    double *restrict Lp = L;
    for (int i = 0; i < N; i++) {
        const double *up   = (i > 0)     ? Ap + (size_t)(i-1)*N : Ap + (size_t)N;
        const double *down = (i < N - 1) ? Ap + (size_t)(i+1)*N : Ap + (size_t)(N-2)*N;
        const double *row  = Ap + (size_t)i*N;
        double *out = Lp + (size_t)i*N;
        for (int j = 1; j < N-1; j++)
            out[j] = ((up[j] + down[j]) + row[j-1]) + row[j+1] - 4.0*row[j];
        double l2 = row[1] + row[1];
        out[0] = (up[0] + down[0]) + l2 - 4.0*row[0];
        double r2 = row[N-2] + row[N-2];
        out[N-1] = (up[N-1] + down[N-1]) + r2 - 4.0*row[N-1];
    }
}

NOFMA static void set_frozen(Ctx *c, const double *uf, const double *vf, double B) {
    int NN = c->N * c->N;
    for (int i = 0; i < NN; i++) {
        double a = uf[i], b = vf[i];
        c->cu[i]  = 2.0*a*b - (B + 1.0);
        c->cuv[i] = a*a;
        c->cu2[i] = B - 2.0*a*b;
        c->cu2v[i] = -(a*a);
    }
}

NOFMA static void rhs_eval(Ctx *c, double alpha, double A, double B) {
    int N = c->N;
    laplacian(c->trial_u, c->lap_u, N);
    laplacian(c->trial_v, c->lap_v, N);
    double invh2 = 1.0 / (c->hgrid * c->hgrid);
    const double *restrict u = c->trial_u, *restrict v = c->trial_v;
    const double *restrict LU = c->lap_u, *restrict LV = c->lap_v;
    double *restrict du = c->fu, *restrict dvv = c->fv;
    for (int i = 0; i < N*N; i++) {
        du[i]  = A + u[i]*u[i]*v[i] - (B + 1.0)*u[i] + alpha*LU[i]*invh2;
        dvv[i] = B*u[i] - u[i]*u[i]*v[i] + alpha*LV[i]*invh2;
    }
}

NOFMA static void matvec(Ctx *c, const double *du, const double *dv, double hbeta0, double alpha,
                         double *out_du, double *out_dv) {
    int N = c->N;
    laplacian(du, c->lap_u, N);
    laplacian(dv, c->lap_v, N);
    double invh2 = 1.0 / (c->hgrid * c->hgrid);
    const double *restrict CU = c->cu, *restrict CUV = c->cuv;
    const double *restrict CU2 = c->cu2, *restrict CU2V = c->cu2v;
    const double *restrict LU = c->lap_u, *restrict LV = c->lap_v;
    double *restrict OD = out_du, *restrict ODV = out_dv;
    for (int i = 0; i < N*N; i++) {
        OD[i]  = du[i] - hbeta0*((CU[i]*du[i] + CUV[i]*dv[i]) + alpha*LU[i]*invh2);
        ODV[i] = dv[i] - hbeta0*((CU2[i]*du[i] + CU2V[i]*dv[i]) + alpha*LV[i]*invh2);
    }
}

NOFMA static double wrms_norm(const double *au, const double *av, double abstol, double rtol,
                              const double *refu, const double *refv, Ctx *c) {
    int N = c->N;
    double *tu = c->trow_u, *tv = c->trow_v, *dvals = c->dvals;
    for (int i = 0; i < N; i++) {
        const double *aru = au + (size_t)i*N;
        const double *arv = av + (size_t)i*N;
        const double *rfu = refu + (size_t)i*N;
        const double *rfv = refv + (size_t)i*N;
        for (int j = 0; j < N; j++) {
            tu[j] = aru[j] / (abstol + rtol*fabs(rfu[j]));
            tv[j] = arv[j] / (abstol + rtol*fabs(rfv[j]));
        }
        dvals[2*i]   = ddot_rep(N, tu, tu);
        dvals[2*i+1] = ddot_rep(N, tv, tv);
    }
    return sqrt(fold_dots(dvals, N) / (2.0*N*N));
}

NOFMA static void lagrange_weights(const double *nodes, int k1, int deriv, double *weights) {
    double aug[MAXHIST+1][MAXHIST+2];
    for (int row = 0; row < k1; row++)
        for (int col = 0; col < k1; col++) {
            double power = 1.0;
            for (int e = 0; e < row; e++) power = power*nodes[col];
            aug[row][col] = power;
        }
    for (int col = 0; col < k1; col++) aug[col][k1] = 0.0;
    aug[deriv][k1] = 1.0;
    for (int col = 0; col < k1; col++) {
        double pivot = aug[col][col];
        for (int w = 0; w < k1+1; w++) aug[col][w] = aug[col][w] / pivot;
        for (int row = 0; row < k1; row++) {
            if (row != col) {
                double factor = aug[row][col];
                for (int w = 0; w < k1+1; w++) aug[row][w] = aug[row][w] - factor*aug[col][w];
            }
        }
    }
    for (int j = 0; j < k1; j++) weights[j] = aug[j][k1];
}

NOFMA static void gmres_matfree(Ctx *c, const double *rhs_u, const double *rhs_v,
                                double alpha, double B, double hbeta0,
                                double *du, double *dv) {
    int N = c->N;
    int NN = N*N;
    double *dvals = c->dvals;
    (void)B;
    row_dots(rhs_u, rhs_v, rhs_u, rhs_v, N, dvals);
    double beta = sqrt(fold_dots(dvals, N));
    for (int i = 0; i < NN; i++) { du[i] = 0.0; dv[i] = 0.0; }
    if (beta == 0.0) return;
    for (int i = 0; i < NN; i++) {
        c->qu[0][i] = rhs_u[i] / beta;
        c->qv[0][i] = rhs_v[i] / beta;
    }
    c->g[0] = beta;
    int m_used = GMRES_RESTART;
    for (int k = 0; k < GMRES_RESTART; k++) {
        matvec(c, c->qu[k], c->qv[k], hbeta0, alpha, c->wu, c->wv);
        for (int p = 0; p <= k; p++) {
            const double *qpu = c->qu[p], *qpv = c->qv[p];
            double *wu = c->wu, *wv = c->wv;
            row_dots(qpu, qpv, wu, wv, N, dvals);
            double h_pk = fold_dots(dvals, N);
            c->hess[p][k] = h_pk;
            for (int i = 0; i < NN; i++) {
                wu[i] = wu[i] - h_pk*qpu[i];
                wv[i] = wv[i] - h_pk*qpv[i];
            }
        }
        row_dots(c->wu, c->wv, c->wu, c->wv, N, dvals);
        double h_next = sqrt(fold_dots(dvals, N));
        c->hess[k+1][k] = h_next;
        for (int p = 0; p < k; p++) {
            double temp = c->cs[p]*c->hess[p][k] + c->sn[p]*c->hess[p+1][k];
            c->hess[p+1][k] = -c->sn[p]*c->hess[p][k] + c->cs[p]*c->hess[p+1][k];
            c->hess[p][k] = temp;
        }
        double denom = sqrt(c->hess[k][k]*c->hess[k][k] + c->hess[k+1][k]*c->hess[k+1][k]);
        c->cs[k] = c->hess[k][k] / denom;
        c->sn[k] = c->hess[k+1][k] / denom;
        c->hess[k][k] = c->cs[k]*c->hess[k][k] + c->sn[k]*c->hess[k+1][k];
        c->hess[k+1][k] = 0.0;
        double temp = c->cs[k]*c->g[k];
        c->g[k+1] = -c->sn[k]*c->g[k];
        c->g[k] = temp;
        double rel = fabs(c->g[k+1]) / beta;
        if (rel < GMRES_TOL || h_next < 1.0e-13 || k == GMRES_RESTART-1) {
            m_used = k+1;
            break;
        }
        for (int i = 0; i < NN; i++) {
            c->qu[k+1][i] = c->wu[i] / h_next;
            c->qv[k+1][i] = c->wv[i] / h_next;
        }
    }
    for (int row = 0; row < m_used; row++) {
        int rr = m_used - 1 - row;
        double s = c->g[rr];
        for (int col = rr+1; col < m_used; col++) s = s - c->hess[rr][col]*c->y[col];
        c->y[rr] = s / c->hess[rr][rr];
    }
    for (int p = 0; p < m_used; p++) {
        double yp = c->y[p];
        for (int i = 0; i < NN; i++) {
            du[i] = du[i] + c->qu[p][i]*yp;
            dv[i] = dv[i] + c->qv[p][i]*yp;
        }
    }
}

NOFMA void bdf_newton_krylov_fp64(
    double *restrict diagnostics, int64_t *restrict order_history,
    double *restrict u, double *restrict v,
    const double A, const double B, const int64_t N_, const double alpha,
    const int64_t max_steps, const double t_end,
    uint8_t *restrict workspace, const int64_t workspace_size) {
    (void)workspace; (void)workspace_size;
    int N = (int)N_;
    int maxhist = MAXHIST;
    Ctx *c = calloc(1, sizeof(Ctx));
    c->N = N;
    c->hgrid = 1.0 / N;
    size_t NN = (size_t)N * N;
    for (int j = 0; j < maxhist; j++) {
        c->hu[j] = calloc(NN, sizeof(double));
        c->hv[j] = calloc(NN, sizeof(double));
    }
    for (int j = 0; j < MAXQ; j++) {
        c->qu[j] = calloc(NN, sizeof(double));
        c->qv[j] = calloc(NN, sizeof(double));
    }
    double **bufs[] = { &c->wu, &c->wv, &c->lap_u, &c->lap_v, &c->cu, &c->cuv,
                        &c->cu2, &c->cu2v, &c->pred_u, &c->pred_v, &c->trial_u, &c->trial_v,
                        &c->rhs_u, &c->rhs_v, &c->fu, &c->fv, &c->res_u, &c->res_v,
                        &c->step_u, &c->step_v, NULL };
    for (int i = 0; bufs[i]; i++) *bufs[i] = calloc(NN, sizeof(double));
    c->trow_u = calloc((size_t)N, sizeof(double));
    c->trow_v = calloc((size_t)N, sizeof(double));
    c->dvals  = calloc(2*(size_t)N, sizeof(double));

    memcpy(c->hu[0], u, sizeof(double)*NN);
    memcpy(c->hv[0], v, sizeof(double)*NN);
    double hist_t[MAXHIST];
    hist_t[0] = 0.0;
    int n_hist = 1;
    int head = 0;

    double nodes_p[MAXHIST], nodes_c[MAXHIST], w_pred[MAXHIST], c_corr[MAXHIST];

    double t = 0.0, h = H0;
    int order = 1, steps_since_order_change = 0, consecutive_reject = 0;
    long long njev = 0, nsteps = 0;
    int have_jac = 0;

    while (t < t_end && nsteps < max_steps) {
        if (t + h > t_end) h = t_end - t;
        int k = order < n_hist ? order : n_hist;
        double t_new = t + h;

        int deg = k < n_hist - 1 ? k : n_hist - 1;
        for (int j = 0; j <= deg; j++) nodes_p[j] = hist_t[j] - t_new;
        lagrange_weights(nodes_p, deg+1, 0, w_pred);
        for (size_t i = 0; i < NN; i++) { c->pred_u[i] = 0.0; c->pred_v[i] = 0.0; }
        for (int j = 0; j <= deg; j++) {
            int hj = (head + j) % maxhist;
            double wj = w_pred[j];
            for (size_t i = 0; i < NN; i++) {
                c->pred_u[i] = c->pred_u[i] + wj*c->hu[hj][i];
                c->pred_v[i] = c->pred_v[i] + wj*c->hv[hj][i];
            }
        }

        nodes_c[0] = 0.0;
        for (int j = 0; j < k; j++) nodes_c[j+1] = hist_t[j] - t_new;
        lagrange_weights(nodes_c, k+1, 1, c_corr);
        double c0 = c_corr[0];
        double hbeta0 = 1.0 / c0;
        for (size_t i = 0; i < NN; i++) { c->rhs_u[i] = 0.0; c->rhs_v[i] = 0.0; }
        for (int j = 1; j <= k; j++) {
            double r = c_corr[j] / c0;
            int hj = (head + j - 1) % maxhist;
            for (size_t i = 0; i < NN; i++) {
                c->rhs_u[i] = c->rhs_u[i] - r*c->hu[hj][i];
                c->rhs_v[i] = c->rhs_v[i] - r*c->hv[hj][i];
            }
        }

        memcpy(c->trial_u, c->pred_u, sizeof(double)*NN);
        memcpy(c->trial_v, c->pred_v, sizeof(double)*NN);
        if (!have_jac) {
            have_jac = 1;
            njev++;
            set_frozen(c, c->trial_u, c->trial_v, B);
        }

        int converged = 0, refactored_this_step = 0;
        double prev_resnorm = -1.0;
        for (int it = 0; it < MAX_NEWTON; it++) {
            rhs_eval(c, alpha, A, B);
            for (size_t i = 0; i < NN; i++) {
                c->res_u[i] = c->trial_u[i] - hbeta0*c->fu[i] - c->rhs_u[i];
                c->res_v[i] = c->trial_v[i] - hbeta0*c->fv[i] - c->rhs_v[i];
            }
            double resnorm = wrms_norm(c->res_u, c->res_v, NEWTON_RTOL, NEWTON_RTOL,
                                       c->trial_u, c->trial_v, c);
            if (resnorm < 1.0) { converged = 1; break; }
            if (prev_resnorm > 0.0) {
                double rate = resnorm / prev_resnorm;
                if (rate > CONV_DEGRADE && !refactored_this_step) {
                    set_frozen(c, c->trial_u, c->trial_v, B);
                    njev++;
                    refactored_this_step = 1;
                }
            }
            prev_resnorm = resnorm;
            for (size_t i = 0; i < NN; i++) {
                c->res_u[i] = -c->res_u[i];
                c->res_v[i] = -c->res_v[i];
            }
            gmres_matfree(c, c->res_u, c->res_v, alpha, B, hbeta0, c->step_u, c->step_v);
            for (size_t i = 0; i < NN; i++) {
                c->trial_u[i] = c->trial_u[i] + c->step_u[i];
                c->trial_v[i] = c->trial_v[i] + c->step_v[i];
            }
        }

        if (!converged) {
            h = h * 0.25;
            if (order > 1) order = order - 1;
            continue;
        }

        double c_err = 1.0 / (k + 1);
        for (size_t i = 0; i < NN; i++) {
            c->res_u[i] = c_err*(c->trial_u[i] - c->pred_u[i]);
            c->res_v[i] = c_err*(c->trial_v[i] - c->pred_v[i]);
        }
        double err_est = wrms_norm(c->res_u, c->res_v, ABSTOL, RTOL, c->trial_u, c->trial_v, c);

        if (err_est <= 1.0) {
            consecutive_reject = 0;
            t = t_new;
            order_history[nsteps] = order;
            nsteps++;
            for (int j = maxhist-1; j > 0; j--) hist_t[j] = hist_t[j-1];
            hist_t[0] = t;
            head = (head + maxhist - 1) % maxhist;
            memcpy(c->hu[head], c->trial_u, sizeof(double)*NN);
            memcpy(c->hv[head], c->trial_v, sizeof(double)*NN);
            if (n_hist < maxhist) n_hist++;

            steps_since_order_change++;
            if (order < MAX_ORDER && steps_since_order_change > order && n_hist > order + 1) {
                order++;
                steps_since_order_change = 0;
            } else if (refactored_this_step == 1 && order > 1) {
                order--;
                steps_since_order_change = 0;
            }
            double fac;
            if (err_est > 0.0) fac = SAFETY * pow(err_est, -1.0/(k+1));
            else fac = MAX_FACTOR;
            if (fac > MAX_FACTOR) fac = MAX_FACTOR;
            if (fac < MIN_FACTOR) fac = MIN_FACTOR;
            h = h * fac;
        } else {
            consecutive_reject++;
            double fac = SAFETY * pow(err_est, -1.0/(k+1));
            if (fac > 1.0) fac = 1.0;
            if (fac < MIN_FACTOR) fac = MIN_FACTOR;
            h = h * fac;
            if (consecutive_reject >= 2 && order > 1) {
                order--;
                steps_since_order_change = 0;
            }
        }
    }

    memcpy(u, c->hu[head], sizeof(double)*NN);
    memcpy(v, c->hv[head], sizeof(double)*NN);
    diagnostics[0] = (double)nsteps;
    diagnostics[1] = (double)njev;
    diagnostics[2] = t;
}
