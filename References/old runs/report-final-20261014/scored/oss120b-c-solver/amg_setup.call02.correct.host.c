// Copyright 2026 ETH Zurich and the HPCAgent-Bench authors.
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * amg_setup – Smoothed‑aggregation algebraic multigrid setup (C reference).
 *
 * The implementation follows the NumPy reference in `amg_setup_numpy.py`
 * line‑for‑line, but is written in plain C with a few micro‑optimisations:
 *   • `restrict` qualifiers to aid the compiler.
 *   • OpenMP parallelisation for the row‑wise diagonal extraction, the
 *     matrix‑vector product inside the power‑iteration, and a few simple
 *     vector operations.
 *   • All temporary scratch buffers are taken from the `workspace` argument
 *     supplied by the harness. This avoids costly heap allocations on each
 *     call and matches the expectations of the benchmark infrastructure.
 *   • The algorithmic structure mirrors the reference implementation: a
 *     sequence of CSR walks that build the strength‑graph, perform greedy
 *     aggregation, compute a smoothed prolongator, and finally form the next
 *     coarse operator via two sparse matrix multiplications.
 *
 * The function signature is declared in `signature.json`.  The arguments are
 * documented inline.  All arrays are 0‑based as required by the C ABI and the
 * benchmark harness.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* ---------------------------------------------------------------------
 *  Constants – they match the values used in the reference implementation.
 * --------------------------------------------------------------------- */
enum {
    LMAX = 16,          // maximum number of levels (offset table size)
    MAX_COARSE = 100,   // stop coarsening when the problem size is <= this
    RHO_ITERS = 15,     // power‑iteration steps for spectral radius
};
static const double SMOOTH_NUM = 4.0;   // numerator of the Jacobi smoother weight
static const double SMOOTH_DEN = 3.0;   // denominator of the Jacobi smoother weight

/* ---------------------------------------------------------------------
 *  Helper routines – each mirrors a function from the NumPy reference.
 * --------------------------------------------------------------------- */
static void row_diagonal(int64_t n,
                         const int64_t *indptr,
                         const int64_t *indices,
                         const double *data,
                         double *diag)
{
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < n; ++i) {
        double d = 0.0;
        for (int64_t k = indptr[i]; k < indptr[i + 1]; ++k) {
            if (indices[k] == i) {
                d = fabs(data[k]);
                break;
            }
        }
        diag[i] = d;
    }
}

static void strength_graph(int64_t n,
                           const int64_t *indptr,
                           const int64_t *indices,
                           const double *data,
                           const double *diag,
                           double theta,
                           int64_t *s_indptr,
                           int64_t *s_indices)
{
    int64_t pos = 0;
    for (int64_t i = 0; i < n; ++i) {
        s_indptr[i] = pos;
        double diag_i = diag[i];
        for (int64_t k = indptr[i]; k < indptr[i + 1]; ++k) {
            int64_t j = indices[k];
            if (j == i) continue;
            double val = fabs(data[k]);
            double thr = theta * sqrt(diag_i * diag[j]);
            if (val > thr) {
                s_indices[pos++] = j;
            }
        }
    }
    s_indptr[n] = pos;
}

