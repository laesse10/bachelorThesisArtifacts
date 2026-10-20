/* v2: stencil-specialized wavefront SGS-PCG (bitwise-exact), general CSR fallback */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define SGS_SPBLK 512

/* ================= general-CSR fallback ================= */

static void sgs_apply_seq(const double *restrict A_data, const int *restrict A_idx,
                          const int *restrict A_ptr, const double *restrict dg,
                          const double *restrict r, double *restrict y,
                          double *restrict z, int N)
{
    for (int i = 0; i < N; i++) {
        double s = r[i];
        for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++) {
            int j = A_idx[k];
            if (j < i) s = s - A_data[k] * y[j];
        }
        y[i] = s / dg[i];
    }
    for (int ii = 0; ii < N; ii++) {
        int row = N - 1 - ii;
        double s = dg[row] * y[row];
        for (int k = A_ptr[row]; k < A_ptr[row + 1]; k++) {
            int j = A_idx[k];
            if (j > row) s = s - A_data[k] * z[j];
        }
        z[row] = s / dg[row];
    }
}

static void pcg_csr(const double *restrict A_data, const int64_t *restrict A_indices,
                   const int64_t *restrict A_indptr, const double *restrict b,
                   double *restrict x, int N, long niter)
{
    double *r  = (double *)malloc(sizeof(double) * (size_t)N);
    double *y  = (double *)malloc(sizeof(double) * (size_t)N);
    double *z  = (double *)malloc(sizeof(double) * (size_t)N);
    double *p  = (double *)malloc(sizeof(double) * (size_t)N);
    double *q  = (double *)malloc(sizeof(double) * (size_t)N);
    double *dg = (double *)malloc(sizeof(double) * (size_t)N);
    int *A_idx = (int *)malloc(sizeof(int) * (size_t)A_indptr[N]);
    int *A_ptr = (int *)malloc(sizeof(int) * (size_t)(N + 1));
    if (!r || !y || !z || !p || !q || !dg || !A_idx || !A_ptr) {
        free(r); free(y); free(z); free(p); free(q); free(dg); free(A_idx); free(A_ptr);
        return;
    }
    {
        int64_t nnz = A_indptr[N];
        #pragma omp parallel for schedule(static)
        for (int64_t k = 0; k < nnz; k++) A_idx[k] = (int)A_indices[k];
        #pragma omp parallel for schedule(static)
        for (int i = 0; i <= N; i++) A_ptr[i] = (int)A_indptr[i];
    }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) { x[i] = 0.0; r[i] = b[i]; }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        double d = 0.0;
        for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++)
            if (A_idx[k] == i) d = A_data[k];
        dg[i] = d;
    }
    sgs_apply_seq(A_data, A_idx, A_ptr, dg, r, y, z, N);
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) p[i] = z[i];
    double rz = 0.0;
    for (int i = 0; i < N; i++) rz += r[i] * z[i];
    for (long it = 0; it < niter; it++) {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            double acc = 0.0;
            for (int k = A_ptr[i]; k < A_ptr[i + 1]; k++)
                acc = acc + A_data[k] * p[A_idx[k]];
            q[i] = acc;
        }
        double pq = 0.0;
        for (int i = 0; i < N; i++) pq += p[i] * q[i];
        double alpha = rz / pq;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) {
            x[i] = x[i] + alpha * p[i];
            r[i] = r[i] - alpha * q[i];
        }
        sgs_apply_seq(A_data, A_idx, A_ptr, dg, r, y, z, N);
        double rznew = 0.0;
        for (int i = 0; i < N; i++) rznew += r[i] * z[i];
        double beta = rznew / rz;
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < N; i++) p[i] = z[i] + beta * p[i];
        rz = rznew;
    }
    free(r); free(y); free(z); free(p); free(q); free(dg); free(A_idx); free(A_ptr);
}

/* ================= stencil path ================= */

/* one forward x-line: rows base .. base+NX-1 (bitwise-exact per-row order) */
static void fwd_line(const double *restrict vt, const double *restrict r,
                     double *restrict y, double *restrict far,
                     int base, int NX, int N, const int *restrict off)
{
    for (int j = 0; j < NX; j++) far[j] = r[base + j];
    for (int p = 0; p < 12; p++) {
        const double *vp = vt + (size_t)p * N + base;
        const double *ys = y + base + off[p];
        for (int j = 0; j < NX; j++) far[j] = far[j] - vp[j] * ys[j];
    }
    const double *v12 = vt + (size_t)12 * N;
    double yprev = 0.0;
    for (int j = 0; j < NX; j++) {
        int i = base + j;
        y[i] = (far[j] - v12[i] * yprev) / vt[(size_t)13 * N + i];
        yprev = y[i];
    }
}

