/* extractedMatrix-style follow-up variant `dseq` of DaCe's canon/new code for compact_threshold_pack
   (llr40Matrix/v2/dace/emitted/compact_threshold_pack/canon_new): the mask pass, the exclusive scan, the sum and the scatter pass replaced by one sequential
   compaction loop, as the translated C++ does it. Allocation of the (now unused) mask/rank arrays unchanged. */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "cstring"
#include "numeric"
#include "functional"
#include "algorithm"
#include "dace/scan.hpp"

struct canon_cpu_state_t {
    int8_t * __restrict__ __0_compaction_mask_for_17;
    int64_t * __restrict__ __0_compaction_rank_for_17;
};

static DACE_HDFI constexpr int64_t compaction_mask_for_17_idx(int64_t __d0) { return __d0; }
inline void loop_body_0_2_0(canon_cpu_state_t *__state, const double* __restrict__ src, int8_t* __restrict__ compaction_mask_for_17, int64_t compaction_it_for_17_compaction_mask) {
    double src_index;


    src_index = src[compaction_it_for_17_compaction_mask];
    {

        compaction_mask_for_17[compaction_mask_for_17_idx(compaction_it_for_17_compaction_mask)] = (src_index > 0.0);  // for_17_compaction_mask_mask

    }
}

static DACE_HDFI constexpr int64_t src_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t weight_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t packed_idx(int64_t __d0) { return __d0; }
inline void loop_body_0_2_7(canon_cpu_state_t *__state, const int64_t* __restrict__ compaction_rank_for_17, const double* __restrict__ src, const double* __restrict__ weight, double* __restrict__ packed, int64_t compaction_it_for_17_compaction_scatter) {
    double src_index;
    int64_t n_0;


    src_index = src[compaction_it_for_17_compaction_scatter];
    n_0 = (0 + (1 * compaction_rank_for_17[compaction_it_for_17_compaction_scatter]));
    if ((src_index > 0.0)) {
        {

            packed[packed_idx(n_0)] = (src[src_idx(compaction_it_for_17_compaction_scatter)] * weight[weight_idx(compaction_it_for_17_compaction_scatter)]);  // _Mult_

        }
    }

}

static DACE_HDFI constexpr int64_t out_count_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    int64_t compaction_total_for_17;
    int64_t n;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {

        /* follow-up `dseq`: the four passes as one sequential compaction loop */
        int64_t n_ = 0;
        for (int64_t i_ = 0; i_ < LEN_1D; ++i_) {
            if (src[i_] > 0.0) {
                packed[n_] = (src[i_] * weight[i_]);
                n_ += 1;
            }
        }
        compaction_total_for_17 = n_;
    }
    n = (0 + (1 * compaction_total_for_17));
    {

        out_count[out_count_idx(0)] = n;  // assign_21_4

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, out_count, packed, src, weight, LEN_1D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_cpu_state_t *__state = new canon_cpu_state_t();

    if (__result) {
        delete __state;
        return nullptr;
    }
    __state->__0_compaction_mask_for_17 = new (std::align_val_t(64)) int8_t[LEN_1D];
    __state->__0_compaction_rank_for_17 = new (std::align_val_t(64)) int64_t[LEN_1D];

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_cpu(canon_cpu_state_t *__state)
{

    int __err = 0;
    static_assert(std::is_trivially_destructible<int8_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_compaction_mask_for_17, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_compaction_rank_for_17, std::align_val_t(64));
    delete __state;
    return __err;
}
