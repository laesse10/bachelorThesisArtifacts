import sys, time
sys.path.insert(0,'work')
import numpy as np, warnings; warnings.filterwarnings('ignore')
from numba import set_num_threads; set_num_threads(14)
from harness import build, make_loader, run
from ref import initialize
from ref_num import rk4_ensemble_nb

def maxrelerr(f, a,b,ep,t_end, NSYS, NSTEPS):
    y0,_=initialize(NSYS)
    y_ref=np.zeros_like(y0)
    rk4_ensemble_nb(y0,y_ref,NSYS,NSTEPS,a,b,ep,t_end)
    y_c=run(f,y0,NSTEPS,a,b,ep,t_end)
    ref=np.abs(y_ref)
    rel=np.abs(y_c-y_ref)/(ref+1e-300)
    ok = np.all(np.abs(y_c-y_ref) <= 1e-11 + 1e-9*ref)
    return float(np.max(rel)), float(np.max(np.abs(y_c-y_ref))), bool(ok)

def test(lib, NSYS=3000, NSTEPS=1000):
    f=make_loader(lib)
    allok=True
    for (a,b,ep,t_end) in [(1.0,3.0,0.1,5.0),(1.0,3.0,1.0,5.0),(2.0,4.0,0.5,5.0)]:
        for (s,ns) in [(3000,1000),(100,1)]:
            mr,ma,ok=maxrelerr(f,a,b,ep,t_end,s,ns)
            if not ok: allok=False
            print(f'  a={a} b={b} ep={ep} t_end={t_end} NSYS={s} NSTEPS={ns}: ok={ok} maxrel={mr:.3e} maxabs={ma:.3e}')
    # odd tail sizes
    for s in [571, 1001, 3333, 5000]:
        y0,_=initialize(s)
        y_ref=np.zeros_like(y0)
        rk4_ensemble_nb(y0,y_ref,s,525,1.0,3.0,0.1,5.0)
        y_c=run(f,y0,525,1.0,3.0,0.1,5.0)
        ok=np.all(np.abs(y_c-y_ref)<=1e-11+1e-9*np.abs(y_ref))
        if not ok: allok=False
        print(f'  tail NSYS={s} (mod8={s%8}): ok={ok} maxrel={np.max(np.abs(y_c-y_ref)/(np.abs(y_ref)+1e-300)):.3e}')
    return allok

if __name__=="__main__":
    so=sys.argv[1] if len(sys.argv)>1 else "rk4_simd.c"
    lib=so.replace(".c",".so")
    build(so,lib)
    print('Loaded',lib)
    print('ALL OK' if test(lib) else 'SOME FAILED')
