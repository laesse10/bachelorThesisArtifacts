import sys, time
sys.path.insert(0,'work')
import numpy as np, warnings, ctypes; warnings.filterwarnings('ignore')
from harness import make_loader
from ref import initialize

def bench(lib, a,b,ep,t_end, NSYS, NSTEPS, reps=7):
    f = make_loader(lib)
    y0,_ = initialize(NSYS)
    y = np.zeros((NSYS,3), np.float64)
    ws = np.zeros(16, np.uint8)
    wsbuf = (ctypes.c_uint8*16).from_buffer(ws)
    def call():
        f(y.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
          y0.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
          np.int64(NSTEPS), np.int64(NSYS),
          ctypes.c_double(a),ctypes.c_double(b),ctypes.c_double(ep),ctypes.c_double(t_end),
          ctypes.cast(wsbuf,ctypes.POINTER(ctypes.c_uint8)), np.int64(16))
    call()
    ts=[]
    for _ in range(reps):
        t=time.perf_counter(); call(); ts.append(time.perf_counter()-t)
    ts=np.array(ts)*1e3
    return np.median(ts)

if __name__=="__main__":
    libs=sys.argv[1:]
    a,b,ep,t_end=1.0,3.0,0.1,5.0
    sizes=[(284570,525),(284570,1000),(568117,525),(568117,1000)]
    for lib in libs:
        row=[]
        for (NSYS,NSTEPS) in sizes:
            row.append(f"{bench(lib,a,b,ep,t_end,NSYS,NSTEPS):6.1f}")
        print(f"{lib:14s}: " + "  ".join(row) + "  (ms; sizes 284570x525, 284570x1000, 568117x525, 568117x1000)")
