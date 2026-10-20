#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>

#define LMAX 16
#define MAX_COARSE 100
#define SMOOTH_NUM 4.0
#define SMOOTH_DEN 3.0
#define RHO_ITERS 15
#define TMAX 64

typedef struct {
    int32_t *X_ip, *C_ip, *D_ip, *E_ip, *F_ip, *fill;
    int32_t *X_ix, *C_ix, *D_ix, *E_ix, *F_ix;
    double  *X_da, *C_da, *D_da, *E_da;
    double  *diag, *cnt, *tscale, *vv, *ww;
    int32_t *agg;
    double  *acc_p;
    int32_t *mark_p, *touched_p, *cnt_row;
    int32_t T;
} Buffers;

static Buffers B;
static int32_t *g_i32 = NULL;
static double  *g_f64 = NULL;
static int64_t  g_i32_cap = 0, g_f64_cap = 0;

static void ensure_alloc(int64_t npad, int64_t zpad) {
    int32_t T = (int32_t)omp_get_max_threads();
    if (T > TMAX) T = TMAX;
    B.T = T;
    int64_t need_i = 6*(npad+1) + 5*zpad + npad + (int64_t)T*npad*2 + npad + 8;
    int64_t need_f = 4*zpad + 5*npad + (int64_t)T*npad + 8;
    if (need_i > g_i32_cap) { free(g_i32); g_i32 = (int32_t*)aligned_alloc(64, (size_t)need_i*sizeof(int32_t)); g_i32_cap = need_i; }
    if (need_f > g_f64_cap) { free(g_f64); g_f64 = (double*)aligned_alloc(64, (size_t)need_f*sizeof(double)); g_f64_cap = need_f; }
    int32_t *p = g_i32;
    B.X_ip = p; p += npad+1;
    B.C_ip = p; p += npad+1;
    B.D_ip = p; p += npad+1;
    B.E_ip = p; p += npad+1;
    B.F_ip = p; p += npad+1;
    B.fill = p; p += npad+1;
    B.X_ix = p; p += zpad;
    B.C_ix = p; p += zpad;
    B.D_ix = p; p += zpad;
    B.E_ix = p; p += zpad;
    B.F_ix = p; p += zpad;
    B.agg = p; p += npad;
    B.mark_p = p; p += (int64_t)T*npad;
    B.touched_p = p; p += (int64_t)T*npad;
    B.cnt_row = p; p += npad;
    double *q = g_f64;
    B.X_da = q; q += zpad;
    B.C_da = q; q += zpad;
    B.D_da = q; q += zpad;
    B.E_da = q; q += zpad;
    B.diag = q; q += npad;
    B.cnt = q; q += npad;
    B.tscale = q; q += npad;
    B.vv = q; q += npad;
    B.ww = q; q += npad;
    B.acc_p = q; q += (int64_t)T*npad;
}

static void row_diagonal(const int32_t *ip, const int32_t *ix, const double *da, int64_t n, double *diag, int32_t T) {
    #pragma omp parallel for num_threads(T) schedule(static)
    for (int64_t i = 0; i < n; i++) {
        double d = 0.0;
        for (int64_t k = ip[i]; k < ip[i+1]; k++) if (ix[k] == i) d = fabs(da[k]);
        diag[i] = d;
    }
}

static void strength_graph(const int32_t *ip, const int32_t *ix, const double *da,
                           const double *diag, int64_t n, double theta,
                           int32_t *F_ip, int32_t *F_ix, int32_t *cnt_row, int32_t T) {
    #pragma omp parallel for num_threads(T) schedule(static)
    for (int64_t i = 0; i < n; i++) {
        int64_t c = 0;
        for (int64_t k = ip[i]; k < ip[i+1]; k++) {
            int32_t j = ix[k];
            if (j != i && fabs(da[k]) > theta * sqrt(diag[i]*diag[j])) c++;
        }
        cnt_row[i] = (int32_t)c;
    }
    F_ip[0] = 0;
    for (int64_t i = 0; i < n; i++) F_ip[i+1] = F_ip[i] + cnt_row[i];
    #pragma omp parallel for num_threads(T) schedule(static)
    for (int64_t i = 0; i < n; i++) {
        int32_t pos = F_ip[i];
        for (int64_t k = ip[i]; k < ip[i+1]; k++) {
            int32_t j = ix[k];
            if (j != i && fabs(da[k]) > theta * sqrt(diag[i]*diag[j])) F_ix[pos++] = j;
        }
    }
}

