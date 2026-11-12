import sys, time
import numpy as np
sys.path.insert(0, '/shared/agent-4')
from amg_setup_numpy import amg_setup
from gen_stencil import make_stencil_3d

NX, NY, NZ, theta, seed = 16, 16, 16, 0.03, 42
A, A_ix, A_ip = make_stencil_3d(NX, NY, NZ, seed=seed)
n0 = NX*NY*NZ
LMAX=16
level_n = np.zeros(LMAX, dtype=np.int64)
level_nnz = np.zeros(LMAX, dtype=np.int64)
nlevels = np.zeros(1, dtype=np.int64)
agg0 = np.zeros(n0, dtype=np.int64)
t0=time.time()
amg_setup(A, A_ix, A_ip, level_n, level_nnz, nlevels, agg0, NX, NY, NZ, theta)
t1=time.time()
d = np.concatenate([nlevels, level_n, level_nnz])
np.save('/tmp/pure_meta.npy', d)
np.save('/tmp/pure_agg0.npy', agg0)
print(f"pure time {t1-t0:.2f}s")
print("nlevels", nlevels[0])
print("level_n", level_n[:nlevels[0]].tolist())
print("level_nnz", level_nnz[:nlevels[0]].tolist())
print("agg0[:16]", agg0[:16].tolist())
