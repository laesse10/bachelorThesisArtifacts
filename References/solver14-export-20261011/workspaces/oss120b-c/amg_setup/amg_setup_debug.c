/*
 * Optimized C implementation of the smoothed-aggregation AMG setup kernel.
 *
 * This file follows the reference implementation in amg_setup_numpy.py. The
 * algorithm builds a multigrid hierarchy for the 27‑point variable‑coefficient
 * stencil operator supplied in CSR format. The kernel must match the reference
 * output to within the tight tolerances of the benchmark.
 *
 * The function signature is dictated by `signature.json` and must not be
 * changed. All temporary work buffers are allocated from the provided
 * `workspace` pointer. The implementation uses OpenMP for parallelism in the
 * computationally heavy phases (row diagonal extraction, matrix‑vector
 * multiplication and norm reduction) while keeping the more sequential phases
 * (aggregation, CSR multiplication) simple and correct.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>

/*--- Constants matching the reference implementation -----------------------*/
static const int64_t LMAX = 16;          /* maximum number of levels */
static const int64_t MAX_COARSE = 100;   /* stop coarsening when <= this */
static const double SMOOTH_NUM = 4.0;    /* numerator of Jacobi weight */
static const double SMOOTH_DEN = 3.0;    /* denominator of Jacobi weight */
static const int RHO_ITERS = 15;        /* power‑iteration count */

/*--- Helper functions ----------------------------------------------------*/

/* Extract absolute diagonal entries of A. */
static inline void row_diagonal(const int64_t *restrict a_indptr,
                                 const int64_t *restrict a_indices,
                                 const double *restrict a_data,
                                 int64_t n,
                                 double *restrict diag) {
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; ++i) {
        double d = 0.0;
        for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; ++k) {
            if (a_indices[k] == i) {
                d = fabs(a_data[k]);
                // continue scanning in case of duplicate diagonal entries
            }
        }
        diag[i] = d;
    }
}

/* Build the strength‑of‑connection graph S. */
static inline void strength_graph(const int64_t *restrict a_indptr,
                                 const int64_t *restrict a_indices,
                                 const double *restrict a_data,
                                 const double *restrict diag,
                                 int64_t n,
                                 double theta,
                                 int64_t *restrict s_indptr,
                                 int64_t *restrict s_indices) {
    int64_t pos = 0;
    for (int64_t i = 0; i < n; ++i) {
        s_indptr[i] = pos;
        double diag_i = diag[i];
        for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; ++k) {
            int64_t j = a_indices[k];
            if (j != i) {
                double val = fabs(a_data[k]);
                double threshold = theta * sqrt(diag_i * diag[j]);
                if (val > threshold) {
                    s_indices[pos++] = j;
                }
            }
        }
    }
    s_indptr[n] = pos;
}

/* Greedy maximal independent set aggregation. */
static inline void aggregate(const int64_t *restrict s_indptr,
                             const int64_t *restrict s_indices,
                             int64_t n,
                             int64_t *restrict agg,
                             int64_t *restrict na_out) {
    for (int64_t i = 0; i < n; ++i) agg[i] = -1;
    int64_t na = 0;
    /* First pass: seed aggregates */
    for (int64_t i = 0; i < n; ++i) {
        if (agg[i] < 0) {
            int free = 1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; ++k) {
                if (agg[s_indices[k]] >= 0) {
                    free = 0;
                    break;
                }
            }
            if (free) {
                agg[i] = na;
                for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; ++k) {
                    agg[s_indices[k]] = na;
                }
                ++na;
            }
        }
    }
    /* Second pass: assign leftovers */
    for (int64_t i = 0; i < n; ++i) {
        if (agg[i] < 0) {
            int64_t picked = -1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; ++k) {
                int64_t c = agg[s_indices[k]];
                if (c >= 0) {
                    if (picked < 0) {
                        picked = c;
                    }
                }
            }
            if (picked < 0) {
                agg[i] = na;
                ++na;
            } else {
                agg[i] = picked;
            }
        }
    }
    na_out[0] = na;
}

/* Column scaling that orthonormalises the tentative prolongator. */
static inline void tentative_scale(const int64_t *restrict agg,
                                   int64_t n,
                                   int64_t na,
                                   double *restrict cnt,
                                   double *restrict tscale) {
    for (int64_t c = 0; c < na; ++c) cnt[c] = 0.0;
    for (int64_t i = 0; i < n; ++i) {
        cnt[agg[i]] += 1.0;
    }
    for (int64_t i = 0; i < n; ++i) {
        tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
    }
}

