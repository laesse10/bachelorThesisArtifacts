#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <omp.h>

/*
 * Runge–Kutta 4th‑order integration of an ensemble of independent 3‑species
 * Brusselator ODEs.
 *
 * The function follows the reference implementation in `rk4_ensemble_numpy.py`.
 * It operates on flat arrays `y0` and `y` where each system occupies three
 * consecutive doubles (u, v, w). The result for each system is written to `y`
 * after `NSTEPS` uniform RK4 steps.
 *
 * Parameters:
 *   y               – output buffer (size NSYS*3)
 *   y0              – input buffer with initial values (size NSYS*3)
 *   NSTEPS, NSYS    – number of RK4 steps and number of systems
 *   a, b, ep        – model parameters (constant for all systems)
 *   t_end           – final integration time (step size h = t_end/NSTEPS)
 *   workspace,      – scratch area required by the kernel signature; unused here
 *   workspace_size  – size of the scratch area (bytes)
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
    // Pre‑compute constants for the integration.
    const double h  = t_end / (double)NSTEPS;   // step size
    const double h2 = 0.5 * h;                  // half step size
    const double h6 = h / 6.0;                  // one‑sixth step size
    const double inv_ep = 1.0 / ep;            // replace division by multiplication

    // Silence unused‑parameter warnings.
    (void)workspace;
    (void)workspace_size;

    // Independent ODE systems – perfect candidate for parallelism.
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; ++n) {
        // Load the three state variables for this system.
        double u = y0[3 * n];
        double v = y0[3 * n + 1];
        double w = y0[3 * n + 2];

        // Temporary storage for the four RK4 stages.
        double k1u, k1v, k1w;
        double k2u, k2v, k2w;
        double k3u, k3v, k3w;
        double k4u, k4v, k4w;

        // Main integration loop.
        for (int64_t step = 0; step < NSTEPS; ++step) {
            // ---- Stage 1 ----------------------------------------------------
            double u2 = u * u;  // reuse u*u for two RHS components
            k1u = a - (w + 1.0) * u + v * u2;
            k1v = w * u - v * u2;
            k1w = (b - w) * inv_ep - w * u;

            // ---- Stage 2 ----------------------------------------------------
            double u_mid = u + h2 * k1u;
            double v_mid = v + h2 * k1v;
            double w_mid = w + h2 * k1w;
            double u2_mid = u_mid * u_mid;
            k2u = a - (w_mid + 1.0) * u_mid + v_mid * u2_mid;
            k2v = w_mid * u_mid - v_mid * u2_mid;
            k2w = (b - w_mid) * inv_ep - w_mid * u_mid;

            // ---- Stage 3 ----------------------------------------------------
            u_mid = u + h2 * k2u;
            v_mid = v + h2 * k2v;
            w_mid = w + h2 * k2w;
            u2_mid = u_mid * u_mid;
            k3u = a - (w_mid + 1.0) * u_mid + v_mid * u2_mid;
            k3v = w_mid * u_mid - v_mid * u2_mid;
            k3w = (b - w_mid) * inv_ep - w_mid * u_mid;

            // ---- Stage 4 ----------------------------------------------------
            double u_end = u + h * k3u;
            double v_end = v + h * k3v;
            double w_end = w + h * k3w;
            double u2_end = u_end * u_end;
            k4u = a - (w_end + 1.0) * u_end + v_end * u2_end;
            k4v = w_end * u_end - v_end * u2_end;
            k4w = (b - w_end) * inv_ep - w_end * u_end;

            // ---- Combine stages ------------------------------------------------
            u += h6 * (k1u + 2.0 * k2u + 2.0 * k3u + k4u);
            v += h6 * (k1v + 2.0 * k2v + 2.0 * k3v + k4v);
            w += h6 * (k1w + 2.0 * k2w + 2.0 * k3w + k4w);
        }

        // Write the final state back to the output buffer.
        y[3 * n] = u;
        y[3 * n + 1] = v;
        y[3 * n + 2] = w;
    }
}
