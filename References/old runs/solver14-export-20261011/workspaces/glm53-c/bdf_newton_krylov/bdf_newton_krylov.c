#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdatomic.h>

static double wt(void) { return omp_get_wtime(); }

/* centralized sense-reversing barrier */
static atomic_int b_count;
static atomic_int b_sense;
static inline void cbar(int tid, int T, int *my_sense)
{
    int s = *my_sense;
    if (T == 1) return;
    if (atomic_fetch_add_explicit(&b_count, 1, memory_order_acq_rel) == T - 1) {
        atomic_store_explicit(&b_count, 0, memory_order_relaxed);
        atomic_store_explicit(&b_sense, s, memory_order_release);
    } else {
        while (atomic_load_explicit(&b_sense, memory_order_acquire) != s)
            ; /* spin */
    }
    *my_sense = 1 - s;
}

static void kern_bench(void)
{
    const int64_t N = 53, NN = N * N;
    double *a = malloc((size_t)NN * 8), *b = malloc((size_t)NN * 8), *c = malloc((size_t)NN * 8);
    for (int64_t i = 0; i < NN; i++) { a[i] = 0.5 + (i % 7) * 1e-3; b[i] = 1.0; c[i] = i * 1e-9; }

    /* warm up any one-time thread costs */
    #pragma omp parallel
    {
        #pragma omp barrier
    }

    for (int T = 24; T >= 1; T /= 2) {
        double t0, t_omp, t_cus, t_ser;
        /* serial fused lap-like phase for reference */
        t0 = wt();
        for (int r = 0; r < 3000; r++) {
            for (int64_t i = 0; i < NN; i++)
                a[i] = 1.0000001 * a[i] + 0.5 * b[i] + 0.25 * c[i];
        }
        t_ser = (wt() - t0) / 3000 * 1e9;

        /* SPMD with omp barrier */
        t0 = wt();
        #pragma omp parallel num_threads(T)
        {
            int tid = omp_get_thread_num();
            int64_t lo = (int64_t)tid * NN / T, hi = (int64_t)(tid + 1) * NN / T;
            for (int r = 0; r < 3000; r++) {
                for (int64_t i = lo; i < hi; i++)
                    a[i] = 1.0000001 * a[i] + 0.5 * b[i] + 0.25 * c[i];
                #pragma omp barrier
            }
        }
        t_omp = (wt() - t0) / 3000 * 1e9;

        /* SPMD with custom barrier */
        t0 = wt();
        #pragma omp parallel num_threads(T)
        {
            int tid = omp_get_thread_num();
            int64_t lo = (int64_t)tid * NN / T, hi = (int64_t)(tid + 1) * NN / T;
            int ms = 0;
            for (int r = 0; r < 3000; r++) {
                for (int64_t i = lo; i < hi; i++)
                    a[i] = 1.0000001 * a[i] + 0.5 * b[i] + 0.25 * c[i];
                cbar(tid, T, &ms);
            }
        }
        t_cus = (wt() - t0) / 3000 * 1e9;
        printf("T=%2d serial=%.0fns spmd_ompbar=%.0fns spmd_cusbar=%.0fns\n", T, t_ser, t_omp, t_cus);
        fflush(stdout);
    }
    free(a); free(b); free(c);
}

void bdf_newton_krylov_fp64(
    double *restrict diagnostics, int64_t *restrict order_history,
    double *restrict u, double *restrict v,
    const double A, const double B, const int64_t N, const double alpha,
    const int64_t max_steps, const double t_end,
    uint8_t *restrict workspace, const int64_t workspace_size)
{
    (void)diagnostics; (void)order_history; (void)u; (void)v; (void)A; (void)B;
    (void)N; (void)alpha; (void)max_steps; (void)t_end; (void)workspace; (void)workspace_size;
    kern_bench();
}
