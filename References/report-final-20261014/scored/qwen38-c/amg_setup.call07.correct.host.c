#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <omp.h>

/* declared explicitly: the pinned _POSIX_C_SOURCE=199309 hides glibc's madvise/syscall */
extern int madvise(void *addr, size_t len, int advice);
#define AMG_MADV_WILLNEED 3 /* MADV_WILLNEED on Linux */
extern long syscall(long number, ...);

/* ---- physical core count of this container (cached) -------------------------
   Oversubscribing (threads > physical cores) is 2x+ slower here: threads spin
   at OpenMP barriers while their siblings are descheduled. Cap T accordingly. */
#include <sys/syscall.h>
typedef struct { unsigned long w[128]; } AffSet; /* 8192 cpus */
static int phys_cores_detect(void){
    static int cached = 0;
    if (cached > 0) return cached;
    int mt = omp_get_max_threads();
    if (mt < 1) mt = 1;
    AffSet s;
    memset(&s, 0, sizeof(s));
    /* some container runtimes return an odd value but still fill the mask
       correctly, so validate the mask instead of trusting the return code */
    (void)syscall(SYS_sched_getaffinity, 0, sizeof(s), &s);
    int total = 0, maxcpu = 0;
    for (int i = 0; i < 128; i++)
        if (s.w[i]){ total += __builtin_popcountl(s.w[i]); maxcpu = i*64 + 63; }
    if (total < 1){ cached = mt; return cached; }
    uint64_t *keys = malloc((size_t)total*8 + 8);
    int nkeys = 0, ok = 1;
    for (int cpu = 0; cpu <= maxcpu && ok; cpu++){
        if (!(s.w[cpu/64] & (1UL << (cpu%64)))) continue;
        char path[160];
        long pkg = -1, core = -1;
        FILE *f;
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", cpu);
        if ((f = fopen(path, "r"))){ if (fscanf(f, "%ld", &pkg) != 1) pkg = -1; fclose(f); }
        if (pkg >= 0){
            snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/topology/core_id", cpu);
            if ((f = fopen(path, "r"))){ if (fscanf(f, "%ld", &core) != 1) core = -1; fclose(f); }
        }
        if (pkg < 0 || core < 0){ ok = 0; break; }
        uint64_t key = ((uint64_t)pkg << 32) | (uint32_t)core;
        int found = 0;
        for (int i = 0; i < nkeys; i++) if (keys[i] == key){ found = 1; break; }
        if (!found) keys[nkeys++] = key;
    }
    free(keys);
    cached = (!ok || nkeys < 1) ? mt : nkeys;
    return cached;
}

#define LMAX 16
#define MAX_COARSE 100
#define SMOOTH_NUM 4.0
#define SMOOTH_DEN 3.0
#define RHO_ITERS 15

#include <time.h>
static double tg_copy=0, tg_diag=0, tg_strength=0, tg_agg=0, tg_tent=0,
            tg_rho=0, tg_prol=0, tg_ap=0, tg_trans=0, tg_ptap=0, tg_other=0;
static double tg_last;
static inline double tg_now(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return (double)ts.tv_sec + (double)ts.tv_nsec*1e-9; }
static inline void tg_start(void){ tg_last = tg_now(); }
#define TG(NAME) do { double _e = tg_now(); tg_##NAME += _e - tg_last; tg_last = _e; } while(0)

/* ================= workspace ================= */
typedef struct {
    int64_t *cur_indptr; int32_t *cur_idx; double *cur_data;
    int64_t *nxt_indptr; int32_t *nxt_idx; double *nxt_data;
    int64_t *p_indptr;   int32_t *p_idx;   double *p_data;
    int64_t *ap_indptr;  int32_t *ap_idx;  double *ap_data;
    int64_t *pt_indptr;  int32_t *pt_idx;  double *pt_data;
    int64_t *s_indptr;   int32_t *s_idx;
    double *diag, *tscale, *vv, *ww;
    int32_t *agg, *cnt, *rcount;
    int64_t *fill;
    double *pt_acc;
    int32_t *pt_mark;
    int32_t *pt_touched;
    int32_t *tcount;
} WS;

