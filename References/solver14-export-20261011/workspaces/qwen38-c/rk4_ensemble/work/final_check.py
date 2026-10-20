import sys
sys.path.insert(0,'work')
import numpy as np, warnings, time; warnings.filterwarnings('ignore')
from numba import set_num_threads; set_num_threads(14)
from harness import make_loader, run
from ref import initialize
from ref_num import rk4_ensemble_nb

lib = sys.argv[1] if len(sys.argv)>1 else 'rk4_ensemble.so'
build_cmd = f"gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC -Wall -Wextra -std=c23 -D_POSIX_C_SOURCE=199309L -fPIC -c rk4_ensemble.c -o /tmp/fin.o && gcc -shared /tmp/fin.o -o {lib} -lm -fopenmp"
import os; os.system(build_cmd)
f = make_loader(lib)

allok = True
# 4 size classes around ~3000
classes = {
  'mult64': 3072,      # mult of 64
  'odd': 3073,         # odd
  '8xodd': 3064,       # 8*383=3064, 383 odd, not mult 16
  'even_not8': 3074,   # 2*1537, odd -> not mult 4
}
params = [(1.0,3.0,0.1,5.0),(1.0,3.0,1.0,5.0),(2.0,4.0,0.5,5.0)]
for pname,(a,b,ep,t_end) in enumerate(params):
    for cname,nsys in classes.items():
        for nsteps in [525,1000]:
            y0,_=initialize(nsys); y_ref=np.zeros_like(y0)
            rk4_ensemble_nb(y0,y_ref,nsys,nsteps,a,b,ep,t_end)
            y_c=run(f,y0,nsteps,a,b,ep,t_end)
            ok=np.all(np.abs(y_c-y_ref)<=1e-11+1e-9*np.abs(y_ref))
            mr=float(np.max(np.abs(y_c-y_ref)/(np.abs(y_ref)+1e-300)))
            allok=allok and ok
            if not ok:
                print(f"FAIL p{pname} {cname} NSYS={nsys} NSTEPS={nsteps} maxrel={mr:.2e}")
# edge cases
for nsys in [1,2,3,7,8,9,15,16,17,23,24,25]:
    y0,_=initialize(nsys); y_ref=np.zeros_like(y0)
    rk4_ensemble_nb(y0,y_ref,nsys,600,1.0,3.0,0.1,5.0)
    y_c=run(f,y0,600,1.0,3.0,0.1,5.0)
    ok=np.all(np.abs(y_c-y_ref)<=1e-11+1e-9*np.abs(y_ref)); allok=allok and ok
    if not ok: print(f"FAIL edge NSYS={nsys}")
# large-size perf (no reference, just timing) + correctness on one large class-3/4
for (nsys,nsteps) in [(284576,525),(284568,525),(284574,525),(284571,525)]:
    y0,_=initialize(nsys); y_ref=np.zeros_like(y0)
    t0=time.time(); rk4_ensemble_nb(y0,y_ref,nsys,nsteps,1.0,3.0,0.1,5.0); tref=time.time()-t0
    y=np.zeros((nsys,3),np.float64)
    import ctypes; ws=np.zeros(16,np.uint8); wb=(ctypes.c_uint8*16).from_buffer(ws)
    def call():
        f(y.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), y0.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
          np.int64(nsteps),np.int64(nsys),ctypes.c_double(1.0),ctypes.c_double(3.0),ctypes.c_double(0.1),ctypes.c_double(5.0),
          ctypes.cast(wb,ctypes.POINTER(ctypes.c_uint8)),np.int64(16))
    call(); call()
    ts=[]
    for _ in range(5):
        t=time.perf_counter(); call(); ts.append(time.perf_counter()-t)
    ok=np.all(np.abs(y-y_ref)<=1e-11+1e-9*np.abs(y_ref))
    allok=allok and ok
    print(f"NSYS={nsys} (mod24={nsys%24}, mod8={nsys%8}) NSTEPS={nsteps}: t={np.median(ts)*1e3:6.1f}ms  ok={ok}")
print("ALL CORRECT" if allok else "SOME FAILED")
