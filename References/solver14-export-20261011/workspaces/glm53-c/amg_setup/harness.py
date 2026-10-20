import sys, ctypes, time, types
sys.path.insert(0, '/shared/agent-4')
import numpy as np
import gen27
import amg_setup_numpy as ref

_src = open('/shared/agent-4/test_ref.py').read().split("fails = 0")[0]
_tr = types.ModuleType('tr'); exec(_src, _tr.__dict__)
run_c = _tr.run_c

def run_ref(Adata, Aindices, Aindptr, NX, NY, NZ, theta):
    n = NX*NY*NZ
    level_n = np.zeros(16, dtype=np.int64)
    level_nnz = np.zeros(16, dtype=np.int64)
    nlevels = np.zeros(1, dtype=np.int64)
    agg0 = np.zeros(n, dtype=np.int64)
    ref.amg_setup(Adata, Aindices, Aindptr, level_n, level_nnz, nlevels, agg0, NX, NY, NZ, theta)
    return agg0, level_n, level_nnz, nlevels
