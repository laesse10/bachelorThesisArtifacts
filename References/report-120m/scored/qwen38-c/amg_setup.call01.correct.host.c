#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define LMAX 16
#define MAX_COARSE 100
#define SMOOTH_NUM 4.0
#define SMOOTH_DEN 3.0
#define RHO_ITERS 15

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
    uint8_t *pt_mark;
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

/* ================= strength_graph (2-pass, parallel) ================= */
static void strength_graph(int64_t n, const int64_t *indptr, const int32_t *idx,
                           const double *data, const double *diag, double theta,
                           int64_t *s_indptr, int32_t *s_idx, int32_t *rcount){
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++){
        double di = diag[i];
        int32_t c = 0;
        for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
            int32_t j = idx[k];
            if ((int64_t)j != i && fabs(data[k]) > theta * sqrt(di * diag[j])) c++;
        }
        rcount[i] = c;
    }
    int64_t pos = 0;
    for (int64_t i = 0; i < n; i++){ s_indptr[i] = pos; pos += rcount[i]; }
    s_indptr[n] = pos;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++){
        double di = diag[i];
        int64_t base = s_indptr[i], local = 0;
        for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
            int32_t j = idx[k];
            if ((int64_t)j != i && fabs(data[k]) > theta * sqrt(di * diag[j]))
                s_idx[base + local++] = j;
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
        for (int t = 1; t < T; t++){
            #pragma omp for schedule(static)
            for (int64_t c = 0; c < na; c++) hist[c] += hist[(int64_t)t*na + c];
        }
        #pragma omp for schedule(static)
        for (int64_t c = 0; c < na; c++) cnt[c] = hist[c];
    }
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) tscale[i] = 1.0 / sqrt((double)cnt[agg[i]]);
}

/* ================= spectral_radius (parallel SpMV, serial norm) ================= */
static void spectral_radius(int64_t n, const int64_t *indptr, const int32_t *idx,
                            const double *data, const double *diag,
                            double *v, double *w, double *rho_out){
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; i++) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; it++){
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++){
            double acc = 0.0;
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++)
                acc += data[k] * v[idx[k]];
            w[i] = acc / diag[i];
        }
        double nrm = 0.0;
        for (int64_t i = 0; i < n; i++) nrm += w[i] * w[i];
        nrm = sqrt(nrm);
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; i++) v[i] = w[i] / nrm;
        rho = nrm;
    }
    *rho_out = rho;
}

/* ================= smoothed_prolongation (2-pass, parallel) ================= */
static void smoothed_prolongation(int64_t n, const int64_t *indptr, const int32_t *idx,
    const double *data, const double *diag, const int32_t *agg, const double *tscale,
    double omega, int64_t *p_indptr, int32_t *p_idx, double *p_data,
    double *pt_acc, uint8_t *pt_mark, int32_t *pt_touched, int64_t ncap,
    int32_t *rcount, int T){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        double *acc = pt_acc + tid*ncap;
        uint8_t *mark = pt_mark + tid*ncap;
        int32_t *touched = pt_touched + tid*ncap;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++){
            int ntouch = 0;
            int32_t c0 = agg[i];
            if (mark[c0] == 0){ mark[c0] = 1; touched[ntouch++] = c0; }
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
                int32_t cj = agg[idx[k]];
                if (mark[cj] == 0){ mark[cj] = 1; touched[ntouch++] = cj; }
            }
            for (int t = 0; t < ntouch; t++) mark[touched[t]] = 0;
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
            double di = diag[i];
            int32_t c0 = agg[i];
            if (mark[c0] == 0){ mark[c0] = 1; acc[c0] = 0.0; touched[ntouch++] = c0; }
            acc[c0] += tscale[i];
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++){
                int32_t j = idx[k];
                int32_t cj = agg[j];
                if (mark[cj] == 0){ mark[cj] = 1; acc[cj] = 0.0; touched[ntouch++] = cj; }
                double t = omega * (data[k] / di);
                acc[cj] -= t * tscale[j];
            }
            int64_t base = p_indptr[i];
            for (int t = 0; t < ntouch; t++){ int32_t c = touched[t]; p_idx[base+t] = c; p_data[base+t] = acc[c]; }
            for (int t = 0; t < ntouch; t++) mark[touched[t]] = 0;
        }
    }
}

