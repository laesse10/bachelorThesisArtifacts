import numpy as np
lp=np.fromfile("m3/level_ptr.i64",dtype=np.int64)
N=len(lp)-1
w=np.diff(lp)
nz=w[w>0].astype(np.int32)
L=nz.size
open("m3/widths_nonzero.i32","wb").write(np.array([L],np.int32).tobytes()+nz.tobytes())
print("L(nonempty levels)=",L," total rows=",int(w.sum()))
