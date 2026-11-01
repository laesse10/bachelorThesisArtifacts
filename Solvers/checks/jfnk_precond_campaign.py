"""The fast-Poisson change to jfnk_bratu against the reference the skill campaign graded with.

jfnk_precond_across_sizes.py (Table C.2 before 2026-11-01) simulated the reference with a Newton
tolerance of 1e-10 and judged with the fp64 band. The campaign's checkout (083c88e) differs in three
ways: the reference iterates Newton to 1e-14 (since 44dd51b9c, 2026-09-09; in practice it runs to its
20-step cap), the manifest grades u at conditioning_rtol = conditioning_atol = 1e-5 (since 56844a509
and d6d8bd25e, 2026-10-08/10), and held-out sizes are drawn from 8 <= N <= 704 (1024 at 63c34c8b1,
the v1 campaign). This runs the benchmark's own reference and the same solver with a DST
fast-Poisson right preconditioner on the inner GMRES, and gives the difference in both bands.

Needs the reference: export HPCAGENT_BENCH=<HPCAgent-Bench checkout at 083c88e>.
"""
import importlib.util
import os
import sys

import numpy as np
from scipy.fft import dstn, idstn

PATH = "hpcagent_bench/benchmarks/scientific_computing/sparse_linear_algebra/jfnk_bratu/jfnk_bratu_numpy.py"
spec = importlib.util.spec_from_file_location("ref", os.path.join(os.environ["HPCAGENT_BENCH"], PATH))
ref = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ref)

MAX_NEWTON, INNER_TOL, RESTART, NEWTON_RTOL = 20, 1e-4, 50, 1e-14  # jfnk_bratu.yaml / _numpy.py at 083c88e
BANDS = {"fp64": (1e-9, 1e-11), "campaign": (1e-5, 1e-5)}


def budget(a, e, band):
    rtol, atol = BANDS[band]
    atol = max(atol, np.finfo(np.float64).eps * float(np.max(np.abs(e))))  # l = 1: u is elementwise in u0
    return float(np.max(np.abs(a - e) / (atol + rtol * np.abs(e))))


def fast_poisson(N):
    """DST-I solve on the interior: the exact inverse of the 5-point Dirichlet Laplacian."""
    h = 1.0 / (N - 1); k = np.arange(1, N - 1)
    eig = (2 - 2 * np.cos(np.pi * k / (N - 1))) / (h * h); den = eig[:, None] + eig[None, :]

    def apply(v):
        w = np.zeros_like(v); w[1:-1, 1:-1] = idstn(dstn(v[1:-1, 1:-1], type=1) / den, type=1)
        return w
    return apply


def nrm(A):
    return float(np.sqrt((A * A).sum()))


def solve(N, lam, prec=None):
    """The reference's Newton loop and GMRES, counting steps; prec=None reproduces the reference."""
    u = np.zeros((N, N)); F = np.zeros((N, N)); up = np.zeros((N, N)); Fp = np.zeros((N, N))
    Q = np.zeros((RESTART + 1, N, N)); H = np.zeros((RESTART + 1, RESTART))
    ref.bratu_residual(u, F, N, lam); f0 = nrm(F); ks = []
    for _ in range(MAX_NEWTON):
        if nrm(F) <= NEWTON_RTOL * f0:
            break
        cs = np.zeros(RESTART); sn = np.zeros(RESTART); g = np.zeros(RESTART + 1); y = np.zeros(RESTART)
        beta = nrm(F); Q[0] = -F / beta; g[0] = beta; m = RESTART; w = np.zeros((N, N))
        for k in range(RESTART):
            v = Q[k] if prec is None else prec(Q[k])
            ref.bratu_jvp(u, v, F, w, up, Fp, N, lam)
            for p in range(k + 1):
                H[p, k] = float((Q[p] * w).sum()); w -= H[p, k] * Q[p]
            hn = nrm(w); H[k + 1, k] = hn
            for p in range(k):
                t = cs[p] * H[p, k] + sn[p] * H[p + 1, k]; H[p + 1, k] = -sn[p] * H[p, k] + cs[p] * H[p + 1, k]
                H[p, k] = t
            den = np.hypot(H[k, k], H[k + 1, k]); cs[k] = H[k, k] / den; sn[k] = H[k + 1, k] / den
            H[k, k] = cs[k] * H[k, k] + sn[k] * H[k + 1, k]; H[k + 1, k] = 0.0
            t = cs[k] * g[k]; g[k + 1] = -sn[k] * g[k]; g[k] = t
            if abs(g[k + 1]) / beta < INNER_TOL or hn < 1e-13 or k == RESTART - 1:
                m = k + 1; break
            Q[k + 1] = w / hn
        for r in range(m - 1, -1, -1):
            y[r] = (g[r] - H[r, r + 1:m] @ y[r + 1:m]) / H[r, r]
        du = np.tensordot(y[:m], Q[:m], axes=1)
        u += du if prec is None else prec(du); ref.bratu_residual(u, F, N, lam); ks.append(m)
    return u, ks, nrm(F) / f0


sizes = [int(a) for a in sys.argv[1:]] or [32, 48, 64, 96, 128, 256, 512, 704]
print(f"{'lam':>4} {'N':>4}  {'ref: Newton Jv ||F||/||F0||':>28}  {'reordered: fp64 campaign':>24}"
      f"  {'prec: Newton Jv':>16}  {'r fp64':>9} {'r campaign':>10}", flush=True)
for lam in (6.0, 3.0):
    for N in sizes:
        u_bench = np.zeros((N, N)); ref.jfnk_bratu(u_bench, N, lam, MAX_NEWTON, INNER_TOL, RESTART)
        u_ref, k_ref, f_ref = solve(N, lam)
        u_pre, k_pre, _ = solve(N, lam, fast_poisson(N))
        # u_ref is the reference's arithmetic with its sums vectorized: a reordering, not a change.
        print(f"{lam:>4} {N:>4}  {len(k_ref):>10d} {sum(k_ref):>6d} {f_ref:>10.1e}"
              f"  {budget(u_ref, u_bench, 'fp64'):>12.1e} {budget(u_ref, u_bench, 'campaign'):>10.1e}"
              f"  {len(k_pre):>8d} {sum(k_pre):>6d}  {budget(u_pre, u_bench, 'fp64'):>9.1e}"
              f" {budget(u_pre, u_bench, 'campaign'):>10.1e}", flush=True)
