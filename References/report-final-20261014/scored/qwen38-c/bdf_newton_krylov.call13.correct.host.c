#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* The target box's native ISA turns out to run wide (512-bit) FMA at
 * reduced speed, so we pin the vector width to the 256-bit AVX2 class:
 * identical throughput on fast 512-bit cores, and no throttling on the
 * ones that penalize 512-bit floating point.  */
#pragma GCC target ("no-avx512f","no-avx512cd","no-avx512bw","no-avx512dq","no-avx512vl")

/* 256-bit intrinsics (must come after the target pragma) */
#include <immintrin.h>

/* =========================================================================
 * Variable-order variable-step BDF integrator over a Newton-Krylov (GMRES)
 * corrector, 2-D Brusselator reaction-diffusion (Neumann boundaries).
 *
 * Faithful C port of the NumPy reference, restructured for speed:
 *   - ring-buffer state history (no per-step plane shifts)
 *   - fused single-pass row kernels, SIMD-vectorized row loops
 *   - frozen-Jacobian coefficients (ja, jb, jc) precomputed on refactor
 *   - GMRES correction accumulated straight into the trial state
 *
 * The integrator's control flow is a strict serial recurrence; only the
 * row-wise grid passes are SIMD-vectorized.  Each row pass accumulates its
 * per-row sums into plain slots that are combined in fixed row order, so
 * every scalar the integrator branches on is computed in an order that is
 * independent of any threading or vectorization detail.
 *
 * No OpenMP team is ever created: on the target box the cgroup quota
 * admits at most one useful core, and the per-pass construct overhead of a
 * team would exceed any speedup.  Plain `omp simd` pragmas (no team) still
 * drive the vectorization.
 */

#define H0            1.0e-3
#define CONV_DEGRADE  0.3
#define SAFETY        0.9
#define MIN_FACTOR    0.2
#define MAX_FACTOR    6.0

#define RTOL          2.2e-7
#define ABSTOL        1.0e-10
#define NEWTON_RTOL   1.0e-12
#define MAX_ORDER     5
#define MAX_NEWTON    12
#define GMRES_RESTART 30
#define GMRES_TOL     0.1
#define MAXHIST       (MAX_ORDER + 1)
#define MAXROWS       256

/* N^2 planes: hist 12, jac 3, pred 2, trial 2, rhs 2, q 2*(RESTART+1), w 2 */
#define N_PLANES (2 * MAXHIST + 3 + 2 + 2 + 2 + 2 * (GMRES_RESTART + 1) + 2)

/* per-row sum slots (plain stores, one writer per slot) */
static double part_a[2 * MAXROWS];             /* pass A: lrow, lrowb   */
static double part_p[MAXROWS * (GMRES_RESTART + 1)]; /* projection: per p */
static double part_h[MAXROWS];                  /* h2 update             */
static double part_e[MAXROWS];                  /* error estimate        */

/* lagrange_weights: solve V w = e_deriv without pivoting, mirroring the
 * reference bit for bit (latched pivot and factor, in-place row updates). */
static void lagrange_weights(const double *nodes, int k1, int deriv,
                             const int max_order, double *weights)
{
    double aug[MAXHIST][MAXHIST + 1];
    for (int r = 0; r <= max_order; r++)
        for (int c = 0; c <= max_order + 1; c++)
            aug[r][c] = 0.0;
    for (int row = 0; row < k1; row++) {
        for (int col = 0; col < k1; col++) {
            double power = 1.0;
            for (int t = 0; t < row; t++)
                power = power * nodes[col];
            aug[row][col] = power;
        }
    }
    aug[deriv][k1] = 1.0;
    for (int col = 0; col < k1; col++) {
        double pivot = aug[col][col];
        for (int w = 0; w <= k1; w++)
            aug[col][w] = aug[col][w] / pivot;
        for (int row = 0; row < k1; row++) {
            if (row == col)
                continue;
            double factor = aug[row][col];
            for (int w = 0; w <= k1; w++)
                aug[row][w] = aug[row][w] - factor * aug[col][w];
        }
    }
    for (int j = 0; j < k1; j++)
        weights[j] = aug[j][k1];
}

