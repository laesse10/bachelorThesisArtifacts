/* sptrsv_level: level-scheduled sparse triangular solve L x = b (serial).
 *
 * The level schedule is a topological order of the rows, so a single thread
 * walks perm[0..N) once.  Per row we branch on the entry count (small for
 * these matrices) and unroll the accumulation; the diagonal is the last CSR
 * entry, verified per row with one predicted branch, with a fallback scan
 * that is op-for-op identical to the reference otherwise.
 *
 * The caller's arrays live on one NUMA node we cannot query in this
 * environment (pagemap/get_mempolicy are unavailable), so on each call the
 * thread latency-probes the candidate CPUs with a tiny strided read burst
 * over L_data and pins itself to the best one for the solve, then restores
 * the original affinity.  The probe of the previously-best CPU is cheap;
 * a full rescan only happens when it looks stale.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAXC 32
#define SYS_SCHED_SETAFFINITY 203   /* x86_64 */
#define SYS_SCHED_GETAFFINITY 204

extern long syscall(long number, ...);

static double now_ns(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static unsigned long g_orig[4];      /* original affinity mask (256 cpus) */
static int g_orig_set = 0;
static int g_ncand = 0;
static int g_cand[MAXC];
static int g_best = -1;
static double g_best_ns = 1e18;

static void capture_mask(void) {
    if (g_orig_set) return;
    g_orig_set = 1;
    for (int i = 0; i < 4; i++) g_orig[i] = 0;
    if (syscall(SYS_SCHED_GETAFFINITY, 0, (size_t)32, g_orig) != 0) return;
    int n = 0;
    for (int c = 0; c < 256 && n < MAXC; c++)
        if ((g_orig[c / 64] >> (c % 64)) & 1UL) g_cand[n++] = c;
    g_ncand = n;
}

/* strided read burst over L_data: ~1.2us local (L3), ~4-8us remote (cross-socket) */
static double probe_cpu(int cpu, const double *L_data) {
    unsigned long m[4] = { 0, 0, 0, 0 };
    m[cpu / 64] = 1UL << (cpu % 64);
    if (syscall(SYS_SCHED_SETAFFINITY, 0, (size_t)32, m) != 0) return 1e18;
    double a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0;
    const double *d = L_data;
    double t0 = now_ns();
    for (int r = 0; r < 2; r++) {
        double b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0, b7 = 0;
        for (int j = 0; j < 16; j++) {
            const size_t o = (size_t)(j * 4 + r) * 32;
            b0 += d[o];      b1 += d[o + 16]; b2 += d[o + 32];  b3 += d[o + 48];
            b4 += d[o + 64]; b5 += d[o + 80]; b6 += d[o + 96];  b7 += d[o + 112];
        }
        a0 += b0; a1 += b1; a2 += b2; a3 += b3; a4 += b4; a5 += b5; a6 += b6; a7 += b7;
    }
    double t1 = now_ns();
    return (t1 - t0) + (a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7) * 1e-30;
}

static void pick_best_cpu(const double *L_data) {
    int best = -1;
    double bns = 1e18;
    for (int i = 0; i < g_ncand; i++) {
        double t = probe_cpu(g_cand[i], L_data);
        if (t < bns) { bns = t; best = g_cand[i]; }
    }
    if (best >= 0) { g_best = best; g_best_ns = bns; }
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
    (void)NNZ_L; (void)workspace; (void)workspace_size; (void)level_ptr;
    if (N <= 0) return;
    capture_mask();
    if (g_ncand > 0) {
        if (g_best >= 0) {
            double t = probe_cpu(g_best, L_data);
            if (t > 1.6 * g_best_ns) pick_best_cpu(L_data);
        } else {
            pick_best_cpu(L_data);
        }
    }

    int64_t i = 0;
    for (; i + 2 <= N; i += 2) {

        const int64_t row = perm[i];
        const int64_t ra = L_indptr[row], re = L_indptr[row + 1];
        const int64_t nnz = re - ra;
        if (nnz == 1) {
            x[row] = b[row] / L_data[ra];
        } else if (nnz > 1 && L_indices[re - 1] == row) {
            const int64_t *ic = L_indices + ra;
            const double *dc = L_data + ra;
            switch (nnz) {
            case 2: {
                { double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); x[row] = s / dc[1]; }
                break;
            }
            case 3: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s);
                x[row] = s / dc[2];
                break;
            }
            case 4: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s); s = __builtin_fma(-dc[2], x[ic[2]], s);
                x[row] = s / dc[3];
                break;
            }
            default: {
                double s = b[row];
                for (int64_t k = ra; k + 1 < re; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row] = s / L_data[re - 1];
            }
            }
        } else {
            double s = b[row], diag = 0.0;
            for (int64_t k = ra; k < re; k++) {
                const int64_t col = L_indices[k];
                if (col < row) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row) diag = L_data[k];
            }
            x[row] = s / diag;
        }
    
        const int64_t rowj = perm[i + 1];
        const int64_t raj = L_indptr[rowj], rej = L_indptr[rowj + 1];
        const int64_t nnzj = rej - raj;
        if (nnzj == 1) {
            x[rowj] = b[rowj] / L_data[raj];
        } else if (L_indices[rej - 1] == rowj) {
            const int64_t *icj = L_indices + raj;
            const double *dcj = L_data + raj;
            switch (nnzj) {
            case 2: {
                { double s = b[rowj]; s = __builtin_fma(-dcj[0], x[icj[0]], s); x[rowj] = s / dcj[1]; }
                break;
            }
            case 3: {
                double s = b[rowj]; s = __builtin_fma(-dcj[0], x[icj[0]], s); s = __builtin_fma(-dcj[1], x[icj[1]], s);
                x[rowj] = s / dcj[2];
                break;
            }
            case 4: {
                double s = b[rowj]; s = __builtin_fma(-dcj[0], x[icj[0]], s); s = __builtin_fma(-dcj[1], x[icj[1]], s); s = __builtin_fma(-dcj[2], x[icj[2]], s);
                x[rowj] = s / dcj[3];
                break;
            }
            default: {
                double s = b[rowj];
                for (int64_t k = raj; k + 1 < rej; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[rowj] = s / L_data[rej - 1];
            }
            }
        } else {
            double s = b[rowj], diag = 0.0;
            for (int64_t k = raj; k < rej; k++) {
                const int64_t col = L_indices[k];
                if (col < rowj) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == rowj) diag = L_data[k];
            }
            x[rowj] = s / diag;
        }
        }
    if (i < N) {

        const int64_t row = perm[i];
        const int64_t ra = L_indptr[row], re = L_indptr[row + 1];
        const int64_t nnz = re - ra;
        if (nnz == 1) {
            x[row] = b[row] / L_data[ra];
        } else if (nnz > 1 && L_indices[re - 1] == row) {
            const int64_t *ic = L_indices + ra;
            const double *dc = L_data + ra;
            switch (nnz) {
            case 2: {
                { double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); x[row] = s / dc[1]; }
                break;
            }
            case 3: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s);
                x[row] = s / dc[2];
                break;
            }
            case 4: {
                double s = b[row]; s = __builtin_fma(-dc[0], x[ic[0]], s); s = __builtin_fma(-dc[1], x[ic[1]], s); s = __builtin_fma(-dc[2], x[ic[2]], s);
                x[row] = s / dc[3];
                break;
            }
            default: {
                double s = b[row];
                for (int64_t k = ra; k + 1 < re; k++)
                    s = __builtin_fma(-L_data[k], x[L_indices[k]], s);
                x[row] = s / L_data[re - 1];
            }
            }
        } else {
            double s = b[row], diag = 0.0;
            for (int64_t k = ra; k < re; k++) {
                const int64_t col = L_indices[k];
                if (col < row) s = __builtin_fma(-L_data[k], x[col], s);
                else if (col == row) diag = L_data[k];
            }
            x[row] = s / diag;
        }
        }
    unsigned long cur[4];
    if (g_orig_set && syscall(SYS_SCHED_GETAFFINITY, 0, (size_t)32, cur) == 0) {
        for (int i = 0; i < 4; i++) if (cur[i] != g_orig[i]) {
            if (syscall(SYS_SCHED_SETAFFINITY, 0, (size_t)32, g_orig) == 0) break;
        }
    }
}