static int32_t aggregate(const int32_t *F_ip, const int32_t *F_ix, int64_t n, int32_t *agg) {
    for (int64_t i = 0; i < n; i++) agg[i] = -1;
    int32_t na = 0;
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int free = 1;
            for (int64_t k = F_ip[i]; k < F_ip[i+1]; k++) if (agg[F_ix[k]] >= 0) { free = 0; break; }
            if (free) { agg[i] = na; for (int64_t k = F_ip[i]; k < F_ip[i+1]; k++) agg[F_ix[k]] = na; na++; }
        }
    }
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int32_t picked = -1;
            for (int64_t k = F_ip[i]; k < F_ip[i+1]; k++) { int32_t c = agg[F_ix[k]]; if (c >= 0) { picked = c; break; } }
            if (picked < 0) { agg[i] = na; na++; } else agg[i] = picked;
        }
    }
    return na;
}

static void tentative_scale(const int32_t *agg, int64_t n, int32_t na, double *cnt, double *tscale,
                            int32_t *scratch_i32, int64_t npad, int32_t T) {
    #pragma omp parallel num_threads(T)
    {
        int32_t tid = omp_get_thread_num();
        int32_t *lc = scratch_i32 + (int64_t)tid*npad;
        for (int64_t c = 0; c < na; c++) lc[c] = 0;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) lc[agg[i]]++;
        #pragma omp barrier
        #pragma omp for schedule(static)
        for (int64_t c = 0; c < na; c++) {
            int64_t s = 0;
            for (int32_t t = 0; t < T; t++) s += scratch_i32[(int64_t)t*npad + c];
            cnt[c] = (double)s;
        }
        #pragma omp barrier
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
    }
}

static double spectral_radius(const int32_t *ip, const int32_t *ix, const double *da,
                              const double *diag, int64_t n, double *v, double *w, int32_t T) {
    for (int64_t i = 0; i < n; i++) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; it++) {
        #pragma omp parallel for num_threads(T) schedule(static)
        for (int64_t i = 0; i < n; i++) {
            double acc = 0.0;
            for (int64_t k = ip[i]; k < ip[i+1]; k++) acc += da[k] * v[ix[k]];
            w[i] = acc / diag[i];
        }
        double nrm = 0.0;
        for (int64_t i = 0; i < n; i++) nrm += w[i]*w[i];
        nrm = sqrt(nrm);
        #pragma omp parallel for num_threads(T) schedule(static)
        for (int64_t i = 0; i < n; i++) v[i] = w[i]/nrm;
        rho = nrm;
    }
    return rho;
}

/* Reset each thread's mark[0..na) to -1 so the row-id stamping starts clean. */
static void init_mark(int64_t na, int32_t *mark_g, int32_t T, int64_t npad) {
    #pragma omp parallel for num_threads(T) schedule(static)
    for (int32_t t = 0; t < T; t++) {
        memset(mark_g + (int64_t)t*npad, 0xFF, (size_t)na*sizeof(int32_t));
    }
}

