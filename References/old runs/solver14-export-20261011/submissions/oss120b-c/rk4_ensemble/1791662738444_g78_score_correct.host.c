#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <omp.h>

/*
 * Runge-Kutta 4th order integration for NSYS independent 3-species Brusselator ODEs.
 * Parameters:
 *   y  - output array of size NSYS*3 (final state)
 *   y0 - input array of size NSYS*3 (initial state)
 *   NSTEPS - number of time steps to integrate
 *   NSYS - number of independent systems (size of y0 and y divided by 3)
 *   a, b, ep - model parameters
 *   t_end - final time (integration interval [0, t_end])
 *   workspace, workspace_size - unused (required by signature)
 */

void rk4_ensemble_fp64(double *restrict y,
                        const double *restrict y0,
                        const int64_t NSTEPS,
                        const int64_t NSYS,
                        const double a,
                        const double b,
                        const double ep,
                        const double t_end,
                        uint8_t *restrict workspace,
                        const int64_t workspace_size) {
    // Time step size
    const double h = t_end / (double)NSTEPS;
    // No need for workspace in this implementation
    (void)workspace;
    (void)workspace_size;

    // Parallelize over the independent systems.
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; ++n) {
        // Load initial conditions for system n.
        double u = y0[3 * n];
        double v = y0[3 * n + 1];
        double w = y0[3 * n + 2];

        // RK4 stages.
        double k1u, k1v, k1w;
        double k2u, k2v, k2w;
        double k3u, k3v, k3w;
        double k4u, k4v, k4w;

        for (int64_t step = 0; step < NSTEPS; ++step) {
            // Stage 1: evaluate RHS at (u, v, w)
            k1u = a - (w + 1.0) * u + v * u * u;
            k1v = w * u - v * u * u;
            k1w = (b - w) / ep - w * u;

            // Stage 2: evaluate at midpoint using k1
            double u_mid = u + 0.5 * h * k1u;
            double v_mid = v + 0.5 * h * k1v;
            double w_mid = w + 0.5 * h * k1w;
            k2u = a - (w_mid + 1.0) * u_mid + v_mid * u_mid * u_mid;
            k2v = w_mid * u_mid - v_mid * u_mid * u_mid;
            k2w = (b - w_mid) / ep - w_mid * u_mid;

            // Stage 3: evaluate at midpoint using k2
            u_mid = u + 0.5 * h * k2u;
            v_mid = v + 0.5 * h * k2v;
            w_mid = w + 0.5 * h * k2w;
            k3u = a - (w_mid + 1.0) * u_mid + v_mid * u_mid * u_mid;
            k3v = w_mid * u_mid - v_mid * u_mid * u_mid;
            k3w = (b - w_mid) / ep - w_mid * u_mid;

            // Stage 4: evaluate at end using k3
            double u_end = u + h * k3u;
            double v_end = v + h * k3v;
            double w_end = w + h * k3w;
            k4u = a - (w_end + 1.0) * u_end + v_end * u_end * u_end;
            k4v = w_end * u_end - v_end * u_end * u_end;
            k4w = (b - w_end) / ep - w_end * u_end;

            // Combine stages to produce next state.
            u += (h / 6.0) * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
            v += (h / 6.0) * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
            w += (h / 6.0) * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
        }

        // Store final state.
        y[3 * n] = u;
        y[3 * n + 1] = v;
        y[3 * n + 2] = w;
    }
}

