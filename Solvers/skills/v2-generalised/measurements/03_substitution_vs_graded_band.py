import sys
import numpy as np, scipy.sparse as sp
sys.path.insert(0, "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")
from hpcagent_bench.support.helpers.sparse.generators import make_stencil_3d

RTOL, ATOL = 1e-9, 1e-11   # the fp64 band from docs/numerical_validation.md

def levels_natural(A):
    n=A.shape[0]; ip,ic=A.indptr,A.indices; lev=np.zeros(n,dtype=np.int64)
    for i in range(n):
        c=ic[ip[i]:ip[i+1]]; p=c[c<i]; lev[i]=(lev[p].max()+1) if p.size else 0
    return lev

class LevelTri:
    def __init__(s,T,diag,lev):
        s.diag=diag; o=np.argsort(lev,kind="stable")
        bnd=np.searchsorted(lev[o],np.arange(lev.max()+2))
        s.groups=[o[bnd[l]:bnd[l+1]] for l in range(lev.max()+1)]
        T=T.tocsr(); s.rows=[T[g] for g in s.groups]
    def solve(s,b):
        x=np.zeros_like(b)
        for g,R in zip(s.groups,s.rows): x[g]=(b[g]-R@x)/s.diag[g]
        return x

def pcg(A, b, niter, apply_M):
    """The reference recurrence, with a swappable preconditioner. x0 = 0."""
    n=A.shape[0]; x=np.zeros(n); r=b.copy()
    z=apply_M(r); p=z.copy(); rz=float(r@z)
    for _ in range(niter):
        q=A@p; alpha=rz/float(p@q); x+=alpha*p; r-=alpha*q
        z=apply_M(r); rzn=float(r@z); p=z+(rzn/rz)*p; rz=rzn
    return x

def rel(a,e):
    """Worst per-element violation of |a-e| <= atol + rtol*|e|, as a multiple of the budget."""
    return float(np.max(np.abs(a-e)/(ATOL+RTOL*np.abs(e))))

for k, niter in ((16,25),(32,50)):
    A=make_stencil_3d(k,k,k).tocsr(); n=A.shape[0]; d=A.diagonal()
    L=sp.tril(A,-1).tocsr(); U=sp.triu(A,1).tocsr(); lev=levels_natural(A)
    fwd=LevelTri(L,d,lev); bwd=LevelTri(U,d,lev.max()-lev)
    rng=np.random.default_rng(42); b=A@rng.random(n)

    def sgs_rowloop(r):
        y=np.zeros(n); ip,ic,va=A.indptr,A.indices,A.data
        for i in range(n):
            s=r[i]
            for kk in range(ip[i],ip[i+1]):
                j=ic[kk]
                if j<i: s-=va[kk]*y[j]
            y[i]=s/d[i]
        z=np.zeros(n)
        for ii in range(n-1,-1,-1):
            s=d[ii]*y[ii]
            for kk in range(ip[ii],ip[ii+1]):
                j=ic[kk]
                if j>ii: s-=va[kk]*z[j]
            z[ii]=s/d[ii]
        return z
    def sgs_levels(r): return bwd.solve(d*fwd.solve(r))
    def gs2(r,ns):
        y=np.zeros(n)
        for _ in range(ns): y=(r-L@y)/d
        w=d*y; z=np.zeros(n)
        for _ in range(ns): z=(w-U@z)/d
        return z
    ip,ic=A.indptr,A.indices; color=-np.ones(n,dtype=np.int64)
    for i in range(n):
        used=set(color[ic[ip[i]:ip[i+1]]].tolist()); c=0
        while c in used: c+=1
        color[i]=c
    groups=[np.where(color==c)[0] for c in range(color.max()+1)]; Ag=[A[g] for g in groups]
    def csgs(r):
        z=np.zeros(n)
        for g,R in zip(groups,Ag): z[g]=(r[g]-R@z+d[g]*z[g])/d[g]
        for g,R in list(zip(groups,Ag))[::-1]: z[g]=(r[g]-R@z+d[g]*z[g])/d[g]
        return z

    ref = pcg(A,b,niter,sgs_rowloop)
    print(f"\n===== sgs_pcg preset {k}^3, niter={niter} (a FIXED sweep count, not a solve to tolerance)")
    print(f"     graded band: |a-e| <= {ATOL} + {RTOL}*|e|;  a variant passes only at <= 1.0x")
    for name, f in [("level-scheduled SGS (same ordering)", sgs_levels),
                    ("Jacobi instead of SGS", lambda r: r/d),
                    ("two-stage SGS, 3 inner Jacobi", lambda r: gs2(r,3)),
                    ("two-stage SGS, 10 inner Jacobi", lambda r: gs2(r,10)),
                    ("multicolour SGS (8 colours)", csgs)]:
        got = pcg(A,b,niter,f)
        v = rel(got, ref)
        print(f"  {name:38s} worst element = {v:10.3e} x budget   "
              f"{'PASSES' if v<=1.0 else 'FAILS'}")
