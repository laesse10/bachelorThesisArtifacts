import sys, time, os
sys.path.insert(0,'work')
import numpy as np, warnings; warnings.filterwarnings('ignore')
from harness import build, make_loader, run
from ref import initialize

def bench(lib, a,b,ep,t_end, NSYS, NSTEPS, reps=5):
    f = make_loader(lib)
    y0,_ = initialize(NSYS)
    run(f, y0, NSTEPS, a,b,ep,t_end)  # warmup
    best=1e9
    for _ in range(reps):
        t=time.perf_counter()
        run(f, y0, NSTEPS, a,b,ep,t_end)
        best=min(best, time.perf_counter()-t)
    return best

if __name__=="__main__":
    so = sys.argv[1]
    lib = so.replace(".c",".so")
    build(so, lib)
    a,b,ep,t_end=1.0,3.0,0.1,5.0
    for (NSYS,NSTEPS) in [(284570,525),(284570,1000),(568117,525),(568117,1000)]:
        t = bench(lib,a,b,ep,t_end,NSYS,NSTEPS)
        print(f"NSYS={NSYS} NSTEPS={NSTEPS}: {t*1e3:8.1f} ms")
