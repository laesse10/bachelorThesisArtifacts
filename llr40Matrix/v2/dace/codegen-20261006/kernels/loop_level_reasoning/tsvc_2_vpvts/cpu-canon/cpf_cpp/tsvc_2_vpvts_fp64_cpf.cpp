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
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_vpvts_fp64(double * __restrict__ a, const double * __restrict__ b, int64_t LEN_1D, int64_t S, const uint8_t * __restrict__ workspace, int64_t workspace_size)
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
        double b_slice_times_S;
        double _wcr_priv__Add____out;
        b_slice_times_S = (b[b_idx(_loop_it_0)] * double(S));  // _Mult_
        _wcr_priv__Add____out = b_slice_times_S;  // _Add_
        a[a_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + _wcr_priv__Add____out);  // augassign
    }
}
