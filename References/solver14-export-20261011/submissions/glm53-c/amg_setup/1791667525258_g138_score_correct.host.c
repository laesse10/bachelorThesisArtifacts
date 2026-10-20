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

/* ---------------- faithful sequential port of the numpy reference ---------------- */

static void row_diagonal(const int64_t *a_indptr, const int64_t *a_indices,
                         const double *a_data, int64_t n, double *diag) {
    for (int64_t i = 0; i < n; i++) {
        diag[i] = 0.0;
        for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; k++) {
            if (a_indices[k] == i) diag[i] = fabs(a_data[k]);
        }
    }
}

static void strength_graph(const int64_t *a_indptr, const int64_t *a_indices,
                           const double *a_data, const double *diag, int64_t n,
                           double theta, int64_t *s_indptr, int64_t *s_indices) {
    int64_t pos = 0;
    for (int64_t i = 0; i < n; i++) {
        s_indptr[i] = pos;
        for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; k++) {
            int64_t j = a_indices[k];
            if (j != i) {
                if (fabs(a_data[k]) > theta * sqrt(diag[i] * diag[j])) {
                    s_indices[pos++] = j;
                }
            }
        }
    }
    s_indptr[n] = pos;
}

static int64_t aggregate(const int64_t *s_indptr, const int64_t *s_indices,
                         int64_t n, int64_t *agg) {
    for (int64_t i = 0; i < n; i++) agg[i] = -1;
    int64_t na = 0;
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int free = 1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++) {
                if (agg[s_indices[k]] >= 0) { free = 0; break; }
            }
            if (free) {
                agg[i] = na;
                for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++)
                    agg[s_indices[k]] = na;
                na++;
            }
        }
    }
    for (int64_t i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int64_t picked = -1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; k++) {
                int64_t c = agg[s_indices[k]];
                if (c >= 0) { if (picked < 0) picked = c; }
            }
            if (picked < 0) { agg[i] = na; na++; }
            else agg[i] = picked;
        }
    }
    return na;
}

static void tentative_scale(const int64_t *agg, int64_t n, int64_t na,
                            double *cnt, double *tscale) {
    for (int64_t c = 0; c < na; c++) cnt[c] = 0.0;
    for (int64_t i = 0; i < n; i++) cnt[agg[i]] += 1.0;
    for (int64_t i = 0; i < n; i++) tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
}

static void spectral_radius(const int64_t *a_indptr, const int64_t *a_indices,
                            const double *a_data, const double *diag, int64_t n,
                            double *v, double *w, double *rho_out) {
    for (int64_t i = 0; i < n; i++) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; it++) {
        for (int64_t i = 0; i < n; i++) {
            double acc = 0.0;
            for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; k++)
                acc += a_data[k] * v[a_indices[k]];
            w[i] = acc / diag[i];
        }
        double nrm = 0.0;
        for (int64_t i = 0; i < n; i++) nrm += w[i] * w[i];
        nrm = sqrt(nrm);
        for (int64_t i = 0; i < n; i++) v[i] = w[i] / nrm;
        rho = nrm;
    }
    *rho_out = rho;
}

static void smoothed_prolongation(const int64_t *a_indptr, const int64_t *a_indices,
                                  const double *a_data, const double *diag,
                                  const int64_t *agg, const double *tscale, int64_t n,
                                  double omega, int64_t *p_indptr, int64_t *p_indices,
                                  double *p_data, double *acc, int64_t *mark,
                                  int64_t *touched) {
    int64_t pos = 0;
    for (int64_t i = 0; i < n; i++) {
        p_indptr[i] = pos;
        int64_t ntouch = 0;
        int64_t c0 = agg[i];
        if (mark[c0] != i) {
            mark[c0] = i;
            acc[c0] = 0.0;
            touched[ntouch++] = c0;
        }
        acc[c0] += tscale[i];
        for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; k++) {
            int64_t cj = agg[a_indices[k]];
            if (mark[cj] != i) {
                mark[cj] = i;
                acc[cj] = 0.0;
                touched[ntouch++] = cj;
            }
            acc[cj] -= omega * (a_data[k] / diag[i]) * tscale[a_indices[k]];
        }
        for (int64_t t = 0; t < ntouch; t++) {
            int64_t c = touched[t];
            p_indices[pos] = c;
            p_data[pos] = acc[c];
            pos++;
        }
    }
    p_indptr[n] = pos;
}

