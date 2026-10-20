#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <immintrin.h>
#include <stdatomic.h>

/* Fine-grained DAG (out-of-order) sparse triangular solve:
   row r computed once all strictly-lower columns are solved.
   Per-row readiness counters (spread in memory) + a lock-free MPMC queue
   with per-slot commit flags (correct publish order). */

static int32_t     *g_dep  = NULL; /* # strictly-lower entries per row   */
static int32_t     *g_rem  = NULL; /* remaining deps per row (per-call)  */
static int32_t     *g_rev_p= NULL; /* reverse-adjacency CSR ptr (N+1)    */
static int32_t     *g_rev_x= NULL; /* reverse-adjacency rows (E)         */
static int32_t     *g_qbuf = NULL; /* ready-queue data (N)               */
static _Atomic int *g_qcmt = NULL; /* ready-queue per-slot commit (N)    */
static uint64_t     g_fp   = 0;
static int64_t      g_capN = 0, g_capE = 0;

static void ensure_cap(int64_t N, int64_t E){
  if(N>g_capN){
    g_capN=N;
    free(g_dep);free(g_rem);free(g_qbuf);free(g_qcmt);
    g_dep = (int32_t*)    malloc((size_t)N*4);
    g_rem = (int32_t*)    malloc((size_t)N*4);
    g_qbuf= (int32_t*)    malloc((size_t)N*4);
    g_qcmt= (_Atomic int*)malloc((size_t)N*4);
    if(g_qcmt) for(int64_t i=0;i<N;i++) atomic_store_explicit(&g_qcmt[i],0,memory_order_relaxed);
  }
  if(E>g_capE){
    g_capE=E;
    free(g_rev_p);free(g_rev_x);
    g_rev_p=(int32_t*)malloc((size_t)(N+1)*4);
    g_rev_x=(int32_t*)malloc((size_t)E*4);
  }
}

static void build_analysis(const int64_t *L_indices, const int64_t *L_indptr, int64_t N, int64_t NNZ){
  ensure_cap(N, NNZ);
  for(int64_t r=0;r<N;r++){
    int64_t lo=L_indptr[r], hi=L_indptr[r+1];
    int off;
    if(hi>lo && L_indices[hi-1]==r) off=(int)(hi-1-lo);
    else { off=0; for(int64_t k=lo;k<hi;k++) if(L_indices[k]<r) off++; }
    g_dep[r]=off;
  }
  for(int64_t r=0;r<N;r++) g_rem[r]=0;               /* temp degree */
  for(int64_t r=0;r<N;r++){
    int64_t lo=L_indptr[r]; int off=g_dep[r];
    for(int k=0;k<off;k++) g_rem[L_indices[lo+k]]++;
  }
  g_rev_p[0]=0;
  for(int64_t r=0;r<N;r++) g_rev_p[r+1]=g_rev_p[r]+g_rem[r];
  for(int64_t r=0;r<N;r++) g_rem[r]=0;               /* cursor */
  for(int64_t r=0;r<N;r++){
    int64_t lo=L_indptr[r]; int off=g_dep[r];
    for(int k=0;k<off;k++){
      int64_t c=L_indices[lo+k];
      g_rev_x[g_rev_p[c]+g_rem[c]++]=(int32_t)r;
    }
  }
}

void sptrsv_level_csr_fp64(
    const double *restrict L_data,
    const int64_t *restrict L_indices,
    const int64_t *restrict L_indptr,
    const double *restrict b,
    const int64_t *restrict level_ptr,
    const int64_t *restrict perm,
    double *restrict x,
    const int64_t N,
    const int64_t NNZ_L,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
  (void)level_ptr;(void)perm;(void)workspace;(void)workspace_size;
  if(N<=0) return;

  uint64_t fp=(uint64_t)N*0x9E3779B97F4A7C15ull+(uint64_t)NNZ_L*0xC2B2AE3D27D4EB4Full;
  if(NNZ_L>0){
    fp^=(uint64_t)L_indices[0]*0x85EBCA77C2B2AE63ull;
    fp^=(uint64_t)L_indices[NNZ_L-1]*0x27D4EB2F165667C5ull;
    for(int64_t k=0;k<NNZ_L;k+=65537) fp=(fp^(uint64_t)L_indices[k])*0x100000001B3ull;
    for(int64_t r=0;r<N;r+=32769)     fp=(fp^(uint64_t)L_indptr[r])*0x9E3779B97F4A7C15ull;
  }
  if(fp!=g_fp){ build_analysis(L_indices,L_indptr,N,NNZ_L); g_fp=fp; }

  int32_t *rem=g_rem, *dep=g_dep;
  int32_t *qbuf=g_qbuf; _Atomic int *qcmt=g_qcmt;
  _Atomic long qh=0, qt=0;

  #pragma omp parallel
  {
    int nt=omp_get_num_threads();
    int tid=omp_get_thread_num();
    long per=(N+nt-1)/nt;
    long a=tid*per, c=a+per; if(c>N)c=N;
    for(long r=a;r<c;r++) rem[r]=dep[r];
    #pragma omp barrier
    for(long r=a;r<c;r++){
      if(dep[r]==0){
        long p=atomic_fetch_add_explicit(&qh,1,memory_order_relaxed);
        qbuf[p]=(int32_t)r;
        atomic_store_explicit(&qcmt[p],1,memory_order_release);
      }
    }
    for(;;){
      int32_t r=-1;
      for(;;){
        long t=atomic_load_explicit(&qt,memory_order_relaxed);
        long h=atomic_load_explicit(&qh,memory_order_acquire);
        if(t>=h) break;
        if(atomic_load_explicit(&qcmt[t],memory_order_acquire)){
          long old=t;
          if(atomic_compare_exchange_weak_explicit(&qt,&old,old+1,memory_order_relaxed,memory_order_relaxed)){
            r=qbuf[t];
            atomic_store_explicit(&qcmt[t],0,memory_order_relaxed);
            break;
          }
        } else _mm_pause();
      }
      if(r<0){
        if(atomic_load_explicit(&qt,memory_order_acquire)>=N) break;
        _mm_pause();
        continue;
      }
      long lo=L_indptr[r]; int off=dep[r];
      double s=b[r];
      for(int k=0;k<off;k++) s-=L_data[lo+k]*x[L_indices[lo+k]];
      x[r]=s/L_data[lo+off];
      atomic_thread_fence(memory_order_release);
      int32_t rp0=g_rev_p[r], rp1=g_rev_p[r+1];
      for(int e=rp0;e<rp1;e++){
        int32_t s2=g_rev_x[e];
        int old=__atomic_fetch_sub(&rem[s2],1,__ATOMIC_ACQ_REL);
        if(old==1){
          long p=atomic_fetch_add_explicit(&qh,1,memory_order_relaxed);
          qbuf[p]=s2;
          atomic_store_explicit(&qcmt[p],1,memory_order_release);
        }
      }
    }
  }
}
