/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double x;
    double y;
    double b_index;
    double b_slice_plus_x;
    double b_slice_x_plus_y;
    double a_slice;
    double y_0;
    double x_0;

    {

        x = b[b_idx((LEN_1D - 1))];  // copy_b_to_x
        y = b[b_idx((LEN_1D - 2))];  // copy_b_to_y

    }

    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            b_index = b[b_idx(i)];  // copy_b_to_b_index
            b_slice_plus_x = (b_index + x);  // _Add_
            b_slice_x_plus_y = (b_slice_plus_x + y);  // _Add_
            a_slice = (b_slice_x_plus_y * 0.333);  // _Mult_
            a[a_idx(i)] = a_slice;  // assign_19_8
            y_0 = (x + 0.0);  // _Add_
            y = y_0;  // assign_20_8
            x_0 = b[b_idx(i)];  // copy_b_to_x_0
            x = x_0;  // assign_21_8

        }

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
