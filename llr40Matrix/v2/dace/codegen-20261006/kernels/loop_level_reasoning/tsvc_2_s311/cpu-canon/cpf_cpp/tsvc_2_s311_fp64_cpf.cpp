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
static constexpr inline int64_t sum_out_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s311_fp64(const double * __restrict__ a, double * __restrict__ sum_out, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {
        double* _out;
        _out = &sum_out[0];

        sum_out[sum_out_idx(0)] = 0.0;  // assign_16_4
        // reduction over the given axes with the given operator
        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t _i0 = 0; _i0 < LEN_1D; _i0 += 1) {
            {  // identity
                double __out;
                __out = a[a_idx(_i0)];
                // conflicting accumulation on _out[0] -- NOT parallel-reduced: the writers can collide, so this serializes.
                // Reducing it into a tree is an optimization the canonicalization did not find.
                #pragma omp atomic update
                *(_out) = *(_out) + (__out);
            }
        }

    }
}
