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
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t e_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s319_fp64(double * __restrict__ a, double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double _priv_sum_val;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {

        {  // assign_16_4
            double __out;
            __out = 0.0;
            _priv_sum_val = __out;
        }

    }
    {
        double sum_val_0;

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for reduction(+:_priv_sum_val)
        for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
            double sum_val_plus_a_slice;
            double a_fwd;
            double b_fwd;
            double _fused_inc;
            double _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
            a_fwd = (c[c_idx(_loop_it_1)] + d[d_idx(_loop_it_1)]);  // _Add_
            b_fwd = (c[c_idx(_loop_it_1)] + e[e_idx(_loop_it_1)]);  // _Add_
            _fused_inc = (a_fwd + b_fwd);  // _fuse_red_1
            sum_val_plus_a_slice = _fused_inc;  // _Add_
            _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out = sum_val_plus_a_slice;  // _assign_out_sum_val_plus_a_slice_to__priv_sum_val
            {  // copy__wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out_to__priv_sum_val
                double _cpy_out;
                _cpy_out = _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
                *(&_priv_sum_val) = *(&_priv_sum_val) + (_cpy_out);
            }
            a[a_idx(_loop_it_1)] = (c[c_idx(_loop_it_1)] + d[d_idx(_loop_it_1)]);  // _Add_
            b[b_idx(_loop_it_1)] = (c[c_idx(_loop_it_1)] + e[e_idx(_loop_it_1)]);  // _Add_
        }
        {  // copy__priv_sum_val_to_sum_val_0
            double _cpy_in = _priv_sum_val;
            sum_val_0 = _cpy_in;
        }
        b[b_idx(0)] = sum_val_0;  // _assign_sum_val_0_to_b

    }
}
