#!/bin/bash
set -e
SIZ="$1"; SEED="${2:-7}"
NX=$(echo $SIZ | cut -d, -f1); NY=$(echo $SIZ | cut -d, -f2); NZ=$(echo $SIZ | cut -d, -f3)
TH="${4:-0.03}"
# generate + reference
python3 -c "
import sys, numpy as np
sys.path.insert(0,'/shared/agent-4')
from gen_stencil import make_stencil_3d
import amg_setup_numpy as ref
NX,NY,NZ,seed,th=$NX,$NY,$NZ,$SEED,float('$TH')
n0=NX*NY*NZ
data,indices,indptr=make_stencil_3d(NX,NY,NZ,seed=seed)
with open('/tmp/csr.bin','wb') as f:
    for v in (NX,NY,NZ): f.write(int(v).to_bytes(8,'little'))
    f.write(np.float64(th).tobytes())
    f.write(data.astype('<f8').tobytes())
    f.write(indices.astype('<i8').tobytes())
    f.write(indptr.astype('<i8').tobytes())
agg0=np.zeros(n0,dtype=np.int64); level_n=np.zeros(16,dtype=np.int64); level_nnz=np.zeros(16,dtype=np.int64); nlevels=np.zeros(1,dtype=np.int64)
ref.amg_setup(data,indices,indptr,level_n,level_nnz,nlevels,agg0,NX,NY,NZ,th)
np.save('/tmp/ref.npy', np.array({'nlevels':int(nlevels[0]),'level_n':level_n,'level_nnz':level_nnz,'agg0':agg0}))
print('ref done: nlevels',int(nlevels[0]))
"
# build driver+kernel
gcc -O3 -march=native -fopenmp -fno-math-errno -fno-trapping-math -fno-signed-zeros -ffp-contract=fast -fstrict-aliasing -fPIC -std=c23 driver.c amg_setup.c -o /tmp/driver -lm 2>&1 | head
OMP_NUM_THREADS=$3 /tmp/driver /tmp/csr.bin > /tmp/c_out.txt
python3 -c "
import numpy as np, re
ref=np.load('/tmp/ref.npy',allow_pickle=True).item()
lines=open('/tmp/c_out.txt').read().split('\n')
def parse(line,kw):
    return [int(x) for x in line.split()[1:]]
nlevels_c=int(parse(lines[0],'nlevels')[0])
level_n_c=parse(lines[1],'level_n')
level_nnz_c=parse(lines[2],'level_nnz')
agg0_c=parse(lines[3],'agg0')
rn=ref['nlevels']; rln=ref['level_n'].tolist(); rlnn=ref['level_nnz'].tolist(); ragg=ref['agg0'].tolist()
ok=True
if nlevels_c!=rn: ok=False; print('MISMATCH nlevels',nlevels_c,rn)
if level_n_c!=rln: ok=False; print('MISMATCH level_n\n C:',level_n_c,'\n R:',rln)
if level_nnz_c!=rlnn: ok=False; print('MISMATCH level_nnz\n C:',level_nnz_c,'\n R:',rlnn)
na=min(len(agg0_c),len(ragg))
d=[(i,a,b) for i,(a,b) in enumerate(zip(agg0_c[:na],ragg[:na])) if a!=b]
if d: ok=False; print('MISMATCH agg0: first diffs',d[:10],'total',len(d))
print('RESULT:', 'PASS' if ok else 'FAIL')
"