/* ============ csr_transpose (fully parallel, preserves row-order in columns) ============
   Entry (i,col) -> position t_indptr[col] + #{i' < i : P[i',col]!=0}.
   hist: T*ncol int32 thread-local counts, then top-down inclusive merge over planes.
   rank: T*ncol int32, holds the exclusive offset per thread, then running position.
   tot:  ncol int32 column totals (scratch that is free at the call site).        */
static void csr_transpose(int64_t nrow, int64_t ncol,
    const int64_t *indptr, const int32_t *idx, const double *data,
    int64_t *t_indptr, int32_t *t_idx, double *t_data,
    int T, int32_t *hist, int32_t *rank, int32_t *tot){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        int32_t *h = hist + (int64_t)tid*ncol;
        for (int64_t c = 0; c < ncol; c++) h[c] = 0;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++)
            for (int64_t k = indptr[i]; k < indptr[i+1]; k++) h[idx[k]]++;
        // top-down inclusive merge: hist[t][c] = sum_{s<=t} count[s][c]
        for (int t = 1; t < T; t++){
            #pragma omp for schedule(static)
            for (int64_t c = 0; c < ncol; c++)
                hist[(int64_t)t*ncol+c] += hist[(int64_t)(t-1)*ncol+c];
        }
        #pragma omp for schedule(static)
        for (int64_t c = 0; c < ncol; c++) tot[c] = hist[(int64_t)(T-1)*ncol+c];
        #pragma omp single
        {
            t_indptr[0] = 0;
            for (int64_t c = 0; c < ncol; c++) t_indptr[c+1] = t_indptr[c] + tot[c];
        }
        // exclusive offset of thread t = inclusive sum up to t-1 (= merged plane t-1)
        for (int t = 1; t < T; t++){
            #pragma omp for schedule(static)
            for (int64_t c = 0; c < ncol; c++)
                rank[(int64_t)t*ncol+c] = hist[(int64_t)(t-1)*ncol+c];
        }
        int32_t *rk = rank + (int64_t)tid*ncol;
        if (tid == 0){ for (int64_t c = 0; c < ncol; c++) rk[c] = 0; }
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
    int64_t ncap, double *pt_acc, uint8_t *pt_mark, int32_t *pt_touched,
    int32_t *rcount, int T){
    #pragma omp parallel num_threads(T)
    {
        int tid = omp_get_thread_num();
        double *acc = pt_acc + tid*ncap;
        uint8_t *mark = pt_mark + tid*ncap;
        int32_t *touched = pt_touched + tid*ncap;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++){
            int ntouch = 0;
            for (int64_t ka = a_indptr[i]; ka < a_indptr[i+1]; ka++){
                int32_t j = a_idx[ka];
                for (int64_t kb = b_indptr[j]; kb < b_indptr[j+1]; kb++){
                    int32_t col = b_idx[kb];
                    if (mark[col] == 0){ mark[col] = 1; touched[ntouch++] = col; }
                }
            }
            for (int t = 0; t < ntouch; t++) mark[touched[t]] = 0;
            rcount[i] = ntouch;
        }
        #pragma omp single
        {
            int64_t pos = 0;
            for (int64_t i = 0; i < nrow; i++){ c_indptr[i] = pos; pos += rcount[i]; }
            c_indptr[nrow] = pos;
        }
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++){
            int ntouch = 0;
            for (int64_t ka = a_indptr[i]; ka < a_indptr[i+1]; ka++){
                int32_t j = a_idx[ka];
                double av = a_data[ka];
                for (int64_t kb = b_indptr[j]; kb < b_indptr[j+1]; kb++){
                    int32_t col = b_idx[kb];
                    if (mark[col] == 0){ mark[col] = 1; acc[col] = 0.0; touched[ntouch++] = col; }
                    acc[col] += av * b_data[kb];
                }
            }
            int64_t base = c_indptr[i];
            for (int t = 0; t < ntouch; t++){ int32_t c = touched[t]; c_idx[base+t] = c; c_data[base+t] = acc[c]; }
            for (int t = 0; t < ntouch; t++) mark[touched[t]] = 0;
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
    int64_t per_thread_bytes = n0 * (8 + 1 + 4 + 4 + 4); /* acc+mark+touched+tcount+headroom */
    int64_t avail = workspace_size - fixed_used;
    if (avail < per_thread_bytes) avail = per_thread_bytes;
    int T = (int)(avail / per_thread_bytes);
    if (T > mt) T = mt;
    if (T < 1) T = 1;
    if (T > 128) T = 128;
    w.pt_acc     = (double*) carve(&wp, (size_t)T*n0*8);
    w.pt_mark    = (uint8_t*)carve(&wp, (size_t)T*n0);
    w.pt_touched = (int32_t*)carve(&wp, (size_t)T*n0*4);
    w.tcount     = (int32_t*)carve(&wp, (size_t)T*n0*4);
    int64_t used = (int64_t)(wp - (char*)workspace);
    if (used > workspace_size){
        for (int lv=0; lv<LMAX; lv++){ level_n[lv]=0; level_nnz[lv]=0; }
        nlevels[0] = 1;
        return;
    }

    for (int lv = 0; lv < LMAX; lv++){ level_n[lv] = 0; level_nnz[lv] = 0; }

    for (int64_t i = 0; i <= n0; i++) w.cur_indptr[i] = A_indptr[i];
    #pragma omp parallel for schedule(static)
    for (int64_t k = 0; k < nnz0; k++){ w.cur_idx[k] = (int32_t)A_indices[k]; w.cur_data[k] = A_data[k]; }

    int64_t n = n0;
    level_n[0] = n0; level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int lv = 0; lv < LMAX-1; lv++){
        if (n > MAX_COARSE){
            row_diagonal(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag);
            strength_graph(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag, theta,
                           w.s_indptr, w.s_idx, w.rcount);
            int64_t na = aggregate(n, w.s_indptr, w.s_idx, w.agg);
            if (na < n){
                if (depth == 1){
                    #pragma omp parallel for schedule(static)
                    for (int64_t i = 0; i < n0; i++) agg0[i] = w.agg[i];
                }
                tentative_scale(n, na, w.agg, w.cnt, w.tscale, T, w.tcount);
                double rho;
                spectral_radius(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag,
                                w.vv, w.ww, &rho);
                double omega = SMOOTH_NUM / (SMOOTH_DEN * rho);
                int64_t ncap = na;
                smoothed_prolongation(n, w.cur_indptr, w.cur_idx, w.cur_data, w.diag,
                    w.agg, w.tscale, omega, w.p_indptr, w.p_idx, w.p_data,
                    w.pt_acc, w.pt_mark, w.pt_touched, ncap, w.rcount, T);
                csr_matmul(n, w.cur_indptr, w.cur_idx, w.cur_data,
                           w.p_indptr, w.p_idx, w.p_data,
                           w.ap_indptr, w.ap_idx, w.ap_data,
                           ncap, w.pt_acc, w.pt_mark, w.pt_touched, w.rcount, T);
                csr_transpose(n, na, w.p_indptr, w.p_idx, w.p_data,
                              w.pt_indptr, w.pt_idx, w.pt_data,
                              T, w.tcount, w.pt_touched, w.cnt);
                csr_matmul(na, w.pt_indptr, w.pt_idx, w.pt_data,
                           w.ap_indptr, w.ap_idx, w.ap_data,
                           w.nxt_indptr, w.nxt_idx, w.nxt_data,
                           ncap, w.pt_acc, w.pt_mark, w.pt_touched, w.rcount, T);
                level_n[depth] = na;
                level_nnz[depth] = w.nxt_indptr[na];
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
}
