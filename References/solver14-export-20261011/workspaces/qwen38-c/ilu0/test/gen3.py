import numpy as np, sys

def gen3(N=82654, seed=7, block0=14203, nblocks=970, f0=2, pf=2, ff=2):
    rng = np.random.default_rng(seed)
    adj = [set() for _ in range(N)]
    # block boundaries: bounds[0]=block0, bounds[nblocks]=N
    bounds = [int(block0 + round(b*(N-block0)/nblocks)) for b in range(nblocks+1)]
    bounds[0] = block0; bounds[-1] = N
    # block of row i
    block = np.searchsorted(bounds[1:], np.arange(N), side='right')
    nb = nblocks
    for i in range(N):
        b = int(block[i])
        adj[i].add(i)
        if b == 0:
            for _ in range(f0):
                # forward to a later block
                cb = int(rng.integers(1, nb))
                lo, hi = bounds[cb], bounds[cb+1]
                j = int(rng.integers(lo, hi))
                adj[i].add(j); adj[j].add(i)
        else:
            # 1 parent in block b-1 (ensures level==b)
            lo, hi = bounds[b-1], bounds[b]
            k = int(rng.integers(lo, hi)); adj[i].add(k); adj[k].add(i)
            # extra parents in random earlier blocks
            for _ in range(pf):
                cb = int(rng.integers(0, b))
                lo, hi = bounds[cb], bounds[cb+1]
                k = int(rng.integers(lo, hi)); adj[i].add(k); adj[k].add(i)
            # forward to later blocks
            for _ in range(ff):
                if b+1 < nb:
                    cb = int(rng.integers(b+1, nb))
                    lo, hi = bounds[cb], bounds[cb+1]
                    j = int(rng.integers(lo, hi)); adj[i].add(j); adj[j].add(i)
    indptr = np.zeros(N+1, np.int64)
    cols = []
    for i in range(N):
        c = sorted(adj[i])
        cols.extend(c)
        indptr[i+1] = indptr[i] + len(c)
    indices = np.array(cols, np.int64)
    nnz = len(indices)
    data = np.zeros(nnz)
    rng2 = np.random.default_rng(seed+1)
    for i in range(N):
        a, bb = indptr[i], indptr[i+1]
        off = [0.0]
        s = 0.0
        for j in range(a, bb):
            v = rng2.uniform(-1.0, 1.0)
            data[j] = v
            if indices[j] != i: s += abs(v)
        for j in range(a, bb):
            if indices[j] == i: data[j] = s + 0.5
    data.tofile("data.bin"); indices.tofile("indices.bin"); indptr.tofile("indptr.bin")
    rl = np.diff(indptr)
    print("N", N, "nnz", nnz, "rowlen min/max/mean", rl.min(), rl.max(), round(rl.mean(),3))

if __name__ == "__main__":
    gen3()
