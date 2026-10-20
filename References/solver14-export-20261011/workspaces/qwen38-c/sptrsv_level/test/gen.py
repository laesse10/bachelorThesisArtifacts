import numpy as np

def grid_matrix(I, J, stencil="diagonal"):
    """2D grid I x J row-major, lower triangular incl diagonal.
    stencil: which strictly-lower neighbors to connect.
    5pt: (i-1,j),(i,j-1)
    9pt/45: (i-1,j-1),(i-1,j),(i-1,j+1),(i,j-1)
    """
    N = I*J
    idx = np.zeros(N, dtype=np.int64)  # node -> global index
    rows=[]; cols=[]; data=[]
    for i in range(I):
        for j in range(J):
            r = i*J + j
            neigh = [(i,j)]
            if stencil=="5pt":
                cands = [(i-1,j),(i,j-1)]
            elif stencil=="9pt":
                cands = [(i-1,j-1),(i-1,j),(i-1,j+1),(i,j-1)]
            else:
                cands = []
            for (ii,jj) in cands:
                if 0<=ii<I and 0<=jj<J:
                    neigh.append((ii,jj))
            for (ii,jj) in neigh:
                rows.append(r); cols.append(ii*J+jj); data.append(1.0)
    # build CSR with ascending cols
    rows=np.array(rows); cols=np.array(cols); data=np.array(data)
    order = np.argsort(rows* N + cols, kind="stable")
    rows=rows[order]; cols=cols[order]; data=data[order]
    # make it SPD-ish: diagonal positive, offdiag small
    # find diagonal
    indptr = np.zeros(N+1, dtype=np.int64)
    for r in range(N):
        pass
    # build via dict-free: since rows sorted ascending by construction
    indptr[0]=0
    k=0
    L_data=np.zeros(len(data)); L_indices=np.zeros(len(data),dtype=np.int64)
    L_data[:]=data; L_indices[:]=cols
    # rows is sorted; build indptr
    cnt = np.bincount(rows, minlength=N)
    indptr = np.concatenate([[0], np.cumsum(cnt)]).astype(np.int64)
    # ensure column ascending within row (sort was global by rows then cols) good
    # SPD-ish scaling: off-diagonal small
    for r in range(N):
        for kk in range(indptr[r], indptr[r+1]):
            if L_indices[kk] != r:
                L_data[kk] *= 0.1
    return indptr, L_indices, L_data, (I,J)

def build(N=82654, stencil="9pt"):
    I=286; J=289  # 82654
    indptr, L_indices, L_data, shape = grid_matrix(I,J,stencil)
    return indptr, L_indices, L_data, shape

if __name__=="__main__":
    import sys
    for st in ["5pt","9pt"]:
        indptr, idx, data, shape = build(stencil=st)
        N=len(indptr)-1
        nnz=len(data)
        print(f"stencil={st} shape={shape} N={N} NNZ_L={nnz} avg_offdiag={ (nnz-N)/N:.3f}")
