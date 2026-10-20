#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>

#define MAX_STEPS 200000
#define H0 1.0e-4
#define RTOL 1.0e-6
#define ATOL 1.0e-9

/* ---------------- scalar path (tail systems) ---------------- */

#define RHS(Y1, Y2, Y3, O1, O2, O3)                                   \
    do {                                                              \
        (O1) = -0.04 * (Y1) + 1.0e4 * (Y2) * (Y3);                    \
        (O2) = 0.04 * (Y1) - 1.0e4 * (Y2) * (Y3) - 3.0e7 * (Y2) * (Y2); \
        (O3) = 3.0e7 * (Y2) * (Y2);                                   \
    } while (0)

__attribute__((optimize("-ffp-contract=off")))
static void integrate_one(const double *restrict y0row, double *restrict yrow,
                          int64_t *restrict nacc, int64_t *restrict nrej,
                          double t_end)
{
    double y1 = y0row[0], y2 = y0row[1], y3 = y0row[2];
    double t = 0.0, h = H0;
    int64_t accepts = 0, rejects = 0, steps = 0;

    while (t < t_end && steps < MAX_STEPS) {
        double k1_1, k1_2, k1_3, k2_1, k2_2, k2_3, k3_1, k3_2, k3_3;
        double k4_1, k4_2, k4_3, k5_1, k5_2, k5_3, k6_1, k6_2, k6_3;
        double k7_1, k7_2, k7_3, s1, s2, s3, y4_1, y4_2, y4_3;
        double e1, e2, e3, enorm, ynorm, err, factor;

        if (t + h > t_end)
            h = t_end - t;

        RHS(y1, y2, y3, k1_1, k1_2, k1_3);

        RHS(y1 + h * (1.0 / 5.0) * k1_1,
            y2 + h * (1.0 / 5.0) * k1_2,
            y3 + h * (1.0 / 5.0) * k1_3,
            k2_1, k2_2, k2_3);

        RHS(y1 + h * (3.0 / 40.0 * k1_1 + 9.0 / 40.0 * k2_1),
            y2 + h * (3.0 / 40.0 * k1_2 + 9.0 / 40.0 * k2_2),
            y3 + h * (3.0 / 40.0 * k1_3 + 9.0 / 40.0 * k2_3),
            k3_1, k3_2, k3_3);

        RHS(y1 + h * (44.0 / 45.0 * k1_1 - 56.0 / 15.0 * k2_1 + 32.0 / 9.0 * k3_1),
            y2 + h * (44.0 / 45.0 * k1_2 - 56.0 / 15.0 * k2_2 + 32.0 / 9.0 * k3_2),
            y3 + h * (44.0 / 45.0 * k1_3 - 56.0 / 15.0 * k2_3 + 32.0 / 9.0 * k3_3),
            k4_1, k4_2, k4_3);

        RHS(y1 + h * (19372.0 / 6561.0 * k1_1 - 25360.0 / 2187.0 * k2_1
                          + 64448.0 / 6561.0 * k3_1 - 212.0 / 729.0 * k4_1),
            y2 + h * (19372.0 / 6561.0 * k1_2 - 25360.0 / 2187.0 * k2_2
                          + 64448.0 / 6561.0 * k3_2 - 212.0 / 729.0 * k4_2),
            y3 + h * (19372.0 / 6561.0 * k1_3 - 25360.0 / 2187.0 * k2_3
                          + 64448.0 / 6561.0 * k3_3 - 212.0 / 729.0 * k4_3),
            k5_1, k5_2, k5_3);

        RHS(y1 + h * (9017.0 / 3168.0 * k1_1 - 355.0 / 33.0 * k2_1
                          + 46732.0 / 5247.0 * k3_1 + 49.0 / 176.0 * k4_1
                          - 5103.0 / 18656.0 * k5_1),
            y2 + h * (9017.0 / 3168.0 * k1_2 - 355.0 / 33.0 * k2_2
                          + 46732.0 / 5247.0 * k3_2 + 49.0 / 176.0 * k4_2
                          - 5103.0 / 18656.0 * k5_2),
            y3 + h * (9017.0 / 3168.0 * k1_3 - 355.0 / 33.0 * k2_3
                          + 46732.0 / 5247.0 * k3_3 + 49.0 / 176.0 * k4_3
                          - 5103.0 / 18656.0 * k5_3),
            k6_1, k6_2, k6_3);

        s1 = y1 + h * (35.0 / 384.0 * k1_1 + 500.0 / 1113.0 * k3_1
                           + 125.0 / 192.0 * k4_1 - 2187.0 / 6784.0 * k5_1
                           + 11.0 / 84.0 * k6_1);
        s2 = y2 + h * (35.0 / 384.0 * k1_2 + 500.0 / 1113.0 * k3_2
                           + 125.0 / 192.0 * k4_2 - 2187.0 / 6784.0 * k5_2
                           + 11.0 / 84.0 * k6_2);
        s3 = y3 + h * (35.0 / 384.0 * k1_3 + 500.0 / 1113.0 * k3_3
                           + 125.0 / 192.0 * k4_3 - 2187.0 / 6784.0 * k5_3
                           + 11.0 / 84.0 * k6_3);

        RHS(s1, s2, s3, k7_1, k7_2, k7_3);

        y4_1 = y1 + h * (5179.0 / 57600.0 * k1_1 + 7571.0 / 16695.0 * k3_1
                            + 393.0 / 640.0 * k4_1 - 92097.0 / 339200.0 * k5_1
                            + 187.0 / 2100.0 * k6_1 + 1.0 / 40.0 * k7_1);
        y4_2 = y2 + h * (5179.0 / 57600.0 * k1_2 + 7571.0 / 16695.0 * k3_2
                            + 393.0 / 640.0 * k4_2 - 92097.0 / 339200.0 * k5_2
                            + 187.0 / 2100.0 * k6_2 + 1.0 / 40.0 * k7_2);
        y4_3 = y3 + h * (5179.0 / 57600.0 * k1_3 + 7571.0 / 16695.0 * k3_3
                            + 393.0 / 640.0 * k4_3 - 92097.0 / 339200.0 * k5_3
                            + 187.0 / 2100.0 * k6_3 + 1.0 / 40.0 * k7_3);

        e1 = s1 - y4_1;
        e2 = s2 - y4_2;
        e3 = s3 - y4_3;
        enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
        ynorm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);
        err = enorm / (ATOL + RTOL * ynorm);
        steps = steps + 1;
        if (err <= 1.0) {
            y1 = s1;
            y2 = s2;
            y3 = s3;
            t = t + h;
            accepts = accepts + 1;
        } else {
            rejects = rejects + 1;
        }
        if (err > 0.0) {
            factor = 0.9 * pow(err, -0.2);
            if (factor > 5.0)
                factor = 5.0;
            if (factor < 0.2)
                factor = 0.2;
        } else {
            factor = 5.0;
        }
        h = h * factor;
    }
    yrow[0] = y1;
    yrow[1] = y2;
    yrow[2] = y3;
    *nacc = accepts;
    *nrej = rejects;
}

