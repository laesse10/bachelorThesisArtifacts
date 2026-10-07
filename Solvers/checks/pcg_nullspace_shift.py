"""Why a preconditioner swap in sgs_pcg misses by nine orders: the iterate, and the null space.

sgs_pcg's operator is a weighted graph Laplacian: every row sums to zero, so it is singular with the
constant vector in its null space (measurement 02 of the skill). Preconditioned CG then converges to
a solution whose constant component depends on the preconditioner. This runs the reference
recurrence (x0 = 0) with SGS and with Jacobi, at the graded iteration count and again to full
convergence, and splits their difference into its mean (the null-space component) and the rest.

Needs the benchmark's operator generator: export HPCAGENT_BENCH=<HPCAgent-Bench checkout>.
"""
import os
import sys

import numpy as np
import scipy.sparse as sp
from scipy.sparse.linalg import spsolve_triangular

sys.path.insert(0, os.environ["HPCAGENT_BENCH"])
from hpcagent_bench.support.helpers.sparse.generators import make_stencil_3d  # noqa: E402

RTOL, ATOL = 1e-9, 1e-11


def budget(a, e):
    return float(np.max(np.abs(a - e) / (ATOL + RTOL * np.abs(e))))


def pcg(A, b, niter, apply_M):
    """The reference recurrence with a swappable preconditioner, x0 = 0."""
    x = np.zeros(A.shape[0]); r = b.copy(); z = apply_M(r); p = z.copy(); rz = r @ z
    for _ in range(niter):
        q = A @ p; alpha = rz / (p @ q); x += alpha * p; r -= alpha * q
        z = apply_M(r); rzn = r @ z; p = z + (rzn / rz) * p; rz = rzn
    return x, np.linalg.norm(b - A @ x) / np.linalg.norm(b)


for k, graded in ((16, 25), (32, 50)):
    A = make_stencil_3d(k, k, k).tocsr(); n = A.shape[0]; d = A.diagonal()
    DL = sp.tril(A).tocsr(); DU = sp.triu(A).tocsr()

    def sgs(r):
        return spsolve_triangular(DU, d * spsolve_triangular(DL, r, lower=True), lower=False)

    def jacobi(r):
        return r / d

    b = A @ np.random.default_rng(42).random(n)          # as sgs_pcg.initialize builds it
    for niter in (graded, 300):
        xs, rs = pcg(A, b, niter, sgs)
        xj, rj = pcg(A, b, niter, jacobi)
        shift = (xj - xs).mean()
        print(f"{k}^3, {niter:3d} iterations: relative residual SGS {rs:.1e}, Jacobi {rj:.1e} | "
              f"budget {budget(xj, xs):.1e} | mean of difference {shift:+.1e} | "
              f"budget once the mean is removed {budget(xj - shift, xs):.1e}")
