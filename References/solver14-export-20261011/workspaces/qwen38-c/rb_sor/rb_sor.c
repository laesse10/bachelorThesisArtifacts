/* Red-black SOR on a 2-D grid, optimized for this environment.
 *
 *  - persistent pinned thread pool (avoids the very expensive per-call
 *    OpenMP region spawn on this machine, ~2 ms per region)
 *  - per-call CPU placement chosen from the NUMA node owning the inputs
 *  - cheap flag-based barriers (two per timestep)
 *  - hand-written AVX-512 inner loop (8 stride-2 points per block, dense
 *    512-bit window loads; FMA-free update so rounding matches the
 *    reference's separate mul/add sequence)
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <immintrin.h>

/* ---------------- raw syscalls (strict -std=c23 / _POSIX_C_SOURCE build) -- */
static inline long sys3(long n, long a1, long a2, long a3) {
    long r;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3)
                       : "rcx", "r8", "r9", "r10", "r11", "memory");
    return r;
}
static inline long sys4(long n, long a1, long a2, long a3, long a4) {
    long r; register long r10 __asm__("r10") = a4;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
                       : "rcx", "r8", "r9", "r11", "memory");
    return r;
}
static inline long sys5(long n, long a1, long a2, long a3, long a4, long a5) {
    long r;
    register long r8 __asm__("r8") = a5;
    register long r10 __asm__("r10") = a4;
    __asm__ volatile ("syscall" : "=a"(r) : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
                       : "rcx", "r9", "r11", "memory");
    return r;
}
#define SY_FUTEX 202L
#define FUTEX_WAIT_PRIVATE 128L
#define FUTEX_WAKE_PRIVATE 129L
#define SY_SCHED_AFFINITY 204L
#define SY_SCHED_SETAFFINITY 203L
#define SY_GET_MEMPOLICY 239L

static inline void futex_wait(long *a, long val) { (void)sys3(SY_FUTEX, (long)a, FUTEX_WAIT_PRIVATE, val); }
static inline long futex_wake(long *a, long cnt) { return sys3(SY_FUTEX, (long)a, FUTEX_WAKE_PRIVATE, cnt); }

struct cpumask { unsigned long bits[4]; }; /* 256 CPUs */
static void get_my_cpus(struct cpumask *m) {
    memset(m, 0, sizeof(*m));
    (void)sys3(SY_SCHED_AFFINITY, 0, (long)sizeof(*m), (long)m);
}
static int set_my_cpu(int cpu) {
    struct cpumask m;
    memset(&m, 0, sizeof(m));
    m.bits[cpu >> 6] = 1UL << (cpu & 63);
    return (int)sys3(SY_SCHED_SETAFFINITY, 0, (long)sizeof(m), (long)&m);
}
static void set_my_mask(const struct cpumask *m) {
    (void)sys3(SY_SCHED_SETAFFINITY, 0, (long)sizeof(*m), (long)m);
}
static long addr_node(unsigned long addr) {
    unsigned long node = 12345;
    long r = sys5(SY_GET_MEMPOLICY, (long)&node, 0, 0, (long)addr, 0x3L);
    if (r) return -1;
    return (long)node;
}
static long mono_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000000L + (long)ts.tv_nsec;
}

/* ---------------- topology ---------------- */
#define MAXNODE 16
#define MAXCPU_PER_NODE 160
static int g_node_count = 0;
static int g_node_cpus[MAXNODE][MAXCPU_PER_NODE];
static int g_node_ncpu[MAXNODE];
static unsigned long g_allowed[4];

