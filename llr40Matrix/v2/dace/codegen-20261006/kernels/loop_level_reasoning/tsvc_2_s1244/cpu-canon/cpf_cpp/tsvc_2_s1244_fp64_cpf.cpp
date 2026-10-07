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
#include <limits>
#include <utility>

// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline T cpf_min(const T& value) {
    return value;
}
template <typename T, typename... Ts>
static constexpr inline typename std::common_type<T, Ts...>::type cpf_min(const T& a, const Ts&... rest) {
    return (cpf_min(rest...) < a) ? cpf_min(rest...) : a;
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R int_ceil(const T& numerator, const U& denominator) {
    return (numerator + denominator - 1) / denominator;
}
template <typename T>
static constexpr inline void py_divmod(const T& numerator, const T& denominator, T& quotient,
                                       T& remainder) {
    if constexpr (std::is_floating_point_v<T>) {
        const T ratio = numerator / denominator;
        remainder = std::fmod(numerator, denominator);
        if (denominator == 0) {
            quotient = ratio;
            return;
        }
        quotient = (numerator - remainder) / denominator;
        if (remainder == 0) {
            remainder = std::copysign(static_cast<T>(0), denominator);
        } else if ((denominator < 0) != (remainder < 0)) {
            remainder += denominator;
            quotient -= 1;
        }
        if (quotient == 0) {
            quotient = std::copysign(static_cast<T>(0), ratio);
        } else {
            const T floored = std::floor(quotient);
            quotient = quotient - floored > static_cast<T>(0.5) ? floored + 1 : floored;
        }
    } else if (denominator == 0) {
        quotient = 0;
        remainder = 0;
    } else if (numerator == std::numeric_limits<T>::min() && denominator == static_cast<T>(-1)) {
        quotient = numerator;
        remainder = 0;
    } else {
        quotient = static_cast<T>(numerator / denominator);
        remainder = static_cast<T>(numerator % denominator);
        if (remainder != 0 && ((remainder < 0) != (denominator < 0))) {
            quotient = static_cast<T>(quotient - 1);
            remainder = static_cast<T>(remainder + denominator);
        }
    }
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R py_floor(const T& numerator, const U& denominator) {
    R quotient = 0;
    R remainder = 0;
    py_divmod(static_cast<R>(numerator), static_cast<R>(denominator), quotient, remainder);
    return quotient;
}
#ifdef _OPENMP
#include <omp.h>
#endif
static constexpr inline int64_t a_antidep_seam_size(int64_t __dace_num_threads) { return (__dace_num_threads + 1); }
static constexpr inline int64_t _cpy_in_idx(int64_t __d0, int64_t LEN_1D, int64_t __dace_num_threads) { return (__d0 * int_ceil((LEN_1D - 2), __dace_num_threads)); }
static constexpr inline int64_t _cpy_out_idx(int64_t __d0) { return __d0; }
static inline void copy_a_to_a_antidep_seam_sdfg_0_3_4(const double* __restrict__ a, double* __restrict__ a_antidep_seam, int64_t LEN_1D, int64_t __dace_num_threads) {

    {
        const double* _cpy_in;
        _cpy_in = &a[1];
        double* _cpy_out;
        _cpy_out = &a_antidep_seam[0];

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t __i0 = 0; __i0 < int_ceil((LEN_1D - 2), int_ceil((LEN_1D - 2), __dace_num_threads)); __i0 += 1) {
            _cpy_out[_cpy_out_idx(__i0)] = _cpy_in[_cpy_in_idx(__i0, LEN_1D, __dace_num_threads)];  // copy_a_to_a_antidep_seam_tasklet
        }

    }
}

static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_antidep_seam_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static inline void nested_a_split_snapshot_0_1_7(const double* __restrict__ b, const double* __restrict__ c, double* __restrict__ a, double* __restrict__ d, int64_t LEN_1D, int64_t __dace_num_threads, int64_t antidep_chunk__loop_it_1) {

    // sequential -- carried: WAR on a[_loop_it_1 + 1]
    // settled: proven, an iteration overwrites what an earlier one still has to read; focus: restructure around the dependence (reorder, block, rewrite as a scan or reduction), or optimize the work inside
    for (int64_t _loop_it_1 = antidep_chunk__loop_it_1; (_loop_it_1 < cpf_min((LEN_1D - 2), ((antidep_chunk__loop_it_1 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1))); _loop_it_1 = (_loop_it_1 + 1)) {
        {
            double c_slice_times_c_slice;
            double b_slice_plus_c_slice_c_slice;
            double b_slice_times_b_slice;
            double b_slice_c_slice_c_slice_plus_b_slice_b_slice;
            double __map_fusion_a;

            b_slice_times_b_slice = (b[b_idx(_loop_it_1)] * b[b_idx(_loop_it_1)]);  // _Mult_
            c_slice_times_c_slice = (c[c_idx(_loop_it_1)] * c[c_idx(_loop_it_1)]);  // _Mult_
            b_slice_plus_c_slice_c_slice = (b[b_idx(_loop_it_1)] + c_slice_times_c_slice);  // _Add_
            b_slice_c_slice_c_slice_plus_b_slice_b_slice = (b_slice_plus_c_slice_c_slice + b_slice_times_b_slice);  // _Add_
            __map_fusion_a = (b_slice_c_slice_c_slice_plus_b_slice_b_slice + c[c_idx(_loop_it_1)]);  // _Add_
            a[a_idx(_loop_it_1)] = __map_fusion_a;  // copy___map_fusion_a_to_a
            d[d_idx(_loop_it_1)] = (__map_fusion_a + a[a_idx((_loop_it_1 + 1))]);  // _Add_

        }

    }

}

extern "C" void tsvc_2_s1244_fp64(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    #ifdef _OPENMP
    const int __dace_num_threads = omp_get_max_threads();
    #else
    const int __dace_num_threads = 1;
    #endif
    double* __restrict__ a_antidep_seam = new (std::align_val_t(64)) double[a_antidep_seam_size(__dace_num_threads)];
    int64_t __dace_rng_0;
    int64_t __dace_rng_1;

    {

        {  // check_assumption_0
            if ((__dace_num_threads < 0)) {
                std::abort();
            }
        }

    }
    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {

        copy_a_to_a_antidep_seam_sdfg_0_3_4(&a[0], &a_antidep_seam[0], LEN_1D, __dace_num_threads);
        a_antidep_seam[a_antidep_seam_idx(int_ceil((LEN_1D - 2), int_ceil((LEN_1D - 2), __dace_num_threads)))] = a[a_idx((LEN_1D - 1))];  // copy_a_to_a_antidep_seam

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    for (int64_t _loop_it_1 = 0; _loop_it_1 < (cpf_min(0, (LEN_1D - 2)) + 1); _loop_it_1 += 1) {
        double c_slice_times_c_slice_0;
        double b_slice_plus_c_slice_c_slice_0;
        double b_slice_times_b_slice_0;
        double b_slice_c_slice_c_slice_plus_b_slice_b_slice_0;
        double __map_fusion_a_0;
        b_slice_times_b_slice_0 = (b[b_idx(_loop_it_1)] * b[b_idx(_loop_it_1)]);  // _Mult_
        c_slice_times_c_slice_0 = (c[c_idx(_loop_it_1)] * c[c_idx(_loop_it_1)]);  // _Mult_
        b_slice_plus_c_slice_c_slice_0 = (b[b_idx(_loop_it_1)] + c_slice_times_c_slice_0);  // _Add_
        b_slice_c_slice_c_slice_plus_b_slice_b_slice_0 = (b_slice_plus_c_slice_c_slice_0 + b_slice_times_b_slice_0);  // _Add_
        __map_fusion_a_0 = (b_slice_c_slice_c_slice_plus_b_slice_b_slice_0 + c[c_idx(_loop_it_1)]);  // _Add_
        d[d_idx(_loop_it_1)] = (__map_fusion_a_0 + a_antidep_seam[a_antidep_seam_idx(0)]);  // _Add_
        a[a_idx(_loop_it_1)] = __map_fusion_a_0;  // copy___map_fusion_a_0_to_a
    }
    __dace_rng_0 = int_ceil((LEN_1D - 2), __dace_num_threads);

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    assert((__dace_rng_0) > 0 && "Map single_state_body_0_map requires a positive step");
    #pragma omp parallel for
    for (int64_t antidep_chunk__loop_it_1 = 1; antidep_chunk__loop_it_1 < (LEN_1D - 1); antidep_chunk__loop_it_1 += __dace_rng_0) {
        nested_a_split_snapshot_0_1_7(&b[0], &c[0], &a[0], &d[0], LEN_1D, __dace_num_threads, antidep_chunk__loop_it_1);
    }
    __dace_rng_1 = int_ceil((LEN_1D - 2), __dace_num_threads);

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    assert((__dace_rng_1) > 0 && "Map single_state_body_0_map requires a positive step");
    #pragma omp parallel for
    for (int64_t antidep_chunk__loop_it_1 = 1; antidep_chunk__loop_it_1 < (LEN_1D - 1); antidep_chunk__loop_it_1 += __dace_rng_1) {
        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        for (int64_t _loop_it_1 = cpf_min((LEN_1D - 2), ((antidep_chunk__loop_it_1 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1)); _loop_it_1 < (cpf_min((LEN_1D - 2), ((antidep_chunk__loop_it_1 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1)) + 1); _loop_it_1 += 1) {
            double c_slice_times_c_slice_1;
            double b_slice_plus_c_slice_c_slice_1;
            double b_slice_times_b_slice_1;
            double b_slice_c_slice_c_slice_plus_b_slice_b_slice_1;
            double __map_fusion_a_1;
            b_slice_times_b_slice_1 = (b[b_idx(_loop_it_1)] * b[b_idx(_loop_it_1)]);  // _Mult_
            c_slice_times_c_slice_1 = (c[c_idx(_loop_it_1)] * c[c_idx(_loop_it_1)]);  // _Mult_
            b_slice_plus_c_slice_c_slice_1 = (b[b_idx(_loop_it_1)] + c_slice_times_c_slice_1);  // _Add_
            b_slice_c_slice_c_slice_plus_b_slice_b_slice_1 = (b_slice_plus_c_slice_c_slice_1 + b_slice_times_b_slice_1);  // _Add_
            __map_fusion_a_1 = (b_slice_c_slice_c_slice_plus_b_slice_b_slice_1 + c[c_idx(_loop_it_1)]);  // _Add_
            d[d_idx(_loop_it_1)] = (__map_fusion_a_1 + a_antidep_seam[a_antidep_seam_idx((py_floor((antidep_chunk__loop_it_1 - 1), int_ceil((LEN_1D - 2), __dace_num_threads)) + 1))]);  // _Add_
            a[a_idx(_loop_it_1)] = __map_fusion_a_1;  // copy___map_fusion_a_1_to_a
        }
    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](a_antidep_seam, std::align_val_t(64));
}
