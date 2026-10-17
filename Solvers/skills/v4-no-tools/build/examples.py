"""Every measured number on the v4 pages, on operators that are NOT in the benchmark corpus.

    python3 build/examples.py [section ...]      sections: levels subst iters spectrum cap lanczos qr

Self-contained: numpy and scipy only, no paths, no network. Not shipped to agents.

Operators used here, chosen to differ from every benchmark kernel:
  - a 2-D 9-point (8-neighbour) grid operator with edge weights log-uniform on [1, 100] and Dirichlet
    ghost neighbours (SPD), in lexicographic order;
  - the same graph without ghost neighbours (a singular graph Laplacian);
  - the nonlinear problem  -Lap(u) + u^3 = f  on the unit square, solved by Jacobian-free Newton-Krylov;
  - a random dense matrix for QR.
"""

import sys
import time

import numpy as np
import scipy.linalg as sla
import scipy.sparse as sp
import scipy.sparse.linalg as spla
from scipy.fft import dstn, idstn

RTOL, ATOL = 1e-9, 1e-11


def budget(got, ref):
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))


# ----------------------------------------------------------------------------------------- operators
OFFS = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]


def nine_point(k, seed=0, dirichlet=True):
    """8-neighbour grid graph on k x k, symmetric edge weights log-uniform on [1, 100].
    dirichlet=True adds, for every neighbour that falls outside the grid, a ghost edge weight to the
    diagonal (SPD); False gives the pure graph Laplacian (singular, constant null vector)."""
    rng = np.random.default_rng(seed)
    logw = lambda size: np.exp(rng.uniform(0, np.log(100), size=size))
    y, x = np.divmod(np.arange(k * k), k)
    rows, cols, vals = [], [], []
    diag = np.zeros(k * k)
    for dy, dx in OFFS:
        if (dy, dx) < (0, 0):
            continue                                          # each undirected edge once
        yy, xx = y + dy, x + dx
        inside = (yy >= 0) & (yy < k) & (xx >= 0) & (xx < k)
        i = np.arange(k * k)[inside]; j = (yy * k + xx)[inside]; w = logw(i.size)
        rows += [i, j]; cols += [j, i]; vals += [-w, -w]
        np.add.at(diag, i, w); np.add.at(diag, j, w)
    if dirichlet:
        for dy, dx in OFFS:
            yy, xx = y + dy, x + dx
            outside = ~((yy >= 0) & (yy < k) & (xx >= 0) & (xx < k))
            diag[outside] += logw(int(outside.sum()))
    A = sp.csr_matrix((np.concatenate(vals), (np.concatenate(rows), np.concatenate(cols))),
                      shape=(k * k, k * k)) + sp.diags(diag)
    A = A.tocsr(); A.sort_indices()
    return A


def levels_natural(A):
    lev = np.zeros(A.shape[0], dtype=np.int64)
    ip, ic = A.indptr, A.indices
    for i in range(A.shape[0]):
        c = ic[ip[i]:ip[i + 1]]
        p = c[c < i]
        lev[i] = lev[p].max() + 1 if p.size else 0
    return lev


class LevelTri:
    """Triangular solve with a strictly triangular part T and diagonal d, scheduled by levels."""

    def __init__(self, T, d, lev):
        self.d = d
        order = np.argsort(lev, kind="stable")
        bnd = np.searchsorted(lev[order], np.arange(lev.max() + 2))
        self.groups = [order[bnd[l]:bnd[l + 1]] for l in range(lev.max() + 1)]
        T = T.tocsr()
        self.rows = [T[g] for g in self.groups]

    def solve(self, b):
        x = np.zeros_like(b)
        for g, R in zip(self.groups, self.rows):
            x[g] = (b[g] - R @ x) / self.d[g]
        return x


def seq_lower(Lfull, b):
    """Row-by-row forward substitution in row order: the sequential reference."""
    ip, ic, va = Lfull.indptr, Lfull.indices, Lfull.data
    x = np.zeros_like(b)
    for i in range(len(b)):
        s = b[i]; di = 0.0
        for q in range(ip[i], ip[i + 1]):
            j = ic[q]
            if j < i:
                s -= va[q] * x[j]
            elif j == i:
                di = va[q]
        x[i] = s / di
    return x