static void csr_transpose(const int64_t *indptr, const int64_t *indices,
                          const double *data, int64_t nrow, int64_t ncol,
                          int64_t *t_indptr, int64_t *t_indices, double *t_data,
                          int64_t *fill) {
    for (int64_t c = 0; c <= ncol; c++) t_indptr[c] = 0;
    for (int64_t k = 0; k < indptr[nrow]; k++)
        t_indptr[indices[k] + 1]++;
    for (int64_t c = 0; c < ncol; c++)
        t_indptr[c + 1] += t_indptr[c];
    for (int64_t c = 0; c < ncol; c++) fill[c] = t_indptr[c];
    for (int64_t i = 0; i < nrow; i++) {
        for (int64_t k = indptr[i]; k < indptr[i + 1]; k++) {
            int64_t col = indices[k];
            t_indices[fill[col]] = i;
            t_data[fill[col]] = data[k];
            fill[col]++;
        }
    }
}

static void csr_matmul(const int64_t *a_indptr, const int64_t *a_indices,
                       const double *a_data, const int64_t *b_indptr,
                       const int64_t *b_indices, const double *b_data, int64_t nrow,
                       int64_t *c_indptr, int64_t *c_indices, double *c_data,
                       double *acc, int64_t *mark, int64_t *touched) {
    int64_t pos = 0;
    for (int64_t i = 0; i < nrow; i++) {
        c_indptr[i] = pos;
        int64_t ntouch = 0;
        for (int64_t ka = a_indptr[i]; ka < a_indptr[i + 1]; ka++) {
            int64_t j = a_indices[ka];
            double av = a_data[ka];
            for (int64_t kb = b_indptr[j]; kb < b_indptr[j + 1]; kb++) {
                int64_t col = b_indices[kb];
                if (mark[col] != i) {
                    mark[col] = i;
                    acc[col] = 0.0;
                    touched[ntouch++] = col;
                }
                acc[col] += av * b_data[kb];
            }
        }
        for (int64_t t = 0; t < ntouch; t++) {
            int64_t col = touched[t];
            c_indices[pos] = col;
            c_data[pos] = acc[col];
            pos++;
        }
    }
    c_indptr[nrow] = pos;
}