/* one backward x-line: rows base+NX-1 .. base (bitwise-exact per-row order) */
static void bwd_line(const double *restrict vt, const double *restrict y,
                     double *restrict z, int base, int NX, int N,
                     const int *restrict off)
{
    const double *v13 = vt + (size_t)13 * N;
    const double *v14 = vt + (size_t)14 * N;
    /* u[j] = diag[i]*y[i] precomputed (separate muls, matching reference rounding) */
    double *vz = z;
    for (int j = 0; j < NX; j++) vz[base + j] = v13[base + j] * y[base + j];
    double znext = 0.0;
    for (int j = NX - 1; j >= 0; j--) {
        int i = base + j;
        double s = z[i];
        s = s - v14[i] * znext;
        for (int p = 15; p <= 26; p++)
            s = s - vt[(size_t)p * N + i] * z[i + off[p]];
        z[i] = s / v13[i];
        znext = z[i];
    }
}

static void fwd_sweep(const double *restrict vt, const double *restrict r,
                      double *restrict y, double *restrict far,
                      int NX, int NY, int NZ, int N, const int *restrict off)
{
    const int L = 2 * (NZ - 1) + (NY - 1);
    for (int l = 0; l <= L; l++) {
        int lo = l - 2 * (NZ - 1); if (lo < 0) lo = 0;
        int hi = l; if (hi > NY - 1) hi = NY - 1;
        if (((l - lo) & 1) != 0) lo++;
        #pragma omp for schedule(static)
        for (int iy = lo; iy <= hi; iy += 2) {
            int iz = (l - iy) >> 1;
            fwd_line(vt, r, y, far, iy * NX + iz * NX * NY, NX, N, off);
        }
    }
}

static void bwd_sweep(const double *restrict vt, const double *restrict y,
                      double *restrict z,
                      int NX, int NY, int NZ, int N, const int *restrict off)
{
    const int L = 2 * (NZ - 1) + (NY - 1);
    for (int l = L; l >= 0; l--) {
        int lo = l - 2 * (NZ - 1); if (lo < 0) lo = 0;
        int hi = l; if (hi > NY - 1) hi = NY - 1;
        if (((l - lo) & 1) != 0) lo++;
        #pragma omp for schedule(static)
        for (int iy = lo; iy <= hi; iy += 2) {
            int iz = (l - iy) >> 1;
            bwd_line(vt, y, z, iy * NX + iz * NX * NY, NX, N, off);
        }
    }
}

