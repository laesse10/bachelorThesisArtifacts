// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/*
 * Fixed-step RK4 integration for a large ensemble of independent 3-species
 * Brusselator ODEs.
 *
 * For each system n (0 <= n < NSYS) we integrate the ODE system:
 *   du/dt = a - (w+1)*u + v*u*u
 *   dv/dt = w*u - v*u*u
 *   dw/dt = (b - w)/ep - w*u
 * from initial condition (y0[n,0], y0[n,1], y0[n,2]) over NSTEPS uniform steps.
 * The final state is stored in y[n,0..2].
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
    // Workspace not needed in this implementation.
    (void)workspace;
    (void)workspace_size;

    const double h = t_end / (double)NSTEPS;
    const double inv_ep = 1.0 / ep;
    const double half_h = 0.5 * h;
    const double sixth_h = h / 6.0;

    // Parallel over the ensemble. Each iteration works on a single independent system.
    #pragma omp parallel for schedule(static)
    for (int64_t n = 0; n < NSYS; ++n) {
        // Load initial conditions.
        const double *in = y0 + n * 3;
        double u = in[0];
        double v = in[1];
        double w = in[2];

        // Temporary variables for RK4 stages.
        double k1_u, k1_v, k1_w;
        double k2_u, k2_v, k2_w;
        double k3_u, k3_v, k3_w;
        double k4_u, k4_v, k4_w;

        for (int64_t step = 0; step < NSTEPS; ++step) {
            // ---- k1 ----
            {
                const double u2 = u * u;
                const double vu2 = v * u2;
                k1_u = a - (w + 1.0) * u + vu2;
                k1_v = w * u - vu2;
                k1_w = (b - w) * inv_ep - w * u;
            }

            // ---- k2 ----
            {
                const double u_mid = u + half_h * k1_u;
                const double v_mid = v + half_h * k1_v;
                const double w_mid = w + half_h * k1_w;
                const double u2 = u_mid * u_mid;
                const double vu2 = v_mid * u2;
                k2_u = a - (w_mid + 1.0) * u_mid + vu2;
                k2_v = w_mid * u_mid - vu2;
                k2_w = (b - w_mid) * inv_ep - w_mid * u_mid;
            }

            // ---- k3 ----
            {
                const double u_mid = u + half_h * k2_u;
                const double v_mid = v + half_h * k2_v;
                const double w_mid = w + half_h * k2_w;
                const double u2 = u_mid * u_mid;
                const double vu2 = v_mid * u2;
                k3_u = a - (w_mid + 1.0) * u_mid + vu2;
                k3_v = w_mid * u_mid - vu2;
                k3_w = (b - w_mid) * inv_ep - w_mid * u_mid;
            }

            // ---- k4 ----
            {
                const double u_end = u + h * k3_u;
                const double v_end = v + h * k3_v;
                const double w_end = w + h * k3_w;
                const double u2 = u_end * u_end;
                const double vu2 = v_end * u2;
                k4_u = a - (w_end + 1.0) * u_end + vu2;
                k4_v = w_end * u_end - vu2;
                k4_w = (b - w_end) * inv_ep - w_end * u_end;
            }

            // Update state
            u += sixth_h * (k1_u + 2.0 * k2_u + 2.0 * k3_u + k4_u);
            v += sixth_h * (k1_v + 2.0 * k2_v + 2.0 * k3_v + k4_v);
            w += sixth_h * (k1_w + 2.0 * k2_w + 2.0 * k3_w + k4_w);
        }

        // Store final state
        double *out = y + n * 3;
        out[0] = u;
        out[1] = v;
        out[2] = w;
    }
}

