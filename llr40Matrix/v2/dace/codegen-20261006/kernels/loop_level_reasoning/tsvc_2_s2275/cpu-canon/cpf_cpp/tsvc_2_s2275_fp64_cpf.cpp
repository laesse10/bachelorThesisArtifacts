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
static constexpr inline int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s2275_fp64(double * __restrict__ a, double * __restrict__ aa, const double * __restrict__ b, const double * __restrict__ bb, const double * __restrict__ c, const double * __restrict__ cc, const double * __restrict__ d, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
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
    #pragma omp parallel for
    for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
        for (int64_t _loop_it_2 = 0; _loop_it_2 < LEN_2D; _loop_it_2 += 1) {
            double bb_slice_times_cc_slice;
            double _wcr_priv__Add____out;
            bb_slice_times_cc_slice = (bb[bb_idx(_loop_it_1, _loop_it_2, LEN_2D)] * cc[cc_idx(_loop_it_1, _loop_it_2, LEN_2D)]);  // _Mult_
            _wcr_priv__Add____out = bb_slice_times_cc_slice;  // _Add_
            aa[aa_idx(_loop_it_1, _loop_it_2, LEN_2D)] = (aa[aa_idx(_loop_it_1, _loop_it_2, LEN_2D)] + _wcr_priv__Add____out);  // augassign
        }
    }
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _loop_it_3 = 0; _loop_it_3 < LEN_2D; _loop_it_3 += 1) {
        double c_slice_times_d_slice;
        c_slice_times_d_slice = (c[c_idx(_loop_it_3)] * d[d_idx(_loop_it_3)]);  // _Mult_
        a[a_idx(_loop_it_3)] = (b[b_idx(_loop_it_3)] + c_slice_times_d_slice);  // _Add_
    }
}
