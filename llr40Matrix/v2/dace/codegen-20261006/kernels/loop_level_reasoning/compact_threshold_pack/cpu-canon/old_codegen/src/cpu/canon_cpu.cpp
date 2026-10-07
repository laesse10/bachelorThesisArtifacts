/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
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

inline void loop_body_0_2_0(canon_cpu_state_t *__state, const double* __restrict__ src, int8_t* __restrict__ compaction_mask_for_17, int64_t compaction_it_for_17_compaction_mask) {
    double src_index;


    src_index = src[compaction_it_for_17_compaction_mask];
    {

        {
            int8_t __out;

            ///////////////////
            // Tasklet code (for_17_compaction_mask_mask)
            __out = (src_index > 0.0);
            ///////////////////

            compaction_mask_for_17[compaction_it_for_17_compaction_mask] = __out;
        }

    }
}

inline void loop_body_0_2_7(canon_cpu_state_t *__state, const int64_t* __restrict__ compaction_rank_for_17, const double* __restrict__ src, const double* __restrict__ weight, double* __restrict__ packed, int64_t compaction_it_for_17_compaction_scatter) {
    double src_index;
    int64_t n_0;


    src_index = src[compaction_it_for_17_compaction_scatter];
    n_0 = (0 + (1 * compaction_rank_for_17[compaction_it_for_17_compaction_scatter]));
    if ((src_index > 0.0)) {
        {

            {
                double __in1 = src[compaction_it_for_17_compaction_scatter];
                double __in2 = weight[compaction_it_for_17_compaction_scatter];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                packed[n_0] = __out;
            }

        }
    }

}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    int64_t compaction_total_for_17;
    int64_t n;

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {

        {
            #pragma omp parallel for
            for (int64_t compaction_it_for_17_compaction_mask = 0; compaction_it_for_17_compaction_mask < LEN_1D; compaction_it_for_17_compaction_mask += 1) {
                loop_body_0_2_0(__state, &src[0], &__state->__0_compaction_mask_for_17[0], compaction_it_for_17_compaction_mask);
            }
        }
        {
            int8_t* __restrict__ _scan_in = &__state->__0_compaction_mask_for_17[0];
            int64_t* __restrict__ _scan_out = __state->__0_compaction_rank_for_17;

            ///////////////////
            ::dace::scan::exclusive_sum(_scan_in, _scan_in + (LEN_1D), _scan_out, static_cast<int64_t>(0));
            ///////////////////

        }
        {
            int8_t* __restrict__ _reduce_in = &__state->__0_compaction_mask_for_17[0];
            int64_t* __restrict__ _reduce_out = &compaction_total_for_17;

            ///////////////////
            _reduce_out[0] = 0;
            _reduce_out[0] = ::dace::reduce::sum(_reduce_in, (long)(LEN_1D), (long)(1), _reduce_out[0]);
            ///////////////////

        }
        {
            #pragma omp parallel for
            for (int64_t compaction_it_for_17_compaction_scatter = 0; compaction_it_for_17_compaction_scatter < LEN_1D; compaction_it_for_17_compaction_scatter += 1) {
                loop_body_0_2_7(__state, &__state->__0_compaction_rank_for_17[0], &src[0], &weight[0], &packed[0], compaction_it_for_17_compaction_scatter);
            }
        }

    }
    n = (0 + (1 * compaction_total_for_17));
    {

        {
            int64_t __out;

            ///////////////////
            // Tasklet code (assign_21_4)
            __out = n;
            ///////////////////

            out_count[0] = __out;
        }

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