static inline void *carve(char **p, size_t bytes){
    size_t off = ((size_t)*p + 63u) & ~(size_t)63u;
    *p = (char*)(off + bytes);
    return (void*)off;
}

/* ================= row_diagonal (parallel) ================= */
static void row_diagonal(int64_t n, const int64_t *indptr, const int32_t *idx,
                         const double *data, double *diag){
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++){
        double d = 0.0;
        for (int64_t k = indptr[i]; k < indptr[i+1]; k++)
            if (idx[k] == (int32_t)i){ d = fabs(data[k]); break; }
        diag[i] = d;
    }
}

/* ================= strength_graph (2-pass, parallel) =================
   Pass 1 stores each entry's keep-decision as one byte (each entry k is owned
   by exactly one row/thread: no races, no clearing -- the next level rewrites
   every byte it will read). Pass 2 reads the byte instead of recomputing the
   sqrt: same selection, bit-exact. smask: uint8 scratch >= nnz bytes, aliased
   onto p_idx which is free during this phase. */
static void strength_graph(int64_t n, const int64_t *indptr, const int32_t *idx,
                           const double *data, const double *diag, double theta,
                           int64_t *s_indptr, int32_t *s_idx, int32_t *rcount,
                           uint8_t *smask, int T){
    #pragma omp parallel num_threads(T)
    {
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++){
            double di = diag[i];
            int32_t c = 0;
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
                int32_t j = idx[k];
                uint8_t keep = ((int64_t)j != i && fabs(data[k]) > theta * sqrt(di * diag[j]));
                smask[k] = keep;
                c += keep;
            }
            rcount[i] = c;
        }
        #pragma omp single
        {
            int64_t pos = 0;
            for (int64_t i = 0; i < n; i++){ s_indptr[i] = pos; pos += rcount[i]; }
            s_indptr[n] = pos;
        }
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++){
            int64_t base = s_indptr[i], local = 0;
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++)
                if (smask[k]) s_idx[base + local++] = idx[k];
        }
    }
}

/* ================= aggregate (sequential, exact) ================= */
static int64_t aggregate(int64_t n, const int64_t *s_indptr, const int32_t *s_idx,
                         int32_t *agg){
    for (int64_t i = 0; i < n; i++) agg[i] = -1;
    int64_t na = 0;
    for (int64_t i = 0; i < n; i++){
        if (agg[i] < 0){
            int free = 1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i+1]; k++)
                if (agg[s_idx[k]] >= 0) free = 0;
            if (free == 1){
                agg[i] = (int32_t)na;
                for (int64_t k = s_indptr[i]; k < s_indptr[i+1]; k++) agg[s_idx[k]] = (int32_t)na;
                na++;
            }
        }
    }
    for (int64_t i = 0; i < n; i++){
        if (agg[i] < 0){
            int32_t picked = -1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i+1]; k++){
                int32_t c = agg[s_idx[k]];
                if (c >= 0 && picked < 0) picked = c;
            }
            if (picked < 0){ agg[i] = (int32_t)na; na++; }
            else agg[i] = picked;
        }
    }
    return na;
}

/* ================= tentative_scale (parallel, thread-local histogram) ================= */
static void tentative_scale(int64_t n, int64_t na, const int32_t *agg,
                            int32_t *cnt, double *tscale, int T, int32_t *hist){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        int32_t *h = hist + (int64_t)tid*na;
        for (int64_t c = 0; c < na; c++) h[c] = 0;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) h[agg[i]]++;
        #pragma omp for schedule(static)
        for (int64_t c = 0; c < na; c++){
            int32_t s = 0;
            for (int t = 0; t < T; t++) s += hist[(int64_t)t*na + c];
            cnt[c] = s;
        }
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) tscale[i] = 1.0 / sqrt((double)cnt[agg[i]]);
    }
}

/* ================= spectral_radius (parallel SpMV, serial norm) =================
   One persistent team for all RHO_ITERS iterations: 3 sub-ops per iteration as
   #pragma omp for / single, instead of 3*RHO_ITERS separate regions (45 team
   create/destroy + barriers). The norm stays a strictly serial accumulation in
   i-order (bit-exact vs the reference). */
