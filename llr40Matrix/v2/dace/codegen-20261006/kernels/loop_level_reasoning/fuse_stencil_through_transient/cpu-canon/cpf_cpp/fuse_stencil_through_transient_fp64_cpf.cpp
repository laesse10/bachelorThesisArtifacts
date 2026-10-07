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
static constexpr inline int64_t out_idx(int64_t __d0) { return __d0; }
extern "C" void fuse_stencil_through_transient_fp64(const double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _loop_it_1 = 1; _loop_it_1 < (LEN_1D - 2); _loop_it_1 += 1) {
        double __tmp1;
        double __tmp3;
        double __tmp0;
        double __tmp2;
        __tmp1 = (a[a_idx((_loop_it_1 - 1))] + a[a_idx(_loop_it_1)]);  // _Add_
        __tmp0 = (__tmp1 + a[a_idx((_loop_it_1 + 1))]);  // _Add_
        __tmp3 = (a[a_idx(_loop_it_1)] + a[a_idx((_loop_it_1 + 1))]);  // _Add_
        __tmp2 = (__tmp3 + a[a_idx((_loop_it_1 + 2))]);  // _Add_
        out[out_idx(_loop_it_1)] = (__tmp0 * __tmp2);  // _Mult_
    }
}
