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
extern "C" void tsvc_2_s255_fp64(double * __restrict__ a, const double * __restrict__ b, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double y_2;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double x;
        double y_1;
        double b_slice_plus_x_0;
        double b_slice_x_plus_y_0;

        x = b[b_idx((LEN_1D - 1))];  // _assign_b_to_x
        b_slice_plus_x_0 = (b[b_idx(0)] + x);  // _Add_
        y_1 = b[b_idx((LEN_1D - 2))];  // _assign_b_to_y_1
        b_slice_x_plus_y_0 = (b_slice_plus_x_0 + y_1);  // _Add_
        y_2 = (x + 0.0);  // _Add_
        a[a_idx(0)] = (b_slice_x_plus_y_0 * 0.333);  // _Mult_

    }
    {
        double b_index_1;
        double b_slice_plus_x_1;
        double b_slice_x_plus_y_1;

        b_index_1 = b[b_idx(1)];  // _assign_b_to_b_index_1
        b_slice_plus_x_1 = (b_index_1 + b[b_idx(0)]);  // _Add_
        b_slice_x_plus_y_1 = (b_slice_plus_x_1 + y_2);  // _Add_
        a[a_idx(1)] = (b_slice_x_plus_y_1 * 0.333);  // _Mult_

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _loop_it_0 = 2; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
        double b_index_2;
        double b_slice_plus_x_2;
        double b_slice_x_plus_y_2;
        double y_remat;
        y_remat = (b[b_idx((_loop_it_0 - 2))] + 0.0);  // _Add__remat
        b_index_2 = b[b_idx(_loop_it_0)];  // _assign_b_to_b_index_2
        b_slice_plus_x_2 = (b_index_2 + b[b_idx((_loop_it_0 - 1))]);  // _Add_
        b_slice_x_plus_y_2 = (b_slice_plus_x_2 + y_remat);  // _Add_
        a[a_idx(_loop_it_0)] = (b_slice_x_plus_y_2 * 0.333);  // _Mult_
    }
}
