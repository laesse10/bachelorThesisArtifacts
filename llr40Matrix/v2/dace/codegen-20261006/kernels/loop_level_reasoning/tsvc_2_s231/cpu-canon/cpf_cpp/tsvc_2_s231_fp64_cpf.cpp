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
static constexpr inline int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static inline void nested_sdfg_0_1_0(const double* __restrict__ bb, double* __restrict__ aa, int64_t LEN_2D, int64_t __dace_band, int64_t __dace_num_threads) {

    // sequential -- carried: RAW on aa[_loop_it_1 - 1, _loop_it_0]
    // settled: proven, an iteration reads what an earlier one wrote; focus: restructure around the dependence (reorder, block, rewrite as a scan or reduction), or optimize the work inside
    for (int64_t _loop_it_1 = 1; (_loop_it_1 < LEN_2D); _loop_it_1 = (_loop_it_1 + 1)) {

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        for (int64_t _loop_it_0 = py_floor((LEN_2D * __dace_band), __dace_num_threads); _loop_it_0 < py_floor((LEN_2D * (__dace_band + 1)), __dace_num_threads); _loop_it_0 += 1) {
            double aa_slice_plus_bb_slice;
            double aa_index;
            aa_index = aa[aa_idx((_loop_it_1 - 1), _loop_it_0, LEN_2D)];  // _assign_in_aa_to_aa_index
            aa_slice_plus_bb_slice = (aa_index + bb[bb_idx(_loop_it_1, _loop_it_0, LEN_2D)]);  // _Add_
            aa[aa_idx(_loop_it_1, _loop_it_0, LEN_2D)] = aa_slice_plus_bb_slice;  // _assign_out_aa_slice_plus_bb_slice_to_aa
        }

    }
}

extern "C" void tsvc_2_s231_fp64(double * __restrict__ aa, const double * __restrict__ bb, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    #ifdef _OPENMP
    const int64_t __dace_num_threads = omp_get_max_threads();
    #else
    const int64_t __dace_num_threads = 1;
    #endif

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t __dace_band = 0; __dace_band < __dace_num_threads; __dace_band += 1) {
        nested_sdfg_0_1_0(&bb[0], &aa[0], LEN_2D, __dace_band, __dace_num_threads);
    }
}
