import numpy as np, numba as nb
# Direct copy of the OFFICIAL reference amg_setup (from amg_setup_numpy.py), jitted, with a dump of level-1 operator.
@nb.njit
def amg_setup_dump(A_data, A_indices, A_indptr, level_n, level_nnz, nlevels, agg0, NX, NY, NZ, theta,
                   dump_ip, dump_ix, dump_da):
    LMAX=16; MAX_COARSE=100; SMOOTH_NUM=4.0; SMOOTH_DEN=3.0; RHO_ITERS=15
    n0 = NX*NY*NZ
    nnz0 = (3*NX-2)*(3*NY-2)*(3*NZ-2)
    zpad = (3*(nnz0+n0))//2
    npad = n0
    cur_indptr=np.zeros(npad+1,dtype=np.int64); cur_indices=np.zeros(zpad,dtype=np.int64); cur_data=np.zeros(zpad)
    nxt_indptr=np.zeros(npad+1,dtype=np.int64); nxt_indices=np.zeros(zpad,dtype=np.int64); nxt_data=np.zeros(zpad)
    s_indptr=np.zeros(npad+1,dtype=np.int64); s_indices=np.zeros(zpad,dtype=np.int64)
    p_indptr=np.zeros(npad+1,dtype=np.int64); p_indices=np.zeros(zpad,dtype=np.int64); p_data=np.zeros(zpad)
    pt_indptr=np.zeros(npad+1,dtype=np.int64); pt_indices=np.zeros(zpad,dtype=np.int64); pt_data=np.zeros(zpad)
    ap_indptr=np.zeros(npad+1,dtype=np.int64); ap_indices=np.zeros(zpad,dtype=np.int64); ap_data=np.zeros(zpad)
    diag=np.zeros(npad); agg=np.zeros(npad,dtype=np.int64); cnt=np.zeros(npad); tscale=np.zeros(npad)
    vv=np.zeros(npad); ww=np.zeros(npad); acc=np.zeros(npad); mark=np.zeros(npad,dtype=np.int64)
    touched=np.zeros(npad,dtype=np.int64); fill=np.zeros(npad+1,dtype=np.int64)
    na_out=np.zeros(1,dtype=np.int64); rho_out=np.zeros(1)
    def row_diagonal(ai,ax,ad,n,dg):
        for i in range(n):
            dg[i]=0.0
            for k in range(ai[i],ai[i+1]):
                if ax[k]==i: dg[i]=abs(ad[k])
    def strength_graph(ai,ax,ad,dg,n,th,si,ss):
        pos=0
        for i in range(n):
            si[i]=pos
            for k in range(ai[i],ai[i+1]):
                j=ax[k]
                if j!=i:
                    if abs(ad[k])>th*(dg[i]*dg[j])**0.5: ss[pos]=j; pos=pos+1
        si[n]=pos
    def aggregate(si,ss,n,ag,no):
        for i in range(n): ag[i]=-1
        na=0
        for i in range(n):
            if ag[i]<0:
                free=1
                for k in range(si[i],si[i+1]):
                    if ag[ss[k]]>=0: free=0
                if free==1:
                    ag[i]=na
                    for k in range(si[i],si[i+1]): ag[ss[k]]=na
                    na=na+1
        for i in range(n):
            if ag[i]<0:
                picked=-1
                for k in range(si[i],si[i+1]):
                    c=ag[ss[k]]
                    if c>=0:
                        if picked<0: picked=c
                if picked<0: ag[i]=na; na=na+1
                else: ag[i]=picked
        no[0]=na
    def tentative_scale(ag,n,na,c,ts):
        for cc in range(na): c[cc]=0.0
        for i in range(n): c[ag[i]]=c[ag[i]]+1.0
        for i in range(n): ts[i]=1.0/(c[ag[i]])**0.5
    def spectral_radius(ai,ax,ad,dg,n,v,w,ro):
        for i in range(n): v[i]=1.0
        rho=0.0
        for _it in range(RHO_ITERS):
            for i in range(n):
                ac=0.0
                for k in range(ai[i],ai[i+1]): ac=ac+ad[k]*v[ax[k]]
                w[i]=ac/dg[i]
            nrm=0.0
            for i in range(n): nrm=nrm+w[i]*w[i]
            nrm=nrm**0.5
            for i in range(n): v[i]=w[i]/nrm
            rho=nrm
        ro[0]=rho
    def smoothed_prolongation(ai,ax,ad,dg,ag,ts,n,om,pi,pp,pd,ac,ma,to):
        pos=0
        for i in range(n):
            pi[i]=pos
            nt=0
            c0=ag[i]
            if ma[c0]!=i:
                ma[c0]=i; ac[c0]=0.0; to[nt]=c0; nt=nt+1
            ac[c0]=ac[c0]+ts[i]
            for k in range(ai[i],ai[i+1]):
                cj=ag[ax[k]]
                if ma[cj]!=i:
                    ma[cj]=i; ac[cj]=0.0; to[nt]=cj; nt=nt+1
                ac[cj]=ac[cj]-om*(ad[k]/dg[i])*ts[ax[k]]
            for t in range(nt):
                c=to[t]; pp[pos]=c; pd[pos]=ac[c]; pos=pos+1
        pi[n]=pos
    def csr_transpose(ip,ix,da,nrow,ncol,ti,ttx,td,fl):
        for c in range(ncol+1): ti[c]=0
        for k in range(ip[nrow]): ti[ix[k]+1]=ti[ix[k]+1]+1
        for c in range(ncol): ti[c+1]=ti[c+1]+ti[c]
        for c in range(ncol): fl[c]=ti[c]
        for i in range(nrow):
            for k in range(ip[i],ip[i+1]):
                col=ix[k]; ttx[fl[col]]=i; td[fl[col]]=da[k]; fl[col]=fl[col]+1
    def csr_matmul(aip,aix,ada,bip,bix,bda,nrow,cip,cix,cda,ac,ma,to):
        pos=0
        for i in range(nrow):
            cip[i]=pos
            nt=0
            for ka in range(aip[i],aip[i+1]):
                j=aix[ka]; av=ada[ka]
                for kb in range(bip[j],bip[j+1]):
                    col=bix[kb]
                    if ma[col]!=i:
                        ma[col]=i; ac[col]=0.0; to[nt]=col; nt=nt+1
                    ac[col]=ac[col]+av*bda[kb]
            for t in range(nt):
                col=to[t]; cix[pos]=col; cda[pos]=ac[col]; pos=pos+1
        cip[nrow]=pos
    for i in range(n0+1): cur_indptr[i]=A_indptr[i]
    for k in range(nnz0): cur_indices[k]=A_indices[k]; cur_data[k]=A_data[k]
    for lv in range(LMAX): level_n[lv]=0; level_nnz[lv]=0
    n=n0; level_n[0]=n0; level_nnz[0]=nnz0; depth=1
    for _lv in range(LMAX-1):
        if n>MAX_COARSE:
            row_diagonal(cur_indptr,cur_indices,cur_data,n,diag)
            strength_graph(cur_indptr,cur_indices,cur_data,diag,n,theta,s_indptr,s_indices)
            for c in range(n): mark[c]=-1
            aggregate(s_indptr,s_indices,n,agg,na_out); na=int(na_out[0])
            if na<n:
                if depth==1:
                    for i in range(n0): agg0[i]=agg[i]
                tentative_scale(agg,n,na,cnt,tscale)
                spectral_radius(cur_indptr,cur_indices,cur_data,diag,n,vv,ww,rho_out)
                omega=SMOOTH_NUM/(SMOOTH_DEN*rho_out[0])
                for c in range(na): mark[c]=-1
                smoothed_prolongation(cur_indptr,cur_indices,cur_data,diag,agg,tscale,n,omega,p_indptr,p_indices,p_data,acc,mark,touched)
                for c in range(na): mark[c]=-1
                csr_matmul(cur_indptr,cur_indices,cur_data,p_indptr,p_indices,p_data,n,ap_indptr,ap_indices,ap_data,acc,mark,touched)
                csr_transpose(p_indptr,p_indices,p_data,n,na,pt_indptr,pt_indices,pt_data,fill)
                for c in range(na): mark[c]=-1
                csr_matmul(pt_indptr,pt_indices,pt_data,ap_indptr,ap_indices,ap_data,na,nxt_indptr,nxt_indices,nxt_data,acc,mark,touched)
                if depth==1:
                    m=int(nxt_indptr[na])
                    for kk in range(na+1): dump_ip[kk]=nxt_indptr[kk]
                    for kk in range(m): dump_ix[kk]=nxt_indices[kk]
                    for kk in range(m): dump_da[kk]=nxt_data[kk]
                    # also dump P
                    mp=int(p_indptr[n])
                    for kk in range(mp): p_indices[kk]  # touch
                level_n[depth]=na; level_nnz[depth]=nxt_indptr[na]; depth=depth+1
                for i in range(na+1): cur_indptr[i]=nxt_indptr[i]
                for k in range(nxt_indptr[na]): cur_indices[k]=nxt_indices[k]; cur_data[k]=nxt_data[k]
                n=na
            else:
                n=0
    nlevels[0]=depth

