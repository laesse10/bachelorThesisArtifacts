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

/* =========================================================================
 * Variable-order variable-step BDF integrator over a Newton-Krylov (GMRES)
 * corrector, 2-D Brusselator reaction-diffusion (Neumann boundaries).
 *
 * Faithful C port of the NumPy reference, restructured for speed:
 *   - ring-buffer state history (no per-step plane shifts)
 *   - fused single-pass row kernels, parallelized over rows (OpenMP)
 *   - frozen-Jacobian coefficients (ja, jb, jc) precomputed on refactor
 *   - GMRES correction accumulated straight into the trial state
 *
 * Every thread runs the same deterministic control flow.  All grids are
 * split over rows with a static schedule; each row is owned by exactly
 * one thread, which stores its row-sum into a private slot (plain store,
 * no atomic).  After the barrier every thread combines the slots itself
 * in fixed row order, so every scalar the integrator branches on has the
 * same value on every thread, and the order of summation never depends
 * on the team size.
 *
 * NOTE: no `omp for reduction` clauses are used at all -- on the target
 * toolchain (GCC 16 / libgomp) array reductions silently drop thread
 * contributions at team sizes of 8 and above, which corrupts the GMRES
 * projections.  The slot scheme above is the portable replacement.
 * ========================================================================= */

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

/* One pass-A row: stencil rows um/uc/ul give lu[j] = um[j]+ul[j]+uc[j-1]+
 * uc[j+1]-4uc[j] with mirrored edges (j=0 uses uc[1], j=N-1 uses uc[N-2]).
 * Row-pair tiling outside means the halo rows are loaded once per pair. */
static inline void passA_row(
    const double *um, const double *uc, const double *ul,
    const double *vm, const double *vc, const double *vl,
    const double *ru, const double *rv,
    double *qu0, double *qv0,
    const double A, const double B, const double Bp1,
    const double dcoef, const double hbeta0, const int64_t N,
    double *lrow, double *lrowb)
{
    double lrs = *lrow, lbs = *lrowb;
    double u_ = uc[0], v_ = vc[0];
    double lu = um[0] + ul[0] + uc[1] + uc[1] - 4.0 * u_;
    double lv = vm[0] + vl[0] + vc[1] + vc[1] - 4.0 * v_;
    double u2v = u_ * u_ * v_;
    double fu = A + u2v - Bp1 * u_ + dcoef * lu;
    double fv = B * u_ - u2v + dcoef * lv;
    double ru_ = u_ - hbeta0 * fu - ru[0];
    double rv_ = v_ - hbeta0 * fv - rv[0];
    qu0[0] = -ru_;
    qv0[0] = -rv_;
    double wu_ = NEWTON_RTOL + NEWTON_RTOL * fabs(u_);
    double wv_ = NEWTON_RTOL + NEWTON_RTOL * fabs(v_);
    double du_ = ru_ / wu_;
    double dv_ = rv_ / wv_;
    lrs = lrs + du_ * du_ + dv_ * dv_;
    lbs = lbs + ru_ * ru_ + rv_ * rv_;

    #pragma omp simd reduction(+:lrs,lbs)
    for (int64_t j = 1; j < N - 1; j++) {
        u_ = uc[j], v_ = vc[j];
        lu = um[j] + ul[j] + uc[j - 1] + uc[j + 1] - 4.0 * u_;
        lv = vm[j] + vl[j] + vc[j - 1] + vc[j + 1] - 4.0 * v_;
        u2v = u_ * u_ * v_;
        fu = A + u2v - Bp1 * u_ + dcoef * lu;
        fv = B * u_ - u2v + dcoef * lv;
        ru_ = u_ - hbeta0 * fu - ru[j];
        rv_ = v_ - hbeta0 * fv - rv[j];
        qu0[j] = -ru_;
        qv0[j] = -rv_;
        wu_ = NEWTON_RTOL + NEWTON_RTOL * fabs(u_);
        wv_ = NEWTON_RTOL + NEWTON_RTOL * fabs(v_);
        du_ = ru_ / wu_;
        dv_ = rv_ / wv_;
        lrs = lrs + du_ * du_ + dv_ * dv_;
        lbs = lbs + ru_ * ru_ + rv_ * rv_;
    }

    u_ = uc[N - 1], v_ = vc[N - 1];
    lu = um[N - 1] + ul[N - 1] + uc[N - 2] + uc[N - 2] - 4.0 * u_;
    lv = vm[N - 1] + vl[N - 1] + vc[N - 2] + vc[N - 2] - 4.0 * v_;
    u2v = u_ * u_ * v_;
    fu = A + u2v - Bp1 * u_ + dcoef * lu;
    fv = B * u_ - u2v + dcoef * lv;
    ru_ = u_ - hbeta0 * fu - ru[N - 1];
    rv_ = v_ - hbeta0 * fv - rv[N - 1];
    qu0[N - 1] = -ru_;
    qv0[N - 1] = -rv_;
    wu_ = NEWTON_RTOL + NEWTON_RTOL * fabs(u_);
    wv_ = NEWTON_RTOL + NEWTON_RTOL * fabs(v_);
    du_ = ru_ / wu_;
    dv_ = rv_ / wv_;
    lrs = lrs + du_ * du_ + dv_ * dv_;
    lbs = lbs + ru_ * ru_ + rv_ * rv_;

    *lrow = lrs;
    *lrowb = lbs;
}

