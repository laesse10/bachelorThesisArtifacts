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
static constexpr inline int64_t val_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t w_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t out_idx(int64_t __d0) { return __d0; }
static inline void loop_body_0_1_0(const int64_t* __restrict__ row_ptr, const double* __restrict__ val, const double* __restrict__ w, double* __restrict__ out, int64_t _loop_it_0) {
    double acc;
    double _priv_acc;
    int64_t row_ptr_index;
    int64_t row_ptr_index_0;

    {

        acc = 0.0;  // assign_17_8

    }
    row_ptr_index = row_ptr[_loop_it_0];
    row_ptr_index_0 = row_ptr[(_loop_it_0 + 1)];
    {

        {  // copy_acc_to__priv_acc
            double _cpy_out;
            _cpy_out = acc;
            _priv_acc = _cpy_out;
        }

    }
    {

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        double __acc_1_0__priv_acc = *(&_priv_acc);
        for (int64_t _loop_it_1 = row_ptr_index; _loop_it_1 < row_ptr_index_0; _loop_it_1 += 1) {
            double val_slice_times_w_slice;
            double acc_plus_val_slice_w_slice;
            double _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out;
            val_slice_times_w_slice = (val[val_idx(_loop_it_1)] * w[w_idx(_loop_it_1)]);  // _Mult_
            acc_plus_val_slice_w_slice = val_slice_times_w_slice;  // _Add_
            _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out = acc_plus_val_slice_w_slice;  // _assign_out_acc_plus_val_slice_w_slice_to__priv_acc
            {  // copy__wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out_to__priv_acc
                double _cpy_out;
                _cpy_out = _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out;
                __acc_1_0__priv_acc = __acc_1_0__priv_acc + (_cpy_out);
            }
        }
        *(&_priv_acc) = __acc_1_0__priv_acc;
        {  // copy__priv_acc_to_acc
            double _cpy_in = _priv_acc;
            acc = _cpy_in;
        }
        out[out_idx(_loop_it_0)] = acc;  // _assign_acc_to_out

    }
}

extern "C" void segment_reduce_ragged_fp64(double * __restrict__ out, const int64_t * __restrict__ row_ptr, const double * __restrict__ val, const double * __restrict__ w, int64_t NSEG, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {

        {  // check_assumption_0
            if ((NSEG < 0)) {
                std::abort();
            }
        }

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _loop_it_0 = 0; _loop_it_0 < NSEG; _loop_it_0 += 1) {
        loop_body_0_1_0(&row_ptr[0], &val[0], &w[0], &out[0], _loop_it_0);
    }
}
