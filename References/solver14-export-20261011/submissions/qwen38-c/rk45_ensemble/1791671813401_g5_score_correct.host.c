
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

typedef struct {
    double y1, y2, y3, t, h;
    int64_t accepts, rejects, steps;
    int have_k1;
    double k1a,k1b,k1c, k2a,k2b,k2c, k3a,k3b,k3c,
           k4a,k4b,k4c, k5a,k5b,k5c, k6a,k6b,k6c, k7a,k7b,k7c;
} Sys;
static inline __attribute__((always_inline)) void step_rk45(Sys *S, double t_end) {

    if (S->t + S->h > t_end) S->h = t_end - S->t;
    if (!S->have_k1) {
        double a=S->y1, b=S->y2, c=S->y3;
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k1a=(-0.04*a)+t2; S->k1b=(0.04*a)-t2-t4; S->k1c=t4;
    }
    {
        double a=S->y1+S->h*(1.0/5.0)*S->k1a, b=S->y2+S->h*(1.0/5.0)*S->k1b, c=S->y3+S->h*(1.0/5.0)*S->k1c;
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k2a=(-0.04*a)+t2; S->k2b=(0.04*a)-t2-t4; S->k2c=t4;
    }
    {
        double a=S->y1+S->h*(3.0/40.0*S->k1a + 9.0/40.0*S->k2a),
               b=S->y2+S->h*(3.0/40.0*S->k1b + 9.0/40.0*S->k2b),
               c=S->y3+S->h*(3.0/40.0*S->k1c + 9.0/40.0*S->k2c);
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k3a=(-0.04*a)+t2; S->k3b=(0.04*a)-t2-t4; S->k3c=t4;
    }
    {
        double a=S->y1+S->h*(44.0/45.0*S->k1a - 56.0/15.0*S->k2a + 32.0/9.0*S->k3a),
               b=S->y2+S->h*(44.0/45.0*S->k1b - 56.0/15.0*S->k2b + 32.0/9.0*S->k3b),
               c=S->y3+S->h*(44.0/45.0*S->k1c - 56.0/15.0*S->k2c + 32.0/9.0*S->k3c);
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k4a=(-0.04*a)+t2; S->k4b=(0.04*a)-t2-t4; S->k4c=t4;
    }
    {
        double a=S->y1+S->h*(19372.0/6561.0*S->k1a - 25360.0/2187.0*S->k2a + 64448.0/6561.0*S->k3a - 212.0/729.0*S->k4a),
               b=S->y2+S->h*(19372.0/6561.0*S->k1b - 25360.0/2187.0*S->k2b + 64448.0/6561.0*S->k3b - 212.0/729.0*S->k4b),
               c=S->y3+S->h*(19372.0/6561.0*S->k1c - 25360.0/2187.0*S->k2c + 64448.0/6561.0*S->k3c - 212.0/729.0*S->k4c);
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k5a=(-0.04*a)+t2; S->k5b=(0.04*a)-t2-t4; S->k5c=t4;
    }
    {
        double a=S->y1+S->h*(9017.0/3168.0*S->k1a - 355.0/33.0*S->k2a + 46732.0/5247.0*S->k3a + 49.0/176.0*S->k4a - 5103.0/18656.0*S->k5a),
               b=S->y2+S->h*(9017.0/3168.0*S->k1b - 355.0/33.0*S->k2b + 46732.0/5247.0*S->k3b + 49.0/176.0*S->k4b - 5103.0/18656.0*S->k5b),
               c=S->y3+S->h*(9017.0/3168.0*S->k1c - 355.0/33.0*S->k2c + 46732.0/5247.0*S->k3c + 49.0/176.0*S->k4c - 5103.0/18656.0*S->k5c);
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k6a=(-0.04*a)+t2; S->k6b=(0.04*a)-t2-t4; S->k6c=t4;
    }
    double s1 = S->y1 + S->h*(35.0/384.0*S->k1a + 500.0/1113.0*S->k3a + 125.0/192.0*S->k4a - 2187.0/6784.0*S->k5a + 11.0/84.0*S->k6a);
    double s2 = S->y2 + S->h*(35.0/384.0*S->k1b + 500.0/1113.0*S->k3b + 125.0/192.0*S->k4b - 2187.0/6784.0*S->k5b + 11.0/84.0*S->k6b);
    double s3 = S->y3 + S->h*(35.0/384.0*S->k1c + 500.0/1113.0*S->k3c + 125.0/192.0*S->k4c - 2187.0/6784.0*S->k5c + 11.0/84.0*S->k6c);
    {
        double a=s1, b=s2, c=s3;
        double t2=(1.0e4*b)*c, t4=(3.0e7*b)*b;
        S->k7a=(-0.04*a)+t2; S->k7b=(0.04*a)-t2-t4; S->k7c=t4;
    }
    double y4_1 = S->y1 + S->h*(5179.0/57600.0*S->k1a + 7571.0/16695.0*S->k3a + 393.0/640.0*S->k4a - 92097.0/339200.0*S->k5a + 187.0/2100.0*S->k6a + 1.0/40.0*S->k7a);
    double y4_2 = S->y2 + S->h*(5179.0/57600.0*S->k1b + 7571.0/16695.0*S->k3b + 393.0/640.0*S->k4b - 92097.0/339200.0*S->k5b + 187.0/2100.0*S->k6b + 1.0/40.0*S->k7b);
    double y4_3 = S->y3 + S->h*(5179.0/57600.0*S->k1c + 7571.0/16695.0*S->k3c + 393.0/640.0*S->k4c - 92097.0/339200.0*S->k5c + 187.0/2100.0*S->k6c + 1.0/40.0*S->k7c);
    double e1 = s1 - y4_1, e2 = s2 - y4_2, e3 = s3 - y4_3;
    double enorm = sqrt(e1*e1 + e2*e2 + e3*e3);
    double ynorm = sqrt(s1*s1 + s2*s2 + s3*s3);
    double err = enorm / (ATOL + RTOL * ynorm);
    S->steps++;
    if (err <= 1.0) { S->y1 = s1; S->y2 = s2; S->y3 = s3; S->t += S->h; S->accepts++; S->k1a = S->k7a; S->k1b = S->k7b; S->k1c = S->k7c; S->have_k1 = 1; }
    else { S->rejects++; S->have_k1 = 0; }
    double factor;
    if (err > 0.0) {
        factor = 0.9 * pow(err, -0.2);
        if (factor > 5.0) factor = 5.0;
        if (factor < 0.2) factor = 0.2;
    } else factor = 5.0;
    S->h *= factor;
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
    for (int64_t n0 = 0; n0 < NSYS; n0 += 3) {
        int64_t m0 = n0 + 0;
        int64_t m1 = n0 + 1;
        int64_t m2 = n0 + 2;
        Sys A, B, C;
        A.y1 = 0.0; A.y2 = 0.0; A.y3 = 0.0; A.t = 1.0; A.h = H0; A.accepts = 0; A.rejects = 0; A.steps = 0; A.have_k1 = 0;
        B.y1 = 0.0; B.y2 = 0.0; B.y3 = 0.0; B.t = 1.0; B.h = H0; B.accepts = 0; B.rejects = 0; B.steps = 0; B.have_k1 = 0;
        C.y1 = 0.0; C.y2 = 0.0; C.y3 = 0.0; C.t = 1.0; C.h = H0; C.accepts = 0; C.rejects = 0; C.steps = 0; C.have_k1 = 0;
        if (m0 < NSYS) {
            A.y1 = y0[m0*3+0]; A.y2 = y0[m0*3+1]; A.y3 = y0[m0*3+2];
            A.t = 0.0;
        }
        if (m1 < NSYS) {
            B.y1 = y0[m1*3+0]; B.y2 = y0[m1*3+1]; B.y3 = y0[m1*3+2];
            B.t = 0.0;
        }
        if (m2 < NSYS) {
            C.y1 = y0[m2*3+0]; C.y2 = y0[m2*3+1]; C.y3 = y0[m2*3+2];
            C.t = 0.0;
        }
        while ((A.t < t_end && A.steps < MAX_STEPS) || (B.t < t_end && B.steps < MAX_STEPS) || (C.t < t_end && C.steps < MAX_STEPS)) {
        if (A.t < t_end && A.steps < MAX_STEPS) step_rk45(&A, t_end);
        if (B.t < t_end && B.steps < MAX_STEPS) step_rk45(&B, t_end);
        if (C.t < t_end && C.steps < MAX_STEPS) step_rk45(&C, t_end);
        }
        if (m0 < NSYS) {
            y[m0*3+0] = A.y1; y[m0*3+1] = A.y2; y[m0*3+2] = A.y3;
            n_accept[m0] = A.accepts; n_reject[m0] = A.rejects;
        }
        if (m1 < NSYS) {
            y[m1*3+0] = B.y1; y[m1*3+1] = B.y2; y[m1*3+2] = B.y3;
            n_accept[m1] = B.accepts; n_reject[m1] = B.rejects;
        }
        if (m2 < NSYS) {
            y[m2*3+0] = C.y1; y[m2*3+1] = C.y2; y[m2*3+2] = C.y3;
            n_accept[m2] = C.accepts; n_reject[m2] = C.rejects;
        }
    }
}