def seq_upper(Ufull, b):
    ip, ic, va = Ufull.indptr, Ufull.indices, Ufull.data
    x = np.zeros_like(b)
    for i in range(len(b) - 1, -1, -1):
        s = b[i]; di = 0.0
        for q in range(ip[i], ip[i + 1]):
            j = ic[q]
            if j > i:
                s -= va[q] * x[j]
            elif j == i:
                di = va[q]
        x[i] = s / di
    return x


class Precs:
    def __init__(self, A):
        n = A.shape[0]
        self.A, self.n = A, n
        self.d = A.diagonal()
        self.L = sp.tril(A, -1).tocsr(); self.U = sp.triu(A, 1).tocsr()
        self.Lf = sp.tril(A).tocsr(); self.Uf = sp.triu(A).tocsr()
        lev = levels_natural(A)
        self.lev = lev
        self.fwd = LevelTri(self.L, self.d, lev)
        self.bwd = LevelTri(self.U, self.d, levels_natural_upper(A))
        ip, ic = A.indptr, A.indices
        color = -np.ones(n, dtype=np.int64)
        for i in range(n):
            used = set(color[ic[ip[i]:ip[i + 1]]].tolist()); c = 0
            while c in used:
                c += 1
            color[i] = c
        self.ncolors = color.max() + 1
        self.cgroups = [np.where(color == c)[0] for c in range(self.ncolors)]
        self.crows = [A[g] for g in self.cgroups]

    def sgs_seq(self, r):
        return seq_upper(self.Uf, self.d * seq_lower(self.Lf, r))

    def sgs_levels(self, r):
        return self.bwd.solve(self.d * self.fwd.solve(r))

    def jacobi(self, r):
        return r / self.d

    def gs2(self, r, ns):
        y = np.zeros(self.n)
        for _ in range(ns):
            y = (r - self.L @ y) / self.d
        w = self.d * y; z = np.zeros(self.n)
        for _ in range(ns):
            z = (w - self.U @ z) / self.d
        return z

    def mcsgs(self, r):
        z = np.zeros(self.n); d = self.d
        for g, R in zip(self.cgroups, self.crows):
            z[g] = (r[g] - R @ z + d[g] * z[g]) / d[g]
        for g, R in list(zip(self.cgroups, self.crows))[::-1]:
            z[g] = (r[g] - R @ z + d[g] * z[g]) / d[g]
        return z


def levels_natural_upper(A):
    """Level of each row for the backward sweep: depends on rows j > i."""
    n = A.shape[0]
    lev = np.zeros(n, dtype=np.int64)
    ip, ic = A.indptr, A.indices
    for i in range(n - 1, -1, -1):
        c = ic[ip[i]:ip[i + 1]]
        p = c[c > i]
        lev[i] = lev[p].max() + 1 if p.size else 0
    return lev


def pcg_fixed(A, b, niter, M):
    x = np.zeros_like(b); r = b.copy()
    z = M(r); p = z.copy(); rz = r @ z
    for _ in range(niter):
        q = A @ p; alpha = rz / (p @ q)
        x += alpha * p; r -= alpha * q
        z = M(r); rzn = r @ z
        p = z + (rzn / rz) * p; rz = rzn
    return x


def pcg_tol(A, b, M, tol=1e-8, maxit=5000):
    x = np.zeros_like(b); r = b.copy(); b0 = np.linalg.norm(b)
    z = M(r); p = z.copy(); rz = r @ z
    for it in range(1, maxit + 1):
        q = A @ p; alpha = rz / (p @ q)
        x += alpha * p; r -= alpha * q
        if np.linalg.norm(r) <= tol * b0:
            return it
        z = M(r); rzn = r @ z
        p = z + (rzn / rz) * p; rz = rzn
    return maxit


# ------------------------------------------------------------------------------------------ sections
def sec_levels():
    print("== level structure, 9-point operator, lexicographic order")
    for k in (32, 64, 128, 256):
        A = nine_point(k)
        lev = levels_natural(A)
        cnt = np.bincount(lev)
        print(f"  {k}^2  N={k*k:>6}  levels={lev.max()+1:>5}  avg rows/level={cnt.mean():7.1f}  "
              f"max={cnt.max():>4}  (3k-2={3*k-2}, ceil(k/2)={-(-k//2)})")


