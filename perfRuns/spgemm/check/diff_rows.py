# Per-row comparison of cuBool's C = A*A against scipy on one SuiteSparse graph.
import sys, numpy as np, scipy.io, scipy.sparse as sp
sys.path.insert(0, '/capstor/scratch/cscs/lhulsbergen/spbench/thirdparty/cubool/python')
import pycubool as cb
name, und = sys.argv[1], int(sys.argv[2])
D = '/capstor/scratch/cscs/lhulsbergen/spbench/data'
A = sp.csr_matrix(scipy.io.mmread(f'{D}/{name}/{name}.mtx'))
A.data[:] = 1
if und:
    A = A + A.T
A = sp.csr_matrix(A > 0, dtype=np.int64); A.sort_indices()
n = A.shape[0]
coo = A.tocoo()
print('loaded', name, n, A.nnz, flush=True)
M = cb.Matrix.from_lists((n, n), coo.row.astype(np.uint32), coo.col.astype(np.uint32), is_sorted=True, no_duplicates=True)
C = M.mxm(M)
r, c = C.to_lists()
r = np.ctypeslib.as_array(r).astype(np.int64); c = np.ctypeslib.as_array(c).astype(np.int64)
ref = (A @ A).tocsr(); ref.sort_indices()
print('cubool nvals', C.nvals, 'scipy nnz', ref.nnz, flush=True)
cnt = np.bincount(r, minlength=n); rcnt = np.diff(ref.indptr)
bad = np.nonzero(cnt != rcnt)[0]
print('rows differing:', len(bad), ' extra total:', int((cnt - rcnt)[bad].sum()), flush=True)
deg = np.diff(A.indptr)
prod = np.zeros(n, dtype=np.int64)
nz = deg > 0
prod[nz] = np.add.reduceat(deg[A.indices], A.indptr[:-1][nz])
order = np.argsort(r, kind='stable'); rs = r[order]; cs = c[order]
starts = np.searchsorted(rs, np.arange(n + 1))
for i in bad[:10]:
    cols = cs[starts[i]:starts[i + 1]]
    refc = ref.indices[ref.indptr[i]:ref.indptr[i + 1]]
    print(f'row {i}: cubool {cnt[i]} ref {rcnt[i]} dup_in_row {len(cols) - len(np.unique(cols))} '
          f'not_in_ref {len(np.setdiff1d(cols, refc))} missing {len(np.setdiff1d(refc, cols))} prod {prod[i]}', flush=True)
if len(bad):
    print('prod of differing rows: min', prod[bad].min(), 'max', prod[bad].max(),
          '| rows with prod > 4096 overall:', int((prod > 4096).sum()), flush=True)
