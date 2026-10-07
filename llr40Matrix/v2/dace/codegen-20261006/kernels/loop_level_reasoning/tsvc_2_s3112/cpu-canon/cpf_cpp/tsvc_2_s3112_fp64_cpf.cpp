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

// Functions the DaCe runtime headers would otherwise provide.
template <typename It, typename OutIt, typename T>
static inline void scan_incl_sum(It f, OutIt o, long lo, long hi, T seed) {
    T acc = seed;
    #pragma omp simd reduction(inscan, +:acc)
    for (long i = lo; i < hi; ++i) {
        acc = acc + f[i];
        #pragma omp scan inclusive(acc)
        o[i] = acc;
    }
}
constexpr double __sum = 0.0;
extern "C" void tsvc_2_s3112_fp64(const double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {
        double _scan_seed_b;

        {  // _assign___sum_to__scan_seed_b
            double _in = __sum;
            double _out;
            _out = _in;
            _scan_seed_b = _out;
        }
        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_19_scan_op_op
            scan_incl_sum(a, b, 0L, static_cast<long>(LEN_1D), _scan_seed_b);
        }

    }
}
