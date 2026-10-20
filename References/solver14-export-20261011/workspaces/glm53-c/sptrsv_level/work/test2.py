import numpy as np, ctypes, sys, os
sys.path.insert(0, os.path.dirname(__file__))
from gen import make_matrix, analyze, solve_ref
so, kind, N, seed, reps = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
L_indptr, L_indices, L_data = make_matrix(kind, N, seed)
level_ptr, perm, level = analyze(L_indptr, L_indices, N)
lib = ctypes.CDLL(so); f = lib.sptrsv_level_csr_fp64
P=ctypes.POINTER(ctypes.c_double); PI=ctypes.POINTER(ctypes.c_int64); PU=ctypes.POINTER(ctypes.c_uint8)
f.argtypes=[P,PI,PI,P,PI,PI,P,ctypes.c_int64,ctypes.c_int64,PU,ctypes.c_int64]
dp=lambda a:a.ctypes.data_as(P); di=lambda a:a.ctypes.data_as(PI)
ws=np.zeros(1,dtype=np.uint8)
ok=True
for it in range(reps):
    rng=np.random.default_rng(100+it)
    b=rng.random(N)
    if it>0:  # rescale diagonal like revalue()
        d = rng.random(N)+0.5
        for r in range(N): L_data[L_indptr[r+1]-1]*= d[r]
    x_ref=np.zeros(N); solve_ref(L_indptr,L_indices,L_data,b,level_ptr,perm,x_ref,N)
    x=np.zeros(N)
    f(dp(L_data),di(L_indices),di(L_indptr),dp(b),di(level_ptr),di(perm),dp(x),N,L_data.shape[0],ws.ctypes.data_as(PU),0)
    err=np.max(np.abs(x-x_ref)/np.maximum(np.abs(x_ref),1e-300))
    print(f"call {it}: err={err}", "OK" if err<1e-9 else "FAIL"); ok &= err<1e-9
print("ALL OK" if ok else "FAILED")
