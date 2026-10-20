/*
 * Accurate implementation of the adaptive Dormand–Prince (RK45) integrator for
 * an ensemble of independent stiff ODE systems (Robertson problem). The code
 * follows the reference `rk45_ensemble_numpy.py` exactly, including all
 * coefficients and the adaptive step-size controller. The algorithm is
 * embarrassingly parallel; we parallelise over the outer loop using OpenMP.
 *
 * The reference tolerances are hard‑coded as RTOL = 1e‑4 and ATOL = 1e‑6 – these
 * are the values used by the benchmark driver (they are not part of the C
 * signature). Changing them would modify the trajectory and is therefore not
 * allowed.
 */

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>

static const double H0 = 1.0e-4;          /* initial step size guess */
static const int64_t MAX_STEPS = 200000; /* safety bound on attempts */
static const double RTOL = 1.0e-6;       /* relative tolerance */
static const double ATOL = 1.0e-9;       /* absolute tolerance */

/* Evaluate the Robertson RHS. Output is written to the three pointers. */
static inline void robertson_rhs(double y1, double y2, double y3,
                                 double *k0, double *k1, double *k2) {
    double y2y3 = y2 * y3;
    double y2sq = y2 * y2;
    *k0 = -0.04 * y1 + 1.0e4 * y2y3;
    *k1 = 0.04 * y1 - 1.0e4 * y2y3 - 3.0e7 * y2sq;
    *k2 = 3.0e7 * y2sq;
}

