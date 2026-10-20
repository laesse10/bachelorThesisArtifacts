/* Adaptive Dormand-Prince (RK45) over an ensemble of independent Robertson ODEs.
 * Direct port of rk45_ensemble_numpy.py; outer loop parallelised with OpenMP. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>

#define MAX_STEPS 200000
#define H0 1.0e-4
#ifndef RTOL
#define RTOL 1.0e-6
#endif
#ifndef ATOL
#define ATOL 1.0e-9
#endif

static inline void robertson_rhs(double y1, double y2, double y3,
                                 double *dy1, double *dy2, double *dy3) {
    *dy1 = -0.04 * y1 + 1.0e4 * y2 * y3;
    *dy2 = 0.04 * y1 - 1.0e4 * y2 * y3 - 3.0e7 * y2 * y2;
    *dy3 = 3.0e7 * y2 * y2;
}

void rk45_ensemble_fp64(
    int64_t *restrict n_accept,
    int64_t *restrict n_reject,
    double *restrict y,
    const double *restrict y0,
    const int64_t NSYS,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace;
    (void)workspace_size;
    const double rtol = RTOL;
    const double atol = ATOL;
    const int64_t max_steps = MAX_STEPS;

#if 1
    printf("PROBE NSYS=%ld t_end=%.17g threads=%d y0=[%.17g %.17g %.17g]\n", (long)NSYS,
           t_end, omp_get_max_threads(), y0[0], y0[1], y0[2]);
    fflush(stdout);
#endif

    #pragma omp parallel for schedule(dynamic, 1024)
    for (int64_t n = 0; n < NSYS; n++) {
        double y1 = y0[3 * n + 0];
        double y2 = y0[3 * n + 1];
        double y3 = y0[3 * n + 2];
        double t = 0.0;
        double h = H0;
        int64_t accepts = 0;
        int64_t rejects = 0;
        int64_t steps = 0;
        while (t < t_end && steps < max_steps) {
            if (t + h > t_end) {
                h = t_end - t;
            }
            double k1x, k1y, k1z, k2x, k2y, k2z, k3x, k3y, k3z,
                   k4x, k4y, k4z, k5x, k5y, k5z, k6x, k6y, k6z,
                   k7x, k7y, k7z;
            robertson_rhs(y1, y2, y3, &k1x, &k1y, &k1z);
            robertson_rhs(y1 + h * (1.0 / 5.0) * k1x, y2 + h * (1.0 / 5.0) * k1y,
                          y3 + h * (1.0 / 5.0) * k1z, &k2x, &k2y, &k2z);
            robertson_rhs(y1 + h * (3.0 / 40.0 * k1x + 9.0 / 40.0 * k2x),
                          y2 + h * (3.0 / 40.0 * k1y + 9.0 / 40.0 * k2y),
                          y3 + h * (3.0 / 40.0 * k1z + 9.0 / 40.0 * k2z),
                          &k3x, &k3y, &k3z);
            robertson_rhs(y1 + h * (44.0 / 45.0 * k1x - 56.0 / 15.0 * k2x + 32.0 / 9.0 * k3x),
                          y2 + h * (44.0 / 45.0 * k1y - 56.0 / 15.0 * k2y + 32.0 / 9.0 * k3y),
                          y3 + h * (44.0 / 45.0 * k1z - 56.0 / 15.0 * k2z + 32.0 / 9.0 * k3z),
                          &k4x, &k4y, &k4z);
            robertson_rhs(y1 + h * (19372.0 / 6561.0 * k1x - 25360.0 / 2187.0 * k2x
                                    + 64448.0 / 6561.0 * k3x - 212.0 / 729.0 * k4x),
                          y2 + h * (19372.0 / 6561.0 * k1y - 25360.0 / 2187.0 * k2y
                                    + 64448.0 / 6561.0 * k3y - 212.0 / 729.0 * k4y),
                          y3 + h * (19372.0 / 6561.0 * k1z - 25360.0 / 2187.0 * k2z
                                    + 64448.0 / 6561.0 * k3z - 212.0 / 729.0 * k4z),
                          &k5x, &k5y, &k5z);
            robertson_rhs(y1 + h * (9017.0 / 3168.0 * k1x - 355.0 / 33.0 * k2x
                                    + 46732.0 / 5247.0 * k3x + 49.0 / 176.0 * k4x
                                    - 5103.0 / 18656.0 * k5x),
                          y2 + h * (9017.0 / 3168.0 * k1y - 355.0 / 33.0 * k2y
                                    + 46732.0 / 5247.0 * k3y + 49.0 / 176.0 * k4y
                                    - 5103.0 / 18656.0 * k5y),
                          y3 + h * (9017.0 / 3168.0 * k1z - 355.0 / 33.0 * k2z
                                    + 46732.0 / 5247.0 * k3z + 49.0 / 176.0 * k4z
                                    - 5103.0 / 18656.0 * k5z),
                          &k6x, &k6y, &k6z);
            double s1 = y1 + h * (35.0 / 384.0 * k1x + 500.0 / 1113.0 * k3x
                                  + 125.0 / 192.0 * k4x - 2187.0 / 6784.0 * k5x
                                  + 11.0 / 84.0 * k6x);
            double s2 = y2 + h * (35.0 / 384.0 * k1y + 500.0 / 1113.0 * k3y
                                  + 125.0 / 192.0 * k4y - 2187.0 / 6784.0 * k5y
                                  + 11.0 / 84.0 * k6y);
            double s3 = y3 + h * (35.0 / 384.0 * k1z + 500.0 / 1113.0 * k3z
                                  + 125.0 / 192.0 * k4z - 2187.0 / 6784.0 * k5z
                                  + 11.0 / 84.0 * k6z);
            robertson_rhs(s1, s2, s3, &k7x, &k7y, &k7z);
            double y4_1 = y1 + h * (5179.0 / 57600.0 * k1x + 7571.0 / 16695.0 * k3x
                                    + 393.0 / 640.0 * k4x - 92097.0 / 339200.0 * k5x
                                    + 187.0 / 2100.0 * k6x + 1.0 / 40.0 * k7x);
            double y4_2 = y2 + h * (5179.0 / 57600.0 * k1y + 7571.0 / 16695.0 * k3y
                                    + 393.0 / 640.0 * k4y - 92097.0 / 339200.0 * k5y
                                    + 187.0 / 2100.0 * k6y + 1.0 / 40.0 * k7y);
            double y4_3 = y3 + h * (5179.0 / 57600.0 * k1z + 7571.0 / 16695.0 * k3z
                                    + 393.0 / 640.0 * k4z - 92097.0 / 339200.0 * k5z
                                    + 187.0 / 2100.0 * k6z + 1.0 / 40.0 * k7z);
            double e1 = s1 - y4_1;
            double e2 = s2 - y4_2;
            double e3 = s3 - y4_3;
            double enorm = pow(e1 * e1 + e2 * e2 + e3 * e3, 0.5);
            double ynorm = pow(s1 * s1 + s2 * s2 + s3 * s3, 0.5);
            double err = enorm / (atol + rtol * ynorm);
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
            double factor;
            if (err > 0.0) {
                factor = 0.9 * pow(err, -0.2);
                if (factor > 5.0) {
                    factor = 5.0;
                }
                if (factor < 0.2) {
                    factor = 0.2;
                }
            } else {
                factor = 5.0;
            }
            h = h * factor;
        }
        y[3 * n + 0] = y1;
        y[3 * n + 1] = y2;
        y[3 * n + 2] = y3;
        n_accept[n] = accepts;
        n_reject[n] = rejects;
    }
#if 1
    {
        long long sa = 0, sr = 0;
        for (int64_t i = 0; i < NSYS; i++) { sa += n_accept[i]; sr += n_reject[i]; }
        printf("PROBE sum_a=%lld sum_r=%lld steps_per_sys=%.3f y=[%.17g %.17g %.17g]\n",
               sa, sr, (double)(sa + sr) / (double)NSYS, y[0], y[1], y[2]);
        fflush(stdout);
    }
#endif
}
