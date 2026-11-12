#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define MAX_STEPS 200000LL
#define H0 1.0e-4
#define RTOL 1.0e-6
#define ATOL 1.0e-9

static inline void dopri5_step(
    const double y1, const double y2, const double y3, double h,
    double *s1, double *s2, double *s3, double *err_out)
{
    double k1_0, k1_1, k1_2, k2_0, k2_1, k2_2, k3_0, k3_1, k3_2;
    double k4_0, k4_1, k4_2, k5_0, k5_1, k5_2, k6_0, k6_1, k6_2;
    double k7_0, k7_1, k7_2;

    k1_0 = -0.04 * y1 + 1.0e4 * y2 * y3;
    k1_1 = 0.04 * y1 - 1.0e4 * y2 * y3 - 3.0e7 * y2 * y2;
    k1_2 = 3.0e7 * y2 * y2;

    {
        double p = y1 + h * (1.0/5.0) * k1_0;
        double q = y2 + h * (1.0/5.0) * k1_1;
        double r = y3 + h * (1.0/5.0) * k1_2;
        k2_0 = -0.04 * p + 1.0e4 * q * r;
        k2_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k2_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (3.0/40.0 * k1_0 + 9.0/40.0 * k2_0);
        double q = y2 + h * (3.0/40.0 * k1_1 + 9.0/40.0 * k2_1);
        double r = y3 + h * (3.0/40.0 * k1_2 + 9.0/40.0 * k2_2);
        k3_0 = -0.04 * p + 1.0e4 * q * r;
        k3_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k3_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (44.0/45.0 * k1_0 - 56.0/15.0 * k2_0 + 32.0/9.0 * k3_0);
        double q = y2 + h * (44.0/45.0 * k1_1 - 56.0/15.0 * k2_1 + 32.0/9.0 * k3_1);
        double r = y3 + h * (44.0/45.0 * k1_2 - 56.0/15.0 * k2_2 + 32.0/9.0 * k3_2);
        k4_0 = -0.04 * p + 1.0e4 * q * r;
        k4_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k4_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (19372.0/6561.0 * k1_0 - 25360.0/2187.0 * k2_0
                             + 64448.0/6561.0 * k3_0 - 212.0/729.0 * k4_0);
        double q = y2 + h * (19372.0/6561.0 * k1_1 - 25360.0/2187.0 * k2_1
                             + 64448.0/6561.0 * k3_1 - 212.0/729.0 * k4_1);
        double r = y3 + h * (19372.0/6561.0 * k1_2 - 25360.0/2187.0 * k2_2
                             + 64448.0/6561.0 * k3_2 - 212.0/729.0 * k4_2);
        k5_0 = -0.04 * p + 1.0e4 * q * r;
        k5_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k5_2 = 3.0e7 * q * q;
    }
    {
        double p = y1 + h * (9017.0/3168.0 * k1_0 - 355.0/33.0 * k2_0
                             + 46732.0/5247.0 * k3_0 + 49.0/176.0 * k4_0
                             - 5103.0/18656.0 * k5_0);
        double q = y2 + h * (9017.0/3168.0 * k1_1 - 355.0/33.0 * k2_1
                             + 46732.0/5247.0 * k3_1 + 49.0/176.0 * k4_1
                             - 5103.0/18656.0 * k5_1);
        double r = y3 + h * (9017.0/3168.0 * k1_2 - 355.0/33.0 * k2_2
                             + 46732.0/5247.0 * k3_2 + 49.0/176.0 * k4_2
                             - 5103.0/18656.0 * k5_2);
        k6_0 = -0.04 * p + 1.0e4 * q * r;
        k6_1 = 0.04 * p - 1.0e4 * q * r - 3.0e7 * q * q;
        k6_2 = 3.0e7 * q * q;
    }
    double s1v = y1 + h * (35.0/384.0 * k1_0 + 500.0/1113.0 * k3_0
                           + 125.0/192.0 * k4_0 - 2187.0/6784.0 * k5_0
                           + 11.0/84.0 * k6_0);
    double s2v = y2 + h * (35.0/384.0 * k1_1 + 500.0/1113.0 * k3_1
                           + 125.0/192.0 * k4_1 - 2187.0/6784.0 * k5_1
                           + 11.0/84.0 * k6_1);
    double s3v = y3 + h * (35.0/384.0 * k1_2 + 500.0/1113.0 * k3_2
                           + 125.0/192.0 * k4_2 - 2187.0/6784.0 * k5_2
                           + 11.0/84.0 * k6_2);
    k7_0 = -0.04 * s1v + 1.0e4 * s2v * s3v;
    k7_1 = 0.04 * s1v - 1.0e4 * s2v * s3v - 3.0e7 * s2v * s2v;
    k7_2 = 3.0e7 * s2v * s2v;
    double y4_1 = y1 + h * (5179.0/57600.0 * k1_0 + 7571.0/16695.0 * k3_0
                            + 393.0/640.0 * k4_0 - 92097.0/339200.0 * k5_0
                            + 187.0/2100.0 * k6_0 + 1.0/40.0 * k7_0);
    double y4_2 = y2 + h * (5179.0/57600.0 * k1_1 + 7571.0/16695.0 * k3_1
                            + 393.0/640.0 * k4_1 - 92097.0/339200.0 * k5_1
                            + 187.0/2100.0 * k6_1 + 1.0/40.0 * k7_1);
    double y4_3 = y3 + h * (5179.0/57600.0 * k1_2 + 7571.0/16695.0 * k3_2
                            + 393.0/640.0 * k4_2 - 92097.0/339200.0 * k5_2
                            + 187.0/2100.0 * k6_2 + 1.0/40.0 * k7_2);
    double e1 = s1v - y4_1, e2 = s2v - y4_2, e3 = s3v - y4_3;
    double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
    double ynorm = sqrt(s1v * s1v + s2v * s2v + s3v * s3v);
    *s1 = s1v; *s2 = s2v; *s3 = s3v;
    *err_out = enorm / (ATOL + RTOL * ynorm);
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
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        double y1 = y0[3*n], y2 = y0[3*n+1], y3 = y0[3*n+2];
        double t = 0.0, h = H0;
        int64_t accepts = 0, rejects = 0, steps = 0;
        while (t < t_end && steps < MAX_STEPS) {
            if (t + h > t_end) h = t_end - t;
            double s1, s2, s3, err;
            dopri5_step(y1, y2, y3, h, &s1, &s2, &s3, &err);
            steps = steps + 1;
            if (err <= 1.0) {
                y1 = s1; y2 = s2; y3 = s3;
                t = t + h;
                accepts = accepts + 1;
            } else {
                rejects = rejects + 1;
            }
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
        y[3*n] = y1; y[3*n+1] = y2; y[3*n+2] = y3;
        n_accept[n] = accepts;
        n_reject[n] = rejects;
    }
}