/* Power iteration to estimate spectral radius rho(D^{-1}A). */
static inline void spectral_radius(const int64_t *restrict a_indptr,
                                   const int64_t *restrict a_indices,
                                   const double *restrict a_data,
                                   const double *restrict diag,
                                   int64_t n,
                                   double *restrict v,
                                   double *restrict w,
                                   double *restrict rho_out) {
    for (int64_t i = 0; i < n; ++i) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; ++it) {
        /* w = D^{-1} A v */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            double acc = 0.0;
            for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; ++k) {
                acc += a_data[k] * v[a_indices[k]];
            }
            w[i] = acc / diag[i];
        }
        /* norm of w */
        double nrm = 0.0;
        #pragma omp parallel for reduction(+:nrm) schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            nrm += w[i] * w[i];
        }
        nrm = sqrt(nrm);
        /* v = w / nrm */
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            v[i] = w[i] / nrm;
        }
        rho = nrm;
    }
    rho_out[0] = rho;
}

/* Compute P = (I - omega D^{-1} A) T, where T is the orthonormal tentative prolongator. */
static inline void smoothed_prolongation(const int64_t *restrict a_indptr,
                                         const int64_t *restrict a_indices,
                                         const double *restrict a_data,
                                         const double *restrict diag,
                                         const int64_t *restrict agg,
                                         const double *restrict tscale,
                                         int64_t n,
                                         double omega,
                                         int64_t *restrict p_indptr,
                                         int64_t *restrict p_indices,
                                         double *restrict p_data,
                                         double *restrict acc,
                                         int64_t *restrict mark,
                                         int64_t *restrict touched) {
    int64_t pos = 0;
    for (int64_t i = 0; i < n; ++i) {
        p_indptr[i] = pos;
        int64_t ntouch = 0;
        int64_t c0 = agg[i];
        if (mark[c0] != i) {
            mark[c0] = i;
            acc[c0] = 0.0;
            touched[ntouch++] = c0;
        }
        acc[c0] += tscale[i];
        for (int64_t k = a_indptr[i]; k < a_indptr[i + 1]; ++k) {
            int64_t cj = agg[a_indices[k]];
            if (mark[cj] != i) {
                mark[cj] = i;
                acc[cj] = 0.0;
                touched[ntouch++] = cj;
            }
            acc[cj] -= omega * (a_data[k] / diag[i]) * tscale[a_indices[k]];
        }
        for (int64_t t = 0; t < ntouch; ++t) {
            int64_t c = touched[t];
            p_indices[pos] = c;
            p_data[pos] = acc[c];
            ++pos;
        }
    }
    p_indptr[n] = pos;
}

/* CSR transpose via counting sort. */
static inline void csr_transpose(const int64_t *restrict indptr,
                                 const int64_t *restrict indices,
                                 const double *restrict data,
                                 int64_t nrow,
                                 int64_t ncol,
                                 int64_t *restrict t_indptr,
                                 int64_t *restrict t_indices,
                                 double *restrict t_data,
                                 int64_t *restrict fill) {
    for (int64_t c = 0; c <= ncol; ++c) t_indptr[c] = 0;
    for (int64_t k = 0; k < indptr[nrow]; ++k) {
        int64_t col = indices[k];
        t_indptr[col + 1] += 1;
    }
    for (int64_t c = 0; c < ncol; ++c) {
        t_indptr[c + 1] += t_indptr[c];
    }
    for (int64_t c = 0; c < ncol; ++c) fill[c] = t_indptr[c];
    for (int64_t i = 0; i < nrow; ++i) {
        for (int64_t k = indptr[i]; k < indptr[i + 1]; ++k) {
            int64_t col = indices[k];
            t_indices[fill[col]] = i;
            t_data[fill[col]] = data[k];
            fill[col]++;
        }
    }
}