void bdf_newton_krylov_fp64(
    double *restrict diagnostics,
    int64_t *restrict order_history,
    double *restrict u,
    double *restrict v,
    const double A,
    const double B,
    const int64_t N,
    const double alpha,
    const int64_t max_steps,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    if (N > MAXROWS)
        return;
    const int64_t nn = N * N;
    const int64_t need_bytes = (int64_t)N_PLANES * nn * 8 + 4096;
    uint8_t *ws;
    bool own_ws = false;
    if (workspace != NULL && workspace_size >= need_bytes)
        ws = workspace;
    else {
        ws = (uint8_t *)malloc((size_t)need_bytes);
        if (ws == NULL)
            return;
        own_ws = true;
    }

    double *base = (double *)ws;
    double *hu[MAXHIST];
    double *hv[MAXHIST];
    for (int j = 0; j < MAXHIST; j++) {
        hu[j] = base + (int64_t)j * nn;
        hv[j] = base + (int64_t)(MAXHIST + j) * nn;
    }
    double *ja      = base + (int64_t)(2 * MAXHIST) * nn;            /* 1  */
    double *jb      = ja + nn;                                       /* 1  */
    double *jc      = jb + nn;                                       /* 1  */
    double *u_pred  = jc + nn;                                       /* 1  */
    double *v_pred  = u_pred + nn;                                   /* 1  */
    double *u_trial = v_pred + nn;                                   /* 1  */
    double *v_trial = u_trial + nn;                                  /* 1  */
    double *rhs_u   = v_trial + nn;                                  /* 1  */
    double *rhs_v   = rhs_u + nn;                                    /* 1  */
    double *qup[GMRES_RESTART + 1];
    double *qvp[GMRES_RESTART + 1];
    for (int p = 0; p <= GMRES_RESTART; p++) {
        qup[p] = rhs_v + nn + (int64_t)p * nn;
        qvp[p] = rhs_v + nn + (int64_t)(GMRES_RESTART + 1 + p) * nn;
    }
    double *w_u = qvp[GMRES_RESTART] + nn;                           /* 1  */
    double *w_v = w_u + nn;                                          /* 1  */

    /* sign-bit mask for 256-bit doubles (intrinsic helpers below) */
    const __m256d ksgn = _mm256_castsi256_pd(_mm256_set1_epi32(0x80000000));
    const double h_grid = 1.0 / (double)N;
    const double invh2 = 1.0 / (h_grid * h_grid);
    const double dcoef = alpha * invh2;      /* alpha * invh2 */
    const double Bp1 = B + 1.0;
    const int RM = GMRES_RESTART;
    const double inv2nn = 1.0 / (2.0 * (double)N * (double)N);

    /* The row passes are tiny: a few hundred cycles per row.  Spawning one
     * thread per core (dozens, on the target box) makes the barrier traffic
     * dominate the row work, so cap the team so each thread owns a useful
     * slab of rows. */
    {
        /* Zero the scratch planes and copy the inputs into the history before any pass reads them. */

                for (int64_t i = 0; i < N; i++) {
            memcpy(hu[0] + i * N, u + i * N, (size_t)N * 8);
            memcpy(hv[0] + i * N, v + i * N, (size_t)N * 8);
            for (int p = 2; p < N_PLANES; p++)
                if (p != MAXHIST)          /* plane hv[0] keeps the input v */
                    memset(hu[0] + (size_t)p * nn + i * N, 0.0, (size_t)N * 8);
        }

        double hist_t[MAXHIST];
        double nodes_p[MAXHIST], nodes_c[MAXHIST];
        double w_pred[MAXHIST], c_corr[MAXHIST];
        double hess[GMRES_RESTART + 1][GMRES_RESTART];
        double cs[GMRES_RESTART], sn[GMRES_RESTART];
        double g[GMRES_RESTART + 1];
        double y[GMRES_RESTART];

        int64_t nsteps = 0, njev = 0;
        int n0 = 0, n_hist = 1;
        double t = 0.0, h = H0;
        int order = 1, steps_since_order_change = 0, consecutive_reject = 0;
        bool have_jac = false;
        hist_t[0] = 0.0;

        while (t < t_end && nsteps < max_steps) {
            if (t + h > t_end)
                h = t_end - t;
            const int k = order < n_hist ? order : n_hist;
            const double t_new = t + h;

            /* ---- BDF weights from the accepted time history ---- */
            const int deg = k < (n_hist - 1) ? k : (n_hist - 1);
            for (int j = 0; j <= deg; j++)
                nodes_p[j] = hist_t[(n0 - j + MAXHIST) % MAXHIST] - t_new;
            lagrange_weights(nodes_p, deg + 1, 0, MAX_ORDER, w_pred);
            nodes_c[0] = 0.0;
            for (int j = 0; j < k; j++)
                nodes_c[j + 1] = hist_t[(n0 - j + MAXHIST) % MAXHIST] - t_new;
            lagrange_weights(nodes_c, k + 1, 1, MAX_ORDER, c_corr);
            const double c0 = c_corr[0];
            const double hbeta0 = 1.0 / c0;
            double rj[MAXHIST];
            for (int j = 1; j <= k; j++)
                rj[j] = c_corr[j] / c0;

            /* ---- pass: predictor + corrector rhs, and trial copy ---- */
                        for (int64_t i = 0; i < N; i++) {
                double *up = u_pred + i * N;
                double *vp = v_pred + i * N;
                double *ru = rhs_u + i * N;
                double *rv = rhs_v + i * N;
                double *ut = u_trial + i * N;
                double *vt = v_trial + i * N;
                const double *hrU[MAXHIST];
                const double *hrV[MAXHIST];
                for (int j = 0; j <= deg; j++) {
                    hrU[j] = hu[(n0 - j + MAXHIST) % MAXHIST] + i * N;
                    hrV[j] = hv[(n0 - j + MAXHIST) % MAXHIST] + i * N;
                }
                const double *hq[MAXHIST];
                const double *hvq[MAXHIST];
                for (int j = 1; j <= k; j++) {
                    const int q = (n0 - j + 1 + MAXHIST) % MAXHIST;
                    hq[j] = hu[q] + i * N;
                    hvq[j] = hv[q] + i * N;
                }
                /* p outer, j inner (vectorizable; identical accumulation
                 * order in p as the reference, seeded by the p=0 / p=1
                 * term instead of a zero) */
                #pragma omp simd
                for (int64_t j = 0; j < N; j++) {
                    up[j] = w_pred[0] * hrU[0][j];
                    vp[j] = w_pred[0] * hrV[0][j];
                    ut[j] = up[j];
                    vt[j] = vp[j];
                }
                for (int p = 1; p <= deg; p++) {
                    const double wp = w_pred[p];
                    const double *hpU = hrU[p];
                    const double *hpV = hrV[p];
                    #pragma omp simd
                    for (int64_t j = 0; j < N; j++) {
                        up[j] = up[j] + wp * hpU[j];
                        vp[j] = vp[j] + wp * hpV[j];
                        ut[j] = up[j];
                        vt[j] = vp[j];
                    }
                }
                {
                    const double *hpU = hq[1];
                    const double *hpV = hvq[1];
                    #pragma omp simd
                    for (int64_t j = 0; j < N; j++) {
                        ru[j] = -rj[1] * hpU[j];
                        rv[j] = -rj[1] * hpV[j];
                    }
                }
                for (int p = 2; p <= k; p++) {
                    const double wp = rj[p];
                    const double *hpU = hq[p];
                    const double *hpV = hvq[p];
                    #pragma omp simd
                    for (int64_t j = 0; j < N; j++) {
                        ru[j] = ru[j] - wp * hpU[j];
                        rv[j] = rv[j] - wp * hpV[j];
                    }
                }
            }

            /* ---- freeze the Jacobian on first use ---- */
            if (!have_jac) {
                                for (int64_t i = 0; i < N; i++) {
                    const double *ut = u_trial + i * N;
                    const double *vt = v_trial + i * N;
                    double *au = ja + i * N;
                    double *ab = jb + i * N;
                    double *ac = jc + i * N;
                    for (int64_t j = 0; j < N; j++) {
                        double u_ = ut[j], v_ = vt[j];
                        double uv2 = 2.0 * u_ * v_;
                        au[j] = uv2 - Bp1;
                        ab[j] = u_ * u_;
                        ac[j] = B - uv2;
                    }
                }
                njev += 1;
                have_jac = true;
            }

            /* ================= Newton loop ================= */
            bool converged = false;
            bool refactored_this_step = false;
            double prev_resnorm = -1.0;

            for (int it = 0; it < MAX_NEWTON; it++) {
                /* ---- pass A: f + residual; negated residual -> qup[0],
                 *      qvp[0]; row sums -> slots, combined in row order ---- */
                                for (int64_t i = 0; i < N; i++) {
                    const double *ut_im = u_trial + (i > 0 ? i - 1 : 1) * N;
                    const double *ut_ip = u_trial + (i == N - 1 ? N - 2 : i + 1) * N;
                    const double *vt_im = v_trial + (i > 0 ? i - 1 : 1) * N;
                    const double *vt_ip = v_trial + (i == N - 1 ? N - 2 : i + 1) * N;
                    const double *ru = rhs_u + i * N;
                    const double *rv = rhs_v + i * N;
                    const double *ut = u_trial + i * N;
                    const double *vt = v_trial + i * N;
                    double *qu0 = qup[0] + i * N;
                    double *qv0 = qvp[0] + i * N;
                    double lrow = 0.0, lrowb = 0.0;
                    #pragma omp simd reduction(+:lrow,lrowb)
                    for (int64_t j = 0; j < N; j++) {
                        double u_ = ut[j], v_ = vt[j];
                        int ju = (j == 0) ? 1 : (int)(j - 1);
                        int jd = (j == N - 1) ? (int)(N - 2) : (int)(j + 1);
                        double lu = ut_im[j] + ut_ip[j] + ut[ju] + ut[jd] - 4.0 * u_;
                        double lv = vt_im[j] + vt_ip[j] + vt[ju] + vt[jd] - 4.0 * v_;
                        double u2v = u_ * u_ * v_;
                        double fu = A + u2v - Bp1 * u_ + dcoef * lu;
                        double fv = B * u_ - u2v + dcoef * lv;
                        double ru_ = u_ - hbeta0 * fu - ru[j];
                        double rv_ = v_ - hbeta0 * fv - rv[j];
                        qu0[j] = -ru_;
                        qv0[j] = -rv_;
                        double wu_ = NEWTON_RTOL + NEWTON_RTOL * fabs(u_);
                        double wv_ = NEWTON_RTOL + NEWTON_RTOL * fabs(v_);
                        double du_ = ru_ / wu_;
                        double dv_ = rv_ / wv_;
                        lrow = lrow + du_ * du_ + dv_ * dv_;
                        lrowb = lrowb + ru_ * ru_ + rv_ * rv_;
                    }
                    part_a[i * 2] = lrow;
                    part_a[i * 2 + 1] = lrowb;
                }
                {
                    double lrs = 0.0, lbs = 0.0;
                    for (int64_t i = 0; i < N; i++) {
                        lrs = lrs + part_a[i * 2];
                        lbs = lbs + part_a[i * 2 + 1];
                    }
                    const double resnorm = sqrt(lrs * inv2nn);
                    const double beta = sqrt(lbs);
                    if (resnorm < 1.0) {
                        converged = true;
                        break;
                    }
                    if (prev_resnorm > 0.0) {
                        double rate = resnorm / prev_resnorm;
                        if (rate > CONV_DEGRADE && !refactored_this_step) {
                                                        for (int64_t i = 0; i < N; i++) {
                                const double *ut = u_trial + i * N;
                                const double *vt = v_trial + i * N;
                                double *au = ja + i * N;
                                double *ab = jb + i * N;
                                double *ac = jc + i * N;
                                for (int64_t j = 0; j < N; j++) {
                                    double u_ = ut[j], v_ = vt[j];
                                    double uv2 = 2.0 * u_ * v_;
                                    au[j] = uv2 - Bp1;
                                    ab[j] = u_ * u_;
                                    ac[j] = B - uv2;
                                }
                            }
                            njev += 1;
                            refactored_this_step = true;
                        }
                    }
                    prev_resnorm = resnorm;

                    /* ================= GMRES (matrix-free) ================= */
                    if (beta > 0.0) {
                        const double inv_beta = 1.0 / beta;
                                                for (int64_t i = 0; i < N; i++) {
                            double *a = qup[0] + i * N;
                            double *b = qvp[0] + i * N;
                            #pragma omp simd
                            for (int64_t j = 0; j < N; j++) {
                                a[j] = a[j] * inv_beta;
                                b[j] = b[j] * inv_beta;
                            }
                        }
                        g[0] = beta;
                        int m_used = RM;
                        for (int kk = 0; kk < RM; kk++) {
                            /* fused matvec + projection (one row pass):
                               w = (I - h beta0 J_frozen) q_kk, then
                               hess[p,kk] = <q_p, w>, p = 0..kk         */
                                                        for (int64_t i = 0; i < N; i++) {
                                const double *du_im = qup[kk] + (i > 0 ? i - 1 : 1) * N;
                                const double *du_ip = qup[kk] + (i == N - 1 ? N - 2 : i + 1) * N;
                                const double *dv_im = qvp[kk] + (i > 0 ? i - 1 : 1) * N;
                                const double *dv_ip = qvp[kk] + (i == N - 1 ? N - 2 : i + 1) * N;
                                const double *du = qup[kk] + i * N;
                                const double *dv = qvp[kk] + i * N;
                                const double *au = ja + i * N;
                                const double *ab = jb + i * N;
                                const double *ac = jc + i * N;
                                double *wu = w_u + i * N;
                                double *wv = w_v + i * N;
                                #pragma omp simd
                                for (int64_t j = 0; j < N; j++) {
                                    double du_ = du[j], dv_ = dv[j];
                                    int ju = (j == 0) ? 1 : (int)(j - 1);
                                    int jd = (j == N - 1) ? (int)(N - 2) : (int)(j + 1);
                                    double ldu = du_im[j] + du_ip[j] + du[ju] + du[jd] - 4.0 * du_;
                                    double ldv = dv_im[j] + dv_ip[j] + dv[ju] + dv[jd] - 4.0 * dv_;
                                    double X = au[j] * du_ + ab[j] * dv_ + dcoef * ldu;
                                    double Y = ac[j] * du_ - ab[j] * dv_ + dcoef * ldv;
                                    wu[j] = du_ - hbeta0 * X;
                                    wv[j] = dv_ - hbeta0 * Y;
                                }
                                const double *qrow[GMRES_RESTART + 1];
                                const double *qrowv[GMRES_RESTART + 1];
                                for (int p = 0; p <= kk; p++) {
                                    qrow[p] = qup[p] + i * N;
                                    qrowv[p] = qvp[p] + i * N;
                                }
                                /* four dots per row: the w-row is loaded once
                                   per block instead of per p */
                                const int64_t pfull = (int64_t)((kk + 1) & ~3);
                                for (int64_t p0 = 0; p0 < pfull; p0 += 4) {
                                    double l0 = 0.0, l1 = 0.0, l2 = 0.0, l3 = 0.0;
                                    #pragma omp simd reduction(+:l0,l1,l2,l3)
                                    for (int64_t j = 0; j < N; j++) {
                                        l0 = l0 + qrow[p0][j] * wu[j] + qrowv[p0][j] * wv[j];
                                        l1 = l1 + qrow[p0 + 1][j] * wu[j] + qrowv[p0 + 1][j] * wv[j];
                                        l2 = l2 + qrow[p0 + 2][j] * wu[j] + qrowv[p0 + 2][j] * wv[j];
                                        l3 = l3 + qrow[p0 + 3][j] * wu[j] + qrowv[p0 + 3][j] * wv[j];
                                    }
                                    part_p[i * (GMRES_RESTART + 1) + p0] = l0;
                                    part_p[i * (GMRES_RESTART + 1) + p0 + 1] = l1;
                                    part_p[i * (GMRES_RESTART + 1) + p0 + 2] = l2;
                                    part_p[i * (GMRES_RESTART + 1) + p0 + 3] = l3;
                                }
                                for (int p = pfull; p <= kk; p++) {
                                    double lrow = 0.0;
                                    #pragma omp simd reduction(+:lrow)
                                    for (int64_t j = 0; j < N; j++)
                                        lrow = lrow + qrow[p][j] * wu[j] + qrowv[p][j] * wv[j];
                                    part_p[i * (GMRES_RESTART + 1) + p] = lrow;
                                }
                            }
                            for (int p = 0; p <= kk; p++) {
                                double s = 0.0;
                                for (int64_t i = 0; i < N; i++)
                                    s = s + part_p[i * (GMRES_RESTART + 1) + p];
                                hess[p][kk] = s;
                            }
                            /* update: w -= sum_p hess[p,kk] q_p; h2 = ||w||^2
                               (each 4-element chunk stays in registers over
                               the whole ascending p sweep: no memory round
                               trip between q-rows, per-element op sequence
                               identical to the reference) */
                                                        for (int64_t i = 0; i < N; i++) {
                                double *wu = w_u + i * N;
                                double *wv = w_v + i * N;
                                const int64_t nm4 = (N >> 2) << 2;
                                __m256d lu = _mm256_setzero_pd();
                                __m256d lv = _mm256_setzero_pd();
                                for (int64_t j0 = 0; j0 < nm4; j0 += 4) {
                                    __m256d xu = _mm256_loadu_pd(wu + j0);
                                    __m256d xv = _mm256_loadu_pd(wv + j0);
                                    for (int p = 0; p <= kk; p++) {
                                        /* xu -= hp*q as fmadd(-hp, q, xu):
                                           this toolchain's _mm256_fnmsub_pd
                                           computes the wrong formula */
                                        const __m256d nhp =
                                            _mm256_xor_pd(_mm256_set1_pd(hess[p][kk]), ksgn);
                                        xu = _mm256_fmadd_pd(nhp, _mm256_loadu_pd(qup[p] + i * N + j0), xu);
                                        xv = _mm256_fmadd_pd(nhp, _mm256_loadu_pd(qvp[p] + i * N + j0), xv);
                                    }
                                    _mm256_storeu_pd(wu + j0, xu);
                                    _mm256_storeu_pd(wv + j0, xv);
                                    lu = _mm256_fmadd_pd(xu, xu, lu);
                                    lv = _mm256_fmadd_pd(xv, xv, lv);
                                }
                                double lrow = 0.0;
                                for (int j = (int)nm4; j < N; j++) {
                                    double tu = wu[j], tv = wv[j];
                                    for (int p = 0; p <= kk; p++) {
                                        tu = tu - hess[p][kk] * qup[p][i * N + j];
                                        tv = tv - hess[p][kk] * qvp[p][i * N + j];
                                    }
                                    wu[j] = tu;
                                    wv[j] = tv;
                                    lrow = lrow + tu * tu + tv * tv;
                                }
                                double pu[4], pv[4];
                                _mm256_storeu_pd(pu, lu);
                                _mm256_storeu_pd(pv, lv);
                                lrow = lrow + pu[0] + pu[1] + pu[2] + pu[3];
                                lrow = lrow + pv[0] + pv[1] + pv[2] + pv[3];
                                part_h[i] = lrow;
                            }
                            {
                                double lh2 = 0.0;
                                for (int64_t i = 0; i < N; i++)
                                    lh2 = lh2 + part_h[i];
                                const double h_next = sqrt(lh2);
                                hess[kk + 1][kk] = h_next;

                                for (int p = 0; p < kk; p++) {
                                    double temp = cs[p] * hess[p][kk] + sn[p] * hess[p + 1][kk];
                                    hess[p + 1][kk] = -sn[p] * hess[p][kk] + cs[p] * hess[p + 1][kk];
                                    hess[p][kk] = temp;
                                }
                                double denom = sqrt(hess[kk][kk] * hess[kk][kk] +
                                                    hess[kk + 1][kk] * hess[kk + 1][kk]);
                                cs[kk] = hess[kk][kk] / denom;
                                sn[kk] = hess[kk + 1][kk] / denom;
                                hess[kk][kk] = cs[kk] * hess[kk][kk] + sn[kk] * hess[kk + 1][kk];
                                hess[kk + 1][kk] = 0.0;
                                double temp = cs[kk] * g[kk];
                                g[kk + 1] = -sn[kk] * g[kk];
                                g[kk] = temp;

                                const double rel = fabs(g[kk + 1]) / beta;
                                if (rel < GMRES_TOL || h_next < 1.0e-13 || kk == RM - 1) {
                                    m_used = kk + 1;
                                    break;
                                }
                                const double inv_hn = 1.0 / h_next;
                                                                for (int64_t i = 0; i < N; i++) {
                                    double *wu = w_u + i * N;
                                    double *wv = w_v + i * N;
                                    double *nq = qup[kk + 1] + i * N;
                                    double *nq2 = qvp[kk + 1] + i * N;
                                    #pragma omp simd
                                    for (int64_t j = 0; j < N; j++) {
                                        nq[j] = wu[j] * inv_hn;
                                        nq2[j] = wv[j] * inv_hn;
                                    }
                                }
                            }
                        }
                        /* back substitution, then apply the correction to the
                         * trial state (accumulated from zero, like the
                         * reference's step buffer) */
                        for (int row = 0; row < m_used; row++) {
                            int rr = m_used - 1 - row;
                            double s = g[rr];
                            for (int col = rr + 1; col < m_used; col++)
                                s = s - hess[rr][col] * y[col];
                            y[rr] = s / hess[rr][rr];
                        }
                        /* accumulate the correction straight into the trial
                         * state: each 4-element chunk stays in registers
                         * over the whole ascending p sweep */
                                                for (int64_t i = 0; i < N; i++) {
                            double *ut = u_trial + i * N;
                            double *vt = v_trial + i * N;
                            const int64_t nm4 = (N >> 2) << 2;
                            for (int64_t j0 = 0; j0 < nm4; j0 += 4) {
                                __m256d xu = _mm256_loadu_pd(ut + j0);
                                __m256d xv = _mm256_loadu_pd(vt + j0);
                                for (int p = 0; p < m_used; p++) {
                                    const __m256d hp = _mm256_set1_pd(y[p]);
                                    xu = _mm256_fmadd_pd(hp, _mm256_loadu_pd(qup[p] + i * N + j0), xu);
                                    xv = _mm256_fmadd_pd(hp, _mm256_loadu_pd(qvp[p] + i * N + j0), xv);
                                }
                                _mm256_storeu_pd(ut + j0, xu);
                                _mm256_storeu_pd(vt + j0, xv);
                            }
                            for (int j = (int)nm4; j < N; j++) {
                                double su = ut[j], sv = vt[j];
                                for (int p = 0; p < m_used; p++) {
                                    su = su + y[p] * qup[p][i * N + j];
                                    sv = sv + y[p] * qvp[p][i * N + j];
                                }
                                ut[j] = su;
                                vt[j] = sv;
                            }
                        }
                    }
                }
            }

            if (!converged) {
                h = h * 0.25;
                if (order > 1)
                    order -= 1;
                continue;
            }

            /* ---- error estimate (BDF local error, separate tolerance) ---- */
            const double c_err = 1.0 / (double)(k + 1);
                        for (int64_t i = 0; i < N; i++) {
                const double *ut = u_trial + i * N;
                const double *vt = v_trial + i * N;
                const double *up = u_pred + i * N;
                const double *vp = v_pred + i * N;
                double lrow = 0.0;
                #pragma omp simd reduction(+:lrow)
                for (int64_t j = 0; j < N; j++) {
                    double eu = c_err * (ut[j] - up[j]);
                    double ev = c_err * (vt[j] - vp[j]);
                    double wu_ = ABSTOL + RTOL * fabs(ut[j]);
                    double wv_ = ABSTOL + RTOL * fabs(vt[j]);
                    double du_ = eu / wu_;
                    double dv_ = ev / wv_;
                    lrow = lrow + du_ * du_ + dv_ * dv_;
                }
                part_e[i] = lrow;
            }
            {
                double les = 0.0;
                for (int64_t i = 0; i < N; i++)
                    les = les + part_e[i];
                const double err_est = sqrt(les * inv2nn);

                if (err_est <= 1.0) {
                    consecutive_reject = 0;
                    t = t_new;
                    order_history[nsteps] = order;
                    nsteps += 1;

                    n0 = (n0 + 1) % MAXHIST;
                                        for (int64_t i = 0; i < N; i++) {
                        memcpy(hu[n0] + i * N, u_trial + i * N, (size_t)N * 8);
                        memcpy(hv[n0] + i * N, v_trial + i * N, (size_t)N * 8);
                    }
                    hist_t[n0] = t;
                    if (n_hist < MAXHIST)
                        n_hist += 1;

                    steps_since_order_change += 1;
                    if (order < MAX_ORDER && steps_since_order_change > order &&
                        n_hist > order + 1) {
                        order += 1;
                        steps_since_order_change = 0;
                    } else if (refactored_this_step && order > 1) {
                        order -= 1;
                        steps_since_order_change = 0;
                    }

                    double fac;
                    if (err_est > 0.0)
                        fac = SAFETY * pow(err_est, -1.0 / (double)(k + 1));
                    else
                        fac = MAX_FACTOR;
                    fac = MIN_FACTOR > fac ? MIN_FACTOR : fac;
                    fac = MAX_FACTOR < fac ? MAX_FACTOR : fac;
                    h = h * fac;
                } else {
                    consecutive_reject += 1;
                    double fac = SAFETY * pow(err_est, -1.0 / (double)(k + 1));
                    fac = MIN_FACTOR > fac ? MIN_FACTOR : fac;
                    fac = 1.0 < fac ? 1.0 : fac;
                    h = h * fac;
                    if (consecutive_reject >= 2 && order > 1) {
                        order -= 1;
                        steps_since_order_change = 0;
                    }
                }
            }
        }

        {
            memcpy(u, hu[n0], (size_t)nn * 8);
            memcpy(v, hv[n0], (size_t)nn * 8);
            diagnostics[0] = (double)nsteps;
            diagnostics[1] = (double)njev;
            diagnostics[2] = t;
        }
    }

    if (own_ws)
        free(ws);
}
