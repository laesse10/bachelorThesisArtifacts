import numpy as np, time, sys
from gen import build

def analyze_fast(indptr, indices, N):
    level = np.zeros(N, dtype=np.int64)
    for r in range(N):
        lo, hi = indptr[r], indptr[r+1]
        if lo<hi:
            cols = indices[lo:hi]
            cols = cols[cols<r]
            if cols.size:
                level[r] = level[cols].max()+1
            else:
                level[r]=0
        else:
            level[r]=0
    return level

for st in ["5pt","9pt"]:
    indptr, idx, data, shape = build(stencil=st)
    N=len(indptr)-1
    t0=time.time()
    level = analyze_fast(indptr, idx, N)
    dt=time.time()-t0
    L = level.max()+1
    counts = np.bincount(level, minlength=L)
    widths = counts[counts>0]
    print(f"stencil={st} N={N} NNZ={len(data)} nlevels={L} time={dt:.2f}s")
    print(f"  width: min={widths.min()} max={widths.max()} mean={widths.mean():.1f}")
    print(f"  level0 count={counts[0]}  (=rows with no strictly-lower dep)")
    # cumulative: what fraction of rows are in last K levels etc
    print(f"  top-5 widths: {widths[-5:]}")
    # serial fraction estimate: fraction of rows with >=1 offdiag
    off = np.diff(indptr)-1  # minus diagonal approx
    print(f"  rows with offdiag: {(off>0).sum()}")
