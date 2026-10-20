import sys
sys.path.insert(0,'work')
import numpy as np, warnings, os; warnings.filterwarnings('ignore')
from numba import set_num_threads; set_num_threads(14)
from harness import make_loader, run
from ref import initialize
from ref_num import rk4_ensemble_nb

lib='rk4_final.so'
os.system(f"gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC -Wall -Wextra -std=c23 -D_POSIX_C_SOURCE=199309L -fPIC -c rk4_ensemble.c -o /tmp/fin.o && gcc -shared /tmp/fin.o -o {lib} -lm -fopenmp")
f = make_loader(lib)

allok=True; ntests=0
# size classes
sizes=[64,128,256,512, 3073(0) if False else 3073, 2048, 1024, 4096, 3333, 1001, 571, 2040, 2042, 1280, 64, 16, 8, 1, 15, 31]
sizes=[64,128,256,512,3073,2048,1024,4096,3333,1001,571,2040,2042,1280,16,8,1,15,31]
params=[(1.0,3.0,0.1,5.0),(1.0,3.0,1.0,5.0),(2.0,4.0,0.5,10.0),(3.0,3.0,1.0,5.0),(0.5,2.0,2.0,8.0),(1.5,2.5,0.3,3.0)]
nsteps=[1,17,525,700,1000]
import itertools
# Reduce combos for speed: test all params x a subset of sizes, and full for a few
for (a,b,ep,t_end) in params:
    for ns in [2048,3073,1001,2042,16,1]:
        for ns_ in [nsteps] if False else [525,1000]:
            y0,_=initialize(ns); y_ref=np.zeros_like(y0)
            rk4_ensemble_nb(y0,y_ref,ns,ns_,a,b,ep,t_end)
            y_c=run(f,y0,ns_,a,b,ep,t_end)
            ok=np.all(np.abs(y_c-y_ref)<=1e-11+1e-9*np.abs(y_ref)); ntests+=1
            allok=allok and ok
            if not ok: print(f"FAIL a={a} b={b} ep={ep} t_end={t_end} NSYS={ns} NSTEPS={ns_}")
# edge NSTEPS
for ns in [2048,3073]:
    for ns_ in [1,17]:
        y0,_=initialize(ns); y_ref=np.zeros_like(y0)
        rk4_ensemble_nb(y0,y_ref,ns,ns_,1.0,3.0,0.1,5.0)
        y_c=run(f,y0,ns_,1.0,3.0,0.1,5.0)
        ok=np.all(np.abs(y_c-y_ref)<=1e-11+1e-9*np.abs(y_ref)); ntests+=1
        allok=allok and ok
        if not ok: print(f"FAIL edge NSYS={ns} NSTEPS={ns_}")
print(f"{ntests} tests: {'ALL CORRECT' if allok else 'SOME FAILED'}")