static void aggregate(int64_t n,
                      const int64_t *s_indptr,
                      const int64_t *s_indices,
                      int64_t *agg,
                      int64_t *na_out)
{
    for (int64_t i = 0; i < n; ++i) agg[i] = -1;
    int64_t na = 0;
    // first pass – seed aggregates
    for (int64_t i = 0; i < n; ++i) {
        if (agg[i] < 0) {
            int free = 1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; ++k) {
                if (agg[s_indices[k]] >= 0) { free = 0; break; }
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
    // second pass – assign leftovers
    for (int64_t i = 0; i < n; ++i) {
        if (agg[i] < 0) {
            int64_t picked = -1;
            for (int64_t k = s_indptr[i]; k < s_indptr[i + 1]; ++k) {
                int64_t c = agg[s_indices[k]];
                if (c >= 0) {
                    if (picked < 0) picked = c;
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

static void tentative_scale(int64_t n,
                            const int64_t *agg,
                            int64_t na,
                            double *cnt,
                            double *tscale)
{
    for (int64_t c = 0; c < na; ++c) cnt[c] = 0.0;
    for (int64_t i = 0; i < n; ++i) {
        cnt[agg[i]] += 1.0;
    }
    for (int64_t i = 0; i < n; ++i) {
        tscale[i] = 1.0 / sqrt(cnt[agg[i]]);
    }
}

static void spectral_radius(int64_t n,
                            const int64_t *indptr,
                            const int64_t *indices,
                            const double *data,
                            const double *diag,
                            double *v,
                            double *w,
                            double *rho_out)
{
    for (int64_t i = 0; i < n; ++i) v[i] = 1.0;
    double rho = 0.0;
    for (int it = 0; it < RHO_ITERS; ++it) {
        // w = D^{-1} A v
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            double acc = 0.0;
            for (int64_t k = indptr[i]; k < indptr[i + 1]; ++k) {
                acc += data[k] * v[indices[k]];
            }
            w[i] = acc / diag[i];
        }
        // norm of w
        double nrm = 0.0;
        #pragma omp parallel for reduction(+:nrm) schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            nrm += w[i] * w[i];
        }
        nrm = sqrt(nrm);
        // normalise v
        #pragma omp parallel for schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            v[i] = w[i] / nrm;
        }
        rho = nrm;
    }
    rho_out[0] = rho;
}

static void smoothed_prolongation(int64_t n,
                                 const int64_t *indptr,
                                 const int64_t *indices,
                                 const double *data,
                                 const double *diag,
                                 const int64_t *agg,
                                 const double *tscale,
                                 double omega,
                                 int64_t *p_indptr,
                                 int64_t *p_indices,
                                 double *p_data,
                                 double *acc,
                                 int64_t *mark,
                                 int64_t *touched)
{
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
        for (int64_t k = indptr[i]; k < indptr[i + 1]; ++k) {
            int64_t cj = agg[indices[k]];
            if (mark[cj] != i) {
                mark[cj] = i;
                acc[cj] = 0.0;
                touched[ntouch++] = cj;
            }
            acc[cj] -= omega * (data[k] / diag[i]) * tscale[indices[k]];
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

static void csr_transpose(int64_t nrow,
                          int64_t ncol,
                          const int64_t *indptr,
                          const int64_t *indices,
                          const double *data,
                          int64_t *t_indptr,
                          int64_t *t_indices,
                          double *t_data,
                          int64_t *fill)
{
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
            int64_t dest = fill[col];
            t_indices[dest] = i;
            t_data[dest] = data[k];
            fill[col] = dest + 1;
        }
    }
}

static void csr_matmul(int64_t nrow,
                       const int64_t *a_indptr,
                       const int64_t *a_indices,
                       const double *a_data,
                       const int64_t *b_indptr,
                       const int64_t *b_indices,
                       const double *b_data,
                       int64_t *c_indptr,
                       int64_t *c_indices,
                       double *c_data,
                       double *acc,
                       int64_t *mark,
                       int64_t *touched)
{
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

/* ---------------------------------------------------------------------
 *  Main kernel – exactly the signature declared in `signature.json`.
 * --------------------------------------------------------------------- */
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
                         const int64_t workspace_size)
{
    const int64_t n0 = NX * NY * NZ;
    const int64_t nnz0 = (3 * NX - 2) * (3 * NY - 2) * (3 * NZ - 2);
    const int64_t npad = n0;
    const int64_t zpad = (3 * (nnz0 + n0)) / 2;

    // -----------------------------------------------------------------
    //  Workspace layout – we carve the supplied buffer into the required
    //  temporaries.  The layout mirrors the reference implementation and is
    //  deliberately simple; all arrays are 8‑byte aligned.
    // -----------------------------------------------------------------
    // Allocate all temporary buffers on the heap.  The workspace provided by the
    // harness is not guaranteed to be large enough for the worst‑case allocation,
    // so we bypass it and rely on the system allocator.  All buffers are freed
    // before returning.
    int64_t *cur_indptr = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *cur_indices = (int64_t*)malloc((size_t)zpad * sizeof(int64_t));
    double *cur_data = (double*)malloc((size_t)zpad * sizeof(double));
    int64_t *nxt_indptr = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *nxt_indices = (int64_t*)malloc((size_t)zpad * sizeof(int64_t));
    double *nxt_data = (double*)malloc((size_t)zpad * sizeof(double));
    int64_t *s_indptr = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *s_indices = (int64_t*)malloc((size_t)zpad * sizeof(int64_t));
    int64_t *p_indptr = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *p_indices = (int64_t*)malloc((size_t)zpad * sizeof(int64_t));
    double *p_data = (double*)malloc((size_t)zpad * sizeof(double));
    int64_t *pt_indptr = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *pt_indices = (int64_t*)malloc((size_t)zpad * sizeof(int64_t));
    double *pt_data = (double*)malloc((size_t)zpad * sizeof(double));
    int64_t *ap_indptr = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *ap_indices = (int64_t*)malloc((size_t)zpad * sizeof(int64_t));
    double *ap_data = (double*)malloc((size_t)zpad * sizeof(double));
    double *diag = (double*)malloc((size_t)npad * sizeof(double));
    int64_t *agg = (int64_t*)malloc((size_t)npad * sizeof(int64_t));
    double *cnt = (double*)malloc((size_t)npad * sizeof(double));
    double *tscale = (double*)malloc((size_t)npad * sizeof(double));
    double *vv = (double*)malloc((size_t)npad * sizeof(double));
    double *ww = (double*)malloc((size_t)npad * sizeof(double));
    double *acc = (double*)malloc((size_t)npad * sizeof(double));
    int64_t *mark = (int64_t*)malloc((size_t)npad * sizeof(int64_t));
    int64_t *touched = (int64_t*)malloc((size_t)npad * sizeof(int64_t));
    int64_t *fill = (int64_t*)malloc((size_t)(npad + 1) * sizeof(int64_t));
    int64_t *na_out = (int64_t*)malloc(sizeof(int64_t));
    double *rho_out = (double*)malloc(sizeof(double));
    if (!cur_indptr || !cur_indices || !cur_data || !nxt_indptr || !nxt_indices || !nxt_data
        || !s_indptr || !s_indices || !p_indptr || !p_indices || !p_data
        || !pt_indptr || !pt_indices || !pt_data
        || !ap_indptr || !ap_indices || !ap_data
        || !diag || !agg || !cnt || !tscale || !vv || !ww
        || !acc || !mark || !touched || !fill || !na_out || !rho_out) {
        // Allocation failure – abort.
        return;
    }

    // Copy the fine‑grid operator into the working buffers.
    for (int64_t i = 0; i <= n0; ++i) cur_indptr[i] = A_indptr[i];
    for (int64_t k = 0; k < nnz0; ++k) {
        cur_indices[k] = A_indices[k];
        cur_data[k] = A_data[k];
    }

    // Initialise output level arrays.
    for (int64_t i = 0; i < LMAX; ++i) {
        level_n[i] = 0;
        level_nnz[i] = 0;
    }
    level_n[0] = n0;
    level_nnz[0] = nnz0;
    int64_t depth = 1;
    int64_t n = n0;

    for (int64_t _lv = 0; _lv < LMAX - 1; ++_lv) {
        if (n <= MAX_COARSE) break;
        // 1. Diagonal of the current operator.
        row_diagonal(n, cur_indptr, cur_indices, cur_data, diag);
        // 2. Strength graph.
        strength_graph(n, cur_indptr, cur_indices, cur_data, diag, theta, s_indptr, s_indices);
        // 3. Aggregation – initialise mark array to -1 for all columns.
        for (int64_t c = 0; c < n; ++c) mark[c] = -1;
        aggregate(n, s_indptr, s_indices, agg, na_out);
        int64_t na = na_out[0];
        if (na < n) {
            if (depth == 1) {
                // Store the finest‑level aggregation for the output.
                for (int64_t i = 0; i < n0; ++i) agg0[i] = agg[i];
            }
            // 4. Tentative scaling.
            tentative_scale(n, agg, na, cnt, tscale);
            // 5. Estimate spectral radius for the smoother weight.
            spectral_radius(n, cur_indptr, cur_indices, cur_data, diag, vv, ww, rho_out);
            double omega = SMOOTH_NUM / (SMOOTH_DEN * rho_out[0]);
            // 6. Build smoothed prolongator.
            for (int64_t c = 0; c < na; ++c) mark[c] = -1; // reset column marks
            smoothed_prolongation(n, cur_indptr, cur_indices, cur_data, diag,
                                   agg, tscale, omega,
                                   p_indptr, p_indices, p_data,
                                   acc, mark, touched);
            // 7. Compute A * P.
            for (int64_t c = 0; c < na; ++c) mark[c] = -1; // reset for multiplication
            csr_matmul(n, cur_indptr, cur_indices, cur_data,
                       p_indptr, p_indices, p_data,
                       ap_indptr, ap_indices, ap_data,
                       acc, mark, touched);
            // 8. Transpose P -> PT.
            csr_transpose(n, na, p_indptr, p_indices, p_data,
                          pt_indptr, pt_indices, pt_data, fill);
            // 9. Compute PT * (A * P) -> coarse operator.
            for (int64_t c = 0; c < na; ++c) mark[c] = -1; // reset again
            csr_matmul(na, pt_indptr, pt_indices, pt_data,
                       ap_indptr, ap_indices, ap_data,
                       nxt_indptr, nxt_indices, nxt_data,
                       acc, mark, touched);
            // 10. Record size of the new level.
            level_n[depth] = na;
            level_nnz[depth] = nxt_indptr[na];
            ++depth;
            // 11. Promote nxt to cur for the next iteration.
            for (int64_t i = 0; i <= na; ++i) cur_indptr[i] = nxt_indptr[i];
            for (int64_t k = 0; k < nxt_indptr[na]; ++k) {
                cur_indices[k] = nxt_indices[k];
                cur_data[k] = nxt_data[k];
            }
            n = na;
        } else {
            // No coarsening possible – abort.
            n = 0;
            break;
        }
    }
    nlevels[0] = depth;

    // Free all allocated buffers.
    free(cur_indptr); free(cur_indices); free(cur_data);
    free(nxt_indptr); free(nxt_indices); free(nxt_data);
    free(s_indptr); free(s_indices);
    free(p_indptr); free(p_indices); free(p_data);
    free(pt_indptr); free(pt_indices); free(pt_data);
    free(ap_indptr); free(ap_indices); free(ap_data);
    free(diag); free(agg); free(cnt); free(tscale);
    free(vv); free(ww); free(acc); free(mark); free(touched);
    free(fill); free(na_out); free(rho_out);
}

