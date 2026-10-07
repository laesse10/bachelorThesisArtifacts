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
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R int_ceil(const T& numerator, const U& denominator) {
    return (numerator + denominator - 1) / denominator;
}
static constexpr inline int64_t _in_idx(int64_t __d0) { return (2 * __d0); }
extern "C" void quasi_affine_reduce_odd_fp64(const double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {
        const double* _in;
        _in = &a[1];

        {  // assign_16_4
            double __out;
            __out = 0.0;
            out[0] = __out;
        }
        // reduction over the given axes with the given operator
        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for reduction(+:out[0:1])
        for (int64_t _i0 = 0; _i0 < int_ceil((LEN_1D - 1), 2); _i0 += 1) {
            {  // identity
                double __out;
                __out = _in[_in_idx(_i0)];
                *(out) = *(out) + (__out);
            }
        }

    }
}
