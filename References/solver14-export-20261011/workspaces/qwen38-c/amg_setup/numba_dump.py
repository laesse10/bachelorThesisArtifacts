import sys, numpy as np
sys.path.insert(0,'/shared/agent-4')
from gen_stencil import make_stencil_3d
import numba_ref as R

NX,NY,NZ,seed,th = int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3]),int(sys.argv[4]),float(sys.argv[5])
n0=NX*NY*NZ
data,indices,indptr=make_stencil_3d(NX,NY,NZ,seed=seed)
npad=n0; nnz0=(3*NX-2)*(3*NY-2)*(3*NZ-2); zpad=(3*(nnz0+n0))//2
def I(): return np.zeros(npad+1,dtype=np.int64)
def Id(x): return np.zeros(x,dtype=np.int64)
def D(): return np.zeros(zpad)
out={}
cur_indptr=I(); cur_indices=Id(zpad); cur_data=D()
nxt_indptr=I(); nxt_indices=Id(zpad); nxt_data=D()
s_indptr=I(); s_indices=Id(zpad)
p_indptr=I(); p_indices=Id(zpad); p_data=D()
pt_indptr=I(); pt_indices=Id(zpad); pt_data=D()
ap_indptr=I(); ap_indices=Id(zpad); ap_data=D()
diag=np.zeros(npad); agg=Id(npad); cnt=np.zeros(npad); tscale=np.zeros(npad)
vv=np.zeros(npad); ww=np.zeros(npad); acc=np.zeros(npad); mark=Id(npad); touched=Id(npad); fill=Id(npad+1)
na_out=np.zeros(1,dtype=np.int64); rho_out=np.zeros(1)
for i in range(n0+1): cur_indptr[i]=indptr[i]
for k in range(nnz0): cur_indices[k]=indices[k]; cur_data[k]=data[k]
n=n0
for lvl in range(3):
    if n<=100: break
    R._row_diagonal(cur_indptr,cur_indices,cur_data,n,diag)
    R._strength_graph(cur_indptr,cur_indices,cur_data,diag,n,th,s_indptr,s_indices)
    for c in range(n): mark[c]=-1
    R._aggregate(s_indptr,s_indices,n,agg,na_out); na=int(na_out[0])
    if na>=n: break
    R._tentative_scale(agg,n,na,cnt,tscale)
    R._spectral_radius(cur_indptr,cur_indices,cur_data,diag,n,vv,ww,rho_out)
    omega=4.0/(3.0*rho_out[0])
    for c in range(na): mark[c]=-1
    R._smoothed_prolongation(cur_indptr,cur_indices,cur_data,diag,agg,tscale,n,omega,p_indptr,p_indices,p_data,acc,mark,touched)
    for c in range(na): mark[c]=-1
    R._csr_matmul(cur_indptr,cur_indices,cur_data,p_indptr,p_indices,p_data,n,ap_indptr,ap_indices,ap_data,acc,mark,touched)
    R._csr_transpose(p_indptr,p_indices,p_data,n,na,pt_indptr,pt_indices,pt_data,fill)
    for c in range(na): mark[c]=-1
    R._csr_matmul(pt_indptr,pt_indices,pt_data,ap_indptr,ap_indices,ap_data,na,nxt_indptr,nxt_indices,nxt_data,acc,mark,touched)
    nextnz=int(nxt_indptr[na])
    out['A_%d_ip'%lvl]=nxt_indptr[:na+1]; out['A_%d_ix'%lvl]=nxt_indices[:nextnz]; out['A_%d_da'%lvl]=nxt_data[:nextnz]
    out['A_%d_nnz'%lvl]=np.array([nextnz]); out['A_%d_n'%lvl]=np.array([na])
    out['P_%d_da'%lvl]=p_data[:int(p_indptr[n])]
    for i in range(na+1): cur_indptr[i]=nxt_indptr[i]
    for k in range(nextnz): cur_indices[k]=nxt_indices[k]; cur_data[k]=nxt_data[k]
    n=na
np.savez('/tmp/numbaid.npz',**out)
print('numba dumped', {k:int(out[k][0]) for k in out if k.endswith('nnz')})
