import numpy as np, os
from ref import analyze, solve
def build_csr(I,J,cands_fn):
    N=I*J; rows=[];cols=[]
    for i in range(I):
        for j in range(J):
            r=i*J+j
            for (ii,jj) in [(i,j)]+cands_fn(i,j,I,J):
                if 0<=ii<I and 0<=jj<J: rows.append(r); cols.append(ii*J+jj)
    rows=np.array(rows); cols=np.array(cols)
    order=np.argsort(rows.astype(np.int64)*N+cols,kind="stable")
    rows=rows[order]; cols=cols[order]
    cnt=np.bincount(rows,minlength=N)
    indptr=np.concatenate([[0],np.cumsum(cnt)]).astype(np.int64)
    return indptr, cols.astype(np.int64), len(rows)
I,J=18,20; N=I*J
indptr,idx,nnz=build_csr(I,J,lambda i,j,I,J:[(i-1,j-1),(i-1,j),(i,j-1)])
rng=np.random.default_rng(0)
data=rng.uniform(-0.5,0.5,nnz)
for r in range(N):
  for k in range(indptr[r],indptr[r+1]):
    if idx[k]==r: data[k]=1.0+abs(rng.random())
b=rng.random(N)
lp=np.zeros(N+1,dtype=np.int64); pm=np.zeros(N,dtype=np.int64)
lv=analyze(indptr,idx,lp,pm,N)
x=np.zeros(N); solve(indptr,idx,data,b,lp,pm,x,N)
d="ms"; os.makedirs(d,exist_ok=True)
np.array([N,nnz],np.int64).tofile(d+"/hdr.i64")
indptr.tofile(d+"/indptr.i64"); idx.tofile(d+"/indices.i64"); data.tofile(d+"/data.f64")
b.tofile(d+"/b.f64"); lp.tofile(d+"/level_ptr.i64"); pm.tofile(d+"/perm.i64"); x.tofile(d+"/xref.f64")
print("small N=%d nnz=%d levels=%d"%(N,nnz,lv.max()+1))
