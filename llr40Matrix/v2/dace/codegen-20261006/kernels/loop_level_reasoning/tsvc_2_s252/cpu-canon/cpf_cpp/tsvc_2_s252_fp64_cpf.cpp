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
constexpr double t = 0.0;
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s252_fp64(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double s_0;

        s_0 = (b[b_idx(0)] * c[c_idx(0)]);  // _Mult_
        {  // _Add_
            double __in2 = t;
            a[a_idx(0)] = (s_0 + __in2);
        }
        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t _loop_it_0 = 1; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
            double s_1;
            double t_remat;
            double t_remat_0;
            s_1 = (b[b_idx(_loop_it_0)] * c[c_idx(_loop_it_0)]);  // _Mult_
            t_remat = (b[b_idx((_loop_it_0 - 1))] * c[c_idx((_loop_it_0 - 1))]);  // _Mult__remat
            t_remat_0 = (t_remat + 0.0);  // _Add__remat
            a[a_idx(_loop_it_0)] = (s_1 + t_remat_0);  // _Add_
        }

    }
}
