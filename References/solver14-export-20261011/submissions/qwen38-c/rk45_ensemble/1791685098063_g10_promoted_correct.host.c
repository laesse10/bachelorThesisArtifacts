/* v1 style but k1..k6 components declared as separate doubles; expect FMA contraction. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#define MAX_STEPS 200000
#define H0 1.0e-4
#define RTOL 1.0e-6
#define ATOL 1.0e-9

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
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        double y1 = y0[n*3+0], y2 = y0[n*3+1], y3 = y0[n*3+2];
        double t = 0.0, h = H0;
        int64_t accepts = 0, rejects = 0, steps = 0;
        double k1a, k1b, k1c, k2a, k2b, k2c, k3a, k3b, k3c,
               k4a, k4b, k4c, k5a, k5b, k5c, k6a, k6b, k6c;
        while (t < t_end && steps < MAX_STEPS) {
            if (t + h > t_end) h = t_end - t;
            { double a=y1, b=y2, c=y3;
              k1a = (-0.04*a) + (1.0e4*b)*c;
              k1b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k1c = (3.0e7*b)*b; }
            { double a=y1+h*(1.0/5.0)*k1a, b=y2+h*(1.0/5.0)*k1b, c=y3+h*(1.0/5.0)*k1c;
              k2a = (-0.04*a) + (1.0e4*b)*c;
              k2b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k2c = (3.0e7*b)*b; }
            { double a=y1+h*(3.0/40.0*k1a + 9.0/40.0*k2a),
                     b=y2+h*(3.0/40.0*k1b + 9.0/40.0*k2b),
                     c=y3+h*(3.0/40.0*k1c + 9.0/40.0*k2c);
              k3a = (-0.04*a) + (1.0e4*b)*c;
              k3b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k3c = (3.0e7*b)*b; }
            { double a=y1+h*(44.0/45.0*k1a - 56.0/15.0*k2a + 32.0/9.0*k3a),
                     b=y2+h*(44.0/45.0*k1b - 56.0/15.0*k2b + 32.0/9.0*k3b),
                     c=y3+h*(44.0/45.0*k1c - 56.0/15.0*k2c + 32.0/9.0*k3c);
              k4a = (-0.04*a) + (1.0e4*b)*c;
              k4b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k4c = (3.0e7*b)*b; }
            { double a=y1+h*(19372.0/6561.0*k1a - 25360.0/2187.0*k2a + 64448.0/6561.0*k3a - 212.0/729.0*k4a),
                     b=y2+h*(19372.0/6561.0*k1b - 25360.0/2187.0*k2b + 64448.0/6561.0*k3b - 212.0/729.0*k4b),
                     c=y3+h*(19372.0/6561.0*k1c - 25360.0/2187.0*k2c + 64448.0/6561.0*k3c - 212.0/729.0*k4c);
              k5a = (-0.04*a) + (1.0e4*b)*c;
              k5b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k5c = (3.0e7*b)*b; }
            { double a=y1+h*(9017.0/3168.0*k1a - 355.0/33.0*k2a + 46732.0/5247.0*k3a + 49.0/176.0*k4a - 5103.0/18656.0*k5a),
                     b=y2+h*(9017.0/3168.0*k1b - 355.0/33.0*k2b + 46732.0/5247.0*k3b + 49.0/176.0*k4b - 5103.0/18656.0*k5b),
                     c=y3+h*(9017.0/3168.0*k1c - 355.0/33.0*k2c + 46732.0/5247.0*k3c + 49.0/176.0*k4c - 5103.0/18656.0*k5c);
              k6a = (-0.04*a) + (1.0e4*b)*c;
              k6b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k6c = (3.0e7*b)*b; }
            double s1 = y1 + h*(35.0/384.0*k1a + 500.0/1113.0*k3a + 125.0/192.0*k4a - 2187.0/6784.0*k5a + 11.0/84.0*k6a);
            double s2 = y2 + h*(35.0/384.0*k1b + 500.0/1113.0*k3b + 125.0/192.0*k4b - 2187.0/6784.0*k5b + 11.0/84.0*k6b);
            double s3 = y3 + h*(35.0/384.0*k1c + 500.0/1113.0*k3c + 125.0/192.0*k4c - 2187.0/6784.0*k5c + 11.0/84.0*k6c);
            double k7a, k7b, k7c;
            { double a=s1, b=s2, c=s3;
              k7a = (-0.04*a) + (1.0e4*b)*c;
              k7b = (0.04*a) - (1.0e4*b)*c - (3.0e7*b)*b;
              k7c = (3.0e7*b)*b; }
            double y4_1 = y1 + h*(5179.0/57600.0*k1a + 7571.0/16695.0*k3a + 393.0/640.0*k4a - 92097.0/339200.0*k5a + 187.0/2100.0*k6a + 1.0/40.0*k7a);
            double y4_2 = y2 + h*(5179.0/57600.0*k1b + 7571.0/16695.0*k3b + 393.0/640.0*k4b - 92097.0/339200.0*k5b + 187.0/2100.0*k6b + 1.0/40.0*k7b);
            double y4_3 = y3 + h*(5179.0/57600.0*k1c + 7571.0/16695.0*k3c + 393.0/640.0*k4c - 92097.0/339200.0*k5c + 187.0/2100.0*k6c + 1.0/40.0*k7c);
            double e1 = s1 - y4_1, e2 = s2 - y4_2, e3 = s3 - y4_3;
            double enorm = sqrt(e1*e1 + e2*e2 + e3*e3);
            double ynorm = sqrt(s1*s1 + s2*s2 + s3*s3);
            double err = enorm / (ATOL + RTOL * ynorm);
            steps++;
            if (err <= 1.0) { y1 = s1; y2 = s2; y3 = s3; t += h; accepts++; }
            else rejects++;
            double factor;
            if (err > 0.0) {
                factor = 0.9 * pow(err, -0.2);
                if (factor > 5.0) factor = 5.0;
                if (factor < 0.2) factor = 0.2;
            } else factor = 5.0;
            h *= factor;
        }
        y[n*3+0] = y1; y[n*3+1] = y2; y[n*3+2] = y3;
        n_accept[n] = accepts; n_reject[n] = rejects;
    }
}
