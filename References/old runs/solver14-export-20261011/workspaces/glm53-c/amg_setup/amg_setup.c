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

typedef int32_t i32;
typedef int64_t i64;

static void row_diagonal_par(const i32 *restrict ip, const i32 *restrict ix,
                             const double *restrict a, i64 n, double *restrict diag) {
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        double d = 0.0;
        for (i64 k = ip[i]; k < ip[i+1]; k++) {
            if (ix[k] == (i32)i) d = fabs(a[k]);
        }
        diag[i] = d;
    }
}

static void strength_par(const i32 *restrict ip, const i32 *restrict ix,
                         const double *restrict a, const double *restrict diag,
                         i64 n, double theta, i32 *restrict s_ip, i32 *restrict s_ix) {
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        i64 c = 0;
        for (i64 k = ip[i]; k < ip[i+1]; k++) {
            i32 j = ix[k];
            if (j != (i32)i && fabs(a[k]) > theta * sqrt(diag[i] * diag[j])) c++;
        }
        s_ip[i] = (i32)c;
    }
    i64 pos = 0;
    for (i64 i = 0; i < n; i++) { i32 c = s_ip[i]; s_ip[i] = (i32)pos; pos += c; }
    s_ip[n] = (i32)pos;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        i64 p = s_ip[i];
        for (i64 k = ip[i]; k < ip[i+1]; k++) {
            i32 j = ix[k];
            if (j != (i32)i && fabs(a[k]) > theta * sqrt(diag[i] * diag[j]))
                s_ix[p++] = j;
        }
    }
}

static i64 aggregate_seq(const i32 *restrict s_ip, const i32 *restrict s_ix,
                         i64 n, i32 *restrict agg) {
    for (i64 i = 0; i < n; i++) agg[i] = -1;
    i64 na = 0;
    for (i64 i = 0; i < n; i++) {
        if (agg[i] < 0) {
            int isfree = 1;
            for (i64 k = s_ip[i]; k < s_ip[i+1]; k++) {
                if (agg[s_ix[k]] >= 0) { isfree = 0; break; }
            }
            if (isfree) {
                i32 na32 = (i32)na;
                agg[i] = na32;
                for (i64 k = s_ip[i]; k < s_ip[i+1]; k++) agg[s_ix[k]] = na32;
                na++;
            }
        }
    }
    for (i64 i = 0; i < n; i++) {
        if (agg[i] < 0) {
            i32 picked = -1;
            for (i64 k = s_ip[i]; k < s_ip[i+1]; k++) {
                i32 c = agg[s_ix[k]];
                if (c >= 0) { if (picked < 0) picked = c; }
            }
            if (picked < 0) { agg[i] = (i32)na; na++; }
            else agg[i] = picked;
        }
    }
    return na;
}

static void tentative_scale_par(const i32 *restrict agg, i64 n, i64 na,
                                double *restrict cnt, double *restrict tscale) {
    #pragma omp parallel for schedule(static)
    for (i64 c = 0; c < na; c++) cnt[c] = 0.0;
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++) {
        #pragma omp atomic
        cnt[agg[i]] += 1.0;
    }
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < n; i++)
        tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
}

/* power iteration; the norm is summed single-threaded in row order so values stay
   bit-identical to the sequential reference */
static void spectral_radius_par(const i32 *restrict ip, const i32 *restrict ix,
                                const double *restrict a, const double *restrict diag,
                                i64 n, double *restrict v, double *restrict w,
                                double *restrict rho_out) {
    double rho = 0.0;
    #pragma omp parallel
    {
        for (int it = 0; it < RHO_ITERS; it++) {
            #pragma omp for schedule(static)
            for (i64 i = 0; i < n; i++) {
                double acc = 0.0;
                for (i64 k = ip[i]; k < ip[i+1]; k++)
                    acc += a[k] * v[ix[k]];
                w[i] = acc / diag[i];
            }
            #pragma omp single
            {
                double nrm = 0.0;
                for (i64 i = 0; i < n; i++) nrm += w[i] * w[i];
                nrm = sqrt(nrm);
                for (i64 i = 0; i < n; i++) v[i] = w[i] / nrm;
                rho = nrm;
            }
        }
    }
    *rho_out = rho;
}
