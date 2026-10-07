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
static inline void scan_excl_sum(It f, OutIt o, long lo, long hi, T seed) {
    T acc = seed;
    #pragma omp simd reduction(inscan, +:acc)
    for (long i = lo; i < hi; ++i) {
        o[i] = acc;
        #pragma omp scan exclusive(acc)
        acc = acc + f[i];
    }
}
static constexpr inline int64_t compaction_mask_for_17_idx(int64_t __d0) { return __d0; }
static inline void loop_body_0_2_2(const double* __restrict__ src, int8_t* __restrict__ compaction_mask_for_17, int64_t compaction_it_for_17_compaction_mask) {
    double src_index;


    src_index = src[compaction_it_for_17_compaction_mask];
    {

        compaction_mask_for_17[compaction_mask_for_17_idx(compaction_it_for_17_compaction_mask)] = static_cast<int8_t>((src_index > 0.0));  // for_17_compaction_mask_mask

    }
}

static constexpr inline int64_t src_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t weight_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t packed_idx(int64_t __d0) { return __d0; }
static inline void loop_body_0_3_1(const int64_t* __restrict__ compaction_rank_for_17, const double* __restrict__ src, const double* __restrict__ weight, double* __restrict__ packed, int64_t compaction_it_for_17_compaction_scatter) {
    double src_index;
    int64_t n_0;


    src_index = src[compaction_it_for_17_compaction_scatter];
    n_0 = compaction_rank_for_17[compaction_it_for_17_compaction_scatter];
    if ((src_index > 0.0)) {
        {

            packed[packed_idx(n_0)] = (src[src_idx(compaction_it_for_17_compaction_scatter)] * weight[weight_idx(compaction_it_for_17_compaction_scatter)]);  // _Mult_

        }
    }

}

static constexpr inline int64_t out_count_idx(int64_t __d0) { return __d0; }
extern "C" void compact_threshold_pack_fp64(int64_t * __restrict__ out_count, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    int8_t* __restrict__ compaction_mask_for_17 = new (std::align_val_t(64)) int8_t[LEN_1D];
    int64_t* __restrict__ compaction_rank_for_17 = new (std::align_val_t(64)) int64_t[LEN_1D];
    int64_t compaction_total_for_17;
    int64_t n;

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
    for (int64_t compaction_it_for_17_compaction_mask = 0; compaction_it_for_17_compaction_mask < LEN_1D; compaction_it_for_17_compaction_mask += 1) {
        loop_body_0_2_2(&src[0], &compaction_mask_for_17[0], compaction_it_for_17_compaction_mask);
    }

    // reduction over the given axes with the given operator
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _o0 = 0; _o0 < 1; _o0 += 1) {
        {  // reduce_init
            int64_t __out;
            __out = 0;
            compaction_total_for_17 = __out;
        }
    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for reduction(+:compaction_total_for_17)
    for (int64_t _i0 = 0; _i0 < LEN_1D; _i0 += 1) {
        {  // identity
            int64_t __out;
            __out = compaction_mask_for_17[compaction_mask_for_17_idx(_i0)];
            *(&compaction_total_for_17) = *(&compaction_total_for_17) + (__out);
        }
    }
    {

        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_17_compaction_scan_scan
            scan_excl_sum(compaction_mask_for_17, compaction_rank_for_17, 0L, static_cast<long>(LEN_1D), static_cast<int64_t>(0));
        }
        // parallel -- the iterations are independent
        // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
        #pragma omp parallel for
        for (int64_t compaction_it_for_17_compaction_scatter = 0; compaction_it_for_17_compaction_scatter < LEN_1D; compaction_it_for_17_compaction_scatter += 1) {
            loop_body_0_3_1(&compaction_rank_for_17[0], &src[0], &weight[0], &packed[0], compaction_it_for_17_compaction_scatter);
        }

    }
    n = compaction_total_for_17;
    {

        out_count[out_count_idx(0)] = n;  // assign_21_4

    }
    static_assert(std::is_trivially_destructible<int8_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](compaction_mask_for_17, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](compaction_rank_for_17, std::align_val_t(64));
}