/* ---------------- exact scalar pow helper (never vectorized) ---------------- */

__attribute__((noinline))
__attribute__((optimize("no-tree-vectorize", "no-tree-slp-vectorize")))
static void pow8(const double *restrict e, double *restrict f)
{
    for (int i = 0; i < 8; i++)
        f[i] = 0.9 * pow(e[i], -0.2);
}

/* ---------------- vector path: 8 systems in lockstep ---------------- */

#define V(a, b) _mm512_mul_pd(a, b)
#define A(a, b) _mm512_add_pd(a, b)
#define S(a, b) _mm512_sub_pd(a, b)

#define VRHS(Y1, Y2, Y3, O1, O2, O3, TMPA, TMPB)                    \
    do {                                                            \
        TMPA = V(c1e4, Y2);                                         \
        TMPA = V(TMPA, Y3);                                         \
        TMPB = V(c3e7, Y2);                                         \
        TMPB = V(TMPB, Y2);                                         \
        (O1) = A(V(cm004, Y1), TMPA);                               \
        (O2) = S(S(V(c004, Y1), TMPA), TMPB);                    \
        (O3) = TMPB;                                                \
    } while (0)

__attribute__((optimize("-ffp-contract=off")))
static void integrate_group(const double *restrict y0row, double *restrict yrow,
                            int64_t *restrict nacc, int64_t *restrict nrej,
                            double t_end)
{
    const __m512d t_end_v = _mm512_set1_pd(t_end);
    const __m512d c004 = _mm512_set1_pd(0.04);
    const __m512d cm004 = _mm512_set1_pd(-0.04);
    const __m512d c1e4 = _mm512_set1_pd(1.0e4);
    const __m512d c3e7 = _mm512_set1_pd(3.0e7);
    const __m512d c_atol = _mm512_set1_pd(ATOL);
    const __m512d c_rtol = _mm512_set1_pd(RTOL);
    const __m512d one = _mm512_set1_pd(1.0);
    const __m512d c02 = _mm512_set1_pd(0.2);
    const __m512d c5 = _mm512_set1_pd(5.0);

    __m512d y1, y2, y3, t, h;
    {
        double a1[8] __attribute__((aligned(64)));
        double a2[8] __attribute__((aligned(64)));
        double a3[8] __attribute__((aligned(64)));
        for (int i = 0; i < 8; i++) {
            a1[i] = y0row[3 * i];
            a2[i] = y0row[3 * i + 1];
            a3[i] = y0row[3 * i + 2];
        }
        y1 = _mm512_load_pd(a1);
        y2 = _mm512_load_pd(a2);
        y3 = _mm512_load_pd(a3);
    }
    t = _mm512_setzero_pd();
    h = _mm512_set1_pd(H0);
    __m512d acc = _mm512_setzero_pd();
    __m512d rej = _mm512_setzero_pd();
    int64_t steps = 0;

    while (steps < MAX_STEPS) {
        __mmask8 active = _mm512_cmp_pd_mask(t, t_end_v, _CMP_LT_OQ);
        if (active == 0)
            break;

        __m512d k1_1, k1_2, k1_3, k2_1, k2_2, k2_3, k3_1, k3_2, k3_3;
        __m512d k4_1, k4_2, k4_3, k5_1, k5_2, k5_3, k6_1, k6_2, k6_3;
        __m512d k7_1, k7_2, k7_3, s1, s2, s3, y4_1, y4_2, y4_3;
        __m512d ta, tb;

        /* clip step to t_end */
        __mmask8 clipm = _mm512_cmp_pd_mask(A(t, h), t_end_v, _CMP_GT_OQ);
        h = _mm512_mask_sub_pd(h, clipm, t_end_v, t);

        VRHS(y1, y2, y3, k1_1, k1_2, k1_3, ta, tb);

        VRHS(A(y1, V(V(h, _mm512_set1_pd(1.0 / 5.0)), k1_1)),
             A(y2, V(V(h, _mm512_set1_pd(1.0 / 5.0)), k1_2)),
             A(y3, V(V(h, _mm512_set1_pd(1.0 / 5.0)), k1_3)),
             k2_1, k2_2, k2_3, ta, tb);

        VRHS(A(y1, V(h, A(V(_mm512_set1_pd(3.0 / 40.0), k1_1),
                            V(_mm512_set1_pd(9.0 / 40.0), k2_1)))),
             A(y2, V(h, A(V(_mm512_set1_pd(3.0 / 40.0), k1_2),
                            V(_mm512_set1_pd(9.0 / 40.0), k2_2)))),
             A(y3, V(h, A(V(_mm512_set1_pd(3.0 / 40.0), k1_3),
                            V(_mm512_set1_pd(9.0 / 40.0), k2_3)))),
             k3_1, k3_2, k3_3, ta, tb);

        VRHS(A(y1, V(h, A(S(V(_mm512_set1_pd(44.0 / 45.0), k1_1),
                            V(_mm512_set1_pd(56.0 / 15.0), k2_1)),
                            V(_mm512_set1_pd(32.0 / 9.0), k3_1)))),
             A(y2, V(h, A(S(V(_mm512_set1_pd(44.0 / 45.0), k1_2),
                            V(_mm512_set1_pd(56.0 / 15.0), k2_2)),
                            V(_mm512_set1_pd(32.0 / 9.0), k3_2)))),
             A(y3, V(h, A(S(V(_mm512_set1_pd(44.0 / 45.0), k1_3),
                            V(_mm512_set1_pd(56.0 / 15.0), k2_3)),
                            V(_mm512_set1_pd(32.0 / 9.0), k3_3)))),
             k4_1, k4_2, k4_3, ta, tb);

        VRHS(A(y1, V(h, S(A(S(V(_mm512_set1_pd(19372.0 / 6561.0), k1_1),
                            V(_mm512_set1_pd(25360.0 / 2187.0), k2_1)),
                            V(_mm512_set1_pd(64448.0 / 6561.0), k3_1)),
                            V(_mm512_set1_pd(212.0 / 729.0), k4_1)))),
             A(y2, V(h, S(A(S(V(_mm512_set1_pd(19372.0 / 6561.0), k1_2),
                            V(_mm512_set1_pd(25360.0 / 2187.0), k2_2)),
                            V(_mm512_set1_pd(64448.0 / 6561.0), k3_2)),
                            V(_mm512_set1_pd(212.0 / 729.0), k4_2)))),
             A(y3, V(h, S(A(S(V(_mm512_set1_pd(19372.0 / 6561.0), k1_3),
                            V(_mm512_set1_pd(25360.0 / 2187.0), k2_3)),
                            V(_mm512_set1_pd(64448.0 / 6561.0), k3_3)),
                            V(_mm512_set1_pd(212.0 / 729.0), k4_3)))),
             k5_1, k5_2, k5_3, ta, tb);

        VRHS(A(y1, V(h, S(A(A(S(V(_mm512_set1_pd(9017.0 / 3168.0), k1_1),
                            V(_mm512_set1_pd(355.0 / 33.0), k2_1)),
                            V(_mm512_set1_pd(46732.0 / 5247.0), k3_1)),
                            V(_mm512_set1_pd(49.0 / 176.0), k4_1)),
                            V(_mm512_set1_pd(5103.0 / 18656.0), k5_1)))),
             A(y2, V(h, S(A(A(S(V(_mm512_set1_pd(9017.0 / 3168.0), k1_2),
                            V(_mm512_set1_pd(355.0 / 33.0), k2_2)),
                            V(_mm512_set1_pd(46732.0 / 5247.0), k3_2)),
                            V(_mm512_set1_pd(49.0 / 176.0), k4_2)),
                            V(_mm512_set1_pd(5103.0 / 18656.0), k5_2)))),
             A(y3, V(h, S(A(A(S(V(_mm512_set1_pd(9017.0 / 3168.0), k1_3),
                            V(_mm512_set1_pd(355.0 / 33.0), k2_3)),
                            V(_mm512_set1_pd(46732.0 / 5247.0), k3_3)),
                            V(_mm512_set1_pd(49.0 / 176.0), k4_3)),
                            V(_mm512_set1_pd(5103.0 / 18656.0), k5_3)))),
             k6_1, k6_2, k6_3, ta, tb);

        s1 = A(y1, V(h, A(S(A(A(V(_mm512_set1_pd(35.0 / 384.0), k1_1),
                        V(_mm512_set1_pd(500.0 / 1113.0), k3_1)),
                        V(_mm512_set1_pd(125.0 / 192.0), k4_1)),
                        V(_mm512_set1_pd(2187.0 / 6784.0), k5_1)),
                        V(_mm512_set1_pd(11.0 / 84.0), k6_1))));
        s2 = A(y2, V(h, A(S(A(A(V(_mm512_set1_pd(35.0 / 384.0), k1_2),
                        V(_mm512_set1_pd(500.0 / 1113.0), k3_2)),
                        V(_mm512_set1_pd(125.0 / 192.0), k4_2)),
                        V(_mm512_set1_pd(2187.0 / 6784.0), k5_2)),
                        V(_mm512_set1_pd(11.0 / 84.0), k6_2))));
        s3 = A(y3, V(h, A(S(A(A(V(_mm512_set1_pd(35.0 / 384.0), k1_3),
                        V(_mm512_set1_pd(500.0 / 1113.0), k3_3)),
                        V(_mm512_set1_pd(125.0 / 192.0), k4_3)),
                        V(_mm512_set1_pd(2187.0 / 6784.0), k5_3)),
                        V(_mm512_set1_pd(11.0 / 84.0), k6_3))));

        VRHS(s1, s2, s3, k7_1, k7_2, k7_3, ta, tb);

        y4_1 = A(y1, V(h, A(A(S(A(A(V(_mm512_set1_pd(5179.0 / 57600.0), k1_1),
                        V(_mm512_set1_pd(7571.0 / 16695.0), k3_1)),
                        V(_mm512_set1_pd(393.0 / 640.0), k4_1)),
                        V(_mm512_set1_pd(92097.0 / 339200.0), k5_1)),
                        V(_mm512_set1_pd(187.0 / 2100.0), k6_1)),
                        V(_mm512_set1_pd(1.0 / 40.0), k7_1))));
        y4_2 = A(y2, V(h, A(A(S(A(A(V(_mm512_set1_pd(5179.0 / 57600.0), k1_2),
                        V(_mm512_set1_pd(7571.0 / 16695.0), k3_2)),
                        V(_mm512_set1_pd(393.0 / 640.0), k4_2)),
                        V(_mm512_set1_pd(92097.0 / 339200.0), k5_2)),
                        V(_mm512_set1_pd(187.0 / 2100.0), k6_2)),
                        V(_mm512_set1_pd(1.0 / 40.0), k7_2))));
        y4_3 = A(y3, V(h, A(A(S(A(A(V(_mm512_set1_pd(5179.0 / 57600.0), k1_3),
                        V(_mm512_set1_pd(7571.0 / 16695.0), k3_3)),
                        V(_mm512_set1_pd(393.0 / 640.0), k4_3)),
                        V(_mm512_set1_pd(92097.0 / 339200.0), k5_3)),
                        V(_mm512_set1_pd(187.0 / 2100.0), k6_3)),
                        V(_mm512_set1_pd(1.0 / 40.0), k7_3))));

        __m512d e1 = S(s1, y4_1);
        __m512d e2 = S(s2, y4_2);
        __m512d e3 = S(s3, y4_3);
        __m512d enorm = _mm512_sqrt_pd(A(A(V(e1, e1), V(e2, e2)), V(e3, e3)));
        __m512d ynorm = _mm512_sqrt_pd(A(A(V(s1, s1), V(s2, s2)), V(s3, s3)));
        __m512d err = _mm512_div_pd(enorm, A(c_atol, V(c_rtol, ynorm)));
        steps = steps + 1;

        __mmask8 accm = _mm512_cmp_pd_mask(err, one, _CMP_LE_OQ) & active;
        y1 = _mm512_mask_mov_pd(y1, accm, s1);
        y2 = _mm512_mask_mov_pd(y2, accm, s2);
        y3 = _mm512_mask_mov_pd(y3, accm, s3);
        t = _mm512_mask_add_pd(t, accm, t, h);
        acc = _mm512_mask_add_pd(acc, accm, acc, one);
        __mmask8 rejm = active & ~accm;
        rej = _mm512_mask_add_pd(rej, rejm, rej, one);

        double ebuf[8] __attribute__((aligned(64)));
        double fbuf[8] __attribute__((aligned(64)));
        _mm512_store_pd(ebuf, err);
        pow8(ebuf, fbuf);
        __m512d fv = _mm512_load_pd(fbuf);
        fv = _mm512_max_pd(fv, c02);
        fv = _mm512_min_pd(fv, c5);
        h = _mm512_mask_mul_pd(h, active, h, fv);
    }

    double o1[8] __attribute__((aligned(64)));
    double o2[8] __attribute__((aligned(64)));
    double o3[8] __attribute__((aligned(64)));
    double oa[8] __attribute__((aligned(64)));
    double or_[8] __attribute__((aligned(64)));
    _mm512_store_pd(o1, y1);
    _mm512_store_pd(o2, y2);
    _mm512_store_pd(o3, y3);
    _mm512_store_pd(oa, acc);
    _mm512_store_pd(or_, rej);
    for (int i = 0; i < 8; i++) {
        yrow[3 * i] = o1[i];
        yrow[3 * i + 1] = o2[i];
        yrow[3 * i + 2] = o3[i];
        nacc[i] = (int64_t)oa[i];
        nrej[i] = (int64_t)or_[i];
    }
}

void rk45_ensemble_fp64(
    int64_t *restrict n_accept,
    int64_t *restrict n_reject,
    double *restrict y,
    const double *restrict y0,
    const int64_t NSYS,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace;
    (void)workspace_size;
    const int64_t ngroups = NSYS / 8;
    int64_t g;
    #pragma omp parallel
    {
        #pragma omp for schedule(static)
        for (g = 0; g < ngroups; g++) {
            integrate_group(y0 + 24 * g, y + 24 * g, n_accept + 8 * g,
                            n_reject + 8 * g, t_end);
        }
        #pragma omp single
        {
            for (g = 8 * ngroups; g < NSYS; g++) {
                integrate_one(y0 + 3 * g, y + 3 * g, &n_accept[g],
                              &n_reject[g], t_end);
            }
        }
    }
}
