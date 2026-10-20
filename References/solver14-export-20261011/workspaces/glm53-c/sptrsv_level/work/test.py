import numpy as np, ctypes, sys, os, time
sys.path.insert(0, os.path.dirname(__file__))
from gen import make_matrix, analyze, solve_ref

so = sys.argv[1] if len(sys.argv) > 1 else "/shared/agent-6/work/libkernel.so"
kind = sys.argv[2] if len(sys.argv) > 2 else "mesh"
N = int(sys.argv[3]) if len(sys.argv) > 3 else 20000
seed = int(sys.argv[4]) if len(sys.argv) > 4 else 0

L_indptr, L_indices, L_data = make_matrix(kind, N, seed)
level_ptr, perm, level = analyze(L_indptr, L_indices, N)
rng = np.random.default_rng(7)
b = rng.random(N)
x_ref = np.zeros(N)
solve_ref(L_indptr, L_indices, L_data, b, level_ptr, perm, x_ref, N)

lib = ctypes.CDLL(so)
f = lib.sptrsv_level_csr_fp64
P = ctypes.POINTER(ctypes.c_double); PI = ctypes.POINTER(ctypes.c_int64); PU = ctypes.POINTER(ctypes.c_uint8)
f.argtypes = [P, PI, PI, P, PI, PI, P, ctypes.c_int64, ctypes.c_int64, PU, ctypes.c_int64]
def dp(a): return a.ctypes.data_as(P)
def di(a): return a.ctypes.data_as(PI)

x = np.zeros(N)
ws = np.zeros(1, dtype=np.uint8)
f(dp(L_data), di(L_indices), di(L_indptr), dp(b), di(level_ptr), di(perm), dp(x),
  N, L_data.shape[0], ws.ctypes.data_as(PU), 0)
err = np.max(np.abs(x - x_ref) / np.maximum(np.abs(x_ref), 1e-300))
print("max rel err:", err)
print("OK" if err < 1e-9 else "FAIL")
