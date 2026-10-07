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
static constexpr inline int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static inline void nested_sdfg_0_1_0(double* __restrict__ a, int64_t LEN_2D) {

    // sequential -- carried: RAW on a[_loop_it_0 - 1, _loop_it_1]
    // settled: proven, an iteration reads what an earlier one wrote; focus: restructure around the dependence (reorder, block, rewrite as a scan or reduction), or optimize the work inside
    for (int64_t _loop_it_0 = 1; (_loop_it_0 < LEN_2D); _loop_it_0 = (_loop_it_0 + 1)) {

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp for
        for (int64_t _loop_it_1 = 0; _loop_it_1 < (LEN_2D - 1); _loop_it_1 += 1) {
            double a_slice_a_slice_plus_a_slice;
            double a_index_0;
            double a_slice_plus_a_slice;
            double a_index_1;
            double a_index;
            a_index = a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index
            a_index_0 = a[a_idx((_loop_it_0 - 1), _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index_0
            a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
            a_index_1 = a[a_idx((_loop_it_0 - 1), (_loop_it_1 + 1), LEN_2D)];  // _assign_in_a_to_a_index_1
            a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = a_slice_a_slice_plus_a_slice;  // _assign_out_a_slice_a_slice_plus_a_slice_to_a
        }

    }
}

extern "C" void wf_diff_skew_fp64(double * __restrict__ a, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
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
    {
        #pragma omp parallel
        {
            nested_sdfg_0_1_0(&a[0], LEN_2D);
        }
    }
}
