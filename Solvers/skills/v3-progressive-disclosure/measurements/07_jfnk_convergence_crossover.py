import sys, numpy as np
sys.path.insert(0,"/capstor/scratch/cscs/lhulsbergen/solver kernels/jfnk_bratu")
RTOL, ATOL = 1e-9, 1e-11
def viol(a,e): return float(np.max(np.abs(a-e)/(ATOL+RTOL*np.abs(e))))

def residual(u,N,lam):
    h=1.0/(N-1); F=np.zeros_like(u)
    F[0,:]=u[0,:]; F[N-1,:]=u[N-1,:]; F[:,0]=u[:,0]; F[:,N-1]=u[:,N-1]
    F[1:-1,1:-1]=(4*u[1:-1,1:-1]-u[:-2,1:-1]-u[2:,1:-1]-u[1:-1,:-2]-u[1:-1,2:])/(h*h)-lam*np.exp(u[1:-1,1:-1])
    return F
def nrm(A): return float(np.sqrt((A*A).sum()))
def jvp(u,v,Fu,N,lam):
    nv=nrm(v)
    if nv==0.0: return np.zeros_like(v)
    eps=np.sqrt(np.finfo(np.float64).eps)*(1.0+nrm(u))/nv
    return (residual(u+eps*v,N,lam)-Fu)/eps

def gmres(u,Fu,N,lam,restart,tol,prec=None):
    Q=np.zeros((N,N,restart+1)); H=np.zeros((restart+1,restart))
    cs=np.zeros(restart); sn=np.zeros(restart); g=np.zeros(restart+1); y=np.zeros(restart)
    beta=nrm(Fu); Q[:,:,0]=-Fu/beta; g[0]=beta; m=restart
    for k in range(restart):
        v = Q[:,:,k] if prec is None else prec(Q[:,:,k])
        w=jvp(u,v,Fu,N,lam)
        for p in range(k+1):
            H[p,k]=float((Q[:,:,p]*w).sum()); w-=H[p,k]*Q[:,:,p]
        hn=nrm(w); H[k+1,k]=hn
        for p in range(k):
            t=cs[p]*H[p,k]+sn[p]*H[p+1,k]; H[p+1,k]=-sn[p]*H[p,k]+cs[p]*H[p+1,k]; H[p,k]=t
        den=np.hypot(H[k,k],H[k+1,k]); cs[k]=H[k,k]/den; sn[k]=H[k+1,k]/den
        H[k,k]=cs[k]*H[k,k]+sn[k]*H[k+1,k]; H[k+1,k]=0.0
        t=cs[k]*g[k]; g[k+1]=-sn[k]*g[k]; g[k]=t
        if abs(g[k+1])/beta<tol or hn<1e-13 or k==restart-1: m=k+1; break
        Q[:,:,k+1]=w/hn
    for r in range(m-1,-1,-1):
        s=g[r]
        for c in range(r+1,m): s-=H[r,c]*y[c]
        y[r]=s/H[r,r]
    du=np.zeros((N,N))
    for p in range(m): du+=Q[:,:,p]*y[p]
    return (du if prec is None else prec(du)), m

def solve(N,lam,max_newton,inner_tol,restart,prec=None):
    u=np.zeros((N,N)); F=residual(u,N,lam); f0=nrm(F); ks=[]
    for _ in range(max_newton):
        if nrm(F)<=1e-10*f0: break
        du,m=gmres(u,F,N,lam,restart,inner_tol,prec); ks.append(m)
        u+=du; F=residual(u,N,lam)
    return u,ks
for N in (32, 48, 64, 96, 128):
    lam=6.0
    u=np.zeros((N,N)); F=residual(u,N,lam); f0=nrm(F); steps=0
    for _ in range(20):
        if nrm(F)<=1e-10*f0: break
        du,m=gmres(u,F,N,lam,50,1e-4,None); u+=du; F=residual(u,N,lam); steps+=1
    print(f"  N={N:>4}  newton steps used={steps:>3}/20   final ||F||/||F0|| = {nrm(F)/f0:.3e}   "
          f"{'CONVERGED' if nrm(F)<=1e-10*f0 else 'CAPPED at max_newton -- output is a fixed iterate'}")
