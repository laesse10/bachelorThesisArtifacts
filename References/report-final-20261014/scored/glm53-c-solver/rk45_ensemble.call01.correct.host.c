#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef RTOL
#define RTOL 1e-6
#endif
#ifndef ATOL
#define ATOL 1e-9
#endif

#define MAX_STEPS 200000
#define H0 1.0e-4

static inline void rhs(double y1, double y2, double y3,
                       double *d1, double *d2, double *d3) {
    double t2 = 3.0e7 * y2;
    *d1 = -0.04 * y1 + 1.0e4 * y2 * y3;
    *d2 = 0.04 * y1 - 1.0e4 * y2 * y3 - t2 * y2;
    *d3 = t2 * y2;
}

static void integrate_system(const double *y0row, double *yrow,
                            int64_t *acc, int64_t *rej, double t_end) {
    double y1 = y0row[0], y2 = y0row[1], y3 = y0row[2];
    double t = 0.0, h = H0;
    int64_t accepts = 0, rejects = 0, steps = 0;
    while (t < t_end && steps < MAX_STEPS) {
        if (t + h > t_end) h = t_end - t;
        double k1a, k1b, k1c;
        rhs(y1, y2, y3, &k1a, &k1b, &k1c);
        double a1 = y1 + h * (1.0 / 5.0 * k1a);
        double a2 = y2 + h * (1.0 / 5.0 * k1b);
        double a3 = y3 + h * (1.0 / 5.0 * k1c);
        double k2a, k2b, k2c;
        rhs(a1, a2, a3, &k2a, &k2b, &k2c);
        a1 = y1 + h * (3.0 / 40.0 * k1a + 9.0 / 40.0 * k2a);
        a2 = y2 + h * (3.0 / 40.0 * k1b + 9.0 / 40.0 * k2b);
        a3 = y3 + h * (3.0 / 40.0 * k1c + 9.0 / 40.0 * k2c);
        double k3a, k3b, k3c;
        rhs(a1, a2, a3, &k3a, &k3b, &k3c);
        a1 = y1 + h * (44.0 / 45.0 * k1a - 56.0 / 15.0 * k2a + 32.0 / 9.0 * k3a);
        a2 = y2 + h * (44.0 / 45.0 * k1b - 56.0 / 15.0 * k2b + 32.0 / 9.0 * k3b);
        a3 = y3 + h * (44.0 / 45.0 * k1c - 56.0 / 15.0 * k2c + 32.0 / 9.0 * k3c);
        double k4a, k4b, k4c;
        rhs(a1, a2, a3, &k4a, &k4b, &k4c);
        a1 = y1 + h * (19372.0 / 6561.0 * k1a - 25360.0 / 2187.0 * k2a + 64448.0 / 6561.0 * k3a - 212.0 / 729.0 * k4a);
        a2 = y2 + h * (19372.0 / 6561.0 * k1b - 25360.0 / 2187.0 * k2b + 64448.0 / 6561.0 * k3b - 212.0 / 729.0 * k4b);
        a3 = y3 + h * (19372.0 / 6561.0 * k1c - 25360.0 / 2187.0 * k2c + 64448.0 / 6561.0 * k3c - 212.0 / 729.0 * k4c);
        double k5a, k5b, k5c;
        rhs(a1, a2, a3, &k5a, &k5b, &k5c);
        a1 = y1 + h * (9017.0 / 3168.0 * k1a - 355.0 / 33.0 * k2a + 46732.0 / 5247.0 * k3a + 49.0 / 176.0 * k4a - 5103.0 / 18656.0 * k5a);
        a2 = y2 + h * (9017.0 / 3168.0 * k1b - 355.0 / 33.0 * k2b + 46732.0 / 5247.0 * k3b + 49.0 / 176.0 * k4b - 5103.0 / 18656.0 * k5b);
        a3 = y3 + h * (9017.0 / 3168.0 * k1c - 355.0 / 33.0 * k2c + 46732.0 / 5247.0 * k3c + 49.0 / 176.0 * k4c - 5103.0 / 18656.0 * k5c);
        double k6a, k6b, k6c;
        rhs(a1, a2, a3, &k6a, &k6b, &k6c);
        double s1 = y1 + h * (35.0 / 384.0 * k1a + 500.0 / 1113.0 * k3a + 125.0 / 192.0 * k4a - 2187.0 / 6784.0 * k5a + 11.0 / 84.0 * k6a);
        double s2 = y2 + h * (35.0 / 384.0 * k1b + 500.0 / 1113.0 * k3b + 125.0 / 192.0 * k4b - 2187.0 / 6784.0 * k5b + 11.0 / 84.0 * k6b);
        double s3 = y3 + h * (35.0 / 384.0 * k1c + 500.0 / 1113.0 * k3c + 125.0 / 192.0 * k4c - 2187.0 / 6784.0 * k5c + 11.0 / 84.0 * k6c);
        double k7a, k7b, k7c;
        rhs(s1, s2, s3, &k7a, &k7b, &k7c);
        double y41 = y1 + h * (5179.0 / 57600.0 * k1a + 7571.0 / 16695.0 * k3a + 393.0 / 640.0 * k4a - 92097.0 / 339200.0 * k5a + 187.0 / 2100.0 * k6a + 1.0 / 40.0 * k7a);
        double y42 = y2 + h * (5179.0 / 57600.0 * k1b + 7571.0 / 16695.0 * k3b + 393.0 / 640.0 * k4b - 92097.0 / 339200.0 * k5b + 187.0 / 2100.0 * k6b + 1.0 / 40.0 * k7b);
        double y43 = y3 + h * (5179.0 / 57600.0 * k1c + 7571.0 / 16695.0 * k3c + 393.0 / 640.0 * k4c - 92097.0 / 339200.0 * k5c + 187.0 / 2100.0 * k6c + 1.0 / 40.0 * k7c);
        double e1 = s1 - y41, e2 = s2 - y42, e3 = s3 - y43;
        double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
        double ynorm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);
        double err = enorm / (ATOL + RTOL * ynorm);
        steps++;
        if (err <= 1.0) {
            y1 = s1; y2 = s2; y3 = s3;
            t += h;
            accepts++;
        } else {
            rejects++;
        }
        double factor;
        if (err > 0.0) {
            factor = 0.9 * pow(err, -0.2);
            if (factor > 5.0) factor = 5.0;
            if (factor < 0.2) factor = 0.2;
        } else {
            factor = 5.0;
        }
        h *= factor;
    }
    yrow[0] = y1; yrow[1] = y2; yrow[2] = y3;
    *acc = accepts; *rej = rejects;
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
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; n++) {
        integrate_system(&y0[3 * n], &y[3 * n], &n_accept[n], &n_reject[n], t_end);
    }
}
