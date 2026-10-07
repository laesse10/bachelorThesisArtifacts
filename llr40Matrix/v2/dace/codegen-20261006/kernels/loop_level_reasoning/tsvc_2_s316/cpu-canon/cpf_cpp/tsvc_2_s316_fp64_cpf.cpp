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
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t result_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s316_fp64(const double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double _arg_max_buf_for_17;
    double x;



    // reduction over the given axes with the given operator
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _o0 = 0; _o0 < 1; _o0 += 1) {
        {  // reduce_init
            double __out;
            __out = 1.7976931348623157e+308;
            _arg_max_buf_for_17 = __out;
        }
    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for reduction(min:_arg_max_buf_for_17)
    for (int64_t _i0 = 0; _i0 < LEN_1D; _i0 += 1) {
        {  // identity
            double __out;
            __out = a[a_idx(_i0)];
            *(&_arg_max_buf_for_17) = std::min(*(&_arg_max_buf_for_17), __out);
        }
    }

    x = _arg_max_buf_for_17;
    {

        result[result_idx(0)] = x;  // symassign

    }
}
