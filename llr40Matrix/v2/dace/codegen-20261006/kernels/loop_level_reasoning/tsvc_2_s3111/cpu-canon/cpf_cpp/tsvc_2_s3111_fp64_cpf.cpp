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
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s3111_fp64(const double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
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

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for reduction(+:_priv_sum_val)
        for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
            double sum_val_plus_a_slice;
            double sum_val_masked_val;
            double _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
            sum_val_masked_val = ((a[a_idx(_loop_it_0)] > 0.0) ? a[a_idx(_loop_it_0)] : 0.0);  // sum_val_mask
            sum_val_plus_a_slice = sum_val_masked_val;  // _Add_
            _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out = sum_val_plus_a_slice;  // _assign_out_sum_val_plus_a_slice_to__priv_sum_val
            {  // copy__wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out_to__priv_sum_val
                double _cpy_out;
                _cpy_out = _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
                *(&_priv_sum_val) = *(&_priv_sum_val) + (_cpy_out);
            }
        }
        {  // _assign_sum_val_0_to_b
            double _in = _priv_sum_val;
            b[b_idx(0)] = _in;
        }

    }
}
