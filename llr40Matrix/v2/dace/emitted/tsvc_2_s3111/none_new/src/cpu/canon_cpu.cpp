/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double sum_val;
    double a_index_0;
    double sum_val_plus_a_slice;
    double a_index;

    {

        sum_val = 0.0;  // assign_16_4

    }

    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];

        if ((a_index > 0.0)) {
            {

                a_index_0 = a[a_idx(i)];  // copy_a_to_a_index_0
                sum_val_plus_a_slice = (sum_val + a_index_0);  // _Add_
                sum_val = sum_val_plus_a_slice;  // assign_19_12

            }
        }


    }

    {

        b[b_idx(0)] = sum_val;  // assign_20_4

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, LEN_1D);
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
