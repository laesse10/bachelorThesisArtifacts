#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
static double nowsec(void){struct timespec ts;clock_gettime(CLOCK_MONOTONIC,&ts);return ts.tv_sec+1e-9*ts.tv_nsec;}
static int64_t *load64(const char*p,const char*s,size_t*n){char fn[512];snprintf(fn,sizeof fn,"%s%s.npy",p,s);FILE*f=fopen(fn,"rb");char h[10];fread(h,1,10,f);uint16_t l;memcpy(&l,h+8,2);size_t o=10+l+((16-(10+l)%16)%16);fseek(f,0,SEEK_END);long fs=ftell(f);fseek(f,o,SEEK_SET);char*b=malloc(fs-o);fread(b,1,fs-o,f);fclose(f);*n=(fs-o)/8;return(int64_t*)b;}
int main(int argc,char**argv){
    size_t ni,nc; int64_t*ip=load64(argv[1],"_indptr",&ni); int64_t*ci=load64(argv[1],"_indices",&nc);
    int64_t N=ni-1; int32_t*level=malloc(4.0*N);
    double best=1e9;
    for(int r=0;r<5;r++){
        memset(level,0,4.0*N);
        double t0=nowsec();
        for(int64_t i=0;i<N;i++){
            int64_t rs=ip[i],re=ip[i+1]; int32_t mx=0;
            for(int64_t k=rs;k<re;k++){int64_t c=ci[k]; if(c>=i)break; if(level[c]>mx)mx=level[c];}
            level[i]=mx+1;
        }
        double dt=nowsec()-t0; if(dt<best)best=dt;
    }
    int64_t L=0; for(int64_t i=0;i<N;i++) if(level[i]>L)L=level[i];
    printf("level compute: %.4f ms  maxlevel=%ld\n", best*1e3,(long)L);
    return 0;
}
