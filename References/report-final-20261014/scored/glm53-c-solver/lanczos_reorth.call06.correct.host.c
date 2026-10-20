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

    /* ---- stencil detection: 7-point pattern with offsets {-S,-T,-1,0,1,T,S} ---- */
    int use_stencil = (NX > 1 && NY > 1 && NZ > 1);
    double *restrict sv = NULL;
    int32_t *restrict i32 = NULL;
    if (use_stencil) {
        sv = (double *)aligned_alloc(64, sizeof(double) * 7 * (size_t)N);
        if (!sv) use_stencil = 0;
    }
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
    if (!use_stencil) {
        free(sv); sv = NULL;
        if (N < ((int64_t)1 << 30)) {
            i32 = (int32_t *)aligned_alloc(64, sizeof(int32_t) * (size_t)A_indptr[N]);
            if (i32) {
                #pragma omp parallel for num_threads(nthr) schedule(static)
                for (int64_t k = 0; k < A_indptr[N]; k++) i32[k] = (int32_t)A_indices[k];
            }
        }
    }

    /* ---- buffers ---- */
    double *restrict qb[3];   /* haloed rotating columns */
    for (int t = 0; t < 3; t++)
        qb[t] = (double *)aligned_alloc(64, sizeof(double) * HN);
    double *restrict w = (double *)aligned_alloc(64, sizeof(double) * (size_t)N);
    double *restrict cbuf = (double *)aligned_alloc(64, sizeof(double) * (size_t)nthr * 2 * (size_t)m);
    double *restrict ps   = (double *)aligned_alloc(64, sizeof(double) * (size_t)nthr);

    if (!qb[0] || !qb[1] || !qb[2] || !w || !cbuf || !ps) {
        for (int t = 0; t < 3; t++) free(qb[t]);
        free(w); free(cbuf); free(ps); free(sv); free(i32);
        return;
    }

    /* first touch: distribute pages, zero ghosts */
    #pragma omp parallel num_threads(nthr)
    {
        #pragma omp for schedule(static)
        for (int64_t t = 0; t < 3 * (int64_t)HN; t++) {
            int64_t which = t / (int64_t)HN;
            size_t idx = (size_t)(t % HN);
            qb[which][idx] = 0.0;
        }
        #pragma omp for schedule(static)
        for (int64_t i = 0; i < N; i++) w[i] = 0.0;
        #pragma omp for schedule(static)
        for (int64_t t = 0; t < (int64_t)nthr * 2 * m; t++) cbuf[t] = 0.0;
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
        qb[0][halo + (size_t)i] = v;
    }

    const double *restrict sv0 = sv ? sv               : NULL;
    const double *restrict sv1 = sv ? sv + (size_t)N   : NULL;
    const double *restrict sv2 = sv ? sv + 2*(size_t)N : NULL;
    const double *restrict sv3 = sv ? sv + 3*(size_t)N : NULL;
    const double *restrict sv4 = sv ? sv + 4*(size_t)N : NULL;
    const double *restrict sv5 = sv ? sv + 5*(size_t)N : NULL;
    const double *restrict sv6 = sv ? sv + 6*(size_t)N : NULL;
    const int32_t *restrict ai32 = i32;

    #pragma omp parallel num_threads(nthr)
    {
        const int tid = omp_get_thread_num();
        double *restrict cloc  = cbuf + (size_t)tid * (size_t)m;
        double *restrict myc1  = cbuf + (size_t)(nthr + tid) * (size_t)m;
        double *restrict myps  = ps + tid;
        const int64_t nb4 = (N + 3) / 4;   /* 4-row blocks for sweep 1 */
        double beta_prev = 0.0;

        for (int64_t j = 0; j < m; j++) {
            const int64_t k = j + 1;
            const int ci = (int)(j % 3);
            const double *restrict qch = qb[ci];           /* haloed current column */
            const double *restrict qp  = qb[(ci + 2) % 3] + halo; /* previous column */
            double *restrict qn = qb[(ci + 1) % 3] + halo;

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

            /* ---- sweep 1: w -= a q_j ; c1 = Q(:,0:k)^T w   (4-row blocks) ---- */
            for (int64_t p = 0; p < k; p++) cloc[p] = 0.0;
            #pragma omp for schedule(static)
            for (int64_t bb = 0; bb < nb4; bb++) {
                const int64_t i0 = bb * 4;
                const int64_t i1 = (i0 + 4 <= N) ? i0 + 4 : N;
                const double w0 = (i0 < i1) ? (w[i0] - a * qch[halo + (size_t)i0]) : 0.0;
                const double w1 = (i0 + 1 < i1) ? (w[i0+1] - a * qch[halo + (size_t)i0 + 1]) : 0.0;
                const double w2 = (i0 + 2 < i1) ? (w[i0+2] - a * qch[halo + (size_t)i0 + 2]) : 0.0;
                const double w3 = (i0 + 3 < i1) ? (w[i0+3] - a * qch[halo + (size_t)i0 + 3]) : 0.0;
                if (i0 < i1) w[i0] = w0;
                if (i0 + 1 < i1) w[i0+1] = w1;
                if (i0 + 2 < i1) w[i0+2] = w2;
                if (i0 + 3 < i1) w[i0+3] = w3;
                const double *restrict q0 = Q + (size_t)i0 * (size_t)m;
                const double *restrict q1 = q0 + (size_t)m;
                const double *restrict q2 = q0 + 2*(size_t)m;
                const double *restrict q3 = q0 + 3*(size_t)m;
                for (int64_t p = 0; p < k; p++)
                    cloc[p] += q0[p]*w0 + q1[p]*w1 + q2[p]*w2 + q3[p]*w3;
            }
            /* redundant private reduction of c1 (barrier of the for already passed) */
            for (int64_t p = 0; p < k; p++) {
                double s = 0.0;
                for (int t = 0; t < nthr; t++) s += cbuf[(size_t)t * (size_t)m + p];
                myc1[p] = s;
            }

            /* ---- sweep 2: w -= Q(: 0:k) c1 ; norm partials ---- */
            double npar = 0.0;
            const int64_t k4 = k & ~(int64_t)3;
            #pragma omp for schedule(static) nowait
            for (int64_t i = 0; i < N; i++) {
                const double *restrict qr = Q + (size_t)i * (size_t)m;
                double s0 = 0.0, s1 = 0.0, s2 = 0.0, s3 = 0.0;
                for (int64_t p = 0; p < k4; p += 4) {
                    s0 += qr[p]   * myc1[p];
                    s1 += qr[p+1] * myc1[p+1];
                    s2 += qr[p+2] * myc1[p+2];
                    s3 += qr[p+3] * myc1[p+3];
                }
                for (int64_t p = k4; p < k; p++) s0 += qr[p] * myc1[p];
                double v = w[i] - ((s0 + s1) + (s2 + s3));
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

            /* ---- pass C: Q(:,j+1) = w / b_j ---- */
            if (j + 1 < m) {
                const double ib = 1.0 / b_j;
                #pragma omp for schedule(static)
                for (int64_t i = 0; i < N; i++) {
                    double v = w[i] * ib;
                    Q[(size_t)i * (size_t)m + (size_t)(j + 1)] = v;
                    qn[i] = v;
                }
            }
        }
    }

    for (int t = 0; t < 3; t++) free(qb[t]);
    free(w); free(cbuf); free(ps); free(sv); free(i32);
}
