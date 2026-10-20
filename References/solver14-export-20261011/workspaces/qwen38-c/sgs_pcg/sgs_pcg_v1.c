#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef SYS_set_mempolicy
#define SYS_set_mempolicy 238
#endif
extern long syscall(long number, ...);
#define MPOL_DEFAULT_I 0
#define MPOL_INTERLEAVE_I 2

static void set_policy_interleave(void) {
    unsigned long mask = 0;
    int nmax = 0;
    for (int k = 0; k < 64; k++) {
        char path[64];
        snprintf(path, sizeof path, "/sys/devices/system/node/node%d", k);
        if (access(path, F_OK) == 0) {
            mask |= 1UL << k;
            nmax = k + 1;
        }
    }
    if (nmax >= 2) (void)syscall(SYS_set_mempolicy, MPOL_INTERLEAVE_I, &mask, (unsigned long)nmax);
}

static void set_policy_default(void) {
    (void)syscall(SYS_set_mempolicy, MPOL_DEFAULT_I, NULL, 0);
}

typedef int64_t i64;

static double dot8(const double *restrict a, const double *restrict b, i64 n) {
    double total = 0.0;
    i64 m = n & ~7LL;
    #pragma omp parallel for schedule(static) reduction(+:total)
    for (i64 i = 0; i < m; i += 8) {
        double s0 = a[i+0]*b[i+0], s1 = a[i+1]*b[i+1], s2 = a[i+2]*b[i+2], s3 = a[i+3]*b[i+3];
        double s4 = a[i+4]*b[i+4], s5 = a[i+5]*b[i+5], s6 = a[i+6]*b[i+6], s7 = a[i+7]*b[i+7];
        total += (s0 + s1) + (s2 + s3) + (s4 + s5) + (s6 + s7);
    }
    for (i64 i = m; i < n; i++) total += a[i]*b[i];
    return total;
}

/* One SGS sweep, serial in row order (exact reference semantics). */
static void sweep_fwd(const double *restrict A_data, const i64 *restrict A_indices,
                      const i64 *restrict A_indptr, const unsigned char *restrict diagpos,
                      const double *restrict r, const double *restrict invD,
                      double *restrict y, i64 N) {
    for (i64 i = 0; i < N; i++) {
        i64 k0 = A_indptr[i];
        int dp = diagpos[i];
        double s = r[i];
        for (i64 k = k0; k < k0 + dp; k++) s -= A_data[k] * y[A_indices[k]];
        y[i] = s * invD[i];
    }
}

static void sweep_bwd(const double *restrict A_data, const i64 *restrict A_indices,
                      const i64 *restrict A_indptr, const unsigned char *restrict diagpos,
                      const double *restrict D, const double *restrict y,
                      const double *restrict invD, double *restrict z, i64 N) {
    for (i64 i = N - 1; i >= 0; i--) {
        i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
        int dp = diagpos[i];
        double s = D[i] * y[i];
        for (i64 k = k0 + dp + 1; k < k1; k++) s -= A_data[k] * z[A_indices[k]];
        z[i] = s * invD[i];
    }
}

static void sgs_apply(const double *restrict A_data, const i64 *restrict A_indices,
                      const i64 *restrict A_indptr, const unsigned char *restrict diagpos,
                      const double *restrict D, const double *restrict invD,
                      const double *restrict r, double *restrict y, double *restrict z, i64 N) {
    sweep_fwd(A_data, A_indices, A_indptr, diagpos, r, invD, y, N);
    sweep_bwd(A_data, A_indices, A_indptr, diagpos, D, y, invD, z, N);
}

void sgs_pcg_csr_fp64(const double *restrict A_data, const i64 *restrict A_indices,
                      const i64 *restrict A_indptr, const double *restrict b,
                      double *restrict x, const i64 NX, const i64 NY, const i64 NZ,
                      const i64 niter, uint8_t *restrict workspace, const i64 workspace_size)
{
    (void)workspace; (void)workspace_size;
    const i64 N = NX * NY * NZ;
    set_policy_interleave();
    omp_set_dynamic(0);

    double *r    = (double *)malloc(sizeof(double) * (size_t)N);
    double *z    = (double *)malloc(sizeof(double) * (size_t)N);
    double *y    = (double *)malloc(sizeof(double) * (size_t)N);
    double *p    = (double *)malloc(sizeof(double) * (size_t)N);
    double *q    = (double *)malloc(sizeof(double) * (size_t)N);
    double *D    = (double *)malloc(sizeof(double) * (size_t)N);
    double *invD = (double *)malloc(sizeof(double) * (size_t)N);
    unsigned char *diagpos = (unsigned char *)malloc((size_t)N);
    if (!r || !z || !y || !p || !q || !D || !invD || !diagpos) {
        fprintf(stderr, "sgs_pcg: OOM\n");
        exit(1);
    }

    /* x0 = 0, r0 = b */
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < N; i++) {
        x[i] = 0.0;
        r[i] = b[i];
    }

    /* extract diagonal + its position in each row */
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < N; i++) {
        i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
        i64 kk;
        double d = 0.0;
        int pos = (int)(k1 - k0);
        for (kk = k0; kk < k1; kk++) {
            if (A_indices[kk] == i) { d = A_data[kk]; pos = (int)(kk - k0); break; }
        }
        D[i] = d;
        diagpos[i] = (unsigned char)pos;
    }
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < N; i++) invD[i] = 1.0 / D[i];

    /* prologue: z = M^-1 r0; p = z; rz = r . z */
    sgs_apply(A_data, A_indices, A_indptr, diagpos, D, invD, r, y, z, N);
    #pragma omp parallel for schedule(static)
    for (i64 i = 0; i < N; i++) p[i] = z[i];
    double rz = dot8(r, z, N);

    for (i64 it = 0; it < niter; it++) {
        /* q = A p */
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) {
            double acc = 0.0;
            i64 k0 = A_indptr[i], k1 = A_indptr[i + 1];
            for (i64 k = k0; k < k1; k++) acc += A_data[k] * p[A_indices[k]];
            q[i] = acc;
        }
        double pq = dot8(p, q, N);
        double alpha = rz / pq;
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) {
            x[i] += alpha * p[i];
            r[i] -= alpha * q[i];
        }
        sgs_apply(A_data, A_indices, A_indptr, diagpos, D, invD, r, y, z, N);
        double rznew = dot8(r, z, N);
        double beta = rznew / rz;
        #pragma omp parallel for schedule(static)
        for (i64 i = 0; i < N; i++) p[i] = z[i] + beta * p[i];
        rz = rznew;
    }

    free(r); free(z); free(y); free(p); free(q); free(D); free(invD); free(diagpos);
    set_policy_default();
}
