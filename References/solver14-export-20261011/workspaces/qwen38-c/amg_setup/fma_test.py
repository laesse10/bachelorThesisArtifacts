import sys, time, struct, subprocess, os
import numpy as np
sys.path.insert(0, '/shared/agent-4')
from amg_setup_numpy import amg_setup
from gen_stencil import make_stencil_3d

def run_case(NX,NY,NZ,theta,seed):
    A,A_ix,A_ip=make_stencil_3d(NX,NY,NZ,seed=seed)
    n0=NX*NY*NZ; LMAX=16
    ln=np.zeros(LMAX,dtype=np.int64); lnz=np.zeros(LMAX,dtype=np.int64)
    nl=np.zeros(1,dtype=np.int64); agg0=np.zeros(n0,dtype=np.int64)
    amg_setup(A,A_ix,A_ip,ln,lnz,nl,agg0,NX,NY,NZ,theta)
    with open('/tmp/case.bin','wb') as f:
        f.write(struct.pack('qqqd',NX,NY,NZ,theta))
        f.write(A.astype('<f8').tobytes()); f.write(A_ix.astype('<i8').tobytes()); f.write(A_ip.astype('<i8').tobytes())
    env=dict(os.environ); env['OMP_NUM_THREADS']='14'
    out=subprocess.run(['./driver','/tmp/case.bin'],capture_output=True,text=True,env=env).stdout.strip().split('\n')
    cnl=int(out[0].split()[1]); cln=np.array(out[1].split()[1:],dtype=np.int64)
    clnz=np.array(out[2].split()[1:],dtype=np.int64); cagg=np.array(out[3].split()[1:],dtype=np.int64)
    ok=(nl[0]==cnl and np.array_equal(ln,cln) and np.array_equal(lnz,clnz) and np.array_equal(agg0,cagg))
    return ok, nl[0], ln[:nl[0]].tolist()

for (NX,NY,NZ,theta,seed) in [(32,32,32,0.03,42),(32,32,32,0.03,123),(32,32,32,0.05,999),(32,32,32,0.03,2024)]:
    t0=time.time()
    ok,nl,lvl=run_case(NX,NY,NZ,theta,seed)
    print(f"{NX}x{NY}x{NZ} th={theta} seed={seed}: {'PASS' if ok else 'FAIL'} (nlevels={nl}) {lvl} [{time.time()-t0:.1f}s]","flush=True")
