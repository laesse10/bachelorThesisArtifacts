import numpy as np, sys
a = np.load(sys.argv[1]); b = np.load(sys.argv[2])
d = np.abs(a - b)
rel = d / np.maximum(np.abs(b), 1e-300)
print("max abs", d.max(), "max rel", rel.max())
print("passed 1e-9/1e-11:", np.allclose(a, b, rtol=1e-9, atol=1e-11))
