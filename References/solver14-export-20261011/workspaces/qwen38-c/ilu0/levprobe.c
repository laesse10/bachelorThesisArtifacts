#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void){
  size_t n; FILE*f=fopen("indptr.bin","rb"); fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
  int64_t*ind=malloc(sz); fread(ind,1,sz,f); fclose(f); int64_t N=sz/8-1;
  f=fopen("indices.bin","rb"); fseek(f,0,SEEK_END); sz=ftell(f); fseek(f,0,SEEK_SET);
  int64_t*indc=malloc(sz); fread(indc,1,sz,f); fclose(f);
  int32_t*lev=malloc(4*N); int32_t maxlev=0;
  for(int64_t i=0;i<N;i++){int32_t lv=0; for(int64_t p=ind[i];p<ind[i+1];p++){int64_t k=indc[p]; if(k<i){int32_t c=lev[k]+1; if(c>lv)lv=c;}} lev[i]=lv; if(lv>maxlev)maxlev=lv;}
  int32_t*w=malloc(4*(maxlev+1)); memset(w,0,4*(maxlev+1)); for(int64_t i=0;i<N;i++)w[lev[i]]++;
  int32_t maxw=0,minw=0x7fffffff; long long nz=0; for(int32_t L=0;L<=maxlev;L++)if(w[L]){nz++; if(w[L]>maxw)maxw=w[L]; if(w[L]<minw)minw=w[L];}
  long long bw[8]={0}; for(int32_t L=0;L<=maxlev;L++){int32_t x=w[L]; if(!x)continue; if(x==1)bw[0]++;else if(x<=4)bw[1]++;else if(x<=16)bw[2]++;else if(x<=64)bw[3]++;else if(x<=256)bw[4]++;else if(x<=1024)bw[5]++;else if(x<=4096)bw[6]++;else bw[7]++;}
  printf("N=%ld maxlev=%d nzlevels=%lld maxw=%d minw=%d wbuckets=%lld %lld %lld %lld %lld %lld %lld %lld\n",N,maxlev,nz,maxw,minw,bw[0],bw[1],bw[2],bw[3],bw[4],bw[5],bw[6],bw[7]);
  return 0;
}