static void csr_matmul(const int32_t *aip, const int32_t *aix, const double *ada,
                       const int32_t *bip, const int32_t *bix, const double *bda,
                       int64_t nrow, int64_t ncol, int32_t T,
                       int32_t *cip, int32_t *cix, double *cda,
                       int32_t *cnt_row, double *acc_g, int32_t *mark_g, int32_t *touched_g, int64_t npad) {
    init_mark(ncol, mark_g, T, npad);
    #pragma omp parallel num_threads(T)
    {
        int32_t tid = omp_get_thread_num();
        int32_t *mark = mark_g + (int64_t)tid*npad;
        int32_t *touched = touched_g + (int64_t)tid*npad;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++) {
            int32_t ntouch = 0;
            for (int64_t ka = aip[i]; ka < aip[i+1]; ka++) {
                int32_t j = aix[ka];
                for (int64_t kb = bip[j]; kb < bip[j+1]; kb++) {
                    int32_t col = bix[kb];
                    if (mark[col] != (int32_t)i) { mark[col] = (int32_t)i; touched[ntouch++] = col; }
                }
            }
            cnt_row[i] = ntouch;
        }
    }
    cip[0] = 0;
    for (int64_t i = 0; i < nrow; i++) cip[i+1] = cip[i] + cnt_row[i];
    init_mark(ncol, mark_g, T, npad);
    #pragma omp parallel num_threads(T)
    {
        int32_t tid = omp_get_thread_num();
        double *acc = acc_g + (int64_t)tid*npad;
        int32_t *mark = mark_g + (int64_t)tid*npad;
        int32_t *touched = touched_g + (int64_t)tid*npad;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < nrow; i++) {
            int32_t ntouch = 0;
            for (int64_t ka = aip[i]; ka < aip[i+1]; ka++) {
                int32_t j = aix[ka];
                double av = ada[ka];
                for (int64_t kb = bip[j]; kb < bip[j+1]; kb++) {
                    int32_t col = bix[kb];
                    if (mark[col] != (int32_t)i) { mark[col] = (int32_t)i; acc[col] = 0.0; touched[ntouch++] = col; }
                    acc[col] += av * bda[kb];
                }
            }
            int64_t pos = cip[i];
            for (int32_t t = 0; t < ntouch; t++) { int32_t c = touched[t]; cix[pos] = c; cda[pos] = acc[c]; pos++; }
        }
    }
}

static void smoothed_prolongation(const int32_t *aip, const int32_t *aix, const double *ada,
                                  const double *diag, const int32_t *agg, const double *tscale,
                                  int64_t n, int64_t na, double omega, int32_t T,
                                  int32_t *pip, int32_t *pix, double *pda,
                                  int32_t *cnt_row, double *acc_g, int32_t *mark_g, int32_t *touched_g, int64_t npad) {
    init_mark(na, mark_g, T, npad);
    #pragma omp parallel num_threads(T)
    {
        int32_t tid = omp_get_thread_num();
        int32_t *mark = mark_g + (int64_t)tid*npad;
        int32_t *touched = touched_g + (int64_t)tid*npad;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            int32_t ntouch = 0;
            int32_t c0 = agg[i];
            if (mark[c0] != (int32_t)i) { mark[c0] = (int32_t)i; touched[ntouch++] = c0; }
            for (int64_t k = aip[i]; k < aip[i+1]; k++) {
                int32_t cj = agg[aix[k]];
                if (mark[cj] != (int32_t)i) { mark[cj] = (int32_t)i; touched[ntouch++] = cj; }
            }
            cnt_row[i] = ntouch;
        }
    }
    pip[0] = 0;
    for (int64_t i = 0; i < n; i++) pip[i+1] = pip[i] + cnt_row[i];
    init_mark(na, mark_g, T, npad);
    #pragma omp parallel num_threads(T)
    {
        int32_t tid = omp_get_thread_num();
        double *acc = acc_g + (int64_t)tid*npad;
        int32_t *mark = mark_g + (int64_t)tid*npad;
        int32_t *touched = touched_g + (int64_t)tid*npad;
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < n; i++) {
            int32_t ntouch = 0;
            int32_t c0 = agg[i];
            if (mark[c0] != (int32_t)i) { mark[c0] = (int32_t)i; acc[c0] = 0.0; touched[ntouch++] = c0; }
            acc[c0] += tscale[i];
            for (int64_t k = aip[i]; k < aip[i+1]; k++) {
                int32_t j = aix[k];
                int32_t cj = agg[j];
                if (mark[cj] != (int32_t)i) { mark[cj] = (int32_t)i; acc[cj] = 0.0; touched[ntouch++] = cj; }
                acc[cj] -= omega * (ada[k] / diag[i]) * tscale[j];
            }
            int64_t pos = pip[i];
            for (int32_t t = 0; t < ntouch; t++) { int32_t c = touched[t]; pix[pos] = c; pda[pos] = acc[c]; pos++; }
        }
    }
}

