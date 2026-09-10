import sys, time
import numpy as np, scipy.sparse as sp
from scipy.sparse.linalg import LinearOperator, cg, eigsh
sys.path.insert(0, "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")
from hpcagent_bench.support.helpers.sparse.generators import make_stencil_3d

def levels_natural(A):
    n = A.shape[0]; ip, ic = A.indptr, A.indices
    lev = np.zeros(n, dtype=np.int64)
    for i in range(n):
        c = ic[ip[i]:ip[i+1]]; p = c[c < i]
        lev[i] = (lev[p].max()+1) if p.size else 0
    return lev

class LevelTri:
    def __init__(self, T, diag, lev):
        self.diag = diag
        order = np.argsort(lev, kind="stable")
        bnd = np.searchsorted(lev[order], np.arange(lev.max()+2))
        self.groups = [order[bnd[l]:bnd[l+1]] for l in range(lev.max()+1)]
        T = T.tocsr(); self.rows = [T[g] for g in self.groups]
    def solve(self, b):
        x = np.zeros_like(b)
        for g, R in zip(self.groups, self.rows): x[g] = (b[g] - R @ x) / self.diag[g]
        return x

def ic0_csr(A):
    """IC(0) on A's lower-triangular pattern. Returns (L, min_pivot_arg, ok)."""
    Lm = sp.tril(A).tocsr().copy()
    n = Lm.shape[0]; ip, ic, v = Lm.indptr, Lm.indices, Lm.data
    for i in range(n):
        s0, e0 = ip[i], ip[i+1]
        for t in range(s0, e0):
            j = ic[t]
            if j == i: break
            s1, e1 = ip[j], ip[j+1]
            # dot of row i and row j over shared columns < j
            ci = ic[s0:t]; cj = ic[s1:e1-1]
            common, ii, jj = np.intersect1d(ci, cj, assume_unique=True, return_indices=True)
            acc = float(np.dot(v[s0:t][ii], v[s1:e1-1][jj])) if common.size else 0.0
            v[t] = (v[t] - acc) / v[e1-1]
        piv = v[e0-1] - float(np.dot(v[s0:e0-1], v[s0:e0-1]))
        if piv <= 0.0:
            return Lm, i, piv
        v[e0-1] = np.sqrt(piv)
    return Lm, -1, None

def cheb_dinv(A, d, r, m, lmax, lo_frac=30.0):
    a, b = lmax/lo_frac, 1.1*lmax
    th, de = (b+a)/2.0, (b-a)/2.0
    x = np.zeros_like(r); res = r/d; p = np.zeros_like(r); alpha = beta = 0.0
    for i in range(m):
        if i == 0: p = res.copy(); alpha = 1.0/th
        elif i == 1: beta = 0.5*(de*alpha)**2; alpha = 1.0/(th - beta/alpha); p = res + beta*alpha*p
        else: beta = (de*alpha/2.0)**2; alpha = 1.0/(th - beta/alpha); p = res + beta*alpha*p
        x = x + alpha*p
        res = res - alpha*((A @ p)/d)
    return x

def run(k):
    A = make_stencil_3d(k,k,k).tocsr(); n = A.shape[0]; d = A.diagonal()
    L = sp.tril(A,-1).tocsr(); U = sp.triu(A,1).tocsr()
    lev = levels_natural(A)
    fwd = LevelTri(L, d, lev); bwd = LevelTri(U, d, lev.max()-lev)
    lmax = float(eigsh(A, k=1, which="LM", return_eigenvectors=False, tol=1e-6)[0])
    rng = np.random.default_rng(42); b = A @ rng.random(n)

    def sgs(r): return bwd.solve(d*fwd.solve(r))
    def gs2(r, ns):
        y = np.zeros_like(r)
        for _ in range(ns): y = (r - L@y)/d
        w = d*y; z = np.zeros_like(r)
        for _ in range(ns): z = (w - U@z)/d
        return z
    # multicolour SGS
    ip, ic = A.indptr, A.indices
    color = -np.ones(n, dtype=np.int64)
    for i in range(n):
        used = set(color[ic[ip[i]:ip[i+1]]].tolist()); c = 0
        while c in used: c += 1
        color[i] = c
    groups = [np.where(color==c)[0] for c in range(color.max()+1)]
    Ag = [A[g] for g in groups]
    def csgs(r):
        z = np.zeros_like(r)
        for g, R in zip(groups, Ag): z[g] = (r[g] - R@z + d[g]*z[g])/d[g]
        for g, R in list(zip(groups, Ag))[::-1]: z[g] = (r[g] - R@z + d[g]*z[g])/d[g]
        return z
    # block Jacobi, contiguous blocks
    def make_bjac(bs):
        blocks = []
        for s in range(0, n, bs):
            e = min(s+bs, n)
            blocks.append((slice(s,e), np.linalg.inv(A[s:e, s:e].toarray())))
        def apply(r):
            z = np.empty_like(r)
            for sl, Binv in blocks: z[sl] = Binv @ r[sl]
            return z
        return apply

    print(f"\n===== 27-point operator {k}^3  N={n}  nnz={A.nnz}  lambda_max={lmax:.4g}")
    print(f"  natural-order levels={lev.max()+1}  avg rows/level={n/(lev.max()+1):.0f}  "
          f"max={np.bincount(lev).max()}   |   greedy colours={len(groups)} ({len(groups[0])} pts each)")
    Lic, badrow, piv = ic0_csr(A)
    if badrow >= 0:
        print(f"  IC(0) on this operator: BREAKDOWN at row {badrow} of {n}, pivot={piv:.3e}")
        ic0_op = None
    else:
        licl = levels_natural(Lic); dl = Lic.diagonal()
        f2 = LevelTri(sp.tril(Lic,-1).tocsr(), dl, licl); b2 = LevelTri(sp.triu(Lic.T,1).tocsr(), dl, licl.max()-licl)
        ic0_op = lambda r: b2.solve(f2.solve(r))

    cases = [("none", None), ("Jacobi (diagonal)", lambda r: r/d),
             ("SGS  <- the reference", sgs)]
    for ns in (1,2,3,5,10): cases.append((f"two-stage SGS, {ns} inner Jacobi", (lambda ns: lambda r: gs2(r, ns))(ns)))
    cases.append((f"multicolour SGS ({len(groups)} colours)", csgs))
    for bs in (8, 64): cases.append((f"block Jacobi, block={bs}", make_bjac(bs)))
    for m in (2,4,8,16): cases.append((f"Chebyshev(D^-1A) degree {m}", (lambda m: lambda r: cheb_dinv(A,d,r,m,lmax))(m)))
    if ic0_op: cases.append(("IC(0)", ic0_op))

    base = None
    for name, f in cases:
        M = LinearOperator((n,n), matvec=f) if f else None
        it = [0]
        t0 = time.time()
        x, info = cg(A, b, rtol=1e-8, atol=0.0, maxiter=5000, M=M, callback=lambda xk: it.__setitem__(0, it[0]+1))
        if base is None: base = it[0]
        tag = "  NOT CONVERGED" if info else ""
        print(f"  {name:36s} iters={it[0]:>5}  x{base/max(it[0],1):5.2f} vs unpreconditioned{tag}")

for k in (16, 32): run(k)