static void spectral_radius(int64_t n, const int64_t *indptr, const int32_t *idx,
                            const double *data, const double *diag,
                            double *v, double *w, double *rho_out, int T){
    double rho = 0.0, nrm = 0.0;
    #pragma omp parallel num_threads(T)
    {
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) v[i] = 1.0;
        for (int it = 0; it < RHO_ITERS; it++){
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++){
                double acc = 0.0;
                for (int64_t k = indptr[i]; k < indptr[i+1]; k++)
                    acc += data[k] * v[idx[k]];
                w[i] = acc / diag[i];
            }
            #pragma omp single
            {
                nrm = 0.0;
                for (int64_t i = 0; i < n; i++) nrm += w[i] * w[i];
                nrm = sqrt(nrm);
                rho = nrm;
            }
            #pragma omp for schedule(static)
            for (int64_t i = 0; i < n; i++) v[i] = w[i] / nrm;
        }
    }
    *rho_out = rho;
}

/* ================= smoothed_prolongation (2-pass, parallel) ================= */
static void smoothed_prolongation(int64_t n, const int64_t *indptr, const int32_t *idx,
    const double *data, const double *diag, const int32_t *agg, const double *tscale,
    double omega, int64_t *p_indptr, int32_t *p_idx, double *p_data,
    double *pt_acc, int32_t *pt_mark, int32_t *pt_touched, int64_t ncap,
    int32_t *rcount, int T, int64_t st_base){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        double *acc = pt_acc + tid*ncap;
        int32_t *mark = pt_mark + tid*ncap;
        int32_t *touched = pt_touched + tid*ncap;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++){
            int ntouch = 0;
            int32_t st = (int32_t)(st_base + i + 1);
            int32_t c0 = agg[i];
            if (mark[c0] != st){ mark[c0] = st; touched[ntouch++] = c0; }
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
                int32_t cj = agg[idx[k]];
                if (mark[cj] != st){ mark[cj] = st; touched[ntouch++] = cj; }
            }
            rcount[i] = ntouch;
        }
        #pragma omp single
        {
            int64_t pos = 0;
            for (int64_t i = 0; i < n; i++){ p_indptr[i] = pos; pos += rcount[i]; }
            p_indptr[n] = pos;
        }
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++){
            int ntouch = 0;
            int32_t st = (int32_t)(st_base + n + i + 1);
            double di = diag[i];
            int32_t c0 = agg[i];
            if (mark[c0] != st){ mark[c0] = st; acc[c0] = 0.0; touched[ntouch++] = c0; }
            acc[c0] += tscale[i];
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
                int32_t j = idx[k];
                int32_t cj = agg[j];
                if (mark[cj] != st){ mark[cj] = st; acc[cj] = 0.0; touched[ntouch++] = cj; }
                double t = omega * (data[k] / di);
                acc[cj] -= t * tscale[j];
            }
            int64_t base = p_indptr[i];
            for (int t = 0; t < ntouch; t++){ int32_t c = touched[t]; p_idx[base+t] = c; p_data[base+t] = acc[c]; }
        }
    }
}

/* ============ csr_transpose (fully parallel, preserves row-order in columns) ============
   Entry (i,col) -> position t_indptr[col] + #{i' < i : P[i',col]!=0}.
   hist: T*ncol int32 thread-local counts; merged in one parallel pass into the
         per-thread EXCLUSIVE offsets, which double as the running scatter rank.
   tot:  ncol int32 column totals (scratch that is free at the call site).        */
