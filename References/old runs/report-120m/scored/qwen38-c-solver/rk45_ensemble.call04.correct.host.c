/* Adaptive Dormand-Prince (RK45) over an ensemble of independent Robertson ODEs.
 * Direct port of rk45_ensemble_numpy.py; outer loop parallelised with OpenMP and
 * pairs of systems interleaved to expose latency hiding. Every per-system floating
 * point operation is identical (same expressions, same order) to the serial form. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

/* One RK45 step for the system whose persistent state uses the P_ prefix. */
#define STEP(P)                                                             \
    do {                                                                    \
        if ((P##t) + (P##h) > t_end) {                                      \
            P##h = t_end - (P##t);                                          \
        }                                                                   \
        double k1x, k1y, k1z, k2x, k2y, k2z, k3x, k3y, k3z,                 \
               k4x, k4y, k4z, k5x, k5y, k5z, k6x, k6y, k6z,                 \
               k7x, k7y, k7z;                                               \
        double q1s, q2s, q3s; /* partial 5th-order (b5) sums */             \
        double q14, q24, q34; /* partial 4th-order (b4) sums */             \
        robertson_rhs(P##y1, P##y2, P##y3, &k1x, &k1y, &k1z);               \
        q1s = 35.0 / 384.0 * k1x;                                           \
        q2s = 35.0 / 384.0 * k1y;                                           \
        q3s = 35.0 / 384.0 * k1z;                                           \
        q14 = 5179.0 / 57600.0 * k1x;                                       \
        q24 = 5179.0 / 57600.0 * k1y;                                       \
        q34 = 5179.0 / 57600.0 * k1z;                                       \
        robertson_rhs(P##y1 + P##h * (1.0 / 5.0) * k1x,                     \
                      P##y2 + P##h * (1.0 / 5.0) * k1y,                     \
                      P##y3 + P##h * (1.0 / 5.0) * k1z, &k2x, &k2y, &k2z);  \
        robertson_rhs(P##y1 + P##h * (3.0 / 40.0 * k1x + 9.0 / 40.0 * k2x), \
                      P##y2 + P##h * (3.0 / 40.0 * k1y + 9.0 / 40.0 * k2y), \
                      P##y3 + P##h * (3.0 / 40.0 * k1z + 9.0 / 40.0 * k2z), \
                      &k3x, &k3y, &k3z);                                    \
        q1s = q1s + 500.0 / 1113.0 * k3x;                                   \
        q2s = q2s + 500.0 / 1113.0 * k3y;                                   \
        q3s = q3s + 500.0 / 1113.0 * k3z;                                   \
        q14 = q14 + 7571.0 / 16695.0 * k3x;                                 \
        q24 = q24 + 7571.0 / 16695.0 * k3y;                                 \
        q34 = q34 + 7571.0 / 16695.0 * k3z;                                 \
        robertson_rhs(P##y1 + P##h * (44.0 / 45.0 * k1x - 56.0 / 15.0 * k2x + 32.0 / 9.0 * k3x), \
                      P##y2 + P##h * (44.0 / 45.0 * k1y - 56.0 / 15.0 * k2y + 32.0 / 9.0 * k3y), \
                      P##y3 + P##h * (44.0 / 45.0 * k1z - 56.0 / 15.0 * k2z + 32.0 / 9.0 * k3z), \
                      &k4x, &k4y, &k4z);                                    \
        q1s = q1s + 125.0 / 192.0 * k4x;                                    \
        q2s = q2s + 125.0 / 192.0 * k4y;                                    \
        q3s = q3s + 125.0 / 192.0 * k4z;                                    \
        q14 = q14 + 393.0 / 640.0 * k4x;                                    \
        q24 = q24 + 393.0 / 640.0 * k4y;                                    \
        q34 = q34 + 393.0 / 640.0 * k4z;                                    \
        robertson_rhs(P##y1 + P##h * (19372.0 / 6561.0 * k1x - 25360.0 / 2187.0 * k2x \
                                      + 64448.0 / 6561.0 * k3x - 212.0 / 729.0 * k4x), \
                      P##y2 + P##h * (19372.0 / 6561.0 * k1y - 25360.0 / 2187.0 * k2y \
                                      + 64448.0 / 6561.0 * k3y - 212.0 / 729.0 * k4y), \
                      P##y3 + P##h * (19372.0 / 6561.0 * k1z - 25360.0 / 2187.0 * k2z \
                                      + 64448.0 / 6561.0 * k3z - 212.0 / 729.0 * k4z), \
                      &k5x, &k5y, &k5z);                                    \
        q1s = q1s - 2187.0 / 6784.0 * k5x;                                  \
        q2s = q2s - 2187.0 / 6784.0 * k5y;                                  \
        q3s = q3s - 2187.0 / 6784.0 * k5z;                                  \
        q14 = q14 - 92097.0 / 339200.0 * k5x;                               \
        q24 = q24 - 92097.0 / 339200.0 * k5y;                               \
        q34 = q34 - 92097.0 / 339200.0 * k5z;                               \
        robertson_rhs(P##y1 + P##h * (9017.0 / 3168.0 * k1x - 355.0 / 33.0 * k2x \
                                      + 46732.0 / 5247.0 * k3x + 49.0 / 176.0 * k4x \
                                      - 5103.0 / 18656.0 * k5x), \
                      P##y2 + P##h * (9017.0 / 3168.0 * k1y - 355.0 / 33.0 * k2y \
                                      + 46732.0 / 5247.0 * k3y + 49.0 / 176.0 * k4y \
                                      - 5103.0 / 18656.0 * k5y), \
                      P##y3 + P##h * (9017.0 / 3168.0 * k1z - 355.0 / 33.0 * k2z \
                                      + 46732.0 / 5247.0 * k3z + 49.0 / 176.0 * k4z \
                                      - 5103.0 / 18656.0 * k5z), \
                      &k6x, &k6y, &k6z);                                    \
        q1s = q1s + 11.0 / 84.0 * k6x;                                      \
        q2s = q2s + 11.0 / 84.0 * k6y;                                      \
        q3s = q3s + 11.0 / 84.0 * k6z;                                      \
        q14 = q14 + 187.0 / 2100.0 * k6x;                                   \
        q24 = q24 + 187.0 / 2100.0 * k6y;                                   \
        q34 = q34 + 187.0 / 2100.0 * k6z;                                   \
        double s1 = P##y1 + P##h * q1s;                                     \
        double s2 = P##y2 + P##h * q2s;                                     \
        double s3 = P##y3 + P##h * q3s;                                     \
        robertson_rhs(s1, s2, s3, &k7x, &k7y, &k7z);                        \
        q14 = q14 + 1.0 / 40.0 * k7x;                                       \
        q24 = q24 + 1.0 / 40.0 * k7y;                                       \
        q34 = q34 + 1.0 / 40.0 * k7z;                                       \
        double y4_1 = P##y1 + P##h * q14;                                   \
        double y4_2 = P##y2 + P##h * q24;                                   \
        double y4_3 = P##y3 + P##h * q34;                                   \
        double e1 = s1 - y4_1;                                              \
        double e2 = s2 - y4_2;                                              \
        double e3 = s3 - y4_3;                                              \
        double enorm = pow(e1 * e1 + e2 * e2 + e3 * e3, 0.5);               \
        double ynorm = pow(s1 * s1 + s2 * s2 + s3 * s3, 0.5);               \
        double err = enorm / (atol + rtol * ynorm);                         \
        P##stp = P##stp + 1;                                                \
        if (err <= 1.0) {                                                   \
            P##y1 = s1;                                                     \
            P##y2 = s2;                                                     \
            P##y3 = s3;                                                     \
            P##t = P##t + P##h;                                             \
            P##acc = P##acc + 1;                                            \
        } else {                                                            \
            P##rej = P##rej + 1;                                            \
        }                                                                   \
        double factor;                                                      \
        if (err > 0.0) {                                                    \
            factor = 0.9 * pow(err, -0.2);                                  \
            if (factor > 5.0) {                                             \
                factor = 5.0;                                               \
            }                                                               \
            if (factor < 0.2) {                                             \
                factor = 0.2;                                               \
            }                                                               \
        } else {                                                            \
            factor = 5.0;                                                   \
        }                                                                   \
        P##h = P##h * factor;                                               \
    } while (0)

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
    const int64_t npair = (NSYS + 1) / 2;

    #pragma omp parallel for schedule(dynamic, 512)
    for (int64_t p = 0; p < npair; p++) {
        const int64_t n = 2 * p;
        double Ay1 = y0[3 * n + 0];
        double Ay2 = y0[3 * n + 1];
        double Ay3 = y0[3 * n + 2];
        double At = 0.0;
        double Ah = H0;
        int64_t Aacc = 0;
        int64_t Arej = 0;
        int64_t Astp = 0;
        bool haveB = (n + 1 < NSYS);
        double By1 = 0.0, By2 = 0.0, By3 = 0.0, Bt = 0.0, Bh = H0;
        int64_t Bacc = 0, Brej = 0, Bstp = 0;
        if (haveB) {
            By1 = y0[3 * (n + 1) + 0];
            By2 = y0[3 * (n + 1) + 1];
            By3 = y0[3 * (n + 1) + 2];
        }
        while ((At < t_end && Astp < max_steps) ||
               (haveB && Bt < t_end && Bstp < max_steps)) {
            if (At < t_end && Astp < max_steps) {
                STEP(A);
            }
            if (haveB && Bt < t_end && Bstp < max_steps) {
                STEP(B);
            }
        }
        y[3 * n + 0] = Ay1;
        y[3 * n + 1] = Ay2;
        y[3 * n + 2] = Ay3;
        n_accept[n] = Aacc;
        n_reject[n] = Arej;
        if (haveB) {
            y[3 * (n + 1) + 0] = By1;
            y[3 * (n + 1) + 1] = By2;
            y[3 * (n + 1) + 2] = By3;
            n_accept[n + 1] = Bacc;
            n_reject[n + 1] = Brej;
        }
    }
}