/* C = A B for CSR operands using the mark‑scatter idiom. */
static inline void csr_matmul(const int64_t *restrict a_indptr,
                               const int64_t *restrict a_indices,
                               const double *restrict a_data,
                               const int64_t *restrict b_indptr,
                               const int64_t *restrict b_indices,
                               const double *restrict b_data,
                               int64_t nrow,
                               int64_t *restrict c_indptr,
                               int64_t *restrict c_indices,
                               double *restrict c_data,
                               double *restrict acc,
                               int64_t *restrict mark,
                               int64_t *restrict touched) {
    int64_t pos = 0;
    for (int64_t i = 0; i < nrow; ++i) {
        c_indptr[i] = pos;
        int64_t ntouch = 0;
        for (int64_t ka = a_indptr[i]; ka < a_indptr[i + 1]; ++ka) {
            int64_t j = a_indices[ka];
            double av = a_data[ka];
            for (int64_t kb = b_indptr[j]; kb < b_indptr[j + 1]; ++kb) {
                int64_t col = b_indices[kb];
                if (mark[col] != i) {
                    mark[col] = i;
                    acc[col] = 0.0;
                    touched[ntouch++] = col;
                }
                acc[col] += av * b_data[kb];
            }
        }
        for (int64_t t = 0; t < ntouch; ++t) {
            int64_t col = touched[t];
            c_indices[pos] = col;
            c_data[pos] = acc[col];
            ++pos;
        }
    }
    c_indptr[nrow] = pos;
}