static void parse_cpulist_file(const char *path, int node) {
    FILE *fp = fopen(path, "r");
    if (!fp) return;
    char buf[600];
    size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[n] = 0;
    char *save = NULL;
    for (char *tok = strtok_r(buf, ",", &save); tok; tok = strtok_r(NULL, ",", &save)) {
        char *dash = strchr(tok, '-');
        int a = atoi(tok);
        int b = dash ? atoi(dash + 1) : a;
        for (int c = a; c <= b && c >= 0 && c < 256 && g_node_ncpu[node] < MAXCPU_PER_NODE; c++)
            g_node_cpus[node][g_node_ncpu[node]++] = c;
    }
}
static void topo_init(void) {
    struct cpumask m;
    get_my_cpus(&m);
    for (int i = 0; i < 4; i++) g_allowed[i] = m.bits[i];
    g_node_count = 0;
    char path[128];
    for (int n = 0; n < MAXNODE; n++) {
        g_node_ncpu[n] = 0;
        snprintf(path, sizeof(path), "/sys/devices/system/node/node%d/cpulist", n);
        parse_cpulist_file(path, n);
        if (g_node_ncpu[n]) g_node_count = n + 1;
    }
}
static int cpu_allowed(int c) {
    return c >= 0 && c < 256 && ((g_allowed[c >> 6] >> (c & 63)) & 1UL);
}
static int node_allowed_count(int node) {
    int k = 0;
    if (node < 0 || node >= g_node_count) return 0;
    for (int i = 0; i < g_node_ncpu[node]; i++)
        if (cpu_allowed(g_node_cpus[node][i])) k++;
    return k;
}
/* allowed CPUs of a node; primaries first, each HT sibling right after its primary */
static int node_allowed_cpus(int node, int *out) {
    int k = 0, tmp[MAXCPU_PER_NODE], ntmp = 0;
    if (node < 0 || node >= g_node_count) return 0;
    for (int i = 0; i < g_node_ncpu[node]; i++)
        if (cpu_allowed(g_node_cpus[node][i])) tmp[ntmp++] = g_node_cpus[node][i];
    for (int i = 0; i < ntmp; i++) if (tmp[i] < 96) out[k++] = tmp[i];
    for (int i = 0; i < ntmp; i++) if (tmp[i] >= 96) {
        int prim = tmp[i] - 96, has = 0;
        for (int j = 0; j < k; j++) if (out[j] == prim) has = 1;
        if (has) out[k++] = tmp[i];
    }
    for (int i = 0; i < ntmp; i++) if (tmp[i] >= 96) {
        int prim = tmp[i] - 96, has = 0;
        for (int j = 0; j < k; j++) if (out[j] == prim || out[j] == tmp[i]) has = 1;
        if (!has) out[k++] = tmp[i];
    }
    return k;
}

/* ---------------- inner loop ---------------- */
static void sweep_row(const double *restrict f, double *restrict u, int64_t N,
                      int64_t i, int parity, double h2, double c1, double om) {
    double        *row = u + i * N;
    const double  *up  = row - N;
    const double  *dn  = row + N;
    const double  *fr  = f + i * N;
    int64_t j0 = (((parity - i) & 1) != 0) ? 1 : 2;
    int64_t je = N - 2;
    int64_t j = j0;

    const __m512d vq  = _mm512_set1_pd(0.25);
    const __m512d vh2 = _mm512_set1_pd(h2);
    const __m512d vc1 = _mm512_set1_pd(c1);
    const __m512d vom = _mm512_set1_pd(om);
    const __m512i idx_lr = _mm512_castsi256_si512(_mm256_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14));
    const __m512i idx_r  = _mm512_castsi256_si512(_mm256_setr_epi32(2, 4, 6, 8, 10, 12, 14, 15));
    const __m512i idx_s  = _mm512_castsi256_si512(_mm256_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15));

    for (; j + 14 <= je; j += 16) {
        /* W[k] = row[j-1+k]; W0..W7 in w0, W8..W15 in w1, W16 scalar */
        __m512d w0 = _mm512_loadu_pd(row + j - 1);
        __m512d w1 = _mm512_loadu_pd(row + j + 7);
        double  s16 = row[j + 15];

        __m512d L    = _mm512_permutex2var_pd(w0, idx_lr, w1);
        __m512d R    = _mm512_permutex2var_pd(w0, idx_r, w1);
        R            = _mm512_mask_blend_pd(0x80, R, _mm512_set1_pd(s16));
        __m512d self = _mm512_permutex2var_pd(w0, idx_s, w1);

        __m512d uA = _mm512_loadu_pd(up + j);
        __m512d uB = _mm512_loadu_pd(up + j + 8);
        __m512d u8 = _mm512_permutex2var_pd(uA, idx_lr, uB);
        __m512d dA = _mm512_loadu_pd(dn + j);
        __m512d dB = _mm512_loadu_pd(dn + j + 8);
        __m512d d8 = _mm512_permutex2var_pd(dA, idx_lr, dB);
        __m512d fA = _mm512_loadu_pd(fr + j);
        __m512d fB = _mm512_loadu_pd(fr + j + 8);
        __m512d f8 = _mm512_permutex2var_pd(fA, idx_lr, fB);

        __m512d S = _mm512_add_pd(u8, d8);
        S = _mm512_add_pd(S, L);
        S = _mm512_add_pd(S, R);
        S = _mm512_add_pd(S, _mm512_mul_pd(f8, vh2));
        __m512d g = _mm512_mul_pd(S, vq);
        __m512d o = _mm512_add_pd(_mm512_mul_pd(vc1, self), _mm512_mul_pd(vom, g));

        _mm256_storeu_pd(row + j + 0, _mm512_extractf64x4_pd(o, 0));
        _mm256_storeu_pd(row + j + 8, _mm512_extractf64x4_pd(o, 1));
    }
    for (; j <= je; j += 2) {
        double g = (up[j] + dn[j] + row[j - 1] + row[j + 1] + h2 * fr[j]) * 0.25;
        row[j] = c1 * row[j] + om * g;
    }
}

