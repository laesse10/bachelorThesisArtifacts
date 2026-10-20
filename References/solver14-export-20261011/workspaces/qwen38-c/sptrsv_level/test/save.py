import numpy as np, os
from gen import grid_matrix
from ref import analyze, solve

def build_csr(I,J,cands_fn):
    N=I*J; rows=[];cols=[];
    for i in range(I):
        for j in range(J):
            r=i*J+j
            neigh=[(i,j)]+cands_fn(i,j,I,J)
            for (ii,jj) in neigh:
                if 0<=ii<I and 0<=jj<J:
                    rows.append(r); cols.append(ii*J+jj)
    rows=np.array(rows); cols=np.array(cols)
    order=np.argsort(rows.astype(np.int64)*N+cols,kind="stable")
    rows=rows[order]; cols=cols[order]
    cnt=np.bincount(rows,minlength=N)
    indptr=np.concatenate([[0],np.cumsum(cnt)]).astype(np.int64)
    L_indices=cols.astype(np.int64)
    return indptr,L_indices,len(rows)

def gen(name, cands_fn, seed=1):
    I,J=286,289
    indptr,L_indices,nnz=build_csr(I,J,cands_fn)
    N=len(indptr)-1
    rng=np.random.default_rng(seed)
    L_data=rng.uniform(-0.5,0.5,size=nnz)
    # make diagonal dominant positive
    for r in range(N):
        for k in range(indptr[r],indptr[r+1]):
            if L_indices[k]==r:
                L_data[k]=1.0+abs(rng.random())
    b=rng.random(N)
    level_ptr=np.zeros(N+1,dtype=np.int64); perm=np.zeros(N,dtype=np.int64)
    level=analyze(indptr,L_indices,level_ptr,perm,N)
    x=np.zeros(N)
    solve(indptr,L_indices,L_data,b,level_ptr,perm,x,N)
    d=os.path.join(os.path.dirname(__file__),name)
    os.makedirs(d,exist_ok=True)
    def w(fn,a): a.astype(a.dtype).tofile(os.path.join(d,fn))
    np.array([N,nnz],dtype=np.int64).tofile(os.path.join(d,"hdr.i64"))
    w("indptr.i64",indptr); w("indices.i64",L_indices); w("data.f64",L_data)
    w("b.f64",b); w("level_ptr.i64",level_ptr); w("perm.i64",perm); w("xref.f64",x)
    L=level.max()+1
    counts=np.bincount(level,minlength=L)
    print(f"{name}: N={N} nnz={nnz} levels={L} maxwidth={counts.max()} meanwidth={counts.mean():.1f}")

c3=lambda i,j,I,J: [(i-1,j-1),(i-1,j),(i,j-1)]
c5=lambda i,j,I,J: [(i-1,j),(i,j-1)]
c9=lambda i,j,I,J: [(i-1,j-1),(i-1,j),(i-1,j+1),(i,j-1)]
gen("m3",c3); gen("m5",c5); gen("m9",c9)