void sgs_pcg_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    const double *restrict b,
    double *restrict x,
    const int64_t NX_, const int64_t NY_, const int64_t NZ_,
    const int64_t niter_,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)workspace; (void)workspace_size;
    const int NX = (int)NX_, NY = (int)NY_, NZ = (int)NZ_;
    const int N = NX * NY * NZ;
    const long niter = (long)niter_;
    if (N <= 0) return;

    if (NX < 3 || NY < 3 || NZ < 3) { pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter); return; }

    const int g = NX * NY + NX + 1;

    /* offsets sorted ascending with their (dx,dy,dz) */
    int off[27], dxa[27], dya[27], dza[27];
    {
        int eo[27], ex[27], ey[27], ez[27];
        int c = 0;
        for (int dz = -1; dz <= 1; dz++)
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++) {
                    eo[c] = dx + dy * NX + dz * NX * NY;
                    ex[c] = dx; ey[c] = dy; ez[c] = dz; c++;
                }
        for (int p = 1; p < 27; p++) {  /* insertion sort */
            int o = eo[p], ax = ex[p], ay = ey[p], az = ez[p];
            int q = p - 1;
            while (q >= 0 && eo[q] > o) {
                eo[q + 1] = eo[q]; ex[q + 1] = ex[q]; ey[q + 1] = ey[q]; ez[q + 1] = ez[q];
                q--;
            }
            eo[q + 1] = o; ex[q + 1] = ax; ey[q + 1] = ay; ez[q + 1] = az;
        }
        for (int p = 0; p < 27; p++) { off[p] = eo[p]; dxa[p] = ex[p]; dya[p] = ey[p]; dza[p] = ez[p]; }
    }

    /* position lookup table over offsets [-g, g] */
    int8_t *tbl = (int8_t *)malloc((size_t)(2 * g + 1));
    double *vt = (double *)malloc(sizeof(double) * (size_t)27 * N);
    if (!tbl || !vt) { free(tbl); free(vt); pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter); return; }
    memset(tbl, -1, (size_t)(2 * g + 1));
    for (int p = 0; p < 27; p++) tbl[off[p] + g] = (int8_t)p;

    volatile int ok = 1;
    #pragma omp parallel
    {
        int lok_ok = 1;
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) {
            int ix = i % NX, iz = i / (NX * NY), iy = (i / NX) % NY;
            for (int p = 0; p < 27; p++) vt[(size_t)p * N + i] = 0.0;
            uint32_t mask = 0;
            int64_t k0 = A_indptr[i], k1 = A_indptr[i + 1];
            int64_t jprev = -1;
            for (int64_t k = k0; k < k1; k++) {
                int64_t j = A_indices[k];
                if (j <= jprev) lok_ok = 0;   /* require strictly ascending */
                jprev = j;
                int64_t d = j - i;
                if (d < -g || d > g) { lok_ok = 0; continue; }
                int p = tbl[d + g];
                if (p < 0) { lok_ok = 0; continue; }
                if (ix + dxa[p] < 0 || ix + dxa[p] >= NX ||
                    iy + dya[p] < 0 || iy + dya[p] >= NY ||
                    iz + dza[p] < 0 || iz + dza[p] >= NZ) { lok_ok = 0; continue; }
                if (mask & (1u << p)) { lok_ok = 0; continue; }
                mask |= (1u << p);
                vt[(size_t)p * N + i] = A_data[k];
            }
        }
        if (!lok_ok) ok = 0;
    }
    free(tbl);
    if (!ok) {
        free(vt);
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }

    /* guarded work arrays: valid indices [-g, N+g) */
    const size_t asz = (size_t)N + 2 * (size_t)g;
    double *r_raw = (double *)malloc(sizeof(double) * asz);
    double *y_raw = (double *)malloc(sizeof(double) * asz);
    double *z_raw = (double *)malloc(sizeof(double) * asz);
    double *p_raw = (double *)malloc(sizeof(double) * asz);
    double *q_arr = (double *)malloc(sizeof(double) * (size_t)N);
    if (!r_raw || !y_raw || !z_raw || !p_raw || !q_arr) {
        free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(vt);
        pcg_csr(A_data, A_indices, A_indptr, b, x, N, niter);
        return;
    }
    memset(y_raw, 0, sizeof(double) * asz);
    memset(z_raw, 0, sizeof(double) * asz);
    memset(p_raw, 0, sizeof(double) * asz);
    double *r = r_raw + g, *y = y_raw + g, *z = z_raw + g, *p = p_raw + g;

    double rz = 0.0, alpha = 0.0, beta = 0.0, pq = 0.0, rznew = 0.0;

    #pragma omp parallel
    {
        double *far = (double *)malloc(sizeof(double) * (size_t)NX + 64);
        /* x0 = 0, r0 = b */
        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) { x[i] = 0.0; r[i] = b[i]; }

        fwd_sweep(vt, r, y, far, NX, NY, NZ, N, off);
        bwd_sweep(vt, y, z, NX, NY, NZ, N, off);

        #pragma omp for schedule(static)
        for (int i = 0; i < N; i++) p[i] = z[i];

        #pragma omp single
        {
            rz = 0.0;
            for (int i = 0; i < N; i++) rz += r[i] * z[i];
        }

        for (long it = 0; it < niter; it++) {
            /* q = A p */
            #pragma omp for schedule(static)
            for (int i0 = 0; i0 < N; i0 += SGS_SPBLK) {
                int i1 = i0 + SGS_SPBLK; if (i1 > N) i1 = N;
                double acc[SGS_SPBLK] __attribute__((aligned(64)));
                double tmp[SGS_SPBLK] __attribute__((aligned(64)));
                for (int j = i0; j < i1; j++) acc[j - i0] = 0.0;
                for (int pp = 0; pp < 27; pp++) {
                    const double *vp = vt + (size_t)pp * N;
                    const double *ps = p + off[pp];
                    for (int j = i0; j < i1; j++) tmp[j - i0] = vp[j] * ps[j];
                    for (int j = i0; j < i1; j++) acc[j - i0] = acc[j - i0] + tmp[j - i0];
                }
                for (int j = i0; j < i1; j++) q_arr[j] = acc[j - i0];
            }

            #pragma omp single
            {
                pq = 0.0;
                for (int i = 0; i < N; i++) pq += p[i] * q_arr[i];
                alpha = rz / pq;
            }

            #pragma omp for schedule(static)
            for (int i = 0; i < N; i++) {
                x[i] = x[i] + alpha * p[i];
                r[i] = r[i] - alpha * q_arr[i];
            }

            fwd_sweep(vt, r, y, far, NX, NY, NZ, N, off);
            bwd_sweep(vt, y, z, NX, NY, NZ, N, off);

            #pragma omp single
            {
                rznew = 0.0;
                for (int i = 0; i < N; i++) rznew += r[i] * z[i];
                beta = rznew / rz;
                rz = rznew;
            }

            #pragma omp for schedule(static)
            for (int i = 0; i < N; i++) p[i] = z[i] + beta * p[i];
        }
        free(far);
    }

    free(r_raw); free(y_raw); free(z_raw); free(p_raw); free(q_arr); free(vt);
}
