/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_value_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_index_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    double x;
    double x_0;
    int64_t idx;
    double a_index;
    bool __tmp0;

    {

        x = a[a_idx(0)];  // copy_a_to_x

    }
    idx = 0;

    for (int64_t i = 1; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];

        __tmp0 = (a_index > x);

        if (__tmp0) {
            {

                x_0 = a[a_idx(i)];  // copy_a_to_x_0
                x = x_0;  // assign_20_12

            }
            idx = i;

        }


    }

    {

        out_value[out_value_idx(0)] = x;  // assign_22_4
        out_index[out_index_idx(0)] = idx;  // assign_23_4

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out_index, out_value, LEN_1D);
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