static void csr_transpose(int64_t nrow, int64_t ncol,
    const int64_t *indptr, const int32_t *idx, const double *data,
    int64_t *t_indptr, int32_t *t_idx, double *t_data,
    int T, int32_t *hist, int32_t *tot){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        int32_t *h = hist + (int64_t)tid*ncol;
        for (int64_t c = 0; c < ncol; c++) h[c] = 0;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++)
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++) h[idx[k]]++;
        // exclusive offsets per thread + column totals, one parallel pass
        #pragma omp for schedule(static)
        for (int64_t c = 0; c < ncol; c++){
            int64_t sum = 0;
            for (int t = 0; t < T; t++){
                int32_t cur = hist[(int64_t)t*ncol + c];
                hist[(int64_t)t*ncol + c] = (int32_t)sum;
                sum += cur;
            }
            tot[c] = (int32_t)sum;
        }
        #pragma omp single
        {
            t_indptr[0] = 0;
            for (int64_t c = 0; c < ncol; c++) t_indptr[c+1] = t_indptr[c] + tot[c];
        }
        int32_t *rk = hist + (int64_t)tid*ncol; /* exclusive offsets, then running rank */
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++){
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
                int32_t col = idx[k];
                int64_t dest = t_indptr[col] + rk[col];
                t_idx[dest] = (int32_t)i;
                t_data[dest] = data[k];
                rk[col]++;
            }
        }
    }
}

/* ================= csr_matmul C = A@B (2-pass, parallel) ================= */
static void csr_matmul(int64_t nrow,
    const int64_t *a_indptr, const int32_t *a_idx, const double *a_data,
    const int64_t *b_indptr, const int32_t *b_idx, const double *b_data,
    int64_t *c_indptr, int32_t *c_idx, double *c_data,
    int64_t ncap, double *pt_acc, int32_t *pt_mark, int32_t *pt_touched,
    int32_t *rcount, int T, int64_t st_base){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        double *acc = pt_acc + tid*ncap;
        int32_t *mark = pt_mark + tid*ncap;
        int32_t *touched = pt_touched + tid*ncap;
        #pragma omp for schedule(dynamic, 2048)
        for (int64_t i = 0; i < nrow; i++){
            int ntouch = 0;
            int32_t st = (int32_t)(st_base + i + 1);
            for (int64_t ka = a_indptr[i]; ka < a_indptr[i+1]; ka++){
                int32_t j = a_idx[ka];
                for (int64_t kb = b_indptr[j]; kb < b_indptr[j+1]; kb++){
                    int32_t col = b_idx[kb];
                    if (mark[col] != st){ mark[col] = st; touched[ntouch++] = col; }
                }
            }
            rcount[i] = ntouch;
        }
        #pragma omp single
        {
            int64_t pos = 0;
            for (int64_t i = 0; i < nrow; i++){ c_indptr[i] = pos; pos += rcount[i]; }
            c_indptr[nrow] = pos;
        }
        #pragma omp for schedule(dynamic, 2048)
        for (int64_t i = 0; i < nrow; i++){
            int ntouch = 0;
            int32_t st = (int32_t)(st_base + nrow + i + 1);
            for (int64_t ka = a_indptr[i]; ka < a_indptr[i+1]; ka++){
                int32_t j = a_idx[ka];
                double av = a_data[ka];
                for (int64_t kb = b_indptr[j]; kb < b_indptr[j+1]; kb++){
                    int32_t col = b_idx[kb];
                    if (mark[col] != st){ mark[col] = st; acc[col] = 0.0; touched[ntouch++] = col; }
                    acc[col] += av * b_data[kb];
                }
            }
            int64_t base = c_indptr[i];
            for (int t = 0; t < ntouch; t++){ int32_t c = touched[t]; c_idx[base+t] = c; c_data[base+t] = acc[c]; }
        }
    }
}

