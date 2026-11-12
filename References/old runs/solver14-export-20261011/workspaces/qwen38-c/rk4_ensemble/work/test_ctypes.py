import numpy as np, ctypes, sys, time, os, warnings
warnings.filterwarnings('ignore')
sys.path.insert(0, 'work')
from ref import initialize, rk4_ensemble

def build(so_path, out_path):
    os.system(f"gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math "
             f"-fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC "
             f"-Wall -Wextra -std=c23 -D_POSIX_C_SOURCE=199309L -fPIC "
             f"-c {so_path} -o /tmp/kc.o && "
             f"gcc -shared /tmp/kc.o -o {out_path} -lm -fopenmp")

def run(lib_path, y0, NSTEPS, a, b, ep, t_end, wsbytes):
    lib = ctypes.CDLL(lib_path)
    f = lib.rk4_ensemble_fp64
    f.argtypes = [ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double),
                  ctypes.c_int64, ctypes.c_int64, ctypes.c_double, ctypes.c_double,
                  ctypes.c_double, ctypes.c_double, ctypes.POINTER(ctypes.c_uint8), ctypes.c_int64]
    f.restype = None
    NSYS = y0.shape[0]
    y = np.zeros((NSYS, 3), dtype=np.float64)
    ws = (ctypes.c_uint8 * max(1, wsbytes))()
    f(y.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
      y0.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
      np.int64(NSTEPS), np.int64(NSYS),
      ctypes.c_double(a), ctypes.c_double(b), ctypes.c_double(ep), ctypes.c_double(t_end),
      ctypes.cast(ws, ctypes.POINTER(ctypes.c_uint8)), np.int64(wsbytes))
    return y

def compare(y_ref, y_c, rtol=1e-9, atol=1e-11):
    diff = np.abs(y_c - y_ref)
    tol = atol + rtol * np.abs(y_ref)
    bad = diff > tol
    if np.any(bad):
        idx = np.argmax(bad)
        n = idx // 3
        print(f"  MISMATCH: {bad.sum()} bad, first at flat idx {idx} (system {n}, comp {idx%3})")
        print(f"    ref={y_ref.flat[idx]!r} c={y_c.flat[idx]!r} diff={diff.flat[idx]!r} tol={tol.flat[idx]!r}")
        # relative
        for i in range(min(3, bad.sum())):
            pass
        return False
    return True

if __name__ == "__main__":
    so = sys.argv[1] if len(sys.argv) > 1 else "rk4_ensemble.c"
    libpath = so.replace(".c", ".so")
    build(so, libpath)
    for (a,b,ep,t_end) in [(1.0,3.0,0.01,5.0),(1.0,3.0,1.0,5.0),(2.0,4.0,0.5,10.0)]:
        for NSYS in [100, 1000, 3000]:
            for NSTEPS in [1, 17, 525, 1000]:
                y0, _ = initialize(NSYS)
                y_ref = np.zeros_like(y0)
                rk4_ensemble(y0, y_ref, NSYS, NSTEPS, a, b, ep, t_end)
                y_c = run(libpath, y0, NSTEPS, a, b, ep, t_end, 4096)
                ok = compare(y_ref, y_c)
                status = "OK" if ok else "FAIL"
                print(f"a={a} b={b} ep={ep} t_end={t_end} NSYS={NSYS} NSTEPS={NSTEPS}: {status}")