void amg_setup_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *A_indptr,
    int64_t *restrict agg0,
    int64_t *restrict level_n,
    int64_t *restrict level_nnz,
    int64_t *restrict nlevels,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const double theta,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {
    const int64_t n0 = NX * NY * NZ;
    const int64_t nnz0 = (3 * NX - 2) * (3 * NY - 2) * (3 * NZ - 2);
    const int64_t zpad = (3 * (nnz0 + n0)) / 2;
    const int64_t npad = n0;

    /* allocate work buffers */
    const int64_t Z = zpad, P = npad;
    int64_t *cur_indptr = malloc((P + 1) * sizeof(int64_t));
    int64_t *cur_indices = malloc(Z * sizeof(int64_t));
    double *cur_data = malloc(Z * sizeof(double));
    int64_t *nxt_indptr = malloc((P + 1) * sizeof(int64_t));
    int64_t *nxt_indices = malloc(Z * sizeof(int64_t));
    double *nxt_data = malloc(Z * sizeof(double));
    int64_t *s_indptr = malloc((P + 1) * sizeof(int64_t));
    int64_t *s_indices = malloc(Z * sizeof(int64_t));
    int64_t *p_indptr = malloc((P + 1) * sizeof(int64_t));
    int64_t *p_indices = malloc(Z * sizeof(int64_t));
    double *p_data = malloc(Z * sizeof(double));
    int64_t *pt_indptr = malloc((P + 1) *(int64_t) sizeof(int64_t));
    int64_t *pt_indices = malloc(Z * sizeof(int64_t));
    double *pt_data = malloc(Z * sizeof(double));
    int64_t *ap_indptr = malloc((P + 1) * sizeof(int64_t));
    int64_t *ap_indices = malloc(Z * sizeof(int64_t));
    double *ap_data = malloc(Z * sizeof(double));
    double *diag = malloc(P * sizeof(double));
    int64_t *agg = malloc(P * sizeof(int64_t));
    double *cnt = malloc(P * sizeof(double));
    double *tscale = malloc(P * sizeof(double));
    double *vv = malloc(P * sizeof(double));
    double *ww = malloc(P * sizeof(double));
    double *acc = malloc(P * sizeof(double));
    int64_t *mark = malloc(P * sizeof(int64_t));
    int64_t *touched = malloc(P * sizeof(int64_t));
    int64_t *fill = malloc((P + 1) * sizeof(int64_t));
    if (!cur_indptr || !cur_indices || !cur_data || !nxt_indptr || !nxt_indices ||
        !nxt_data || !s_indptr || !s_indices || !p_indptr || !p_indices || !p_data ||
        !pt_indptr || !pt_indices || !pt_data || !ap_indptr || !ap_indices ||
        !ap_data || !diag || !agg || !cnt || !tscale || !vv || !ww || !acc ||
        !mark || !touched || !fill) {
        /* hopeless; free what we can and bail */
        exit(1);
    }

    for (int64_t i = 0; i <= n0; i++) cur_indptr[i] = A_indptr[i];
    for (int64_t k = 0; k < nnz0; k++) {
        cur_indices[k] = A_indices[k];
        cur_data[k] = A_data[k];
    }

    for (int lv = 0; lv < LMAX; lv++) {
        level_n[lv] = 0;
        level_nnz[lv] = 0;
    }

    int64_t n = n0;
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;
    double rho;
    double omega;

    for (int lv = 0; lv < LMAX - 1; lv++) {
        if (n > MAX_COARSE) {
            row_diagonal(cur_indptr, cur_indices, cur_data, n, diag);
            strength_graph(cur_indptr, cur_indices, cur_data, diag, n, theta,
                          s_indptr, s_indices);
            int64_t na = aggregate(s_indptr, s_indices, n, agg);
            if (na < n) {
                if (depth == 1) {
                    for (int64_t i = 0; i < n0; i++) agg0[i] = agg[i];
                }
                tentative_scale(agg, n, na, cnt, tscale);
                spectral_radius(cur_indptr, cur_indices, cur_data, diag, n, vv, ww, &rho);
                omega = SMOOTH_NUM / (SMOOTH_DEN * rho);
                for (int64_t c = 0; c < na; c++) mark[c] = -1;
                smoothed_prolongation(cur_indptr, cur_indices, cur_data, diag, agg,
                                      tscale, n, omega, p_indptr, p_indices, p_data,
                                      acc, mark, touched);
                for (int64_t c = 0; c < na; c++) mark[c] = -1;
                csr_matmul(cur_indptr, cur_indices, cur_data, p_indptr, p_indices,
                           p_data, n, ap_indptr, ap_indices, ap_data, acc, mark, touched);
                csr_transpose(p_indptr, p_indices, p_data, n, na, pt_indptr,
                              pt_indices, pt_data, fill);
                for (int64_t c = 0; c < na; c++) mark[c] = -1;
                csr_matmul(pt_indptr, pt_indices, pt_data, ap_indptr, ap_indices,
                           ap_data, na, nxt_indptr, nxt_indices, nxt_data, acc, mark,
                           touched);
                level_n[depth] = na;
                level_nnz[depth] = nxt_indptr[na];
                depth++;
                for (int64_t i = 0; i <= na; i++) cur_indptr[i] = nxt_indptr[i];
                for (int64_t k = 0; k < nxt_indptr[na]; k++) {
                    cur_indices[k] = nxt_indices[k];
                    cur_data[k] = nxt_data[k];
                }
                n = na;
            } else {
                n = 0;
            }
        }
    }
    nlevels[0] = depth;

    free(cur_indptr); free(cur_indices); free(cur_data);
    free(nxt_indptr); free(nxt_indices); free(nxt_data);
    free(s_indptr); free(s_indices);
    free(p_indptr); free(p_indices); free(p_data);
    free(pt_indptr); free(pt_indices); free(pt_data);
    free(ap_indptr); free(ap_indices); free(ap_data);
    free(diag); free(agg); free(cnt); free(tscale); free(vv); free(ww);
    free(acc); free(mark); free(touched); free(fill);
}