def sec_subst():
    print("== fixed-sweep PCG, only the preconditioner swapped; worst element as a multiple of the budget")
    for k, niter in ((64, 25), (128, 50)):
        A = nine_point(k); P = Precs(A)
        rng = np.random.default_rng(42); b = A @ rng.random(A.shape[0])
        t = time.time(); ref = pcg_fixed(A, b, niter, P.sgs_seq)
        print(f"  {k}^2, {niter} sweeps  (reference took {time.time()-t:.0f}s, {P.ncolors} colours)")
        for name, M in [("level-scheduled SGS, same ordering", P.sgs_levels),
                        ("Jacobi instead of SGS", P.jacobi),
                        ("two-stage SGS, 3 inner Jacobi", lambda r: P.gs2(r, 3)),
                        ("two-stage SGS, 10 inner Jacobi", lambda r: P.gs2(r, 10)),
                        (f"multicolour SGS, {P.ncolors} colours", P.mcsgs)]:
            print(f"    {name:38s} {budget(pcg_fixed(A, b, niter, M), ref):10.2e}")


def block_jacobi(A, bs):
    n = A.shape[0]; inv = []
    for s in range(0, n, bs):
        e = min(s + bs, n)
        inv.append((s, e, np.linalg.inv(A[s:e, s:e].toarray())))
    def M(r):
        z = np.empty_like(r)
        for s, e, Bi in inv:
            z[s:e] = Bi @ r[s:e]
        return z
    return M


def ilu0(A):
    A = A.tocsr().copy(); A.sort_indices()
    ip, ic, va = A.indptr, A.indices, A.data
    n = A.shape[0]
    pos = [dict(zip(ic[ip[i]:ip[i + 1]].tolist(), range(ip[i], ip[i + 1]))) for i in range(n)]
    dpos = np.array([pos[i][i] for i in range(n)])
    for i in range(n):
        for q in range(ip[i], ip[i + 1]):
            kcol = ic[q]
            if kcol >= i:
                break
            va[q] /= va[dpos[kcol]]
            aik = va[q]
            for q2 in range(ip[kcol], ip[kcol + 1]):
                j = ic[q2]
                if j > kcol and j in pos[i]:
                    va[pos[i][j]] -= aik * va[q2]
    Lu = sp.tril(A, -1).tocsr() + sp.identity(n, format="csr")
    Uu = sp.triu(A).tocsr()
    def M(r):
        return spla.spsolve_triangular(Uu, spla.spsolve_triangular(Lu, r, lower=True), lower=False)
    return M


def sec_iters():
    print("== CG to relative residual 1e-8, iterations")
    res = {}
    for k in (64, 128):
        A = nine_point(k); P = Precs(A)
        b = A @ np.random.default_rng(42).random(A.shape[0])
        rows = [("none", lambda r: r), ("Jacobi", P.jacobi), ("symmetric Gauss-Seidel", P.sgs_levels),
                ("two-stage SGS, 1 inner", lambda r: P.gs2(r, 1)), ("two-stage SGS, 2 inner", lambda r: P.gs2(r, 2)),
                ("two-stage SGS, 3 inner", lambda r: P.gs2(r, 3)), ("two-stage SGS, 10 inner", lambda r: P.gs2(r, 10)),
                (f"multicolour SGS, {P.ncolors} colours", P.mcsgs),
                ("block Jacobi, contiguous 8", block_jacobi(A, 8)), ("block Jacobi, contiguous 64", block_jacobi(A, 64)),
                ("ILU(0) = IC(0) pattern", ilu0(A))]
        for name, M in rows:
            res.setdefault(name, []).append(pcg_tol(A, b, M))
    for name, its in res.items():
        print(f"  {name:32s} {its[0]:>5} {its[1]:>5}   vs none {res['none'][0]/its[0]:.2f}-{res['none'][1]/its[1]:.2f}x")


