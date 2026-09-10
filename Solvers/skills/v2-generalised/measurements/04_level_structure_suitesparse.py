import os, sys, time
os.environ["HPCAGENT_BENCH_CACHE_DIR"]="/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench/hpcagent_bench/.hpcagent_bench_cache"
import numpy as np, scipy.sparse as sp
sys.path.insert(0,"/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-solver")
from hpcagent_bench.support.helpers.sparse.generators import make_suitesparse_csr

def levels_csr(indptr, indices, n):
    lev=np.zeros(n,dtype=np.int64)
    for i in range(n):
        c=indices[indptr[i]:indptr[i+1]]; p=c[c<i]
        lev[i]=(lev[p].max()+1) if p.size else 0
    return lev

for name in ("Schmid/thermal1","Um/offshore","Schmid/thermal2","Oberwolfach/boneS10"):
    t0=time.time()
    ip,ic,dv = make_suitesparse_csr(name, dtype=np.float64, lower=True)
    n=ip.shape[0]-1
    lev=levels_csr(ip,ic,n); cnt=np.bincount(lev); nl=cnt.size
    print(f"{name:24s} N={n:>8} nnz(L)={ic.size:>9} levels={nl:>6} "
          f"avg={n/nl:9.1f} max={cnt.max():>7} max/avg={cnt.max()/(n/nl):6.2f} "
          f"lvls<32rows={(cnt<32).sum():>6}  ({time.time()-t0:.0f}s)")
