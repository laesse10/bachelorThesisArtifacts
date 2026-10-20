import numpy as np, sys
edge = int(sys.argv[1]); d = np.load(f'/shared/agent-7/in{edge}.npz')
def emit(name, arr):
    t = 'int64_t' if arr.dtype == np.int64 else 'double'
    return f'static const {t} {name}[{arr.size}] = {{' + ','.join(repr(x) for x in arr.tolist()) + '};\n'
out = f'#define EDGE {edge}\n'
for nm in ['A_data','A_indices','A_indptr','L_indices','L_indptr','L_to_Lc','Lc_data','Lc_indices','Lc_indptr','b','y']:
    out += emit(nm, d[nm])
out = out.replace('Lc_data[','Lc_data_ref[').replace('y[','y_ref[')
open('testdata.h','w').write(out)
print("wrote", len(out), "bytes")
