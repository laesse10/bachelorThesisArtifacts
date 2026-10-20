import numpy as np
import os

def save(path, indptr, indices, data):
    np.save(path+'_indptr.npy', indptr.astype(np.int64))
    np.save(path+'_indices.npy', indices.astype(np.int64))
    np.save(path+'_data.npy', data.astype(np.float64))
    print(path, "N=", indptr.shape[0]-1, "nnz=", len(indices))

def gen_grid(nx, ny, nz, seed=0):
    rng = np.random.default_rng(seed)
    N = nx*ny*nz
    eid0, eid1 = [], []
    for z in range(nz):
        for y in range(ny):
            base = y*nx + z*nx*ny
            for x in range(nx):
                i = base + x
                if x+1 < nx: eid0.append(i); eid1.append(i+1)
                if y+1 < ny: eid0.append(i); eid1.append(i+nx)
                if z+1 < nz: eid0.append(i); eid1.append(i+nx*ny)
    eid0 = np.array(eid0, dtype=np.int64); eid1 = np.array(eid1, dtype=np.int64)
    off = -rng.uniform(0.5, 1.5, len(eid0))
    deg = np.bincount(eid0, minlength=N) + np.bincount(eid1, minlength=N)
    diag = deg*1.0 + 0.5
    rows = np.concatenate([eid0, eid1, np.arange(N, dtype=np.int64)])
    cols = np.concatenate([eid1, eid0, np.arange(N, dtype=np.int64)])
    vals = np.concatenate([off, off, diag])
    order = np.lexsort((cols, rows))
    rows, cols, vals = rows[order], cols[order], vals[order]
    cnt = np.bincount(rows, minlength=N)
    indptr = np.zeros(N+1, dtype=np.int64); indptr[1:] = np.cumsum(cnt)
    return indptr, cols, vals

def gen_random(N, deg, seed=0, unsorted=False):
    rng = np.random.default_rng(seed)
    # random symmetric graph: each node i connects to deg random later nodes
    e0, e1 = [], []
    for i in range(N):
        k = rng.integers(0, deg+1)
        k = min(k, N-1-i)
        for _ in range(k):
            j = rng.integers(i+1, N)
            e0.append(i); e1.append(int(j))
    e0 = np.array(e0, dtype=np.int64); e1 = np.array(e1, dtype=np.int64)
    off = -rng.uniform(0.5, 1.5, len(e0))
    degc = np.bincount(e0, minlength=N) + np.bincount(e1, minlength=N)
    diag = degc*1.0 + 0.5
    rows = np.concatenate([e0, e1, np.arange(N, dtype=np.int64)])
    cols = np.concatenate([e1, e0, np.arange(N, dtype=np.int64)])
    vals = np.concatenate([off, off, diag])
    order = np.lexsort((cols, rows))
    rows, cols, vals = rows[order], cols[order], vals[order]
    cnt = np.bincount(rows, minlength=N)
    indptr = np.zeros(N+1, dtype=np.int64); indptr[1:] = np.cumsum(cnt)
    if unsorted:
        # shuffle entries within each row
        newcols = cols.copy(); newvals = vals.copy()
        for i in range(N):
            s, e = indptr[i], indptr[i+1]
            if e-s > 1:
                p = rng.permutation(e-s)
                newcols[s:e] = cols[s:e][p]; newvals[s:e] = vals[s:e][p]
        cols, vals = newcols, newvals
    return indptr, cols, vals

if __name__ == "__main__":
    out = "/shared/agent-5/work"
    # big: 3D grid approximating thermal1 (82654 rows, ~7 nnz/row)
    ip, ci, da = gen_grid(17, 26, 187, seed=42)
    save(os.path.join(out, "big"), ip, ci, da)
    # medium grid
    ip, ci, da = gen_grid(11, 13, 40, seed=1)
    save(os.path.join(out, "med"), ip, ci, da)
    # small random tests
    for i, (n, d, s) in enumerate([(50,3,0),(200,5,1),(1000,6,2),(300,4,3)]):
        ip, ci, da = gen_random(n, d, seed=s)
        save(os.path.join(out, f"rnd{i}"), ip, ci, da)
    # unsorted variant
    ip, ci, da = gen_random(400, 5, seed=7, unsorted=True)
    save(os.path.join(out, "uns"), ip, ci, da)
