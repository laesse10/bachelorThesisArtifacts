import numpy as np, sys, time
sys.path.insert(0, "/shared/agent-5")
from ilu0_numpy import ilu0

data = np.fromfile("data.bin", np.float64)
indices = np.fromfile("indices.bin", np.int64)
indptr = np.fromfile("indptr.bin", np.int64)
out_c = np.fromfile("out.bin", np.float64)
t0 = time.time()
out_ref = data.copy()
ilu0(out_ref, indices, indptr, len(indptr) - 1)
print("ref time", time.time() - t0)
d = np.abs(out_c - out_ref)
rel = d / (1e-30 + np.abs(out_ref))
print("max abs", d.max(), "max rel", rel.max())
bad = np.nonzero((d > 1e-11) & (rel > 1e-9))[0]
print("n bad:", len(bad), bad[:10] if len(bad) else "")
i = np.argmax(d)
print("argmax", i, "c=", out_c[i], "ref=", out_ref[i])