/*--- Main kernel -----------------------------------------------------------*/
void amg_setup_csr_fp64(const double *restrict A_data,
                        const int64_t *restrict A_indices,
                        const int64_t *restrict A_indptr,
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
    /* Problem dimensions */
    const int64_t n0 = NX * NY * NZ;
    const int64_t nnz0 = (3 * NX - 2) * (3 * NY - 2) * (3 * NZ - 2);
    const int64_t zpad = (3 * (nnz0 + n0)) / 2;  // upper bound on nonzeros
    const int64_t npad = n0;

    /* Workspace layout – all buffers are sized exactly as the reference version.
     * The layout follows the order of allocation used in the Python reference.
     */
    size_t offset = 0;
    #define ALLOC_T(type, count) ((type *)(workspace + offset))
    #define STEP_T(type, count) offset += (size_t)(count) * sizeof(type)

    int64_t *cur_indptr   = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *cur_indices  = ALLOC_T(int64_t, zpad);   STEP_T(int64_t, zpad);
    double  *cur_data     = ALLOC_T(double,  zpad);   STEP_T(double,  zpad);

    int64_t *nxt_indptr   = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *nxt_indices  = ALLOC_T(int64_t, zpad);   STEP_T(int64_t, zpad);
    double  *nxt_data     = ALLOC_T(double,  zpad);   STEP_T(double,  zpad);

    int64_t *s_indptr     = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *s_indices    = ALLOC_T(int64_t, zpad);   STEP_T(int64_t, zpad);

    int64_t *p_indptr     = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *p_indices    = ALLOC_T(int64_t, zpad);   STEP_T(int64_t, zpad);
    double  *p_data       = ALLOC_T(double,  zpad);   STEP_T(double,  zpad);

    int64_t *pt_indptr    = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *pt_indices   = ALLOC_T(int64_t, zpad);   STEP_T(int64_t, zpad);
    double  *pt_data      = ALLOC_T(double,  zpad);   STEP_T(double,  zpad);

    int64_t *ap_indptr    = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *ap_indices   = ALLOC_T(int64_t, zpad);   STEP_T(int64_t, zpad);
    double  *ap_data      = ALLOC_T(double,  zpad);   STEP_T(double,  zpad);

    double *diag          = ALLOC_T(double, npad);   STEP_T(double, npad);
    int64_t *agg          = ALLOC_T(int64_t, npad); STEP_T(int64_t, npad);
    double *cnt           = ALLOC_T(double, npad);   STEP_T(double, npad);
    double *tscale        = ALLOC_T(double, npad);   STEP_T(double, npad);
    double *vv            = ALLOC_T(double, npad);   STEP_T(double, npad);
    double *ww            = ALLOC_T(double, npad);   STEP_T(double, npad);
    double *acc           = ALLOC_T(double, npad);   STEP_T(double, npad);
    int64_t *mark         = ALLOC_T(int64_t, npad); STEP_T(int64_t, npad);
    int64_t *touched      = ALLOC_T(int64_t, npad); STEP_T(int64_t, npad);
    int64_t *fill         = ALLOC_T(int64_t, npad + 1); STEP_T(int64_t, npad + 1);
    int64_t *na_out       = ALLOC_T(int64_t, 1);     STEP_T(int64_t, 1);
    double  *rho_out      = ALLOC_T(double, 1);      STEP_T(double, 1);

    /* Ensure we don't write beyond the supplied workspace size. */
    if ((int64_t)offset > workspace_size) {
        return; // insufficient workspace – the caller treats this as a failure.
    }

    /* Initialise work buffers to zero (except the copies of A). */
    memset(workspace, 0, offset);

    /* Copy the input CSR matrix into the working buffers. */
    for (int64_t i = 0; i <= n0; ++i) {
        cur_indptr[i] = A_indptr[i];
    }
    for (int64_t k = 0; k < nnz0; ++k) {
        cur_indices[k] = A_indices[k];
        cur_data[k]    = A_data[k];
    }

    /* Initialise level information. */
    for (int i = 0; i < LMAX; ++i) {
        level_n[i]   = 0;
        level_nnz[i] = 0;
    }
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;
    int64_t n = n0;

    /* Main coarsening loop – at most LMAX‑1 levels are generated. */
    for (int lv = 0; lv < LMAX - 1; ++lv) {
        if (n > MAX_COARSE) {
            /* Extract diagonal, build strength graph, and aggregate. */
            row_diagonal(cur_indptr, cur_indices, cur_data, n, diag);
            strength_graph(cur_indptr, cur_indices, cur_data, diag, n, theta, s_indptr, s_indices);
            for (int64_t i = 0; i < n; ++i) mark[i] = -1; // reset mark for aggregation
            aggregate(s_indptr, s_indices, n, agg, na_out);
            int64_t na = na_out[0];
            if (na < n) {
                if (depth == 1) {
                    /* Store the first‑level aggregates for the caller. */
                    for (int64_t i = 0; i < n0; ++i) {
                        agg0[i] = agg[i];
                    }
                }
                /* Build the tentative prolongator scaling. */
                tentative_scale(agg, n, na, cnt, tscale);
                /* Estimate spectral radius for smoothing weight. */
                spectral_radius(cur_indptr, cur_indices, cur_data, diag, n, vv, ww, rho_out);
                double omega = SMOOTH_NUM / (SMOOTH_DEN * rho_out[0]);
                /* Initialise mark for the prolongation construction. */
                for (int64_t i = 0; i < na; ++i) mark[i] = -1;
                smoothed_prolongation(cur_indptr, cur_indices, cur_data, diag,
                                      agg, tscale, n, omega,
                                      p_indptr, p_indices, p_data,
                                      acc, mark, touched);
                /* Compute A * P. */
                for (int64_t i = 0; i < na; ++i) mark[i] = -1;
                csr_matmul(cur_indptr, cur_indices, cur_data,
                           p_indptr, p_indices, p_data,
                           n, ap_indptr, ap_indices, ap_data,
                           acc, mark, touched);
                /* Compute P^T (transpose of P). */
                csr_transpose(p_indptr, p_indices, p_data,
                              n, na, pt_indptr, pt_indices, pt_data, fill);
                /* Compute P^T * (A * P) to obtain the coarse operator. */
                for (int64_t i = 0; i < na; ++i) mark[i] = -1;
                csr_matmul(pt_indptr, pt_indices, pt_data,
                           ap_indptr, ap_indices, ap_data,
                           na, nxt_indptr, nxt_indices, nxt_data,
                           acc, mark, touched);

                /* Record this level. */
                level_n[depth]   = na;
                level_nnz[depth] = nxt_indptr[na];
                ++depth;

                /* Prepare for the next coarsening step – move nxt into cur. */
                for (int64_t i = 0; i <= na; ++i) {
                    cur_indptr[i] = nxt_indptr[i];
                }
                int64_t nnz_nxt = nxt_indptr[na];
                for (int64_t k = 0; k < nnz_nxt; ++k) {
                    cur_indices[k] = nxt_indices[k];
                    cur_data[k]    = nxt_data[k];
                }
                n = na;
            } else {
                /* No reduction – stop coarsening. */
                n = 0;
            }
        } else {
            n = 0; // nothing more to coarsen
        }
    }
    nlevels[0] = depth;
    for (int i = 0; i < LMAX; ++i) {
        printf("%lld ", (long long)level_n[i]);
    }
    printf("\n"); fflush(stdout);
    #undef ALLOC_T
    #undef STEP_T
}