/* Sequential transpose so that each output row (column of M) is in increasing-row order. */
static void csr_transpose(const int32_t *ip, const int32_t *ix, const double *da,
                          int64_t nrow, int64_t ncol, int32_t *tip, int32_t *tix, double *tda, int32_t *fill) {
    for (int64_t c = 0; c <= ncol; c++) tip[c] = 0;
    for (int64_t k = 0; k < ip[nrow]; k++) tip[ix[k]+1]++;
    for (int64_t c = 0; c < ncol; c++) tip[c+1] += tip[c];
    for (int64_t c = 0; c < ncol; c++) fill[c] = tip[c];
    for (int64_t i = 0; i < nrow; i++) {
        for (int64_t k = ip[i]; k < ip[i+1]; k++) {
            int32_t col = ix[k];
            tix[fill[col]] = (int32_t)i;
            tda[fill[col]] = da[k];
            fill[col]++;
        }
    }
}

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
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    int64_t n0 = NX*NY*NZ;
    int64_t nnz0 = (3*NX-2)*(3*NY-2)*(3*NZ-2);
    int64_t npad = n0;
    int64_t zpad = 8*(nnz0+n0);
    ensure_alloc(npad, zpad);
    int32_t T = B.T;

    int32_t *X_ip=B.X_ip, *X_ix=B.X_ix; double *X_da=B.X_da;
    int32_t *C_ip=B.C_ip, *C_ix=B.C_ix; double *C_da=B.C_da;
    int32_t *D_ip=B.D_ip, *D_ix=B.D_ix; double *D_da=B.D_da;
    int32_t *E_ip=B.E_ip, *E_ix=B.E_ix; double *E_da=B.E_da;
    int32_t *F_ip=B.F_ip, *F_ix=B.F_ix;
    double *diag=B.diag, *cnt=B.cnt, *tscale=B.tscale, *vv=B.vv, *ww=B.ww;
    int32_t *agg=B.agg, *cnt_row=B.cnt_row, *fill=B.fill;
    double *acc_p=B.acc_p; int32_t *mark_p=B.mark_p, *touched_p=B.touched_p;

    for (int64_t i = 0; i <= n0; i++) X_ip[i] = (int32_t)A_indptr[i];
    #pragma omp parallel for num_threads(T) schedule(static)
    for (int64_t k = 0; k < nnz0; k++) { X_ix[k] = (int32_t)A_indices[k]; X_da[k] = A_data[k]; }

    for (int lv = 0; lv < LMAX; lv++) { level_n[lv] = 0; level_nnz[lv] = 0; }
    int64_t n = n0;
    level_n[0] = n0; level_nnz[0] = nnz0;
    int64_t depth = 1;

    for (int _lv = 0; _lv < LMAX-1; _lv++) {
        if (n > MAX_COARSE) {
            row_diagonal(X_ip, X_ix, X_da, n, diag, T);
            strength_graph(X_ip, X_ix, X_da, diag, n, theta, F_ip, F_ix, cnt_row, T);
            int32_t na = aggregate(F_ip, F_ix, n, agg);
            if (na < n) {
                if (depth == 1) {
                    #pragma omp parallel for num_threads(T) schedule(static)
                    for (int64_t i = 0; i < n0; i++) agg0[i] = (int64_t)agg[i];
                }
                tentative_scale(agg, n, na, cnt, tscale, touched_p, npad, T);
                double rho = spectral_radius(X_ip, X_ix, X_da, diag, n, vv, ww, T);
                double omega = SMOOTH_NUM / (SMOOTH_DEN * rho);
                smoothed_prolongation(X_ip, X_ix, X_da, diag, agg, tscale, n, na, omega, T, C_ip, C_ix, C_da, cnt_row, acc_p, mark_p, touched_p, npad);
                csr_matmul(X_ip, X_ix, X_da, C_ip, C_ix, C_da, n, na, T, E_ip, E_ix, E_da, cnt_row, acc_p, mark_p, touched_p, npad);
                fprintf(stderr,"DBG lvl n=%lld na=%lld nnz_P=%lld nnz_AP=%lld nnz_A=%lld zpad=%lld\n",(long long)n,(long long)na,(long long)C_ip[n],(long long)E_ip[n],(long long)X_ip[n],(long long)zpad);
                csr_transpose(C_ip, C_ix, C_da, n, na, D_ip, D_ix, D_da, fill);
                csr_matmul(D_ip, D_ix, D_da, E_ip, E_ix, E_da, na, na, T, X_ip, X_ix, X_da, cnt_row, acc_p, mark_p, touched_p, npad);
                fprintf(stderr,"DBG2 nnz_NEXT=%lld nnz_PT=%lld\n",(long long)X_ip[na],(long long)D_ip[na]);
                level_n[depth] = na; level_nnz[depth] = X_ip[na]; depth++;
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;
}