void rk45_ensemble_fp64(int64_t *restrict n_accept,
                        int64_t *restrict n_reject,
                        double *restrict y,
                        const double *restrict y0,
                        const int64_t NSYS,
                        const double t_end,
                        uint8_t *restrict workspace,
                        const int64_t workspace_size) {
    (void)workspace;      // unused but part of the required signature
    (void)workspace_size; // unused

    #pragma omp parallel for schedule(dynamic) default(none) \
        shared(y, y0, n_accept, n_reject, NSYS, t_end)
    for (int64_t n = 0; n < NSYS; ++n) {
        const double *row0 = y0 + n * 3;
        double y1 = row0[0];
        double y2 = row0[1];
        double y3 = row0[2];
        double t = 0.0;
        double h = H0;
        int64_t accepts = 0;
        int64_t rejects = 0;
        int64_t steps = 0;

        while (t < t_end && steps < MAX_STEPS) {
            if (t + h > t_end) {
                h = t_end - t;
            }

            /* Stage 1 */
            double k1_0, k1_1, k1_2;
            robertson_rhs(y1, y2, y3, &k1_0, &k1_1, &k1_2);

            /* Stage 2 */
            double y1_s2 = y1 + h * (1.0/5.0) * k1_0;
            double y2_s2 = y2 + h * (1.0/5.0) * k1_1;
            double y3_s2 = y3 + h * (1.0/5.0) * k1_2;
            double k2_0, k2_1, k2_2;
            robertson_rhs(y1_s2, y2_s2, y3_s2, &k2_0, &k2_1, &k2_2);

            /* Stage 3 */
            double y1_s3 = y1 + h * (3.0/40.0 * k1_0 + 9.0/40.0 * k2_0);
            double y2_s3 = y2 + h * (3.0/40.0 * k1_1 + 9.0/40.0 * k2_1);
            double y3_s3 = y3 + h * (3.0/40.0 * k1_2 + 9.0/40.0 * k2_2);
            double k3_0, k3_1, k3_2;
            robertson_rhs(y1_s3, y2_s3, y3_s3, &k3_0, &k3_1, &k3_2);

            /* Stage 4 */
            double y1_s4 = y1 + h * (44.0/45.0 * k1_0 - 56.0/15.0 * k2_0 + 32.0/9.0 * k3_0);
            double y2_s4 = y2 + h * (44.0/45.0 * k1_1 - 56.0/15.0 * k2_1 + 32.0/9.0 * k3_1);
            double y3_s4 = y3 + h * (44.0/45.0 * k1_2 - 56.0/15.0 * k2_2 + 32.0/9.0 * k3_2);
            double k4_0, k4_1, k4_2;
            robertson_rhs(y1_s4, y2_s4, y3_s4, &k4_0, &k4_1, &k4_2);

            /* Stage 5 */
            double y1_s5 = y1 + h * (19372.0/6561.0 * k1_0 - 25360.0/2187.0 * k2_0 +
                                      64448.0/6561.0 * k3_0 - 212.0/729.0 * k4_0);
            double y2_s5 = y2 + h * (19372.0/6561.0 * k1_1 - 25360.0/2187.0 * k2_1 +
                                      64448.0/6561.0 * k3_1 - 212.0/729.0 * k4_1);
            double y3_s5 = y3 + h * (19372.0/6561.0 * k1_2 - 25360.0/2187.0 * k2_2 +
                                      64448.0/6561.0 * k3_2 - 212.0/729.0 * k4_2);
            double k5_0, k5_1, k5_2;
            robertson_rhs(y1_s5, y2_s5, y3_s5, &k5_0, &k5_1, &k5_2);

            /* Stage 6 */
            double y1_s6 = y1 + h * (9017.0/3168.0 * k1_0 - 355.0/33.0 * k2_0 +
                                      46732.0/5247.0 * k3_0 + 49.0/176.0 * k4_0 -
                                      5103.0/18656.0 * k5_0);
            double y2_s6 = y2 + h * (9017.0/3168.0 * k1_1 - 355.0/33.0 * k2_1 +
                                      46732.0/5247.0 * k3_1 + 49.0/176.0 * k4_1 -
                                      5103.0/18656.0 * k5_1);
            double y3_s6 = y3 + h * (9017.0/3168.0 * k1_2 - 355.0/33.0 * k2_2 +
                                      46732.0/5247.0 * k3_2 + 49.0/176.0 * k4_2 -
                                      5103.0/18656.0 * k5_2);
            double k6_0, k6_1, k6_2;
            robertson_rhs(y1_s6, y2_s6, y3_s6, &k6_0, &k6_1, &k6_2);

            /* 5th order solution (s) */
            double s1 = y1 + h * (35.0/384.0 * k1_0 + 500.0/1113.0 * k3_0 +
                                 125.0/192.0 * k4_0 - 2187.0/6784.0 * k5_0 +
                                 11.0/84.0 * k6_0);
            double s2 = y2 + h * (35.0/384.0 * k1_1 + 500.0/1113.0 * k3_1 +
                                 125.0/192.0 * k4_1 - 2187.0/6784.0 * k5_1 +
                                 11.0/84.0 * k6_1);
            double s3 = y3 + h * (35.0/384.0 * k1_2 + 500.0/1113.0 * k3_2 +
                                 125.0/192.0 * k4_2 - 2187.0/6784.0 * k5_2 +
                                 11.0/84.0 * k6_2);

            /* Stage 7 (FSAL) – evaluate RHS at the 5th‑order estimate */
            double k7_0, k7_1, k7_2;
            robertson_rhs(s1, s2, s3, &k7_0, &k7_1, &k7_2);

            /* 4th order solution (y4) */
            double y4_1 = y1 + h * (5179.0/57600.0 * k1_0 + 7571.0/16695.0 * k3_0 +
                                    393.0/640.0 * k4_0 - 92097.0/339200.0 * k5_0 +
                                    187.0/2100.0 * k6_0 + 1.0/40.0 * k7_0);
            double y4_2 = y2 + h * (5179.0/57600.0 * k1_1 + 7571.0/16695.0 * k3_1 +
                                    393.0/640.0 * k4_1 - 92097.0/339200.0 * k5_1 +
                                    187.0/2100.0 * k6_1 + 1.0/40.0 * k7_1);
            double y4_3 = y3 + h * (5179.0/57600.0 * k1_2 + 7571.0/16695.0 * k3_2 +
                                    393.0/640.0 * k4_2 - 92097.0/339200.0 * k5_2 +
                                    187.0/2100.0 * k6_2 + 1.0/40.0 * k7_2);

            /* Error estimation */
            double e1 = s1 - y4_1;
            double e2 = s2 - y4_2;
            double e3 = s3 - y4_3;
            double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
            double ynorm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);
            double err = enorm / (ATOL + RTOL * ynorm);
            ++steps;

            double factor;
            if (err <= 1.0) {
                /* Accept step */
                y1 = s1;
                y2 = s2;
                y3 = s3;
                t += h;
                ++accepts;
            } else {
                ++rejects;
            }

            if (err > 0.0) {
                factor = 0.9 * pow(err, -0.2);
                if (factor > 5.0) factor = 5.0;
                if (factor < 0.2) factor = 0.2;
            } else {
                factor = 5.0;
            }
            h = h * factor;
        }

        double *row_out = y + n * 3;
        row_out[0] = y1;
        row_out[1] = y2;
        row_out[2] = y3;
        n_accept[n] = accepts;
        n_reject[n] = rejects;
    }
}
