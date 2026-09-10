import sys, time
import numpy as np, scipy.sparse as sp
sys.path.insert(0, "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")
from hpcagent_bench.support.helpers.sparse.generators import make_stencil_3d

def levels_natural(A):
    """level[i] = 1 + max(level[j]: A[i,j]!=0, j<i); natural row order."""
    n = A.shape[0]
    indptr, indices = A.indptr, A.indices
    lev = np.zeros(n, dtype=np.int64)
    for i in range(n):
        cols = indices[indptr[i]:indptr[i+1]]
        prev = cols[cols < i]
        lev[i] = (lev[prev].max() + 1) if prev.size else 0
    return lev

for k in (16, 32, 48, 64):
    t0 = time.time()
    A = make_stencil_3d(k, k, k)
    lev = levels_natural(A)
    cnt = np.bincount(lev)
    n = A.shape[0]
    print(f"27pt {k}^3  N={n:>8}  nnz={A.nnz:>9}  levels={cnt.size:>7}  "
          f"avg_rows/lvl={n/cnt.size:8.1f}  max_rows/lvl={cnt.max():>7}  "
          f"lvls_with_1_row={(cnt==1).sum():>7}  ({time.time()-t0:.1f}s)")