/* ---------------- pool ---------------- */
typedef struct { const double *f; double *u; int64_t N, TSTEPS; double h2, c1, om; } job_t;
typedef struct { int64_t lo, hi; } rng_t;

#define MAXW 13
static pthread_t g_thr[MAXW - 1];
static int g_nworkers = 0;
static pthread_mutex_t g_init_mu = PTHREAD_MUTEX_INITIALIZER;
static volatile int g_pool_ready = 0;

static job_t g_job;
static rng_t g_ranges[MAXW];
static int g_K = 1;
static long g_base = 0;
static _Atomic(long) g_seq = 0;

typedef struct { _Atomic(long) v[2]; char pad[48]; } bar_flag_t;
static _Alignas(64) bar_flag_t g_flags[MAXW];

static _Atomic(int) g_busy = 0;

static int g_spin_us = 5000;
static int g_force_k = 0;
static int g_use_datanode = 1;

static int g_place[MAXW];

static _Atomic(int) g_pin[MAXW];   /* cpu worker w should pin itself to; -1: keep current */
static int g_pin_applied[MAXW];

static int choose_k(int64_t N) {
    int k;
    if (g_force_k) k = g_force_k;
    else if (N <= 256) k = 3;
    else if (N <= 768) k = 5;
    else if (N <= 1536) k = 7;
    else k = 9;
    if (k < 1) k = 1;
    if (k > g_nworkers + 1) k = g_nworkers + 1;
    return k;
}

static void run_job(int w) {
    long b = g_base;
    int K = g_K;
    int64_t lo = g_ranges[w].lo, hi = g_ranges[w].hi;
    const double *f = g_job.f;
    double *u = g_job.u;
    int64_t N = g_job.N, T = g_job.TSTEPS;
    double h2 = g_job.h2, c1 = g_job.c1, om = g_job.om;

    for (int64_t s = 0; s < 2 * T; s++) {
        int p = (int)(s & 1);
        for (int64_t i = lo; i < hi; i++)
            sweep_row(f, u, N, i, p, h2, c1, om);
        int buf = (int)(s & 1);
        long v = b + s;
        atomic_store_explicit(&g_flags[w].v[buf], v, memory_order_release);
        for (int i = 0; i < K; i++) {
            if (i == w) continue;
            long x;
            while ((x = atomic_load_explicit(&g_flags[i].v[buf], memory_order_acquire)) != v)
                __builtin_ia32_pause();
        }
    }
}

static void *pool_worker(void *arg) {
    int w = (int)(long)arg;
    long my = 0;
    for (;;) {
        long s;
        for (;;) {
            s = atomic_load_explicit(&g_seq, memory_order_acquire);
            if (s != my) { my = s; break; }
            long t0 = mono_ns();
            int it = 0;
            while (it < (1 << 24)) {
                __builtin_ia32_pause();
                if ((++it & 1023) == 0) {
                    if (mono_ns() - t0 > (long)g_spin_us * 1000L) break;
                }
            }
            if ((s = atomic_load_explicit(&g_seq, memory_order_acquire)) != my) { my = s; break; }
            futex_wait((long *)&g_seq, my);
        }
        if (w >= g_K) continue;
        int p = atomic_load_explicit(&g_pin[w], __ATOMIC_RELAXED);
        if (p >= 0 && p != g_pin_applied[w]) {
            (void)set_my_cpu(p);
            g_pin_applied[w] = p;
        }
        run_job(w);
    }
    return NULL;
}

