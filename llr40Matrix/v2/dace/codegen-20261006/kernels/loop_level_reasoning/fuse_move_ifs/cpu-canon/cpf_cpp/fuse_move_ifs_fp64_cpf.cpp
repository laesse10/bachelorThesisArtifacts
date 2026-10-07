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
static constexpr inline int64_t src_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t b_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static inline void loop_body_0_1_3(const double* __restrict__ cond, const double* __restrict__ src, double* __restrict__ a, double* __restrict__ b, int64_t K, int64_t LEN_2D, int64_t _loop_it_0, int64_t _loop_it_1) {
    double cond_index;


    cond_index = cond[_loop_it_0];
    if (((cond_index > 0.0) && (K > 0))) {
        {

            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] * 2.0);  // _Mult_
            b[b_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] + 1.0);  // _Add_

        }
    } else if (((! (cond_index > 0.0)) && (K > 0))) {
        {

            b[b_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] + 1.0);  // _Add_

        }
    } else if (((cond_index > 0.0) && (! (K > 0)))) {
        {

            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] * 2.0);  // _Mult_

        }
    }
}

extern "C" void fuse_move_ifs_fp64(double * __restrict__ a, double * __restrict__ b, const double * __restrict__ cond, const double * __restrict__ src, int64_t K, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
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
    for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_2D; _loop_it_0 += 1) {
        for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
            loop_body_0_1_3(&cond[0], &src[0], &a[0], &b[0], K, LEN_2D, _loop_it_0, _loop_it_1);
        }
    }
}