def sec_spectrum():
    print("== singularity and spectrum of the pure graph Laplacian (no Dirichlet ghosts)")
    for k in (32, 64):
        L = nine_point(k, dirichlet=False)
        ones = np.ones(L.shape[0])
        ratio = np.abs(L @ ones).max() / np.abs(L).sum(axis=1).max()
        lmax = spla.eigsh(L, k=1, which="LA", return_eigenvectors=False)[0]
        small = np.sort(spla.eigsh(L, k=3, sigma=-1e-3, which="LM", return_eigenvectors=False))
        lam2 = small[small > 1e-8 * lmax][0]
        print(f"  {k}^2  ||L*1||/||L|| = {ratio:.1e}   smallest eigenvalues {small}   lmax/lambda2 = {lmax/lam2:.0f}")


# --- nonlinear: -Lap(u) + u^3 = f on the unit square, zero Dirichlet, JFNK with one GMRES cycle per step
def residual(u, N, f):
    h = 1.0 / (N + 1)
    up = np.pad(u, 1)
    lap = (4 * u - up[:-2, 1:-1] - up[2:, 1:-1] - up[1:-1, :-2] - up[1:-1, 2:]) / (h * h)
    return lap + u ** 3 - f


def nrm(a):
    return float(np.sqrt((a * a).sum()))


def jvp(u, v, Fu, N, f):
    nv = nrm(v)
    if nv == 0.0:
        return np.zeros_like(v)
    eps = np.sqrt(np.finfo(float).eps) * (1.0 + nrm(u)) / nv
    return (residual(u + eps * v, N, f) - Fu) / eps


def fast_poisson(N):
    h = 1.0 / (N + 1); j = np.arange(1, N + 1)
    lam = (2 - 2 * np.cos(np.pi * j / (N + 1))) / (h * h)
    den = lam[:, None] + lam[None, :]
    return lambda v: idstn(dstn(v, type=1) / den, type=1)


def gmres1(u, Fu, N, f, restart, tol, prec=None):
    Q = np.zeros((restart + 1, N, N)); H = np.zeros((restart + 1, restart))
    cs = np.zeros(restart); sn = np.zeros(restart); g = np.zeros(restart + 1); y = np.zeros(restart)
    beta = nrm(Fu); Q[0] = -Fu / beta; g[0] = beta; m = restart; Av = 0
    for k in range(restart):
        v = Q[k] if prec is None else prec(Q[k])
        w = jvp(u, v, Fu, N, f); Av += 1
        for p in range(k + 1):
            H[p, k] = float((Q[p] * w).sum()); w -= H[p, k] * Q[p]
        hn = nrm(w); H[k + 1, k] = hn
        for p in range(k):
            t = cs[p] * H[p, k] + sn[p] * H[p + 1, k]; H[p + 1, k] = -sn[p] * H[p, k] + cs[p] * H[p + 1, k]; H[p, k] = t
        den = np.hypot(H[k, k], H[k + 1, k]); cs[k] = H[k, k] / den; sn[k] = H[k + 1, k] / den
        H[k, k] = cs[k] * H[k, k] + sn[k] * H[k + 1, k]; H[k + 1, k] = 0.0
        t = cs[k] * g[k]; g[k + 1] = -sn[k] * g[k]; g[k] = t
        if abs(g[k + 1]) / beta < tol or hn < 1e-13 or k == restart - 1:
            m = k + 1; break
        Q[k + 1] = w / hn
    for r in range(m - 1, -1, -1):
        s = g[r]
        for c in range(r + 1, m):
            s -= H[r, c] * y[c]
        y[r] = s / H[r, r]
    du = np.tensordot(y[:m], Q[:m], axes=1)
    return (du if prec is None else prec(du)), Av


def newton(N, f, cap, restart, inner_tol, prec=None, rtol=1e-12):
    u = np.zeros((N, N)); F = residual(u, N, f); f0 = nrm(F); steps = 0; Av = 0
    for _ in range(cap):
        if nrm(F) <= rtol * f0:
            break
        du, a = gmres1(u, F, N, f, restart, inner_tol, prec); Av += a
        u += du; F = residual(u, N, f); steps += 1
    return u, steps, nrm(F) / f0, Av


