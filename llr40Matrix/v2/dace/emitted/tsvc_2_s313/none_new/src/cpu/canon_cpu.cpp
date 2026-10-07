/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t dot_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    double dot_index;
    double a_index;
    double b_index;
    double a_slice_times_b_slice;
    double dot_slice_plus_a_slice_b_slice;

    {

        dot[dot_idx(0)] = 0.0;  // assign_16_4

    }

    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            dot_index = dot[dot_idx(0)];  // copy_dot_to_dot_index
            a_index = a[a_idx(i)];  // copy_a_to_a_index
            b_index = b[b_idx(i)];  // copy_b_to_b_index
            a_slice_times_b_slice = (a_index * b_index);  // _Mult_
            dot_slice_plus_a_slice_b_slice = (dot_index + a_slice_times_b_slice);  // _Add_
            dot[dot_idx(0)] = dot_slice_plus_a_slice_b_slice;  // assign_18_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, dot, LEN_1D);
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
