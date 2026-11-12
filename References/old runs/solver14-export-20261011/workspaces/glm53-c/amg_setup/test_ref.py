import sys, ctypes, time
sys.path.insert(0, '/shared/agent-4')
import numpy as np
import gen27
import amg_setup_numpy as ref

lib = ctypes.CDLL('/tmp/libkernel.so')
lib.amg_setup_csr_fp64.restype = None
lib.amg_setup_csr_fp64.argtypes = [
    np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, flags='C_CONTIGUOUS'),
    ctypes.c_int64, ctypes.c_int64, ctypes.c_int64,
    ctypes.c_double, ctypes.c_void_p, ctypes.c_int64,
]

def run_c(Adata, Aindices, Aindptr, NX, NY, NZ, theta):
    n = NX*NY*NZ
    level_n = np.zeros(16, dtype=np.int64)
    level_nnz = np.zeros(16, dtype=np.int64)
    nlevels = np.zeros(1, dtype=np.int64)
    agg0 = np.zeros(n, dtype=np.int64)
    lib.amg_setup_csr_fp64(Adata, Aindices, Aindptr, agg0, level_n, level_nnz, nlevels,
                           NX, NY, NZ, theta, None, 0)
    return agg0, level_n, level_nnz, nlevels

def run_ref(Adata, Aindices, Aindptr, NX, NY, NZ, theta):
    n = NX*NY*NZ
    level_n = np.zeros(16, dtype=np.int64)
    level_nnz = np.zeros(16, dtype=np.int64)
    nlevels = np.zeros(1, dtype=np.int64)
    agg0 = np.zeros(n, dtype=np.int64)
    ref.amg_setup(Adata, Aindices, Aindptr, level_n, level_nnz, nlevels, agg0, NX, NY, NZ, theta)
    return agg0, level_n, level_nnz, nlevels

fails = 0
for (NX,NY,NZ) in [(8,8,8),(16,16,16),(24,16,8),(8,24,16)]:
    for theta in [0.0, 0.01, 0.03, 0.05, 0.08, 0.3]:
        data, ind, ptr = gen27.make_stencil_3d(NX,NY,NZ,seed=NX*100+NY*10+NZ)
        t0=time.time(); rc = run_c(data, ind, ptr, NX,NY,NZ,theta); tc=time.time()-t0
        t0=time.time(); rr = run_ref(data, ind, ptr, NX,NY,NZ,theta); tr=time.time()-t0
        ok = all(np.array_equal(a,b) for a,b in zip(rc,rr))
        if not ok:
            fails += 1
            print(f"FAIL {NX}x{NY}x{NZ} theta={theta}")
            for name,(a,b) in zip(['agg0','level_n','level_nnz','nlevels'],zip(rc,rr)):
                if not np.array_equal(a,b):
                    print("  ", name, "C:", a[:20] if name=='agg0' else a, " ref:", b[:20] if name=='agg0' else b)
        else:
            print(f"ok  {NX}x{NY}x{NZ} theta={theta}: levels={rc[3][0]} n={rc[1][:5]} c={tc*1e3:.1f}ms ref={tr*1e3:.1f}ms")
print("FAILS:", fails)
