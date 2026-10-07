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
static constexpr inline int64_t src_idx(int64_t __d0) { return __d0; }
static inline void loop_body_0_1_0(const int* __restrict__ ip, const double* __restrict__ src, double* __restrict__ bins, int64_t _loop_it_0) {
    int ip_index;


    ip_index = ip[_loop_it_0];
    {
        double _wcr_priv__Add____out;

        _wcr_priv__Add____out = src[src_idx(_loop_it_0)];  // _Add_
        // conflicting accumulation on bins[ip_index] -- NOT parallel-reduced: the writers can collide, so this serializes.
        // Reducing it into a tree is an optimization the canonicalization did not find.
        #pragma omp atomic update
        *(bins + ip_index) = *(bins + ip_index) + (*(&_wcr_priv__Add____out));

    }
}

extern "C" void scatter_accum_dup_fp64(double * __restrict__ bins, const int * __restrict__ ip, const double * __restrict__ src, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
        loop_body_0_1_0(&ip[0], &src[0], &bins[0], _loop_it_0);
    }
}
