import sys
import numpy as np, scipy.sparse as sp, scipy.linalg as sla
sys.path.insert(0,"/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")
from hpcagent_bench.support.helpers.sparse.generators import make_stencil_3d
RTOL, ATOL = 1e-9, 1e-11
def viol(a,e): return float(np.max(np.abs(a-e)/(ATOL+RTOL*np.abs(e))))

# ---------- lanczos: MGS (reference) vs CGS batched into a GEMV ----------
def lanczos(A, b, m, mode):
    N=A.shape[0]; Q=np.zeros((N,m)); alpha=np.zeros(m); beta=np.zeros(m)
    Q[:,0]=b/np.sqrt(b@b); q_prev=np.zeros(N); beta_prev=0.0
    for j in range(m):
        w=A@Q[:,j]
        if j>0: w-=beta_prev*q_prev
        a=float(Q[:,j]@w); alpha[j]=a; w-=a*Q[:,j]
        for _ in range(2):
            if mode=="mgs":                       # the reference: update w inside the p loop
                for p in range(j+1):
                    dot=float(Q[:,p]@w); w-=dot*Q[:,p]
            else:                                 # batched: one GEMV against the OLD w
                dots=Q[:,:j+1].T@w; w-=Q[:,:j+1]@dots
        bj=float(np.sqrt(w@w)); beta[j]=bj
        q_prev=Q[:,j].copy()
        if j+1<m: Q[:,j+1]=w/bj
        beta_prev=bj
    return Q, alpha, beta

k, m = 16, 50
A=make_stencil_3d(k,k,k).tocsr(); N=A.shape[0]
rng=np.random.default_rng(42); b=rng.random(N)
Qr,ar,br=lanczos(A,b,m,"mgs"); Qc,ac,bc=lanczos(A,b,m,"cgs")
print(f"lanczos_reorth {k}^3 m={m}: batching the reorthogonalization into a GEMV (CGS) vs the "
      f"reference's per-column MGS")
print(f"   Q worst = {viol(Qc,Qr):.3e} x budget    alpha = {viol(ac,ar):.3e}    beta = {viol(bc,br):.3e}")

# ---------- householder QR: LAPACK dgeqrf vs the reference's own reflectors ----------
def ref_qr(A0):
    A=A0.copy(); M,N=A.shape
    V=np.zeros((M,N)); beta=np.zeros(N); Q=np.zeros((M,M)); R=np.zeros((N,N))
    for kk in range(N):
        col=A[kk:M,kk]; normx=np.sqrt(col@col); s=1.0 if A[kk,kk]>=0 else -1.0
        V[kk:M,kk]=col; V[kk,kk]=A[kk,kk]+s*normx
        vv=V[kk:M,kk]@V[kk:M,kk]; beta[kk]=2.0/vv if vv>0 else 0.0
        w=beta[kk]*(V[kk:M,kk]@A[kk:M,kk:N]); A[kk:M,kk:N]-=np.outer(V[kk:M,kk],w)
        R[kk,kk:N]=A[kk,kk:N]
    for i in range(N): Q[i,i]=1.0
    for kk in range(N-1,-1,-1):
        w=beta[kk]*(V[kk:M,kk]@Q[kk:M,:]); Q[kk:M,:]-=np.outer(V[kk:M,kk],w)
    return A,Q,R

M,Nn=2000,64
rng=np.random.default_rng(0); A0=rng.random((M,Nn))
Aref,Qref,Rref=ref_qr(A0)
Ql,Rl=sla.qr(A0, mode="economic")     # LAPACK dgeqrf + dorgqr
print(f"\nhouseholder_qr M={M} N={Nn}: LAPACK dgeqrf/dorgqr vs the reference")
print(f"   R (upper N x N) worst = {viol(np.triu(Rl), np.triu(Rref)):.3e} x budget")
print(f"   Q column-space check: ||Q_lapack - Q_ref[:, :N]||_inf = "
      f"{np.abs(Ql - Qref[:, :Nn].T if Qref.shape[0]==Nn else Ql - Qref[:Ql.shape[0], :Nn]).max():.3e}")
print(f"   reference Q shape {Qref.shape}, LAPACK economic Q shape {Ql.shape}")
print(f"   reference A after factorization: max |below-diagonal| = {np.abs(np.tril(Aref[:Nn,:Nn],-1)).max():.3e}"
      f"  (LAPACK leaves the reflectors there instead)")