static void pool_init(void) {
    struct cpumask m;
    get_my_cpus(&m);
    int nallowed = 0;
    for (int i = 0; i < 256; i++) if ((m.bits[i >> 6] >> (i & 63)) & 1UL) nallowed++;
    g_nworkers = nallowed - 1;
    if (g_nworkers > MAXW - 1) g_nworkers = MAXW - 1;
    if (g_nworkers < 1) g_nworkers = 1;
    topo_init();
    const char *e = getenv("RB_SOR_SPIN_US"); if (e) g_spin_us = atoi(e);
    e = getenv("RB_SOR_K"); if (e) g_force_k = atoi(e);
    e = getenv("RB_SOR_DNODE"); if (e) g_use_datanode = atoi(e);
    for (int w = 1; w <= g_nworkers; w++)
        pthread_create(&g_thr[w - 1], NULL, pool_worker, (void *)(long)w);
    g_pool_ready = 1;
}

void rb_sor_fp64(
    const double *restrict f,
    double *restrict u,
    const int64_t N,
    const int64_t TSTEPS,
    const double omega,
    uint8_t *restrict workspace,
    const int64_t workspace_size) {

    (void)workspace; (void)workspace_size;
    if (N < 3 || TSTEPS < 1) return;

    pthread_mutex_lock(&g_init_mu);
    if (!g_pool_ready) pool_init();
    pthread_mutex_unlock(&g_init_mu);

    while (atomic_exchange_explicit(&g_busy, 1, memory_order_acquire)) {
        struct timespec ts = {0, 200000};
        nanosleep(&ts, NULL);
    }

    struct cpumask oldm;
    get_my_cpus(&oldm);

    long dn = -1;
    if (g_use_datanode) dn = addr_node((unsigned long)u);
    if (dn < 0) dn = addr_node((unsigned long)f);

    g_K = choose_k(N);

    int home = 0, best = -1;
    for (int n = 0; n < g_node_count; n++) {
        int c = node_allowed_count(n);
        if (c > best) { best = c; home = n; }
    }

    int idx = 0;
    int order[3] = { (int)dn, home, -1 };
    for (int pass = 0; pass < 3 && idx < g_K; pass++) {
        int node = order[pass];
        if (node >= 0) {
            int cpus[MAXCPU_PER_NODE], nn = node_allowed_cpus(node, cpus);
            for (int t = 0; t < nn && idx < g_K; t++) {
                int dup = 0;
                for (int t2 = 0; t2 < idx; t2++) if (g_place[t2] == cpus[t]) dup = 1;
                if (!dup) g_place[idx++] = cpus[t];
            }
        } else {
            for (int c = 0; c < 256 && idx < g_K; c++) {
                if (!cpu_allowed(c)) continue;
                int dup = 0;
                for (int t2 = 0; t2 < idx; t2++) if (g_place[t2] == c) dup = 1;
                if (!dup) g_place[idx++] = c;
            }
        }
    }

    int64_t R = N - 2;
    int64_t base_r = R / g_K, rem = R % g_K;
    for (int w = 0; w < g_K; w++) {
        int64_t lo = 1 + w * base_r + (w < rem ? w : rem);
        int64_t hi = lo + base_r + (w < rem ? 1 : 0);
        g_ranges[w].lo = lo;
        g_ranges[w].hi = hi;
    }

    g_job.f = f; g_job.u = u; g_job.N = N; g_job.TSTEPS = TSTEPS;
    double h = 1.0 / (double)(N - 1);
    g_job.h2 = h * h;
    g_job.c1 = 1.0 - omega;
    g_job.om = omega;
    g_base = (g_base + 1) % (1L << 52);

    /* publish placements; workers 1..K-1 self-pin when they pick up the job */
    for (int w = 1; w < MAXW; w++)
        atomic_store_explicit(&g_pin[w], (w < g_K) ? g_place[w] : -1, __ATOMIC_RELAXED);
    g_pin_applied[0] = -1;

    int on_cpu0 = 0;
    {
        int cpu = g_place[0];
        on_cpu0 = (oldm.bits[cpu >> 6] >> (cpu & 63)) & 1UL;
        int only = 1;
        for (int i = 0; i < 256; i++)
            if (((oldm.bits[i >> 6] >> (i & 63)) & 1UL) && i != cpu) only = 0;
        if (on_cpu0 && only) { /* already pinned there */ }
        else set_my_cpu(cpu);
    }

    g_seq = (long)g_seq + 1;
    __atomic_thread_fence(__ATOMIC_RELEASE);
    futex_wake((long *)&g_seq, 127);
    run_job(0);
    set_my_mask(&oldm);
    atomic_store_explicit(&g_busy, 0, memory_order_release);
}
