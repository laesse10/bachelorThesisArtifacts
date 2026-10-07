"""The fast-Poisson change to jfnk_bratu, judged at five grid sizes instead of one.

Measurement 06 of the skill applies a DST fast-Poisson preconditioner to the inner GMRES at N=32
only, and 07 shows where the unpreconditioned reference starts hitting max_newton = 20. This runs
both solvers at every size of 07 and prints the preconditioned result as a multiple of the fp64
budget, so the crossover is visible in the graded quantity itself. Self-contained: NumPy and SciPy.
"""
import numpy as np
from scipy.fft import dstn, idstn

RTOL, ATOL = 1e-9, 1e-11


def budget(a, e):
    return float(np.max(np.abs(a - e) / (ATOL + RTOL * np.abs(e))))


# Solver identical to measurements/06 and 07 (the jfnk_bratu reference recurrence).
def residual(u, N, lam):
    h = 1.0 / (N - 1); F = np.zeros_like(u)
    F[0, :] = u[0, :]; F[N - 1, :] = u[N - 1, :]; F[:, 0] = u[:, 0]; F[:, N - 1] = u[:, N - 1]
    F[1:-1, 1:-1] = (4 * u[1:-1, 1:-1] - u[:-2, 1:-1] - u[2:, 1:-1] - u[1:-1, :-2] - u[1:-1, 2:]) / (h * h) \
        - lam * np.exp(u[1:-1, 1:-1])
    return F


def nrm(A):
    return float(np.sqrt((A * A).sum()))


def jvp(u, v, Fu, N, lam):
    nv = nrm(v)
    if nv == 0.0:
        return np.zeros_like(v)
    eps = np.sqrt(np.finfo(np.float64).eps) * (1.0 + nrm(u)) / nv
    return (residual(u + eps * v, N, lam) - Fu) / eps


def gmres(u, Fu, N, lam, restart, tol, prec=None):
    Q = np.zeros((N, N, restart + 1)); H = np.zeros((restart + 1, restart))
    cs = np.zeros(restart); sn = np.zeros(restart); g = np.zeros(restart + 1); y = np.zeros(restart)
    beta = nrm(Fu); Q[:, :, 0] = -Fu / beta; g[0] = beta; m = restart
    for k in range(restart):
        v = Q[:, :, k] if prec is None else prec(Q[:, :, k])
        w = jvp(u, v, Fu, N, lam)
        for p in range(k + 1):
            H[p, k] = float((Q[:, :, p] * w).sum()); w -= H[p, k] * Q[:, :, p]
        hn = nrm(w); H[k + 1, k] = hn
        for p in range(k):
            t = cs[p] * H[p, k] + sn[p] * H[p + 1, k]; H[p + 1, k] = -sn[p] * H[p, k] + cs[p] * H[p + 1, k]; H[p, k] = t
        den = np.hypot(H[k, k], H[k + 1, k]); cs[k] = H[k, k] / den; sn[k] = H[k + 1, k] / den
        H[k, k] = cs[k] * H[k, k] + sn[k] * H[k + 1, k]; H[k + 1, k] = 0.0
        t = cs[k] * g[k]; g[k + 1] = -sn[k] * g[k]; g[k] = t
        if abs(g[k + 1]) / beta < tol or hn < 1e-13 or k == restart - 1:
            m = k + 1; break
        Q[:, :, k + 1] = w / hn
    for r in range(m - 1, -1, -1):
        s = g[r]
        for c in range(r + 1, m):
            s -= H[r, c] * y[c]
        y[r] = s / H[r, r]
    du = np.zeros((N, N))
    for p in range(m):
        du += Q[:, :, p] * y[p]
    return (du if prec is None else prec(du)), m


def solve(N, lam, max_newton, inner_tol, restart, prec=None):
    u = np.zeros((N, N)); F = residual(u, N, lam); f0 = nrm(F); ks = []
    for _ in range(max_newton):
        if nrm(F) <= 1e-10 * f0:
            break
        du, m = gmres(u, F, N, lam, restart, inner_tol, prec); ks.append(m)
        u += du; F = residual(u, N, lam)
    return u, ks, nrm(F) / f0


def fast_poisson(N):
    """DST-I solve on the interior: the exact inverse of the 5-point Dirichlet Laplacian."""
    h = 1.0 / (N - 1); k = np.arange(1, N - 1)
    eig = (2 - 2 * np.cos(np.pi * k / (N - 1))) / (h * h); den = eig[:, None] + eig[None, :]

    def apply(v):
        w = np.zeros_like(v); w[1:-1, 1:-1] = idstn(dstn(v[1:-1, 1:-1], type=1) / den, type=1)
        return w
    return apply


lam = 6.0
print(f"{'N':>4}  {'reference: Newton, Arnoldi, ||F||/||F0||':>42}  {'preconditioned: Newton, Arnoldi':>32}  budget")
for N in (32, 48, 64, 96, 128):
    uref, kref, fref = solve(N, lam, 20, 1e-4, 50)
    upre, kpre, _ = solve(N, lam, 20, 1e-4, 50, fast_poisson(N))
    v = budget(upre, uref)
    print(f"{N:>4}  {len(kref):>18d}/20 {sum(kref):>8d} {fref:>12.1e}  {len(kpre):>22d} {sum(kpre):>8d}  "
          f"{v:.2e} {'PASSES' if v <= 1.0 else 'FAILS'}")
