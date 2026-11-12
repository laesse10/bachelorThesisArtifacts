import numpy as np, sys

def gen(N, seed=12345, wide=37):
    rng = np.random.default_rng(seed)
    indptr = np.zeros(N+1, np.int64)
    # build pattern: diag + +-1 + +-wide (grid-like), sorted unique
    cols = []
    for i in range(N):
        c = [i]
        for d in (1, wide):
            if i-d >= 0: c.append(i-d)
            if i+d < N: c.append(i+d)
        c = sorted(set(c))
        cols.append(c)
    indptr = np.array([0] + [len(c) for c in cols], np.int64).cumsum()
    indices = np.concatenate([np.array(c, np.int64) for c in cols]) if cols else np.zeros(0, np.int64)
    data = np.zeros(indptr[-1])
    for i in range(N):
        a, b = indptr[i], indptr[i+1]
        off = [data[a:] ]
        vals = rng.uniform(0.05, 1.0, b-a)
        for j in range(b-a):
            if indices[a+j] == i:
                data[a+j] = vals[:j].sum() + vals[j:] .sum() + 0.5  # placeholder
    # simpler: diag = sum of |offdiag| + 0.5
    for i in range(N):
        a, b = indptr[i], indptr[i+1]
        s = 0.0
        for j in range(a, b):
            v = rng.uniform(0.05, 1.0)
            data[j] = v
            if indices[j] != i: s += v
        for j in range(a, b):
            if indices[j] == i: data[j] = s + 0.5
    return data, indices, indptr

if __name__ == "__main__":
    N = int(sys.argv[1]) if len(sys.argv) > 1 else 82654
    data, indices, indptr = gen(N)
    data.tofile("data.bin"); indices.tofile("indices.bin"); indptr.tofile("indptr.bin")
    rl = np.diff(indptr)
    print("N", N, "nnz", len(data), "rowlen min/max/mean", rl.min(), rl.max(), rl.mean())
