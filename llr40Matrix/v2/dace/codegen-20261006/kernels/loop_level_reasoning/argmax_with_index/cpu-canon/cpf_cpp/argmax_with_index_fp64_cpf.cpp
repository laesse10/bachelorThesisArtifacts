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
static constexpr inline int64_t out_index_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t out_value_idx(int64_t __d0) { return __d0; }
extern "C" void argmax_with_index_fp64(const double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double _argmax_val_for_18;
    int64_t _argmax_idx_for_18;
    double x;
    int64_t idx;

    {

        // argument reduction: index of the element the reduction selected
        {  // for_18_argreduce_openmp
            const double* __restrict__ _in = &a[0];
            int64_t _out_idx;
            double _out_val;
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = _in[0]; __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < (LEN_1D); ++__i) {
                const double __v = _in[__i];
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            _argmax_idx_for_18 = _out_idx;
            _argmax_val_for_18 = _out_val;
        }

    }
    x = _argmax_val_for_18;
    idx = _argmax_idx_for_18;
    {

        out_index[out_index_idx(0)] = idx;  // assign_23_4
        out_value[out_value_idx(0)] = x;  // symassign

    }
}
