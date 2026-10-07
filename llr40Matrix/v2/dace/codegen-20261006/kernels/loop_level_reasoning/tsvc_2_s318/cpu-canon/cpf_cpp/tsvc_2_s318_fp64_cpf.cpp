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
static constexpr inline int64_t result_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s318_fp64(const double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double _argfi_val_for_20;
    int64_t _argfi_idx_for_20;
    double maxv;
    int64_t index;

    {

        // argument reduction: index of the element the reduction selected
        {  // for_20_argfi_argreduce_openmp
            const double* __restrict__ _in = &a[0];
            int64_t _out_idx;
            double _out_val;
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = std::abs(_in[(0) * (inc)]); __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < (LEN_1D); ++__i) {
                const double __v = std::abs(_in[(__i) * (inc)]);
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            _argfi_idx_for_20 = _out_idx;
            _argfi_val_for_20 = _out_val;
        }

    }
    maxv = _argfi_val_for_20;
    index = _argfi_idx_for_20;
    {
        double float_index;

        {  // _convert_to_float64_
            float_index = double(index);
        }
        result[result_idx(0)] = (maxv + float_index);  // _Add_

    }
}
