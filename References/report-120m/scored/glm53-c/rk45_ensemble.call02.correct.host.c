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

static inline void robertson_rhs(double y1, double y2, double y3,
                                 double *d0, double *d1, double *d2) {
    *d0 = -0.04 * y1 + 1.0e4 * y2 * y3;
    *d1 = 0.04 * y1 - 1.0e4 * y2 * y3 - 3.0e7 * y2 * y2;
    *d2 = 3.0e7 * y2 * y2;
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

    const int64_t N = NSYS;
    #pragma omp parallel for schedule(static)
    for (int64_t n2 = 0; n2 < (N / 2) * 2; n2 += 2) {
        double A_y1 = y0[3*n2+0], A_y2 = y0[3*n2+1], A_y3 = y0[3*n2+2];
        double B_y1 = y0[3*n2+3], B_y2 = y0[3*n2+4], B_y3 = y0[3*n2+5];
        double A_t = 0.0, A_h = H0;
        double B_t = 0.0, B_h = H0;
        int64_t A_accepts = 0, A_rejects = 0, A_steps = 0;
        int64_t B_accepts = 0, B_rejects = 0, B_steps = 0;
        double A_k1[3], A_k2[3], A_k3[3], A_k4[3], A_k5[3], A_k6[3], A_k7[3];
        double B_k1[3], B_k2[3], B_k3[3], B_k4[3], B_k5[3], B_k6[3], B_k7[3];
        int A_act = (A_t < t_end && A_steps < MAX_STEPS);
        int B_act = (B_t < t_end && B_steps < MAX_STEPS);
        while (A_act || B_act) {
            if (A_act) {
            if (A_t + A_h > t_end) {
                A_h = t_end - A_t;
            }
            robertson_rhs(A_y1, A_y2, A_y3, &A_k1[0], &A_k1[1], &A_k1[2]);
            robertson_rhs(
                A_y1 + A_h * (1.0 / 5.0) * A_k1[0],
                A_y2 + A_h * (1.0 / 5.0) * A_k1[1],
                A_y3 + A_h * (1.0 / 5.0) * A_k1[2],
                &A_k2[0], &A_k2[1], &A_k2[2]);
            robertson_rhs(
                A_y1 + A_h * (3.0 / 40.0 * A_k1[0] + 9.0 / 40.0 * A_k2[0]),
                A_y2 + A_h * (3.0 / 40.0 * A_k1[1] + 9.0 / 40.0 * A_k2[1]),
                A_y3 + A_h * (3.0 / 40.0 * A_k1[2] + 9.0 / 40.0 * A_k2[2]),
                &A_k3[0], &A_k3[1], &A_k3[2]);
            robertson_rhs(
                A_y1 + A_h * (44.0 / 45.0 * A_k1[0] - 56.0 / 15.0 * A_k2[0] + 32.0 / 9.0 * A_k3[0]),
                A_y2 + A_h * (44.0 / 45.0 * A_k1[1] - 56.0 / 15.0 * A_k2[1] + 32.0 / 9.0 * A_k3[1]),
                A_y3 + A_h * (44.0 / 45.0 * A_k1[2] - 56.0 / 15.0 * A_k2[2] + 32.0 / 9.0 * A_k3[2]),
                &A_k4[0], &A_k4[1], &A_k4[2]);
            robertson_rhs(
                A_y1 + A_h * (19372.0 / 6561.0 * A_k1[0] - 25360.0 / 2187.0 * A_k2[0] + 64448.0 / 6561.0 * A_k3[0] - 212.0 / 729.0 * A_k4[0]),
                A_y2 + A_h * (19372.0 / 6561.0 * A_k1[1] - 25360.0 / 2187.0 * A_k2[1] + 64448.0 / 6561.0 * A_k3[1] - 212.0 / 729.0 * A_k4[1]),
                A_y3 + A_h * (19372.0 / 6561.0 * A_k1[2] - 25360.0 / 2187.0 * A_k2[2] + 64448.0 / 6561.0 * A_k3[2] - 212.0 / 729.0 * A_k4[2]),
                &A_k5[0], &A_k5[1], &A_k5[2]);
            robertson_rhs(
                A_y1 + A_h * (9017.0 / 3168.0 * A_k1[0] - 355.0 / 33.0 * A_k2[0] + 46732.0 / 5247.0 * A_k3[0] + 49.0 / 176.0 * A_k4[0] - 5103.0 / 18656.0 * A_k5[0]),
                A_y2 + A_h * (9017.0 / 3168.0 * A_k1[1] - 355.0 / 33.0 * A_k2[1] + 46732.0 / 5247.0 * A_k3[1] + 49.0 / 176.0 * A_k4[1] - 5103.0 / 18656.0 * A_k5[1]),
                A_y3 + A_h * (9017.0 / 3168.0 * A_k1[2] - 355.0 / 33.0 * A_k2[2] + 46732.0 / 5247.0 * A_k3[2] + 49.0 / 176.0 * A_k4[2] - 5103.0 / 18656.0 * A_k5[2]),
                &A_k6[0], &A_k6[1], &A_k6[2]);
            double s1 = A_y1 + A_h * (35.0 / 384.0 * A_k1[0] + 500.0 / 1113.0 * A_k3[0] + 125.0 / 192.0 * A_k4[0] - 2187.0 / 6784.0 * A_k5[0] + 11.0 / 84.0 * A_k6[0]);
            double s2 = A_y2 + A_h * (35.0 / 384.0 * A_k1[1] + 500.0 / 1113.0 * A_k3[1] + 125.0 / 192.0 * A_k4[1] - 2187.0 / 6784.0 * A_k5[1] + 11.0 / 84.0 * A_k6[1]);
            double s3 = A_y3 + A_h * (35.0 / 384.0 * A_k1[2] + 500.0 / 1113.0 * A_k3[2] + 125.0 / 192.0 * A_k4[2] - 2187.0 / 6784.0 * A_k5[2] + 11.0 / 84.0 * A_k6[2]);
            robertson_rhs(s1, s2, s3, &A_k7[0], &A_k7[1], &A_k7[2]);
            double y4_1 = A_y1 + A_h * (5179.0 / 57600.0 * A_k1[0] + 7571.0 / 16695.0 * A_k3[0] + 393.0 / 640.0 * A_k4[0] - 92097.0 / 339200.0 * A_k5[0] + 187.0 / 2100.0 * A_k6[0] + 1.0 / 40.0 * A_k7[0]);
            double y4_2 = A_y2 + A_h * (5179.0 / 57600.0 * A_k1[1] + 7571.0 / 16695.0 * A_k3[1] + 393.0 / 640.0 * A_k4[1] - 92097.0 / 339200.0 * A_k5[1] + 187.0 / 2100.0 * A_k6[1] + 1.0 / 40.0 * A_k7[1]);
            double y4_3 = A_y3 + A_h * (5179.0 / 57600.0 * A_k1[2] + 7571.0 / 16695.0 * A_k3[2] + 393.0 / 640.0 * A_k4[2] - 92097.0 / 339200.0 * A_k5[2] + 187.0 / 2100.0 * A_k6[2] + 1.0 / 40.0 * A_k7[2]);
            double e1 = s1 - y4_1;
            double e2 = s2 - y4_2;
            double e3 = s3 - y4_3;
            double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
            double ynorm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);
            double err = enorm / (ATOL + RTOL * ynorm);
            A_steps = A_steps + 1;
            if (err <= 1.0) {
                A_y1 = s1;
                A_y2 = s2;
                A_y3 = s3;
                A_t = A_t + A_h;
                A_accepts = A_accepts + 1;
            } else {
                A_rejects = A_rejects + 1;
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
            A_h = A_h * factor;
                A_act = (A_t < t_end && A_steps < MAX_STEPS);
            }
            if (B_act) {
            if (B_t + B_h > t_end) {
                B_h = t_end - B_t;
            }
            robertson_rhs(B_y1, B_y2, B_y3, &B_k1[0], &B_k1[1], &B_k1[2]);
            robertson_rhs(
                B_y1 + B_h * (1.0 / 5.0) * B_k1[0],
                B_y2 + B_h * (1.0 / 5.0) * B_k1[1],
                B_y3 + B_h * (1.0 / 5.0) * B_k1[2],
                &B_k2[0], &B_k2[1], &B_k2[2]);
            robertson_rhs(
                B_y1 + B_h * (3.0 / 40.0 * B_k1[0] + 9.0 / 40.0 * B_k2[0]),
                B_y2 + B_h * (3.0 / 40.0 * B_k1[1] + 9.0 / 40.0 * B_k2[1]),
                B_y3 + B_h * (3.0 / 40.0 * B_k1[2] + 9.0 / 40.0 * B_k2[2]),
                &B_k3[0], &B_k3[1], &B_k3[2]);
            robertson_rhs(
                B_y1 + B_h * (44.0 / 45.0 * B_k1[0] - 56.0 / 15.0 * B_k2[0] + 32.0 / 9.0 * B_k3[0]),
                B_y2 + B_h * (44.0 / 45.0 * B_k1[1] - 56.0 / 15.0 * B_k2[1] + 32.0 / 9.0 * B_k3[1]),
                B_y3 + B_h * (44.0 / 45.0 * B_k1[2] - 56.0 / 15.0 * B_k2[2] + 32.0 / 9.0 * B_k3[2]),
                &B_k4[0], &B_k4[1], &B_k4[2]);
            robertson_rhs(
                B_y1 + B_h * (19372.0 / 6561.0 * B_k1[0] - 25360.0 / 2187.0 * B_k2[0] + 64448.0 / 6561.0 * B_k3[0] - 212.0 / 729.0 * B_k4[0]),
                B_y2 + B_h * (19372.0 / 6561.0 * B_k1[1] - 25360.0 / 2187.0 * B_k2[1] + 64448.0 / 6561.0 * B_k3[1] - 212.0 / 729.0 * B_k4[1]),
                B_y3 + B_h * (19372.0 / 6561.0 * B_k1[2] - 25360.0 / 2187.0 * B_k2[2] + 64448.0 / 6561.0 * B_k3[2] - 212.0 / 729.0 * B_k4[2]),
                &B_k5[0], &B_k5[1], &B_k5[2]);
            robertson_rhs(
                B_y1 + B_h * (9017.0 / 3168.0 * B_k1[0] - 355.0 / 33.0 * B_k2[0] + 46732.0 / 5247.0 * B_k3[0] + 49.0 / 176.0 * B_k4[0] - 5103.0 / 18656.0 * B_k5[0]),
                B_y2 + B_h * (9017.0 / 3168.0 * B_k1[1] - 355.0 / 33.0 * B_k2[1] + 46732.0 / 5247.0 * B_k3[1] + 49.0 / 176.0 * B_k4[1] - 5103.0 / 18656.0 * B_k5[1]),
                B_y3 + B_h * (9017.0 / 3168.0 * B_k1[2] - 355.0 / 33.0 * B_k2[2] + 46732.0 / 5247.0 * B_k3[2] + 49.0 / 176.0 * B_k4[2] - 5103.0 / 18656.0 * B_k5[2]),
                &B_k6[0], &B_k6[1], &B_k6[2]);
            double s1 = B_y1 + B_h * (35.0 / 384.0 * B_k1[0] + 500.0 / 1113.0 * B_k3[0] + 125.0 / 192.0 * B_k4[0] - 2187.0 / 6784.0 * B_k5[0] + 11.0 / 84.0 * B_k6[0]);
            double s2 = B_y2 + B_h * (35.0 / 384.0 * B_k1[1] + 500.0 / 1113.0 * B_k3[1] + 125.0 / 192.0 * B_k4[1] - 2187.0 / 6784.0 * B_k5[1] + 11.0 / 84.0 * B_k6[1]);
            double s3 = B_y3 + B_h * (35.0 / 384.0 * B_k1[2] + 500.0 / 1113.0 * B_k3[2] + 125.0 / 192.0 * B_k4[2] - 2187.0 / 6784.0 * B_k5[2] + 11.0 / 84.0 * B_k6[2]);
            robertson_rhs(s1, s2, s3, &B_k7[0], &B_k7[1], &B_k7[2]);
            double y4_1 = B_y1 + B_h * (5179.0 / 57600.0 * B_k1[0] + 7571.0 / 16695.0 * B_k3[0] + 393.0 / 640.0 * B_k4[0] - 92097.0 / 339200.0 * B_k5[0] + 187.0 / 2100.0 * B_k6[0] + 1.0 / 40.0 * B_k7[0]);
            double y4_2 = B_y2 + B_h * (5179.0 / 57600.0 * B_k1[1] + 7571.0 / 16695.0 * B_k3[1] + 393.0 / 640.0 * B_k4[1] - 92097.0 / 339200.0 * B_k5[1] + 187.0 / 2100.0 * B_k6[1] + 1.0 / 40.0 * B_k7[1]);
            double y4_3 = B_y3 + B_h * (5179.0 / 57600.0 * B_k1[2] + 7571.0 / 16695.0 * B_k3[2] + 393.0 / 640.0 * B_k4[2] - 92097.0 / 339200.0 * B_k5[2] + 187.0 / 2100.0 * B_k6[2] + 1.0 / 40.0 * B_k7[2]);
            double e1 = s1 - y4_1;
            double e2 = s2 - y4_2;
            double e3 = s3 - y4_3;
            double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
            double ynorm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);
            double err = enorm / (ATOL + RTOL * ynorm);
            B_steps = B_steps + 1;
            if (err <= 1.0) {
                B_y1 = s1;
                B_y2 = s2;
                B_y3 = s3;
                B_t = B_t + B_h;
                B_accepts = B_accepts + 1;
            } else {
                B_rejects = B_rejects + 1;
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
            B_h = B_h * factor;
                B_act = (B_t < t_end && B_steps < MAX_STEPS);
            }
        }
        y[3*n2+0] = A_y1; y[3*n2+1] = A_y2; y[3*n2+2] = A_y3;
        y[3*n2+3] = B_y1; y[3*n2+4] = B_y2; y[3*n2+5] = B_y3;
        n_accept[n2+0] = A_accepts; n_reject[n2+0] = A_rejects;
        n_accept[n2+1] = B_accepts; n_reject[n2+1] = B_rejects;
    }
    for (int64_t n2 = (N / 2) * 2; n2 < N; n2++) {
        double A_y1 = y0[3*n2], A_y2 = y0[3*n2+1], A_y3 = y0[3*n2+2];
        double A_t = 0.0, A_h = H0;
        int64_t A_accepts = 0, A_rejects = 0, A_steps = 0;
        double A_k1[3], A_k2[3], A_k3[3], A_k4[3], A_k5[3], A_k6[3], A_k7[3];
        int A_act = (A_t < t_end && A_steps < MAX_STEPS);
        while (A_act) {
            if (A_t + A_h > t_end) {
                A_h = t_end - A_t;
            }
            robertson_rhs(A_y1, A_y2, A_y3, &A_k1[0], &A_k1[1], &A_k1[2]);
            robertson_rhs(
                A_y1 + A_h * (1.0 / 5.0) * A_k1[0],
                A_y2 + A_h * (1.0 / 5.0) * A_k1[1],
                A_y3 + A_h * (1.0 / 5.0) * A_k1[2],
                &A_k2[0], &A_k2[1], &A_k2[2]);
            robertson_rhs(
                A_y1 + A_h * (3.0 / 40.0 * A_k1[0] + 9.0 / 40.0 * A_k2[0]),
                A_y2 + A_h * (3.0 / 40.0 * A_k1[1] + 9.0 / 40.0 * A_k2[1]),
                A_y3 + A_h * (3.0 / 40.0 * A_k1[2] + 9.0 / 40.0 * A_k2[2]),
                &A_k3[0], &A_k3[1], &A_k3[2]);
            robertson_rhs(
                A_y1 + A_h * (44.0 / 45.0 * A_k1[0] - 56.0 / 15.0 * A_k2[0] + 32.0 / 9.0 * A_k3[0]),
                A_y2 + A_h * (44.0 / 45.0 * A_k1[1] - 56.0 / 15.0 * A_k2[1] + 32.0 / 9.0 * A_k3[1]),
                A_y3 + A_h * (44.0 / 45.0 * A_k1[2] - 56.0 / 15.0 * A_k2[2] + 32.0 / 9.0 * A_k3[2]),
                &A_k4[0], &A_k4[1], &A_k4[2]);
            robertson_rhs(
                A_y1 + A_h * (19372.0 / 6561.0 * A_k1[0] - 25360.0 / 2187.0 * A_k2[0] + 64448.0 / 6561.0 * A_k3[0] - 212.0 / 729.0 * A_k4[0]),
                A_y2 + A_h * (19372.0 / 6561.0 * A_k1[1] - 25360.0 / 2187.0 * A_k2[1] + 64448.0 / 6561.0 * A_k3[1] - 212.0 / 729.0 * A_k4[1]),
                A_y3 + A_h * (19372.0 / 6561.0 * A_k1[2] - 25360.0 / 2187.0 * A_k2[2] + 64448.0 / 6561.0 * A_k3[2] - 212.0 / 729.0 * A_k4[2]),
                &A_k5[0], &A_k5[1], &A_k5[2]);
            robertson_rhs(
                A_y1 + A_h * (9017.0 / 3168.0 * A_k1[0] - 355.0 / 33.0 * A_k2[0] + 46732.0 / 5247.0 * A_k3[0] + 49.0 / 176.0 * A_k4[0] - 5103.0 / 18656.0 * A_k5[0]),
                A_y2 + A_h * (9017.0 / 3168.0 * A_k1[1] - 355.0 / 33.0 * A_k2[1] + 46732.0 / 5247.0 * A_k3[1] + 49.0 / 176.0 * A_k4[1] - 5103.0 / 18656.0 * A_k5[1]),
                A_y3 + A_h * (9017.0 / 3168.0 * A_k1[2] - 355.0 / 33.0 * A_k2[2] + 46732.0 / 5247.0 * A_k3[2] + 49.0 / 176.0 * A_k4[2] - 5103.0 / 18656.0 * A_k5[2]),
                &A_k6[0], &A_k6[1], &A_k6[2]);
            double s1 = A_y1 + A_h * (35.0 / 384.0 * A_k1[0] + 500.0 / 1113.0 * A_k3[0] + 125.0 / 192.0 * A_k4[0] - 2187.0 / 6784.0 * A_k5[0] + 11.0 / 84.0 * A_k6[0]);
            double s2 = A_y2 + A_h * (35.0 / 384.0 * A_k1[1] + 500.0 / 1113.0 * A_k3[1] + 125.0 / 192.0 * A_k4[1] - 2187.0 / 6784.0 * A_k5[1] + 11.0 / 84.0 * A_k6[1]);
            double s3 = A_y3 + A_h * (35.0 / 384.0 * A_k1[2] + 500.0 / 1113.0 * A_k3[2] + 125.0 / 192.0 * A_k4[2] - 2187.0 / 6784.0 * A_k5[2] + 11.0 / 84.0 * A_k6[2]);
            robertson_rhs(s1, s2, s3, &A_k7[0], &A_k7[1], &A_k7[2]);
            double y4_1 = A_y1 + A_h * (5179.0 / 57600.0 * A_k1[0] + 7571.0 / 16695.0 * A_k3[0] + 393.0 / 640.0 * A_k4[0] - 92097.0 / 339200.0 * A_k5[0] + 187.0 / 2100.0 * A_k6[0] + 1.0 / 40.0 * A_k7[0]);
            double y4_2 = A_y2 + A_h * (5179.0 / 57600.0 * A_k1[1] + 7571.0 / 16695.0 * A_k3[1] + 393.0 / 640.0 * A_k4[1] - 92097.0 / 339200.0 * A_k5[1] + 187.0 / 2100.0 * A_k6[1] + 1.0 / 40.0 * A_k7[1]);
            double y4_3 = A_y3 + A_h * (5179.0 / 57600.0 * A_k1[2] + 7571.0 / 16695.0 * A_k3[2] + 393.0 / 640.0 * A_k4[2] - 92097.0 / 339200.0 * A_k5[2] + 187.0 / 2100.0 * A_k6[2] + 1.0 / 40.0 * A_k7[2]);
            double e1 = s1 - y4_1;
            double e2 = s2 - y4_2;
            double e3 = s3 - y4_3;
            double enorm = sqrt(e1 * e1 + e2 * e2 + e3 * e3);
            double ynorm = sqrt(s1 * s1 + s2 * s2 + s3 * s3);
            double err = enorm / (ATOL + RTOL * ynorm);
            A_steps = A_steps + 1;
            if (err <= 1.0) {
                A_y1 = s1;
                A_y2 = s2;
                A_y3 = s3;
                A_t = A_t + A_h;
                A_accepts = A_accepts + 1;
            } else {
                A_rejects = A_rejects + 1;
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
            A_h = A_h * factor;
            A_act = (A_t < t_end && A_steps < MAX_STEPS);
        }
        y[3*n2] = A_y1; y[3*n2+1] = A_y2; y[3*n2+2] = A_y3;
        n_accept[n2] = A_accepts; n_reject[n2] = A_rejects;
    }
}
