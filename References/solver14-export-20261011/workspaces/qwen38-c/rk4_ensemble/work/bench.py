import sys, time
sys.path.insert(0,'work')
from harness import build, make_loader, run
from ref import initialize
import numpy as np

def bench(lib, a=1.0,b=3.0,ep=0.01,t_end=5.0, NSYS=568117, NSTEPS=1000, reps=3):
    f = make_loader(lib)
    y0,_ = initialize(NSYS)
    # warmup
    run(f, y0, NSTEPS, a,b,ep,t_end)
    best=1e9
    for _ in range(reps):
        t=time.perf_counter()
        run(f, y0, NSTEPS, a,b,ep,t_end)
        best=min(best, time.perf_counter()-t)
    return best

if __name__=="__main__":
    so = sys.argv[1] if len(sys.argv)>1 else "rk4_ensemble.c"
    lib = so.replace(".c",".so")
    build(so, lib)
    import os
    for (a,b,ep,t_end) in [(1.0,3.0,0.01,5.0)]:
        for (NSYS,NSTEPS) in [(284570,525),(284570,1000),(568117,525),(568117,1000)]:
            t = bench(lib,a,b,ep,t_end,NSYS,NSTEPS)
            total_flops = NSYS*NSTEPS*85.0
            print(f"NSYS={NSYS} NSTEPS={NSTEPS}: {t*1e3:.1f} ms   ({total_flops/t/1e9:.1f} GFLOP/s approx)")
