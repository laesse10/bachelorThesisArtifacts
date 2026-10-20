#include <stdio.h>
#include "amg_setup.c"
int main(int argc,char**argv){
    FILE*f=fopen(argv[1],"rb");
    int64_t NX2,NY2,NZ2; double theta;
    fread(&NX2,8,1,f);fread(&NY2,8,1,f);fread(&NZ2,8,1,f);fread(&theta,8,1,f);
    int64_t NX=NX2,NY=NY2,NZ=NZ2,n0=NX*NY*NZ,nnz0=(3*NX-2)*(3*NY-2)*(3*NZ-2);
    double*Ad=malloc(nnz0*8); int64_t*Ai=malloc(nnz0*8); int64_t*Ap=malloc((n0+1)*8);
    fread(Ad,8,nnz0,f);fread(Ai,8,nnz0,f);fread(Ap,8,n0+1,f);fclose(f);
    int64_t npad=n0, zpad=(3*(nnz0+n0))/2;
    ensure_alloc(npad,zpad);
    int32_t T=B.T;
    for(int64_t i=0;i<=n0;i++)B.X_ip[i]=(int32_t)Ap[i];
    for(int64_t k=0;k<nnz0;k++){B.X_ix[k]=(int32_t)Ai[k];B.X_da[k]=Ad[k];}
    int64_t n=n0;
    FILE*g=fopen("/tmp/c_dump.txt","w");
    for(int lvl=0; lvl<3; lvl++){
        if(n<=100) break;
        row_diagonal(B.X_ip,B.X_ix,B.X_da,n,B.diag,T);
        strength_graph(B.X_ip,B.X_ix,B.X_da,B.diag,n,theta,B.F_ip,B.F_ix,B.cnt_row,T);
        int32_t na=aggregate(B.F_ip,B.F_ix,n,B.agg);
        if(na>=n) break;
        tentative_scale(B.agg,n,na,B.cnt,B.tscale,B.touched_p,npad,T);
        double rho=spectral_radius(B.X_ip,B.X_ix,B.X_da,B.diag,n,B.vv,B.ww,T);
        double omega=SMOOTH_NUM/(SMOOTH_DEN*rho);
        if(lvl==0) fprintf(g,"RHO %.17g OMEGA %.17g\n",rho,omega);
        smoothed_prolongation(B.X_ip,B.X_ix,B.X_da,B.diag,B.agg,B.tscale,n,na,omega,T,B.C_ip,B.C_ix,B.C_da,B.cnt_row,B.acc_p,B.mark_p,B.touched_p,npad);
        if(lvl==0){ fprintf(g,"DIAG"); for(int64_t ii=0;ii<n;ii++)fprintf(g," %.17g",B.diag[ii]); fprintf(g,"\n"); fprintf(g,"TSCALE"); for(int64_t ii=0;ii<n;ii++)fprintf(g," %.17g",B.tscale[ii]); fprintf(g,"\n"); fprintf(g,"CNT"); for(int32_t cc=0;cc<na;cc++)fprintf(g," %.17g",B.cnt[cc]); fprintf(g,"\n"); }
        csr_matmul(B.X_ip,B.X_ix,B.X_da,B.C_ip,B.C_ix,B.C_da,n,na,T,B.E_ip,B.E_ix,B.E_da,B.cnt_row,B.acc_p,B.mark_p,B.touched_p,npad);
        csr_transpose(B.C_ip,B.C_ix,B.C_da,n,na,B.D_ip,B.D_ix,B.D_da,B.fill);
        csr_matmul(B.D_ip,B.D_ix,B.D_da,B.E_ip,B.E_ix,B.E_da,na,na,T,B.X_ip,B.X_ix,B.X_da,B.cnt_row,B.acc_p,B.mark_p,B.touched_p,npad);
        int64_t nextnz=B.X_ip[na];
        fprintf(g,"A_%d_ip",lvl); for(int64_t i=0;i<=na;i++)fprintf(g," %d",B.X_ip[i]); fprintf(g,"\n");
        fprintf(g,"A_%d_ix",lvl); for(int64_t i=0;i<nextnz;i++)fprintf(g," %d",B.X_ix[i]); fprintf(g,"\n");
        fprintf(g,"A_%d_da",lvl); for(int64_t i=0;i<nextnz;i++)fprintf(g," %.17g",B.X_da[i]); fprintf(g,"\n");
        fprintf(g,"A_%d_nnz",lvl); fprintf(g," %lld\n",(long long)nextnz);
        fprintf(g,"A_%d_n",lvl); fprintf(g," %d\n",na);
        int64_t pnz=B.C_ip[n];
        fprintf(g,"P_%d_da",lvl); for(int64_t i=0;i<pnz;i++)fprintf(g," %.17g",B.C_da[i]); fprintf(g,"\n");
        n=na;
    }
    fclose(g);
    printf("C dumped\n");
    return 0;
}
