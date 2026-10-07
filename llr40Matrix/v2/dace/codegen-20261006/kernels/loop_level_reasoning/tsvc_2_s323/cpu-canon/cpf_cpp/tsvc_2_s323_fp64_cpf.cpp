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
static constexpr inline int64_t _scan_in_b_size(int64_t LEN_1D) { return (LEN_1D - 1); }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t e_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _scan_in_b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s323_fp64(double * __restrict__ a, double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double* __restrict__ _scan_in_b = new (std::align_val_t(64)) double[_scan_in_b_size(LEN_1D)];

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double _scan_seed_b;

        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t _loop_it_1 = 1; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
            double c_slice_times_d_slice;
            double c_slice_times_e_slice;
            double b_slice;
            double a_fwd;
            c_slice_times_d_slice = (c[c_idx(_loop_it_1)] * d[d_idx(_loop_it_1)]);  // _Mult_
            a_fwd = c_slice_times_d_slice;  // _assign_c_slice_times_d_slice_to_a_fwd
            c_slice_times_e_slice = (c[c_idx(_loop_it_1)] * e[e_idx(_loop_it_1)]);  // _Mult_
            b_slice = (a_fwd + c_slice_times_e_slice);  // _Add_
            _scan_in_b[_scan_in_b_idx((_loop_it_1 - 1))] = b_slice;  // _assign_out_b_slice_to__scan_in_b
        }
        {  // _assign_b_to__scan_seed_b
            double _out;
            _out = b[b_idx(0)];
            _scan_seed_b = _out;
        }
        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_16_0_scan_op
            scan_incl_sum(_scan_in_b, (b + 1), 0L, static_cast<long>((LEN_1D - 1)), _scan_seed_b);
        }
        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t _loop_it_2 = 1; _loop_it_2 < LEN_1D; _loop_it_2 += 1) {
            double nested_sdfg_c_slice_times_d_slice;
            nested_sdfg_c_slice_times_d_slice = (c[c_idx(_loop_it_2)] * d[d_idx(_loop_it_2)]);  // _Mult_
            a[a_idx(_loop_it_2)] = (b[b_idx((_loop_it_2 - 1))] + nested_sdfg_c_slice_times_d_slice);  // _Add_
        }

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_scan_in_b, std::align_val_t(64));
}
