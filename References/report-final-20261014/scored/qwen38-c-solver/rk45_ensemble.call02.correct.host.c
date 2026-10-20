#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Tolerances fixed by the task (the C ABI omits them). Pinned empirically. */
#ifndef RTOL
#define RTOL 1.0e-6
#endif
#ifndef ATOL
#define ATOL 1.0e-9
#endif
#ifndef USE_SQRT
#define USE_SQRT 1
#endif
#define MAX_STEPS 200000
#define H0 1.0e-4

static inline void step_system(int64_t n,
                               const double *restrict y0,
                               double t_end,
                               double *restrict y,
                               int64_t *restrict n_accept,
                               int64_t *restrict n_reject) {
    double y1 = y0[3*n+0];
    double y2 = y0[3*n+1];
    double y3 = y0[3*n+2];
    double t = 0.0, h = H0;
    int64_t accepts = 0, rejects = 0, steps = 0;
    while (t < t_end && steps < MAX_STEPS) {
        if (t + h > t_end) h = t_end - t;
        /* k1 */
        double p = 1.0e4*y2*y3, q = 3.0e7*y2*y2;
        double k1x = -0.04*y1 + p, k1y = 0.04*y1 - p - q, k1z = q;
        /* k2 */
        double s2x = y1 + h*(1.0/5.0)*k1x, s2y = y2 + h*(1.0/5.0)*k1y, s2z = y3 + h*(1.0/5.0)*k1z;
        p = 1.0e4*s2y*s2z; q = 3.0e7*s2y*s2y;
        double k2x = -0.04*s2x + p, k2y = 0.04*s2x - p - q, k2z = q;
        /* k3 */
        double s3x = y1 + h*(3.0/40.0*k1x + 9.0/40.0*k2x),
               s3y = y2 + h*(3.0/40.0*k1y + 9.0/40.0*k2y),
               s3z = y3 + h*(3.0/40.0*k1z + 9.0/40.0*k2z);
        p = 1.0e4*s3y*s3z; q = 3.0e7*s3y*s3y;
        double k3x = -0.04*s3x + p, k3y = 0.04*s3x - p - q, k3z = q;
        /* k4 */
        double s4x = y1 + h*(44.0/45.0*k1x - 56.0/15.0*k2x + 32.0/9.0*k3x),
               s4y = y2 + h*(44.0/45.0*k1y - 56.0/15.0*k2y + 32.0/9.0*k3y),
               s4z = y3 + h*(44.0/45.0*k1z - 56.0/15.0*k2z + 32.0/9.0*k3z);
        p = 1.0e4*s4y*s4z; q = 3.0e7*s4y*s4y;
        double k4x = -0.04*s4x + p, k4y = 0.04*s4x - p - q, k4z = q;
        /* k5 */
        double s5x = y1 + h*(19372.0/6561.0*k1x - 25360.0/2187.0*k2x + 64448.0/6561.0*k3x - 212.0/729.0*k4x),
               s5y = y2 + h*(19372.0/6561.0*k1y - 25360.0/2187.0*k2y + 64448.0/6561.0*k3y - 212.0/729.0*k4y),
               s5z = y3 + h*(19372.0/6561.0*k1z - 25360.0/2187.0*k2z + 64448.0/6561.0*k3z - 212.0/729.0*k4z);
        p = 1.0e4*s5y*s5z; q = 3.0e7*s5y*s5y;
        double k5x = -0.04*s5x + p, k5y = 0.04*s5x - p - q, k5z = q;
        /* k6 */
        double s6x = y1 + h*(9017.0/3168.0*k1x - 355.0/33.0*k2x + 46732.0/5247.0*k3x + 49.0/176.0*k4x - 5103.0/18656.0*k5x),
               s6y = y2 + h*(9017.0/3168.0*k1y - 355.0/33.0*k2y + 46732.0/5247.0*k3y + 49.0/176.0*k4y - 5103.0/18656.0*k5y),
               s6z = y3 + h*(9017.0/3168.0*k1z - 355.0/33.0*k2z + 46732.0/5247.0*k3z + 49.0/176.0*k4z - 5103.0/18656.0*k5z);
        p = 1.0e4*s6y*s6z; q = 3.0e7*s6y*s6y;
        double k6x = -0.04*s6x + p, k6y = 0.04*s6x - p - q, k6z = q;
        /* s (4th-order step = stage-7 point) */
        double s1 = y1 + h*(35.0/384.0*k1x + 500.0/1113.0*k3x + 125.0/192.0*k4x - 2187.0/6784.0*k5x + 11.0/84.0*k6x),
               s2 = y2 + h*(35.0/384.0*k1y + 500.0/1113.0*k3y + 125.0/192.0*k4y - 2187.0/6784.0*k5y + 11.0/84.0*k6y),
               s3 = y3 + h*(35.0/384.0*k1z + 500.0/1113.0*k3z + 125.0/192.0*k4z - 2187.0/6784.0*k5z + 11.0/84.0*k6z);
        /* k7 = RHS(s) */
        p = 1.0e4*s2*s3; q = 3.0e7*s2*s2;
        double k7x = -0.04*s1 + p, k7y = 0.04*s1 - p - q, k7z = q;
        /* y4 (5th-order) */
        double y4_1 = y1 + h*(5179.0/57600.0*k1x + 7571.0/16695.0*k3x + 393.0/640.0*k4x - 92097.0/339200.0*k5x + 187.0/2100.0*k6x + 1.0/40.0*k7x),
               y4_2 = y2 + h*(5179.0/57600.0*k1y + 7571.0/16695.0*k3y + 393.0/640.0*k4y - 92097.0/339200.0*k5y + 187.0/2100.0*k6y + 1.0/40.0*k7y),
               y4_3 = y3 + h*(5179.0/57600.0*k1z + 7571.0/16695.0*k3z + 393.0/640.0*k4z - 92097.0/339200.0*k5z + 187.0/2100.0*k6z + 1.0/40.0*k7z);
        double e1 = s1 - y4_1, e2 = s2 - y4_2, e3 = s3 - y4_3;
#if USE_SQRT
        double enorm = sqrt(e1*e1 + e2*e2 + e3*e3);
        double ynorm = sqrt(s1*s1 + s2*s2 + s3*s3);
#else
        double enorm = pow(e1*e1 + e2*e2 + e3*e3, 0.5);
        double ynorm = pow(s1*s1 + s2*s2 + s3*s3, 0.5);
#endif
        double err = enorm / (ATOL + RTOL*ynorm);
        steps = steps + 1;
        if (err <= 1.0) { y1 = s1; y2 = s2; y3 = s3; t = t + h; accepts = accepts + 1; }
        else { rejects = rejects + 1; }
        double factor;
        if (err > 0.0) {
            factor = 0.9 * (pow(err, -0.2));
            if (factor > 5.0) factor = 5.0;
            if (factor < 0.2) factor = 0.2;
        } else {
            factor = 5.0;
        }
        h = h * factor;
    }
    y[3*n+0] = y1; y[3*n+1] = y2; y[3*n+2] = y3;
    n_accept[n] = accepts;
    n_reject[n] = rejects;
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
    (void)workspace; (void)workspace_size;
    #pragma omp parallel for schedule(dynamic, 512)
    for (int64_t n = 0; n < NSYS; n++) {
        step_system(n, y0, t_end, y, n_accept, n_reject);
    }
}
