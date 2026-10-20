import numpy as np, ctypes, sys, os, warnings
warnings.filterwarnings('ignore')
sys.path.insert(0, 'work')
from ref import initialize, rk4_ensemble

def build(so_path, out_path):
    cmd = (f"gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math "
           f"-fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC "
           f"-Wall -Wextra -std=c23 -D_POSIX_C_SOURCE=199309L -fPIC "
           f"-c {so_path} -o /tmp/kc.o && "
           f"gcc -shared /tmp/kc.o -o {out_path} -lm -fopenmp 2>&1")
    r = os.system(cmd)
    if r != 0:
        raise RuntimeError(f"build failed: {r}")
    return out_path

def make_loader(lib_path):
    import os
    lib = ctypes.CDLL(os.path.abspath(lib_path))
    f = lib.rk4_ensemble_fp64
    f.argtypes = [ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
                  ctypes.c_int64, ctypes.c_int64, ctypes.c_double, ctypes.c_double,
                  ctypes.c_double, ctypes.c_double, ctypes.POINTER(ctypes.c_uint8), ctypes.c_int64]
    f.restype = None
    return f

def run(f, y0, NSTEPS, a, b, ep, t_end, wsbytes=65536):
    NSYS = y0.shape[0]
    y = np.zeros((NSYS, 3), dtype=np.float64)
    ws = (ctypes.c_uint8 * max(1, wsbytes))()
    f(y.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
      y0.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
      np.int64(NSTEPS), np.int64(NSYS),
      ctypes.c_double(a), ctypes.c_double(b), ctypes.c_double(ep), ctypes.c_double(t_end),
      ctypes.cast(ws, ctypes.POINTER(ctypes.c_uint8)), np.int64(wsbytes))
    return y

def compare(y_ref, y_c, rtol=1e-9, atol=1e-11, verbose=False):
    finite_ref = np.isfinite(y_ref)
    diff = np.abs(y_c - y_ref)
    tol = atol + rtol * np.abs(y_ref)
    bad = finite_ref & (diff > tol)
    nan_mismatch = (np.isnan(y_c) != np.isnan(y_ref))
    if np.any(bad) or np.any(nan_mismatch):
        if verbose:
            if np.any(bad):
                idx = np.argmax(bad)
                print(f"  MISMATCH: {bad.sum()} bad, first flat idx {idx} (sys {idx//3}, comp {idx%3})")
                print(f"    ref={y_ref.flat[idx]!r} c={y_c.flat[idx]!r} diff={diff.flat[idx]!r} tol={tol.flat[idx]!r}")
            else:
                idx = np.argmax(nan_mismatch)
                print(f"  NAN mismatch at flat idx {idx}: ref={y_ref.flat[idx]!r} c={y_c.flat[idx]!r}")
        return False
    return True

def correctness(f, a=1.0, b=3.0, ep=0.01, t_end=5.0):
    allok = True
    for NSYS in [100, 1000, 3000]:
        for NSTEPS in [1, 17, 525, 1000]:
            y0, _ = initialize(NSYS)
            y_ref = np.zeros_like(y0)
            rk4_ensemble(y0, y_ref, NSYS, NSTEPS, a, b, ep, t_end)
            y_c = run(f, y0, NSTEPS, a, b, ep, t_end)
            ok = compare(y_ref, y_c)
            if not ok:
                allok = False
                print(f"  FAIL a={a} b={b} ep={ep} t_end={t_end} NSYS={NSYS} NSTEPS={NSTEPS}")
                compare(y_ref, y_c, verbose=True)
    return allok


def maxrelerr(f, a=1.0,b=3.0,ep=0.01,t_end=5.0, NSYS=3000, NSTEPS=1000):
    y0,_=initialize(NSYS)
    y_ref=np.zeros_like(y0)
    rk4_ensemble(y0,y_ref,NSYS,NSTEPS,a,b,ep,t_end)
    y_c=run(f,y0,NSTEPS,a,b,ep,t_end)
    ref=np.abs(y_ref)
    rel=np.abs(y_c-y_ref)/(ref+1e-300)
    return float(np.max(rel)), float(np.max(np.abs(y_c-y_ref)))
