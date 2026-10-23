import sys, numpy as np
sys.path.insert(0, ".")
import tsvc_2_s2710_numba_np as m
n = 1000
rng = np.random.default_rng(0)
a, b, c, d, e, x = (rng.random(n) for _ in range(6))
m.s2710(a, b, c, d, e, x, n)
print("ran ok")
