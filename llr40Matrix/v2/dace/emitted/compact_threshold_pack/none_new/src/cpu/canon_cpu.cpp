/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t src_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t weight_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t packed_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_count_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    double src_index_0;
    double weight_index;
    double packed_slice;
    int64_t n;
    double src_index;


    n = 0;

    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {

        src_index = src[i];

        if ((src_index > 0.0)) {
            {

                src_index_0 = src[src_idx(i)];  // copy_src_to_src_index_0
                weight_index = weight[weight_idx(i)];  // copy_weight_to_weight_index
                packed_slice = (src_index_0 * weight_index);  // _Mult_
                packed[packed_idx(n)] = packed_slice;  // assign_19_12

            }
            n = (n + 1);

        }


    }

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

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_cpu(canon_cpu_state_t *__state)
{

    int __err = 0;
    delete __state;
    return __err;
}
