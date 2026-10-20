import sys, numpy as np
sys.path.insert(0,'/shared/agent-4')
from gen_stencil import make_stencil_3d
import amg_setup_numpy as ref

NX, NY, NZ = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3])
seed = int(sys.argv[4]) if len(sys.argv)>4 else 7
theta = float(sys.argv[5]) if len(sys.argv)>5 else 0.03
n0 = NX*NY*NZ
data, indices, indptr = make_stencil_3d(NX, NY, NZ, seed=seed)

LMAX = ref.LMAX
agg0 = np.zeros(n0, dtype=np.int64)
level_n = np.zeros(LMAX, dtype=np.int64)
level_nnz = np.zeros(LMAX, dtype=np.int64)
nlevels = np.zeros(1, dtype=np.int64)
ref.amg_setup(data, indices, indptr, level_n, level_nnz, nlevels, agg0, NX, NY, NZ, theta)
print("nlevels", int(nlevels[0]))
print("level_n", level_n.tolist())
print("level_nnz", level_nnz.tolist())
print("agg0_first20", agg0[:20].tolist())
print("agg0_unique", np.unique(agg0).shape[0])
np.save("/tmp/ref_out.npy", np.array([nlevels[0], level_n.sum(), level_nnz.sum(), agg0.sum()]))
np.save("/tmp/ref_full.npy", (level_n, level_nnz, agg0, nlevels))
print("SAVED /tmp/ref_full.npy")
