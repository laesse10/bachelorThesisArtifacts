import numpy as np, sys, os
sys.path.insert(0, '/shared/agent-7')
from sparse_cholesky_numpy import sparse_cholesky_symbolic, sparse_cholesky

EDGE = int(os.environ.get("EDGE","12"))
N = EDGE**3
MAXNNZ = 16*EDGE**4
perm = np.zeros(N, dtype=np.int64); iperm = np.zeros(N, dtype=np.int64)
parent = np.zeros(N, dtype=np.int64); snode_ptr = np.zeros(N+1, dtype=np.int64)
Lc_indptr = np.zeros(N+1, dtype=np.int64); Lc_indices = np.zeros(MAXNNZ, dtype=np.int64)
L_indptr = np.zeros(N+1, dtype=np.int64); L_indices = np.zeros(MAXNNZ, dtype=np.int64)
L_to_Lc = np.zeros(MAXNNZ, dtype=np.int64)
sparse_cholesky_symbolic(perm, iperm, parent, snode_ptr, Lc_indptr, Lc_indices, L_indptr, L_indices, L_to_Lc, EDGE)
nnzL = int(Lc_indptr[N])

# Poisson CSR (from sparse_cholesky.py poisson_csr)
NB = ((-1,0,0),(1,0,0),(0,-1,0),(0,1,0),(0,0,-1),(0,0,1))
grid_id = np.arange(N, dtype=np.int64).reshape(EDGE,EDGE,EDGE)
rows=[grid_id.reshape(-1)]; cols=[grid_id.reshape(-1)]; vals=[np.full(N,6.0)]
for dx,dy,dz in NB:
    sx,tx = slice(max(0,-dx),EDGE-max(0,dx)), slice(max(0,dx),EDGE-max(0,-dx))
    sy,ty = slice(max(0,-dy),EDGE-max(0,dy)), slice(max(0,dy),EDGE-max(0,-dy))
    sz,tz = slice(max(0,-dz),EDGE-max(0,dz)), slice(max(0,dz),EDGE-max(0,-dz))
    src = grid_id[sx,sy,sz].reshape(-1); dst = grid_id[tx,ty,tz].reshape(-1)
    rows.append(dst); cols.append(src); vals.append(np.full(dst.size,-1.0))
rows=np.concatenate(rows); cols=np.concatenate(cols); vals=np.concatenate(vals)
order = np.argsort(rows, kind="stable")
rows,cols,vals = rows[order],cols[order],vals[order]
indptr = np.zeros(N+1,dtype=np.int64); indptr[1:] = np.cumsum(np.bincount(rows,minlength=N))
# permute
prows = np.repeat(np.arange(N,dtype=np.int64), np.diff(indptr))
nrows = iperm[prows]; ncols = iperm[cols]
order = np.argsort(nrows, kind="stable")
nrows,ncols,nvals = nrows[order],ncols[order],vals[order]
Ap_indptr = np.zeros(N+1,dtype=np.int64); Ap_indptr[1:] = np.cumsum(np.bincount(nrows,minlength=N))
Ap_indices = ncols.astype(np.int64); Ap_data = nvals

rng = np.random.default_rng(42)
x_true = rng.random(N)
b = np.zeros(N); np.add.at(b, rows, vals * x_true[cols])
b_perm = b[perm]

Lc_data = np.zeros(MAXNNZ); y = np.zeros(N)
sparse_cholesky(Ap_indptr, Ap_indices, Ap_data, Lc_indptr, Lc_indices, Lc_data, L_indptr, L_indices, L_to_Lc, b_perm, y, EDGE)

np.savez('/shared/agent-7/in%d.npz'%EDGE, A_data=Ap_data, A_indices=Ap_indices, A_indptr=Ap_indptr,
         L_indices=L_indices[:nnzL], L_indptr=L_indptr, L_to_Lc=L_to_Lc[:nnzL],
         Lc_data=Lc_data[:nnzL], Lc_indices=Lc_indices[:nnzL], Lc_indptr=Lc_indptr,
         b=b_perm, y=y, parent=parent, snode_ptr=snode_ptr, perm=perm, iperm=iperm)

# stats
lvl = np.zeros(N, dtype=np.int64)
for j in range(N):
    p = parent[j]
    lvl[j] = 0 if p<0 else lvl[p]+1
flops = sum((Lc_indptr[j+1]-Lc_indptr[j])**2 for j in range(N))
print("EDGE",EDGE,"N",N,"nnzL",nnzL,"flops(colcount^2)",flops,"height",lvl.max(),
      "levels hist(top):", np.bincount(lvl)[-8:], "nsnodes", np.sum(snode_ptr[1:]-snode_ptr[:-1] > 0) if False else None)
import collections
# count real supernodes
cnt=0; s=0
while s < N and snode_ptr[s] < N:
    cnt += 1; s += 1
print("real supernodes:", cnt, "max snode width:", max(snode_ptr[s+1]-snode_ptr[s] for s in range(cnt)))
print("max colcount:", max(Lc_indptr[j+1]-Lc_indptr[j] for j in range(N)))
