#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>

/* count CPUs in the process affinity mask, from /proc/self/status */
static int allowed_cpu_count(void) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return 0;
    char line[256];
    int n = 0;
    while (fgets(line, sizeof line, f)) {
        if (line[0]=='C' && line[1]=='p' && line[2]=='u' && line[3]=='s' &&
            line[4]=='_' && line[5]=='a') {
            const char *p = line;
            while (*p && *p != ':') p++;
            if (*p == ':') {
                p++;
                while (*p==' ' || *p=='\t') p++;
                for (; *p; p++) {
                    int c = *p, v;
                    if (c>='0'&&c<='9') v=c-'0';
                    else if (c>='a'&&c<='f') v=c-'a'+10;
                    else if (c>='A'&&c<='F') v=c-'A'+10;
                    else if (c==',') continue;
                    else break;
                    for (int b=0;b<4;b++) if (v&(1<<b)) n++;
                }
            }
            break;
        }
    }
    fclose(f);
    return n;
}

void lanczos_reorth_csr_fp64(
    const double *restrict A_data,
    const int64_t *restrict A_indices,
    const int64_t *restrict A_indptr,
    double *restrict Q,
    double *restrict alpha,
    const double *restrict b,
    double *restrict beta,
    const int64_t NX,
    const int64_t NY,
    const int64_t NZ,
    const int64_t m,
    uint8_t *restrict workspace,
    const int64_t workspace_size)
{
    const int64_t N = NX * NY * NZ;
    (void)workspace; (void)workspace_size;
    if (N <= 0 || m <= 0) return;

    int nthr = omp_get_max_threads();
    {
        int aff = allowed_cpu_count();
        if (aff > 0 && aff < nthr) nthr = aff;
    }
    if (nthr < 1) nthr = 1;

    const int64_t S = NY * NZ;      /* x stride */
    const int64_t T = NZ;           /* y stride */
    const size_t halo = (size_t)S;
    const size_t HN = (size_t)N + 2 * halo;

    /* ---- buffers: one grow-only static pool (avoids per-call mmap + faults) ---- */
    const int K = 8;   /* ring depth / flush batch */
    const size_t need_d = (size_t)K * HN
                        + (size_t)N
                        + 7 * (size_t)N
                        + (size_t)nthr * 2 * (size_t)m
                        + (size_t)nthr;
    const size_t need_i = (A_indptr[N] > 0) ? (size_t)A_indptr[N] : 1;
    static double *pool_d = NULL; static size_t pool_dcap = 0;
    static int32_t *pool_i = NULL; static size_t pool_icap = 0;
    if (pool_dcap < need_d) {
        free(pool_d);
        pool_d = (double *)aligned_alloc(64, (need_d * sizeof(double) + 63) & ~(size_t)63);
        pool_dcap = pool_d ? need_d : 0;
    }
    if (pool_icap < need_i) {
        free(pool_i);
        pool_i = (int32_t *)aligned_alloc(64, (need_i * sizeof(int32_t) + 63) & ~(size_t)63);
        pool_icap = pool_i ? need_i : 0;
    }
    double *restrict ring = pool_d;
    double *restrict w    = pool_d + (size_t)K * HN;
    double *restrict sv   = pool_d + (size_t)K * HN + (size_t)N;
    double *restrict cbuf = pool_d + (size_t)K * HN + (size_t)N + 7 * (size_t)N;
    double *restrict ps   = pool_d + (size_t)K * HN + (size_t)N + 7 * (size_t)N
                           + (size_t)nthr * 2 * (size_t)m;
    int32_t *restrict i32 = pool_i;

    if (!pool_d || !pool_i) {
        return;
    }

    /* zero the ghost zones of every ring slot (pool may hold stale data) */
    #pragma omp parallel num_threads(nthr)
    {
        #pragma omp for schedule(static)
        for (int64_t t = 0; t < (int64_t)K * 2 * (int64_t)halo; t++) {
            int64_t which = t / (2 * (int64_t)halo);
            int64_t r = t % (2 * (int64_t)halo);
            size_t idx = (r < (int64_t)halo) ? (size_t)r
                       : (size_t)N + (size_t)halo + (size_t)(r - (int64_t)halo);
            ring[(size_t)which * HN + idx] = 0.0;
        }
    }

    /* ---- stencil detection: 7-point pattern with offsets {-S,-T,-1,0,1,T,S} ---- */
    int use_stencil = (NX > 1 && NY > 1 && NZ > 1);
    if (use_stencil) {
        int ok = 1;
        #pragma omp parallel for reduction(&:ok) num_threads(nthr) schedule(static)
        for (int64_t i = 0; i < N; i++) {
            const int64_t x = i / S, rem = i % S, y = rem / T, z = rem % T;
            const int64_t end = A_indptr[i + 1];
            int64_t e = A_indptr[i];
            if (x > 0) {
                if (e < end && A_indices[e] == i - S) sv[0*(size_t)N + i] = A_data[e++];
                else ok = 0;
            } else sv[0*(size_t)N + i] = 0.0;
            if (y > 0) {
                if (e < end && A_indices[e] == i - T) sv[1*(size_t)N + i] = A_data[e++];
                else ok = 0;
            } else sv[1*(size_t)N + i] = 0.0;
            if (z > 0) {
                if (e < end && A_indices[e] == i - 1) sv[2*(size_t)N + i] = A_data[e++];
                else ok = 0;
            } else sv[2*(size_t)N + i] = 0.0;
            if (e < end && A_indices[e] == i) sv[3*(size_t)N + i] = A_data[e++];
            else ok = 0;
            if (z < NZ - 1) {
                if (e < end && A_indices[e] == i + 1) sv[4*(size_t)N + i] = A_data[e++];
                else ok = 0;
            } else sv[4*(size_t)N + i] = 0.0;
            if (y < NY - 1) {
                if (e < end && A_indices[e] == i + T) sv[5*(size_t)N + i] = A_data[e++];
                else ok = 0;
            } else sv[5*(size_t)N + i] = 0.0;
            if (x < NX - 1) {
                if (e < end && A_indices[e] == i + S) sv[6*(size_t)N + i] = A_data[e++];
                else ok = 0;
            } else sv[6*(size_t)N + i] = 0.0;
            if (e != end) ok = 0;
        }
        use_stencil = ok;
    }
    if (!use_stencil && N < ((int64_t)1 << 30)) {
        #pragma omp parallel for num_threads(nthr) schedule(static)
        for (int64_t k = 0; k < A_indptr[N]; k++) i32[k] = (int32_t)A_indices[k];
    }


    /* ---- initial basis vector ---- */
    double nrm = 0.0;
    #pragma omp parallel for reduction(+:nrm) num_threads(nthr) schedule(static)
    for (int64_t i = 0; i < N; i++) nrm += b[i] * b[i];
    nrm = sqrt(nrm);
    #pragma omp parallel for num_threads(nthr) schedule(static)
    for (int64_t i = 0; i < N; i++) {
        double v = b[i] / nrm;
        Q[(size_t)i * (size_t)m] = v;
        ring[halo + (size_t)i] = v;
    }

    const double *restrict sv0 = sv ? sv                 : NULL;
    const double *restrict sv1 = sv ? sv + (size_t)N     : NULL;
    const double *restrict sv2 = sv ? sv + 2*(size_t)N   : NULL;
    const double *restrict sv3 = sv ? sv + 3*(size_t)N   : NULL;
    const double *restrict sv4 = sv ? sv + 4*(size_t)N   : NULL;
    const double *restrict sv5 = sv ? sv + 5*(size_t)N   : NULL;
    const double *restrict sv6 = sv ? sv + 6*(size_t)N   : NULL;
    const int32_t *restrict ai32 = i32;

    #pragma omp parallel num_threads(nthr)
    {
        const int tid = omp_get_thread_num();
        double *restrict cloc = cbuf + (size_t)tid * (size_t)m;
        double *restrict myc1 = cbuf + (size_t)(nthr + tid) * (size_t)m;
        double *restrict myps = ps + tid;
        const int64_t nb8 = (N + 7) / 8;   /* 8-row blocks for the c-passes */
        const int64_t TT = 16;             /* Q column tile width for the 2-pass reorth */
        double beta_prev = 0.0;
        int64_t nflushed = 0;              /* columns already stored into Q */
        const double *restrict gtab[8];

        for (int64_t j = 0; j < m; j++) {
            const int64_t k = j + 1;

            /* ---- flush ring columns into Q in batches of K ----
               (before pass C of this step overwrites the oldest slot) */
            if (j >= 1 && ((j + 1) % K == 0 || j == m - 1)) {
                const int64_t lo = nflushed;
                const int64_t hi = j;
                if (lo <= hi) {
                    #pragma omp for schedule(static)
                    for (int64_t i = 0; i < N; i++) {
                        double *restrict qrow = Q + (size_t)i * (size_t)m;
                        __builtin_prefetch(Q + (size_t)(i + 8) * (size_t)m + lo, 1, 0);
                        __builtin_prefetch(Q + (size_t)(i + 24) * (size_t)m + lo, 1, 0);
                        for (int64_t t = 0; t <= hi - lo; t++)
                            qrow[lo + t] = ring[(size_t)((lo + t) % K) * HN + halo + (size_t)i];
                    }
                    nflushed = hi + 1;
                }
            }

            const double *restrict qch = ring + (size_t)(j % K) * HN;
            const double *restrict qp  = ring + (size_t)((j + K - 1) % K) * HN + halo;

            /* ---- matvec w = A q_j ; w -= beta_prev q_prev ; a-partials ---- */
            const double bp = beta_prev;
            double apar = 0.0;
            if (sv) {
                #pragma omp for schedule(static) nowait
                for (int64_t i = 0; i < N; i++) {
                    const size_t h = halo + (size_t)i;
                    const double a0 = sv0[i] * qch[h - S]
                                    + sv1[i] * qch[h - T]
                                    + sv2[i] * qch[h - 1];
                    const double a1 = sv3[i] * qch[h]
                                    + sv4[i] * qch[h + 1];
                    const double a2 = sv5[i] * qch[h + T]
                                    + sv6[i] * qch[h + S];
                    double acc = (a0 + a1) + a2;
                    if (j > 0) acc -= bp * qp[i];
                    w[i] = acc;
                    apar += qch[h] * acc;
                }
            } else if (ai32) {
                #pragma omp for schedule(static) nowait
                for (int64_t i = 0; i < N; i++) {
                    const int64_t s = A_indptr[i], e = A_indptr[i + 1];
                    double acc = 0.0;
                    for (int64_t kk = s; kk < e; kk++)
                        acc += A_data[kk] * qch[halo + (size_t)ai32[kk]];
                    if (j > 0) acc -= bp * qp[i];
                    w[i] = acc;
                    apar += qch[halo + (size_t)i] * acc;
                }
            } else {
                #pragma omp for schedule(static) nowait
                for (int64_t i = 0; i < N; i++) {
                    const int64_t s = A_indptr[i], e = A_indptr[i + 1];
                    double acc = 0.0;
                    for (int64_t kk = s; kk < e; kk++)
                        acc += A_data[kk] * qch[halo + (size_t)A_indices[kk]];
                    if (j > 0) acc -= bp * qp[i];
                    w[i] = acc;
                    apar += qch[halo + (size_t)i] * acc;
                }
            }
            *myps = apar;
            #pragma omp barrier
            double a = 0.0;
            for (int t = 0; t < nthr; t++) a += ps[t];
            if (tid == 0) alpha[j] = a;

            /* ---- sweep 1: w -= a q_j (plain pass over rows) ---- */
            #pragma omp for schedule(static) nowait
            for (int64_t i = 0; i < N; i++) w[i] -= a * qch[halo + (size_t)i];
            #pragma omp barrier

            /* ---- tiled two-pass reorthogonalization over the Q columns ---- */
            for (int64_t p0 = 0; p0 < nflushed; p0 += TT) {
                const int64_t p1 = (p0 + TT <= nflushed) ? p0 + TT : nflushed;
                const int64_t wpc = p1 - p0;
                for (int64_t p = p0; p < p1; p++) cloc[p] = 0.0;
                #pragma omp for schedule(static) nowait
                for (int64_t bb = 0; bb < nb8; bb++) {
                    const int64_t i0 = bb * 8;
                    const int64_t i1 = (i0 + 8 <= N) ? i0 + 8 : N;
                    const double w0 = (i0 + 0 < i1) ? w[i0] : 0.0;
                    const double w1 = (i0 + 1 < i1) ? w[i0 + 1] : 0.0;
                    const double w2 = (i0 + 2 < i1) ? w[i0 + 2] : 0.0;
                    const double w3 = (i0 + 3 < i1) ? w[i0 + 3] : 0.0;
                    const double w4 = (i0 + 4 < i1) ? w[i0 + 4] : 0.0;
                    const double w5 = (i0 + 5 < i1) ? w[i0 + 5] : 0.0;
                    const double w6 = (i0 + 6 < i1) ? w[i0 + 6] : 0.0;
                    const double w7 = (i0 + 7 < i1) ? w[i0 + 7] : 0.0;
                    const double *restrict q0 = Q + (size_t)i0 * (size_t)m + p0;
                    const double *restrict q1 = q0 + m, *restrict q2 = q0 + 2 * m;
                    const double *restrict q3 = q0 + 3 * m, *restrict q4 = q0 + 4 * m;
                    const double *restrict q5 = q0 + 5 * m, *restrict q6 = q0 + 6 * m;
                    const double *restrict q7 = q0 + 7 * m;
                    for (int64_t u = 0; u < wpc; u++) {
                        const double sa = q0[u] * w0 + q1[u] * w1 + q2[u] * w2 + q3[u] * w3;
                        const double sb = q4[u] * w4 + q5[u] * w5 + q6[u] * w6 + q7[u] * w7;
                        cloc[p0 + u] += sa + sb;
                    }
                }
                #pragma omp barrier
                for (int64_t p = p0; p < p1; p++) {
                    double s = 0.0;
                    for (int t = 0; t < nthr; t++) s += cbuf[(size_t)t * (size_t)m + p];
                    myc1[p] = s;
                }
                #pragma omp for schedule(static) nowait
                for (int64_t i = 0; i < N; i++) {
                    const double *restrict qr = Q + (size_t)i * (size_t)m + p0;
                    const double *restrict c1 = myc1 + p0;
                    double s0 = 0.0, s1b = 0.0, s2b = 0.0, s3b = 0.0;
                    for (int64_t u = 0; u + 3 < wpc; u += 4) {
                        s0 += qr[u]     * c1[u];
                        s1b += qr[u + 1] * c1[u + 1];
                        s2b += qr[u + 2] * c1[u + 2];
                        s3b += qr[u + 3] * c1[u + 3];
                    }
                    for (int64_t u = wpc & ~(int64_t)3; u < wpc; u++) s0 += qr[u] * c1[u];
                    w[i] -= (s0 + s1b) + (s2b + s3b);
                }
                #pragma omp barrier
            }

            /* ---- ring tile: c-pass over the newest K-1 ring columns ---- */
            const int64_t nr = k - nflushed;
            for (int64_t t = 0; t < nr; t++)
                gtab[t] = ring + (size_t)((nflushed + t) % K) * HN + halo;
            for (int64_t t = 0; t < nr; t++) cloc[nflushed + t] = 0.0;
            if (nr > 0) {
                #pragma omp for schedule(static) nowait
                for (int64_t bb = 0; bb < nb8; bb++) {
                    const int64_t i0 = bb * 8;
                    const int64_t i1 = (i0 + 8 <= N) ? i0 + 8 : N;
                    const double w0 = (i0 + 0 < i1) ? w[i0] : 0.0;
                    const double w1 = (i0 + 1 < i1) ? w[i0 + 1] : 0.0;
                    const double w2 = (i0 + 2 < i1) ? w[i0 + 2] : 0.0;
                    const double w3 = (i0 + 3 < i1) ? w[i0 + 3] : 0.0;
                    const double w4 = (i0 + 4 < i1) ? w[i0 + 4] : 0.0;
                    const double w5 = (i0 + 5 < i1) ? w[i0 + 5] : 0.0;
                    const double w6 = (i0 + 6 < i1) ? w[i0 + 6] : 0.0;
                    const double w7 = (i0 + 7 < i1) ? w[i0 + 7] : 0.0;
                    for (int64_t t = 0; t < nr; t++) {
                        const double *restrict g = gtab[t] + (size_t)i0;
                        const double sa = g[0] * w0 + g[1] * w1 + g[2] * w2 + g[3] * w3;
                        const double sb = g[4] * w4 + g[5] * w5 + g[6] * w6 + g[7] * w7;
                        cloc[nflushed + t] += sa + sb;
                    }
                }
                #pragma omp barrier
                for (int64_t t = 0; t < nr; t++) {
                    double s = 0.0;
                    for (int th = 0; th < nthr; th++)
                        s += cbuf[(size_t)th * (size_t)m + nflushed + t];
                    myc1[nflushed + t] = s;
                }
            }

            /* ---- final update: subtract the ring columns, take the norm ---- */
            double npar = 0.0;
            #pragma omp for schedule(static) nowait
            for (int64_t i = 0; i < N; i++) {
                const double *restrict gr = ring + halo + (size_t)i;
                double s0 = 0.0, s1b = 0.0, s2b = 0.0, s3b = 0.0;
                for (int64_t t = 0; t + 3 < nr; t += 4) {
                    s0  += gr[(size_t)((nflushed + t) % K) * HN] * myc1[nflushed + t];
                    s1b += gr[(size_t)((nflushed + t + 1) % K) * HN] * myc1[nflushed + t + 1];
                    s2b += gr[(size_t)((nflushed + t + 2) % K) * HN] * myc1[nflushed + t + 2];
                    s3b += gr[(size_t)((nflushed + t + 3) % K) * HN] * myc1[nflushed + t + 3];
                }
                for (int64_t t = nr & ~(int64_t)3; t < nr; t++)
                    s0 += gr[(size_t)((nflushed + t) % K) * HN] * myc1[nflushed + t];
                double v = w[i] - ((s0 + s1b) + (s2b + s3b));
                w[i] = v;
                npar += v * v;
            }
            *myps = npar;
            #pragma omp barrier
            double b_j = 0.0;
            for (int t = 0; t < nthr; t++) b_j += ps[t];
            b_j = sqrt(b_j);
            if (tid == 0) beta[j] = b_j;
            beta_prev = b_j;

            /* ---- pass C: ring[j+1] = w / b_j ---- */
            if (j + 1 < m) {
                const double ib = 1.0 / b_j;
                double *restrict dst = ring + (size_t)((j + 1) % K) * HN + halo;
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N; i++) {
                    __builtin_prefetch(&dst[i + 32], 1, 0);
                    dst[i] = w[i] * ib;
                }
            }
        }
    }

}
