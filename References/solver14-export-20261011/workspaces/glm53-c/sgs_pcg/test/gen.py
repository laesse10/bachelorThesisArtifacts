import numpy as np, scipy.sparse as sp

def make_stencil_3d(NX, NY, NZ, rng=None, variable=True):
    # 27-point variable-coefficient operator, C-order grid: i = ix*NY*NZ + iy*NZ + iz
    rng = rng or np.random.default_rng(0)
    N = NX*NY*NZ
    offs = [(dx,dy,dz) for dx in (-1,0,1) for dy in (-1,0,1) for dz in (-1,0,1)]
    # build COO
    rows=[];cols=[]
    for dx in offs:
        for ix in range(NX):
            jx=ix+dx[0]
            if 0<=jx<NX:
                for iy in range(NY):
                    jy=iy+dx[1]
                    if 0<=jy<NY:
                        base=ix*NY*NZ+iy*NZ
                        jbase=jx*NY*NZ+jy*NZ
                        for iz in range(NZ):
                            jz=iz+dx[2]
                            if 0<=jz<NZ:
                                rows.append(base+iz); cols.append(jbase+jz)
    rows=np.array(rows,dtype=np.int64); cols=np.array(cols,dtype=np.int64)
    if variable:
        vals=rng.random(rows.shape[0])
    else:
        vals=np.ones(rows.shape[0])
    A=sp.coo_matrix((vals,(rows,cols)),shape=(N,N)).tocsr()
    A.sort_indices()
    # reweight so rows sum ~ constant and diag dominant-ish
    A=A.tolil()
    d=A.diagonal()
    rs=np.asarray(A.sum(axis=1)).ravel()
    return A.tocsr()

def reweight(A, rng):
    # keep pattern, reweight couplings, keep row sums
    Ac = A.tocoo()
    v = Ac.data * rng.uniform(0.5,2.0,Ac.nnz)
    B = sp.coo_matrix((v,(Ac.row,Ac.col)),shape=A.shape).tocsr()
    B.sort_indices()
    return B

def save(A, b, path):
    A = A.tocsr(); A.sort_indices()
    np.savez(path, data=A.data, indices=A.indptr.astype(np.int64), cols=A.indices.astype(np.int64), b=b)
