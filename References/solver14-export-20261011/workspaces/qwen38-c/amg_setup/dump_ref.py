import sys, numpy as np
sys.path.insert(0,'/shared/agent-4')
from gen_stencil import make_stencil_3d
import amg_setup_numpy as ref

NX,NY,NZ = int(sys.argv[1]),int(sys.argv[2]),int(sys.argv[3])
seed=int(sys.argv[4]); th=float(sys.argv[5])
n0=NX*NY*NZ
data,indices,indptr=make_stencil_3d(NX,NY,NZ,seed=seed)
npad=n0
zpad=(3*( (3*NX-2)*(3*NY-2)*(3*NZ-2)+n0 ))//2
diag=np.zeros(npad); agg=np.zeros(npad,dtype=np.int64); cnt=np.zeros(npad); tscale=np.zeros(npad)
vv=np.zeros(npad); ww=np.zeros(npad); acc=np.zeros(npad); mark=np.zeros(npad,dtype=np.int64); touched=np.zeros(npad,dtype=np.int64)
p_indptr=np.zeros(npad+1,dtype=np.int64); p_indices=np.zeros(zpad,dtype=np.int64); p_data=np.zeros(zpad)
ap_indptr=np.zeros(npad+1,dtype=np.int64); ap_indices=np.zeros(zpad,dtype=np.int64); ap_data=np.zeros(zpad)
pt_indptr=np.zeros(npad+1,dtype=np.int64); pt_indices=np.zeros(zpad,dtype=np.int64); pt_data=np.zeros(zpad)
nxt_indptr=np.zeros(npad+1,dtype=np.int64); nxt_indices=np.zeros(zpad,dtype=np.int64); nxt_data=np.zeros(zpad)
na_out=np.zeros(1,dtype=np.int64); rho_out=np.zeros(1)
n=n0
ref.row_diagonal(indptr,indices,data,n,diag)
s_indptr=np.zeros(npad+1,dtype=np.int64); s_indices=np.zeros(zpad,dtype=np.int64)
ref.strength_graph(indptr,indices,data,diag,n,th,s_indptr,s_indices)
for c in range(n): mark[c]=-1
ref.aggregate(s_indptr,s_indices,n,agg,na_out)
na=int(na_out[0])
ref.tentative_scale(agg,n,na,cnt,tscale)
ref.spectral_radius(indptr,indices,data,diag,n,vv,ww,rho_out)
rho=float(rho_out[0]); omega=ref.SMOOTH_NUM/(ref.SMOOTH_DEN*rho)
for c in range(na): mark[c]=-1
ref.smoothed_prolongation(indptr,indices,data,diag,agg,tscale,n,omega,p_indptr,p_indices,p_data,acc,mark,touched)
pnz=int(p_indptr[n])
for c in range(na): mark[c]=-1
ref.csr_matmul(indptr,indices,data,p_indptr,p_indices,p_data,n,ap_indptr,ap_indices,ap_data,acc,mark,touched)
apnz=int(ap_indptr[n])
ref.csr_transpose(p_indptr,p_indices,p_data,n,na,pt_indptr,pt_indices,pt_data,fill:=np.zeros(na+1,dtype=np.int64))
for c in range(na): mark[c]=-1
ref.csr_matmul(pt_indptr,pt_indices,pt_data,ap_indptr,ap_indices,ap_data,na,nxt_indptr,nxt_indices,nxt_data,acc,mark,touched)
nextnz=int(nxt_indptr[na])
np.savez('/tmp/ref_int.npz',
         agg0=agg[:n], na=np.array([na]),
         p_indptr=p_indptr[:n+1], p_indices=p_indices[:pnz], p_data=p_data[:pnz], pnz=np.array([pnz]),
         ap_indptr=ap_indptr[:n+1], ap_indices=ap_indices[:apnz], ap_data=ap_data[:apnz], apnz=np.array([apnz]),
         pt_indptr=pt_indptr[:na+1], pt_indices=pt_indices[:pnz], pt_data=pt_data[:pnz],
         nxt_indptr=nxt_indptr[:na+1], nxt_indices=nxt_indices[:nextnz], nxt_data=nxt_data[:nextnz], nextnz=np.array([nextnz]),
         omega=np.array([omega]), rho=np.array([rho]))
print('REF level0: na',na,'pnz',pnz,'apnz',apnz,'nextnz',nextnz,'omega',omega,'rho',rho)
