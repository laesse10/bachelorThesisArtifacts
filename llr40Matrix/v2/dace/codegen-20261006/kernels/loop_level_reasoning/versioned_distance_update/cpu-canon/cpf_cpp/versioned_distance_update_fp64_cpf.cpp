// Rendered by DaCe CPF (canonical parallel form): self-contained, no DaCe runtime.
// Already parallelized, with basic heuristics applied. Every loop names its class first:
//   parallel, sequential -- settled; their parallelism needs no further reasoning.
//   unsure               -- open; the only loops whose parallelism is worth reasoning about.
// Spend the effort on heuristic optimizations and restructuring.
#include <cstdint>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <complex>
#include <numeric>
#include <new>
#include <type_traits>
static constexpr inline int64_t _scan_in_a_size(int64_t K, int64_t LEN_1D) { return ((- K) + LEN_1D); }
static constexpr inline int64_t _scan_coef_a_size(int64_t K, int64_t LEN_1D) { return ((- K) + LEN_1D); }
static constexpr inline int64_t _scan_coef_a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _scan_in_a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _cpy_in_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _cpy_out_idx(int64_t __d0) { return __d0; }
static inline void copy_a_to__scan_seed_a_sdfg_2_0_12(const double* __restrict__ a, double* __restrict__ _cpy_out, int64_t K) {

    {
        const double* _cpy_in;
        _cpy_in = &a[0];

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t __i0 = 0; __i0 < K; __i0 += 1) {
            _cpy_out[_cpy_out_idx(__i0)] = _cpy_in[_cpy_in_idx(__i0)];  // copy_a_to__scan_seed_a_tasklet
        }

    }
}

static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void versioned_distance_update_fp64(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, int64_t K, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double* __restrict__ _scan_seed_a = new (std::align_val_t(64)) double[K];
    double a_index_0;
    double a_index_1;
    double __tmp0_0;
    double __tmp0_1;
    double b_slice_times_c_slice_0;
    double b_slice_times_c_slice_1;
    double __tmp1_0;
    double __tmp1_1;

    {

        {  // check_assumption_0
            if ((K < 0)) {
                std::abort();
            }
        }
        {  // check_assumption_1
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    if ((K >= 1)) {
        {
            double* __restrict__ _scan_in_a = new (std::align_val_t(64)) double[_scan_in_a_size(K, LEN_1D)];
            double* __restrict__ _scan_coef_a = new (std::align_val_t(64)) double[_scan_coef_a_size(K, LEN_1D)];

            // parallel -- the iterations are independent
            // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
            #pragma omp parallel for
            for (int64_t _loop_it_1 = K; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
                _scan_coef_a[_scan_coef_a_idx(((- K) + _loop_it_1))] = 0.75;  // affine_coef
                _scan_in_a[_scan_in_a_idx(((- K) + _loop_it_1))] = (b[b_idx(_loop_it_1)] * c[c_idx(_loop_it_1)]);  // affine_delta
            }
            copy_a_to__scan_seed_a_sdfg_2_0_12(&a[0], &_scan_seed_a[0], K);
            // scan: running (prefix) fold along an axis
            // parallel scan; canonicalization takes the parallel form.
            // Alternative: a sequential loop over parallel maps.
            // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
            // GPU: the scan is usually the better of the two.
            // Both are correct. Measure before choosing.
            {  // for_17_affine_scan_op
                { const long cpf_n = (long)(static_cast<long>(((- K) + LEN_1D))); const long cpf_s = (long)(static_cast<long>(K));
                    if (cpf_s <= 0) std::abort();
                    #pragma omp parallel for
                    for (long cpf_r = 0; cpf_r < cpf_s; ++cpf_r) {
                        if (cpf_r >= cpf_n) continue;
                        (a + K)[cpf_r] = _scan_coef_a[cpf_r] * (_scan_seed_a[cpf_r]) + _scan_in_a[cpf_r];
                        for (long cpf_j = cpf_r + cpf_s; cpf_j < cpf_n; cpf_j += cpf_s) (a + K)[cpf_j] = _scan_coef_a[cpf_j] * (a + K)[cpf_j - cpf_s] + _scan_in_a[cpf_j];
                    }
                };
            }
            static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
            ::operator delete[](_scan_in_a, std::align_val_t(64));
            static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
            ::operator delete[](_scan_coef_a, std::align_val_t(64));

        }
    } else if (((! (K >= 1)) && (K == 0))) {

        // sequential -- pinned by an earlier pass; the dependence test finds the iterations independent
        // settled: the pass keeps the order on purpose (such as the fallback arm of a specialization)
        for (int64_t _loop_it_2 = K; (_loop_it_2 < LEN_1D); _loop_it_2 = (_loop_it_2 + 1)) {
            {

                a_index_0 = a[a_idx(((- K) + _loop_it_2))];  // _assign_a_to_a_index_0
                __tmp0_0 = (0.75 * a_index_0);  // _Mult_
                b_slice_times_c_slice_0 = (b[b_idx(_loop_it_2)] * c[c_idx(_loop_it_2)]);  // _Mult_
                __tmp1_0 = (__tmp0_0 + b_slice_times_c_slice_0);  // _Add_
                a[a_idx(_loop_it_2)] = __tmp1_0;  // _assign___tmp1_0_to_a

            }

        }


    } else {

        // unsure -- never examined for dependences
        // open: not proven either way; kept sequential to be safe, and may well be parallel -- a fact about the indices (a bound, a permutation) settles it
        for (int64_t _loop_it_3 = K; (_loop_it_3 < LEN_1D); _loop_it_3 = (_loop_it_3 + 1)) {
            {

                a_index_1 = a[a_idx(((- K) + _loop_it_3))];  // _assign_a_to_a_index_1
                __tmp0_1 = (0.75 * a_index_1);  // _Mult_
                b_slice_times_c_slice_1 = (b[b_idx(_loop_it_3)] * c[c_idx(_loop_it_3)]);  // _Mult_
                __tmp1_1 = (__tmp0_1 + b_slice_times_c_slice_1);  // _Add_
                a[a_idx(_loop_it_3)] = __tmp1_1;  // _assign___tmp1_1_to_a

            }

        }


    }


    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_scan_seed_a, std::align_val_t(64));
}
