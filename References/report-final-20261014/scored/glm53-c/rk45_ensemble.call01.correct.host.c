#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef RTOL
#define RTOL 1.0e-6
#endif
#ifndef ATOL
#define ATOL 1.0e-9
#endif

#define MAX_STEPS 200000
#define H0 1.0e-4

#define RHS(Y1, Y2, Y3, O1, O2, O3)                              \
    do {                                                          \
        (O1) = -0.04 * (Y1) + 1.0e4 * (Y2) * (Y3);                \
        (O2) = 0.04 * (Y1) - 1.0e4 * (Y2) * (Y3) - 3.0e7 * (Y2) * (Y2); \
        (O3) = 3.0e7 * (Y2) * (Y2);                               \
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
    int64_t i;
    #pragma omp parallel for schedule(static)
    for (i = 0; i < NSYS; i++) {
        integrate_one(y0 + 3 * i, y + 3 * i, &n_accept[i], &n_reject[i], t_end);
    }
}
