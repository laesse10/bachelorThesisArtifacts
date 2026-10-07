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
extern "C" void fuse_diamond_fp64(const double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
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
    for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
        double __map_fusion_t;
        double __map_fusion_v;
        double __map_fusion_u;
        __map_fusion_t = (a[a_idx(_loop_it_0)] * a[a_idx(_loop_it_0)]);  // _Mult_
        __map_fusion_u = (__map_fusion_t + 1.0);  // _Add_
        __map_fusion_v = (__map_fusion_t - 1.0);  // _Sub_
        out[out_idx(_loop_it_0)] = (__map_fusion_u * __map_fusion_v);  // _Mult_
    }
}
