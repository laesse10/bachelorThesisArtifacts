import importlib.util
import numpy as np

# Load the reference implementation
spec = importlib.util.spec_from_file_location('amg_setup_numpy', '/shared/agent-4/amg_setup_numpy.py')
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

NX, NY, NZ = 48, 48, 40
theta = 0.03

# Initialise operator A (CSR)
A, _, _, _, _ = mod.initialize(NX, NY, NZ)
A = A.tocsr()

agg0 = np.zeros(NX * NY * NZ, dtype=np.int64)
level_n = np.zeros(mod.LMAX, dtype=np.int64)
level_nnz = np.zeros(mod.LMAX, dtype=np.int64)
nlevels = np.zeros(1, dtype=np.int64)

mod.amg_setup(A.data, A.indices.astype(np.int64), A.indptr.astype(np.int64), agg0, level_n, level_nnz, nlevels, NX, NY, NZ, theta)

print('Depth:', nlevels[0])
print('level_n[:depth]:', level_n[:nlevels[0]])
print('level_nnz[:depth]:', level_nnz[:nlevels[0]])
