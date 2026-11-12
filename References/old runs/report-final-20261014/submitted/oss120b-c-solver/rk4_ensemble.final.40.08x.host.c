#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * RK4 integration of an ensemble of independent 3-species Brusselator ODEs.
 * Each system has state (u, v, w). The RHS is:
 *   du/dt = a - (w+1)*u + v*u*u
 *   dv/dt = w*u - v*u*u
 *   dw/dt = (b-w)/ep - w*u
 *
 * y0 points to input state array of size NSYS*3 (row-major: [system][component]).
 * y points to output array of same size, where final state after NSTEPS steps is stored.
 *
 * The implementation uses a parallel outer loop over systems and a straightforward
 * RK4 step inner loop. All temporaries are kept on the stack to avoid memory traffic.
 */

void rk4_ensemble_fp64(
    double *restrict y,
    const double *restrict y0,
    const int64_t NSTEPS,
    const int64_t NSYS,
    const double a,
    const double b,
    const double ep,
    const double t_end,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    (void)workspace;      // unused
    (void)workspace_size; // unused

    // Time step size; compute once.
    const double h = t_end / (double)NSTEPS;
    const double inv_ep = 1.0 / ep;
    const double h2 = 0.5 * h;
    const double h6 = h / 6.0;

    // Parallelize over independent systems.
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; ++n) {
        // Load initial state for this system.
        const double *y0_ptr = y0 + 3 * n;
        double u = y0_ptr[0];
        double v = y0_ptr[1];
        double w = y0_ptr[2];

        // RK4 integration loop.
        for (int64_t step = 0; step < NSTEPS; ++step) {
            // ---- k1 ----
            double u2 = u * u; // u squared reused
            double k1u = a - (w + 1.0) * u + v * u2;
            double k1v = w * u - v * u2;
            double k1w = (b - w) * inv_ep - w * u;

            // ---- k2 ----
            double u_k2 = u + h2 * k1u;
            double v_k2 = v + h2 * k1v;
            double w_k2 = w + h2 * k1w;
            double u2_k2 = u_k2 * u_k2;
            double k2u = a - (w_k2 + 1.0) * u_k2 + v_k2 * u2_k2;
            double k2v = w_k2 * u_k2 - v_k2 * u2_k2;
            double k2w = (b - w_k2) * inv_ep - w_k2 * u_k2;

            // ---- k3 ----
            double u_k3 = u + h2 * k2u;
            double v_k3 = v + h2 * k2v;
            double w_k3 = w + h2 * k2w;
            double u2_k3 = u_k3 * u_k3;
            double k3u = a - (w_k3 + 1.0) * u_k3 + v_k3 * u2_k3;
            double k3v = w_k3 * u_k3 - v_k3 * u2_k3;
            double k3w = (b - w_k3) * inv_ep - w_k3 * u_k3;

            // ---- k4 ----
            double u_k4 = u + h * k3u;
            double v_k4 = v + h * k3v;
            double w_k4 = w + h * k3w;
            double u2_k4 = u_k4 * u_k4;
            double k4u = a - (w_k4 + 1.0) * u_k4 + v_k4 * u2_k4;
            double k4v = w_k4 * u_k4 - v_k4 * u2_k4;
            double k4w = (b - w_k4) * inv_ep - w_k4 * u_k4;

            // ---- combine ----
            u += h6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
            v += h6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
            w += h6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
        }

        // Store final state.
        double *y_ptr = y + 3 * n;
        y_ptr[0] = u;
        y_ptr[1] = v;
        y_ptr[2] = w;
    }
}

