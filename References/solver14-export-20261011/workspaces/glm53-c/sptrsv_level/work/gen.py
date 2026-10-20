import numpy as np, sys

def make_matrix(kind, N, seed):
    rng = np.random.default_rng(seed)
    indptr = [0]; indices = []
    if kind == "mesh":
        # 2D mesh-like: node r depends on r-1, r-W, r-W-1 ... (natural ordering)
        W = int(np.sqrt(N*1.0))+1
        for r in range(N):
            cols = set()
            if r > 0: cols.add(r-1)
            if r >= W: cols.add(r-W)
            if r >= W+1: cols.add(r-W-1)
            if r >= W-1 and r-W-1 >= 0: cols.add(r-W+1)
            cols = sorted(c for c in cols if c < r)
            cols.append(r)
            indices += cols; indptr.append(indptr[-1]+len(cols))
    elif kind == "rand":
        for r in range(N):
            k = rng.integers(0, 4)
            cols = sorted(rng.choice(r, size=min(k, r), replace=False).tolist()) if r>0 else []
            cols.append(r)
            indices += cols; indptr.append(indptr[-1]+len(cols))
    L_indices = np.array(indices, dtype=np.int64)
    L_indptr = np.array(indptr, dtype=np.int64)
    L_data = rng.random(L_indices.shape[0])*0.3 + 0.05
    # make diagonal dominant so the solve stays bounded
    diagpos = np.array([indptr[r+1]-1 for r in range(N)])
    L_data[diagpos] = 2.0 + rng.random(N)
    return L_indptr, L_indices, L_data

def analyze(L_indptr, L_indices, N):
    level = np.zeros(N, dtype=np.int64)
    for row in range(N):
        best = -1
        for k in range(L_indptr[row], L_indptr[row+1]):
            col = L_indices[k]
            if col < row and level[col] > best:
                best = level[col]
        level[row] = best + 1
    counts = np.zeros(N, dtype=np.int64)
    for row in range(N): counts[level[row]] += 1
    level_ptr = np.zeros(N+1, dtype=np.int64)
    for lvl in range(N): level_ptr[lvl+1] = level_ptr[lvl] + counts[lvl]
    cursor = level_ptr[:N].copy()
    perm = np.zeros(N, dtype=np.int64)
    for row in range(N):
        home = level[row]; pos = cursor[home]; perm[pos] = row; cursor[home] = pos+1
    return level_ptr, perm, level

def solve_ref(L_indptr, L_indices, L_data, b, level_ptr, perm, x, N):
    for lvl in range(N):
        for idx in range(level_ptr[lvl], level_ptr[lvl+1]):
            row = perm[idx]
            s = b[row]; diag = 0.0
            for k in range(L_indptr[row], L_indptr[row+1]):
                col = L_indices[k]
                if col < row: s -= L_data[k]*x[col]
                elif col == row: diag = L_data[k]
            x[row] = s/diag