def sec_cap():
    print("== JFNK on -Lap(u) + u^3 = f, stop at ||F|| <= 1e-12 ||F0||, Newton cap 20, one GMRES(30) cycle per step, inner tol 1e-4")
    for N in (16, 32, 48, 64, 96, 128):
        xs = (np.arange(1, N + 1)) / (N + 1)
        X, Y = np.meshgrid(xs, xs, indexing="ij")
        f = 200.0 * np.sin(np.pi * X) * np.sin(2 * np.pi * Y) + 100.0
        u0, s0, r0, a0 = newton(N, f, 20, 30, 1e-4)
        u1, s1, r1, a1 = newton(N, f, 20, 30, 1e-4, fast_poisson(N))
        tag = "converged" if r0 <= 1e-12 else "CAPPED"
        print(f"  N={N:>4}  reference: {s0:>2}/20 steps, residual ratio {r0:.1e} {tag:9s} {a0:>4} Jv | "
              f"fast-Poisson preconditioned: {s1:>2} steps, {a1:>4} Jv, budget vs reference {budget(u1, u0):.1e}")


def lanczos(A, b, m, mode):
    N = A.shape[0]; Q = np.zeros((N, m)); alpha = np.zeros(m); beta = np.zeros(m)
    Q[:, 0] = b / np.sqrt(b @ b); q_prev = np.zeros(N); beta_prev = 0.0
    for j in range(m):
        w = A @ Q[:, j]
        if j > 0:
            w -= beta_prev * q_prev
        a = float(Q[:, j] @ w); alpha[j] = a; w -= a * Q[:, j]
        for _ in range(2):
            if mode == "mgs":
                for p in range(j + 1):
                    w -= float(Q[:, p] @ w) * Q[:, p]
            else:
                dots = Q[:, :j + 1].T @ w; w -= Q[:, :j + 1] @ dots
        bj = float(np.sqrt(w @ w)); beta[j] = bj
        q_prev = Q[:, j].copy()
        if j + 1 < m:
            Q[:, j + 1] = w / bj
        beta_prev = bj
    return Q, alpha, beta


def sec_lanczos():
    print("== Lanczos, reorthogonalization twice: per-column MGS (reference) vs one GEMV pair (CGS)")
    A = nine_point(64); b = np.random.default_rng(42).random(A.shape[0])
    for m in (50, 100):
        Qr, ar, br = lanczos(A, b, m, "mgs"); Qc, ac, bc = lanczos(A, b, m, "cgs")
        print(f"  64^2, m={m}:  Q {budget(Qc, Qr):.1e}   alpha {budget(ac, ar):.1e}   beta {budget(bc, br):.1e}")


def ref_qr(A0):
    A = A0.copy(); M, N = A.shape; V = np.zeros((M, N)); beta = np.zeros(N); R = np.zeros((N, N))
    for k in range(N):
        col = A[k:, k]; normx = np.sqrt(col @ col); s = 1.0 if A[k, k] >= 0 else -1.0
        V[k:, k] = col; V[k, k] = A[k, k] + s * normx
        vv = V[k:, k] @ V[k:, k]; beta[k] = 2.0 / vv if vv > 0 else 0.0
        w = beta[k] * (V[k:, k] @ A[k:, k:]); A[k:, k:] -= np.outer(V[k:, k], w)
        R[k, k:] = A[k, k:]
    Q = np.eye(M)[:, :N].copy()
    for k in range(N - 1, -1, -1):
        w = beta[k] * (V[k:, k] @ Q[k:, :]); Q[k:, :] -= np.outer(V[k:, k], w)
    return Q, R


def sec_qr():
    print("== Householder QR: textbook reference vs LAPACK dgeqrf/dorgqr (scipy, economic)")
    M, N = 1500, 80
    A0 = np.random.default_rng(7).standard_normal((M, N))
    Qr, Rr = ref_qr(A0); Ql, Rl = sla.qr(A0, mode="economic")
    print(f"  {M} x {N}:  R {budget(np.triu(Rl), np.triu(Rr)):.1e} of budget   max|Q diff| {np.abs(Ql - Qr).max():.1e}")


SECTIONS = dict(levels=sec_levels, subst=sec_subst, iters=sec_iters, spectrum=sec_spectrum,
                cap=sec_cap, lanczos=sec_lanczos, qr=sec_qr)

if __name__ == "__main__":
    for name in (sys.argv[1:] or SECTIONS):
        t = time.time(); SECTIONS[name](); print(f"  [{name}: {time.time()-t:.0f}s]\n", flush=True)
