import numpy as np, sys
path = sys.argv[1]
ip = np.load(path+'_indptr.npy'); ci = np.load(path+'_indices.npy')
N = len(ip)-1
level = np.zeros(N, dtype=np.int64)
for i in range(N):
    rs, re = ip[i], ip[i+1]
    mx = 0
    for k in range(rs, re):
        c = ci[k]
        if c >= i: break
        if level[c] > mx: mx = level[c]
    level[i] = mx + 1
L = level.max()
cnt = np.bincount(level)
print(path, "maxlevel=", L, "rows/level avg=", N/L, "min rows in a level:", cnt[1:].min(), "max:", cnt.max())
# histogram of level sizes
print("levels with <8 rows:", (cnt[1:]<8).sum(), "of", L)
print("level sizes deciles:", np.percentile(cnt[1:], [10,50,90]).astype(int))
