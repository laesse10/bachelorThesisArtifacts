/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t LEN_2D)
{
    double a_index;
    double a_index_0;
    double a_slice_plus_a_slice;
    double a_index_1;
    double a_slice_a_slice_plus_a_slice;
    double a_slice_a_slice_a_slice_div_3_0;


    for (int64_t i = 1; (i < LEN_2D); i = (i + 1)) {

        for (int64_t j = i; (j < LEN_2D); j = (j + 1)) {
            {

                a_index = a[a_idx(i, j, LEN_2D)];  // copy_a_to_a_index
                a_index_0 = a[a_idx((i - 1), j, LEN_2D)];  // copy_a_to_a_index_0
                a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
                a_index_1 = a[a_idx(i, (j - 1), LEN_2D)];  // copy_a_to_a_index_1
                a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
                a_slice_a_slice_a_slice_div_3_0 = (a_slice_a_slice_plus_a_slice / 3.0);  // _Div_
                a[a_idx(i, j, LEN_2D)] = a_slice_a_slice_a_slice_div_3_0;  // assign_18_12

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, LEN_2D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_2D)
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
