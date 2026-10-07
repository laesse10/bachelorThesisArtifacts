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
#include <utility>

// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline T cpf_max(const T& value) {
    return value;
}
template <typename T, typename... Ts>
static constexpr inline typename std::common_type<T, Ts...>::type cpf_max(const T& a, const Ts&... rest) {
    return (a < cpf_max(rest...)) ? cpf_max(rest...) : a;
}
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
static constexpr inline int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static inline void loop_body_2_0_0(double* __restrict__ a, int64_t LEN_2D, int64_t _skew_p_0, int64_t _skew_t_0) {

    // sequential -- inner tile of a wavefront [64x64]: the original order is kept verbatim, so the tiled result is bit-identical
    for (int64_t _loop_it_0 = (((-64 * _skew_p_0) + (64 * _skew_t_0)) + 1); (_loop_it_0 <= cpf_min((LEN_2D - 1), (((-64 * _skew_p_0) + (64 * _skew_t_0)) + 64))); _loop_it_0 = (_loop_it_0 + 1)) {
        // sequential -- inner tile of a wavefront [64x64]: the original order is kept verbatim, so the tiled result is bit-identical
        for (int64_t _loop_it_1 = cpf_max(_loop_it_0, ((64 * _skew_p_0) + 1)); (_loop_it_1 <= cpf_min((LEN_2D - 1), ((64 * _skew_p_0) + 64))); _loop_it_1 = (_loop_it_1 + 1)) {
            {
                double a_slice_a_slice_plus_a_slice;
                double a_index_0;
                double a_slice_plus_a_slice;
                double a_slice_a_slice_a_slice_div_3_0;
                double a_index;
                double a_index_1;

                a_index = a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)];  // _assign_a_to_a_index
                a_index_0 = a[a_idx((_loop_it_0 - 1), _loop_it_1, LEN_2D)];  // _assign_a_to_a_index_0
                a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
                a_index_1 = a[a_idx(_loop_it_0, (_loop_it_1 - 1), LEN_2D)];  // _assign_a_to_a_index_1
                a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
                a_slice_a_slice_a_slice_div_3_0 = (a_slice_a_slice_plus_a_slice / 3.0);  // _Div_
                a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = a_slice_a_slice_a_slice_div_3_0;  // _assign_a_slice_a_slice_a_slice_div_3_0_to_a

            }

        }

    }
}

static inline void nested_sdfg_0_1_0(double* __restrict__ a, int64_t LEN_2D) {

    // sequential -- wavefront tile diagonal (t = _loop_it_0 + j) [64x64]: the tile diagonal carries every dependence
    // alternatives: the element diagonal, or the unskewed nest -- all three bit-identical; a bigger tile trades kernel launches for block-local barriers
    for (int64_t _skew_t_0 = 0; (_skew_t_0 <= ((2 * int_ceil((LEN_2D - 1), 64)) - 2)); _skew_t_0 = (_skew_t_0 + 1)) {

        // parallel -- wavefront tile column [64x64]: the tiles on one diagonal are independent
        #pragma omp for
        for (int64_t _skew_p_0 = cpf_max(0, ((_skew_t_0 - int_ceil((LEN_2D - 1), 64)) + 1)); _skew_p_0 < (cpf_min(_skew_t_0, (int_ceil((LEN_2D - 1), 64) - 1)) + 1); _skew_p_0 += 1) {
            loop_body_2_0_0(&a[0], LEN_2D, _skew_p_0, _skew_t_0);
        }

    }
}

extern "C" void wf_triangular_fp64(double * __restrict__ a, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        #pragma omp parallel
        {
            nested_sdfg_0_1_0(&a[0], LEN_2D);
        }
    }
}