/* One fused matvec(+projection) row: w = (I - h beta0 J) q with the same
 * mirrored-stencil edge handling; the projections follow on the w row. */
static inline void matvec_row(
    const double *dum, const double *duc, const double *dul,
    const double *dvm, const double *dvc, const double *dvl,
    const double *au, const double *ab, const double *ac,
    double *wu, double *wv,
    const double dcoef, const double hbeta0, const int64_t N)
{
    double du_ = duc[0], dv_ = dvc[0];
    double ldu = dum[0] + dul[0] + duc[1] + duc[1] - 4.0 * du_;
    double ldv = dvm[0] + dvl[0] + dvc[1] + dvc[1] - 4.0 * dv_;
    double X = au[0] * du_ + ab[0] * dv_ + dcoef * ldu;
    double Y = ac[0] * du_ - ab[0] * dv_ + dcoef * ldv;
    wu[0] = du_ - hbeta0 * X;
    wv[0] = dv_ - hbeta0 * Y;

    #pragma omp simd
    for (int64_t j = 1; j < N - 1; j++) {
        du_ = duc[j], dv_ = dvc[j];
        ldu = dum[j] + dul[j] + duc[j - 1] + duc[j + 1] - 4.0 * du_;
        ldv = dvm[j] + dvl[j] + dvc[j - 1] + dvc[j + 1] - 4.0 * dv_;
        X = au[j] * du_ + ab[j] * dv_ + dcoef * ldu;
        Y = ac[j] * du_ - ab[j] * dv_ + dcoef * ldv;
        wu[j] = du_ - hbeta0 * X;
        wv[j] = dv_ - hbeta0 * Y;
    }

    du_ = duc[N - 1], dv_ = dvc[N - 1];
    ldu = dum[N - 1] + dul[N - 1] + duc[N - 2] + duc[N - 2] - 4.0 * du_;
    ldv = dvm[N - 1] + dvl[N - 1] + dvc[N - 2] + dvc[N - 2] - 4.0 * dv_;
    X = au[N - 1] * du_ + ab[N - 1] * dv_ + dcoef * ldu;
    Y = ac[N - 1] * du_ - ab[N - 1] * dv_ + dcoef * ldv;
    wu[N - 1] = du_ - hbeta0 * X;
    wv[N - 1] = dv_ - hbeta0 * Y;
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
    int n_avail = omp_get_max_threads();
    int n_thr = 1;
    if (n_thr > n_avail)
        n_thr = n_avail;

    #pragma omp parallel num_threads(n_thr)
    {
        /* First-touch the workspace from inside the team: pages are born on
         * the node of the thread that writes them first.  Seeding the whole
         * grid (and the initial copy of the inputs) here with one write
         * each makes every thread's row slab home on its own socket, which
         * matters on the multi-socket target box.  */
        #pragma omp for schedule(static)
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
            #pragma omp for schedule(static)
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
                #pragma omp for schedule(static)
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
                #pragma omp for schedule(static)
                for (int64_t i2 = 0; i2 < N; i2 += 2) {
                    const int64_t i1 = i2 + 1;
                    const int has2 = (i1 < N) ? 1 : 0;
                    const double *UA = u_trial + ((i2 > 0) ? i2 - 1 : 1) * N;
                    const double *UB = u_trial + i2 * N;
                    const double *UC = u_trial + i1 * N;
                    const double *UD = u_trial + ((i1 + 1 < N) ? i1 + 1 : N - 2) * N;
                    const double *VA = v_trial + ((i2 > 0) ? i2 - 1 : 1) * N;
                    const double *VB = v_trial + i2 * N;
                    const double *VC = v_trial + i1 * N;
                    const double *VD = v_trial + ((i1 + 1 < N) ? i1 + 1 : N - 2) * N;

                    double lrow = 0.0, lrowb = 0.0;
                    passA_row(UA, UB, has2 ? UC : UA, VA, VB, has2 ? VC : VA,
                              rhs_u + i2 * N, rhs_v + i2 * N,
                              qup[0] + i2 * N, qvp[0] + i2 * N,
                              A, B, Bp1, dcoef, hbeta0, N, &lrow, &lrowb);
                    part_a[i2 * 2] = lrow;
                    part_a[i2 * 2 + 1] = lrowb;
                    if (has2) {
                        lrow = 0.0;
                        lrowb = 0.0;
                        passA_row(UB, UC, UD, VB, VC, VD,
                                  rhs_u + i1 * N, rhs_v + i1 * N,
                                  qup[0] + i1 * N, qvp[0] + i1 * N,
                                  A, B, Bp1, dcoef, hbeta0, N, &lrow, &lrowb);
                        part_a[i1 * 2] = lrow;
                        part_a[i1 * 2 + 1] = lrowb;
                    }
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
                            #pragma omp for schedule(static)
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
                        #pragma omp for schedule(static)
                        for (int64_t i = 0; i < N; i++) {
                            double *a = qup[0] + i * N;
                            double *b = qvp[0] + i * N;
                            for (int64_t j = 0; j < N; j++) {
                                a[j] = a[j] / beta;
                                b[j] = b[j] / beta;
                            }
                        }
                        g[0] = beta;
                        int m_used = RM;
                        for (int kk = 0; kk < RM; kk++) {
                            /* fused matvec + projection (one row pass):
                               w = (I - h beta0 J_frozen) q_kk, then
                               hess[p,kk] = <q_p, w>, p = 0..kk         */
                            #pragma omp for schedule(static)
                            for (int64_t i2 = 0; i2 < N; i2 += 2) {
                                const int64_t i1 = i2 + 1;
                                const int has2 = (i1 < N) ? 1 : 0;
                                const double *QA = qup[kk] + ((i2 > 0) ? i2 - 1 : 1) * N;
                                const double *QB = qup[kk] + i2 * N;
                                const double *QC = qup[kk] + i1 * N;
                                const double *QD = qup[kk] + ((i1 + 1 < N) ? i1 + 1 : N - 2) * N;
                                const double *RA = qvp[kk] + ((i2 > 0) ? i2 - 1 : 1) * N;
                                const double *RB = qvp[kk] + i2 * N;
                                const double *RC = qvp[kk] + i1 * N;
                                const double *RD = qvp[kk] + ((i1 + 1 < N) ? i1 + 1 : N - 2) * N;

                                matvec_row(QA, QB, has2 ? QC : QA, RA, RB, has2 ? RC : RA,
                                           ja + i2 * N, jb + i2 * N, jc + i2 * N,
                                           w_u + i2 * N, w_v + i2 * N,
                                           dcoef, hbeta0, N);
                                const double *qrow[GMRES_RESTART + 1];
                                const double *qrowv[GMRES_RESTART + 1];
                                for (int p = 0; p <= kk; p++) {
                                    qrow[p] = qup[p] + i2 * N;
                                    qrowv[p] = qvp[p] + i2 * N;
                                }
                                for (int p = 0; p <= kk; p++) {
                                    double lrow = 0.0;
                                    #pragma omp simd reduction(+:lrow)
                                    for (int64_t j = 0; j < N; j++)
                                        lrow = lrow + qrow[p][j] * (w_u + i2 * N)[j] + qrowv[p][j] * (w_v + i2 * N)[j];
                                    part_p[i2 * (GMRES_RESTART + 1) + p] = lrow;
                                }
                                if (has2) {
                                    matvec_row(QB, QC, QD, RB, RC, RD,
                                               ja + i1 * N, jb + i1 * N, jc + i1 * N,
                                               w_u + i1 * N, w_v + i1 * N,
                                               dcoef, hbeta0, N);
                                    for (int p = 0; p <= kk; p++) {
                                        qrow[p] = qup[p] + i1 * N;
                                        qrowv[p] = qvp[p] + i1 * N;
                                    }
                                    for (int p = 0; p <= kk; p++) {
                                        double lrow = 0.0;
                                        #pragma omp simd reduction(+:lrow)
                                        for (int64_t j = 0; j < N; j++)
                                            lrow = lrow + qrow[p][j] * (w_u + i1 * N)[j] + qrowv[p][j] * (w_v + i1 * N)[j];
                                        part_p[i1 * (GMRES_RESTART + 1) + p] = lrow;
                                    }
                                }
                            }
                            for (int p = 0; p <= kk; p++) {
                                double s = 0.0;
                                for (int64_t i = 0; i < N; i++)
                                    s = s + part_p[i * (GMRES_RESTART + 1) + p];
                                hess[p][kk] = s;
                            }
                            /* update: w -= sum_p hess[p,kk] q_p; h2 = ||w||^2 */
                            #pragma omp for schedule(static)
                            for (int64_t i = 0; i < N; i++) {
                                double *wu = w_u + i * N;
                                double *wv = w_v + i * N;
                                const double *qrow[GMRES_RESTART + 1];
                                const double *qrowv[GMRES_RESTART + 1];
                                for (int p = 0; p <= kk; p++) {
                                    qrow[p] = qup[p] + i * N;
                                    qrowv[p] = qvp[p] + i * N;
                                }
                                double lrow = 0.0;
                                #pragma omp simd reduction(+:lrow)
                                for (int64_t j = 0; j < N; j++) {
                                    double tu = wu[j], tv = wv[j];
                                    for (int p = 0; p <= kk; p++) {
                                        tu = tu - hess[p][kk] * qrow[p][j];
                                        tv = tv - hess[p][kk] * qrowv[p][j];
                                    }
                                    wu[j] = tu;
                                    wv[j] = tv;
                                    lrow = lrow + tu * tu + tv * tv;
                                }
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
                                #pragma omp for schedule(static)
                                for (int64_t i = 0; i < N; i++) {
                                    double *wu = w_u + i * N;
                                    double *wv = w_v + i * N;
                                    double *nq = qup[kk + 1] + i * N;
                                    double *nq2 = qvp[kk + 1] + i * N;
                                    for (int64_t j = 0; j < N; j++) {
                                        nq[j] = wu[j] / h_next;
                                        nq2[j] = wv[j] / h_next;
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
                        #pragma omp for schedule(static)
                        for (int64_t i = 0; i < N; i++) {
                            double *ut = u_trial + i * N;
                            double *vt = v_trial + i * N;
                            const double *qrow[GMRES_RESTART + 1];
                            const double *qrowv[GMRES_RESTART + 1];
                            for (int p = 0; p < m_used; p++) {
                                qrow[p] = qup[p] + i * N;
                                qrowv[p] = qvp[p] + i * N;
                            }
                            for (int64_t j = 0; j < N; j++) {
                                double su = 0.0, sv = 0.0;
                                for (int p = 0; p < m_used; p++) {
                                    su = su + y[p] * qrow[p][j];
                                    sv = sv + y[p] * qrowv[p][j];
                                }
                                ut[j] = ut[j] + su;
                                vt[j] = vt[j] + sv;
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
            #pragma omp for schedule(static)
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
                    #pragma omp for schedule(static)
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

        #pragma omp single
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