if __name__=='__main__':
    import sys
    sys.path.insert(0,'/shared/agent-4')
    from gen_stencil import make_stencil_3d
    NX,NY,NZ,seed,th=40,96,72,200,0.03
    n0=NX*NY*NZ; nnz0=(3*NX-2)*(3*NY-2)*(3*NZ-2); zpad=(3*(nnz0+n0))//2
    data,indices,indptr=make_stencil_3d(NX,NY,NZ,seed=seed)
    a0=np.zeros(n0,dtype=np.int64); ln=np.zeros(16,dtype=np.int64); lnn=np.zeros(16,dtype=np.int64); nl=np.zeros(1,dtype=np.int64)
    dip=np.zeros(n0+1,dtype=np.int64); dix=np.zeros(zpad,dtype=np.int64); dda=np.zeros(zpad)
    amg_setup_dump(data,indices,indptr,ln,lnn,nl,a0,NX,NY,NZ,th,dip,dix,dda)
    print('A0 dump, level_nnz',lnn[:5].tolist())
    na=int(ln[1]); m=int(dip[na])
    print('A0 na',na,'nnz',m)
    print('A0 per-row nnz first 8:', [int(dip[i+1]-dip[i]) for i in range(8)])
    np.savez('/tmp/numba_a0.npz', dip=dip[:na+1], dix=dix[:m], dda=dda[:m], na=np.array([na]), m=np.array([m]))
    print('saved /tmp/numba_a0.npz')