/* ================= main ================= */
void amg_setup_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    int64_t *restrict agg0,
    int64_t *restrict level_n,
    int64_t *restrict level_nnz,
    int64_t *restrict nlevels,
    const int64_t NX, const int64_t NY, const int64_t NZ,
    const double theta,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    int64_t n0 = NX*NY*NZ;
    int64_t nnz0 = (3*NX-2)*(3*NY-2)*(3*NZ-2);
    int64_t zpad = (3*(nnz0 + n0))/2;
    if (zpad < 1) zpad = 1;

    char *wp = (char*)workspace;
    WS w;
    #define CSR(pfx) \
        w.pfx##_idx   = (int32_t*)carve(&wp, (size_t)zpad*4); \
        w.pfx##_data  = (double*) carve(&wp, (size_t)zpad*8); \
        w.pfx##_indptr= (int64_t*)carve(&wp, (size_t)(n0+1)*8);
    CSR(cur) CSR(nxt) CSR(p) CSR(ap) CSR(pt)
    #undef CSR
    w.s_idx    = (int32_t*)carve(&wp, (size_t)zpad*4);
    w.s_indptr = (int64_t*)carve(&wp, (size_t)(n0+1)*8);
    w.diag   = (double*)carve(&wp, (size_t)n0*8);
    w.tscale = (double*)carve(&wp, (size_t)n0*8);
    w.vv     = (double*)carve(&wp, (size_t)n0*8);
    w.ww     = (double*)carve(&wp, (size_t)n0*8);
    w.agg    = (int32_t*)carve(&wp, (size_t)n0*4);
    w.cnt    = (int32_t*)carve(&wp, (size_t)n0*4);
    w.rcount = (int32_t*)carve(&wp, (size_t)n0*4);
    w.fill   = (int64_t*)carve(&wp, (size_t)(n0+1)*8);
    int64_t fixed_used = (int64_t)(wp - (char*)workspace);

    int mt = omp_get_max_threads();
    if (mt < 1) mt = 1;
    int pc = phys_cores_detect();
    if (pc > 1 && pc < mt) mt = pc; /* cap threads at physical cores -- only
       when the topology report is believable; pc==1 means flat/fake guest
       topology and carries no information */
    int64_t per_thread_bytes = n0 * (8 + 4 + 4 + 4 + 4); /* acc+mark+touched+tcount+headroom */
    int64_t avail = workspace_size - fixed_used;
    if (avail < per_thread_bytes) avail = per_thread_bytes;
    int T = (int)(avail / per_thread_bytes);
    if (T > mt) T = mt;
    if (T < 1) T = 1;
    if (T > 128) T = 128;
    w.pt_acc     = (double*) carve(&wp, (size_t)T*n0*8);
    w.pt_mark    = (int32_t*)carve(&wp, (size_t)T*n0*4);
    w.pt_touched = (int32_t*)carve(&wp, (size_t)T*n0*4);
    w.tcount     = (int32_t*)carve(&wp, (size_t)T*n0*4);
    int64_t used = (int64_t)(wp - (char*)workspace);
    if (used > workspace_size){
        for (int lv=0; lv<LMAX; lv++){ level_n[lv]=0; level_nnz[lv]=0; }
        nlevels[0] = 1;
        return;
    }
    if (used > (1<<20)) (void)madvise(workspace, (size_t)used, AMG_MADV_WILLNEED);

    for (int lv = 0; lv < LMAX; lv++){ level_n[lv] = 0; level_nnz[lv] = 0; }
    tg_copy=tg_diag=tg_strength=tg_agg=tg_tent=tg_rho=tg_prol=tg_ap=tg_trans=tg_ptap=tg_other=0.0;
    tg_start();
    TG(other); /* carve+T setup */

    for (int64_t i = 0; i <= n0; i++) w.cur_indptr[i] = A_indptr[i];
    #pragma omp parallel for schedule(static)
    for (int64_t k = 0; k < nnz0; k++){ w.cur_idx[k] = (int32_t)A_indices[k]; w.cur_data[k] = A_data[k]; }
    TG(copy);

    int64_t n = n0;
    level_n[0] = n0; level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int lv = 0; lv < LMAX-1; lv++){
        if (n > MAX_COARSE){
            row_diagonal(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag);
            TG(diag);
            strength_graph(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag, theta,
                           w.s_indptr, w.s_idx, w.rcount, (uint8_t*)w.p_idx, T);
            TG(strength);
            int64_t na = aggregate(n, w.s_indptr, w.s_idx, w.agg);
            TG(agg);
            if (na < n){
                if (depth == 1){
                    #pragma omp parallel for schedule(static)
                    for (int64_t i = 0; i < n0; i++) agg0[i] = w.agg[i];
                }
                tentative_scale(n, na, w.agg, w.cnt, w.tscale, T, w.tcount);
                TG(tent);
                double rho;
                spectral_radius(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag,
                                w.vv, w.ww, &rho, T);
                TG(rho);
                double omega = SMOOTH_NUM / (SMOOTH_DEN * rho);
                int64_t ncap = na;
                memset(w.pt_mark, 0, (size_t)T*na*sizeof(int32_t));
                smoothed_prolongation(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag,
                    w.agg, w.tscale, omega, w.p_indptr, w.p_idx, w.p_data,
                    w.pt_acc, w.pt_mark, w.pt_touched, ncap, w.rcount, T, 0);
                TG(prol);
                csr_matmul(n, w.cur_indptr, w.cur_idx, w.cur_data,
                           w.p_indptr, w.p_idx, w.p_data,
                           w.ap_indptr, w.ap_idx, w.ap_data,
                           ncap, w.pt_acc, w.pt_mark, w.pt_touched, w.rcount, T, 2*n);
                TG(ap);
                csr_transpose(n, na, w.p_indptr, w.p_idx, w.p_data,
                              w.pt_indptr, w.pt_idx, w.pt_data,
                              T, w.tcount, w.cnt);
                TG(trans);
                csr_matmul(na, w.pt_indptr, w.pt_idx, w.pt_data,
                           w.ap_indptr, w.ap_idx, w.ap_data,
                           w.nxt_indptr, w.nxt_idx, w.nxt_data,
                           ncap, w.pt_acc, w.pt_mark, w.pt_touched, w.rcount, T, 4*n);
                TG(ptap);
                level_n[depth] = na;
                level_nnz[depth] = w.nxt_indptr[na];
#ifdef AMG_TIMING
                fprintf(stderr, "  LV%d: n=%ld na=%ld nnzP=%ld nnzAP=%ld nnzNext=%ld\n",
                    (int)depth, (long)n, (long)na, (long)w.p_indptr[n], (long)w.ap_indptr[n], (long)w.nxt_indptr[na]);
#endif
                depth++;
                int64_t *ti = w.cur_indptr; w.cur_indptr = w.nxt_indptr; w.nxt_indptr = ti;
                int32_t *tx = w.cur_idx;    w.cur_idx = w.nxt_idx;    w.nxt_idx = tx;
                double *td  = w.cur_data;   w.cur_data = w.nxt_data;  w.nxt_data = td;
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;
    TG(other);
    {
        double tot = tg_copy+tg_diag+tg_strength+tg_agg+tg_tent+tg_rho+tg_prol+tg_ap+tg_trans+tg_ptap+tg_other;
        static int printed = 0;
        if (!printed){
            printed = 1;
            printf("TG n0=%ld nnz0=%ld T=%d tot=%.2f copy=%.2f diag=%.2f str=%.2f agg=%.2f tent=%.2f rho=%.2f prol=%.2f ap=%.2f trans=%.2f ptap=%.2f oth=%.2f\n",
                (long)n0,(long)nnz0,T,tot*1e3,
                tg_copy*1e3,tg_diag*1e3,tg_strength*1e3,tg_agg*1e3,tg_tent*1e3,
                tg_rho*1e3,tg_prol*1e3,tg_ap*1e3,tg_trans*1e3,tg_ptap*1e3,tg_other*1e3);
            fflush(stdout);
        }
#ifdef AMG_TIMING
        fprintf(stderr, "TIMING n0=%ld nnz0=%ld T=%d total=%.1fms\n", (long)n0,(long)nnz0,T,tot*1e3);
        fprintf(stderr, "  copy=%.2f diag=%.2f strength=%.2f agg=%.2f tent=%.2f rho=%.2f prol=%.2f ap=%.2f trans=%.2f ptap=%.2f other=%.2f\n",
            tg_copy*1e3,tg_diag*1e3,tg_strength*1e3,tg_agg*1e3,tg_tent*1e3,tg_rho*1e3,tg_prol*1e3,tg_ap*1e3,tg_trans*1e3,tg_ptap*1e3,tg_other*1e3);
#endif
    }
}
