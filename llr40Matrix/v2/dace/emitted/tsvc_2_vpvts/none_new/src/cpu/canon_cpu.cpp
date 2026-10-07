/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D, int64_t S)
{
    double a_index;
    double b_index;
    double b_slice_times_S;
    double a_slice_plus_b_slice_S;


    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            a_index = a[a_idx(i)];  // copy_a_to_a_index
            b_index = b[b_idx(i)];  // copy_b_to_b_index
            b_slice_times_S = (b_index * double(S));  // _Mult_
            a_slice_plus_b_slice_S = (a_index + b_slice_times_S);  // _Add_
            a[a_idx(i)] = a_slice_plus_b_slice_S;  // assign_18_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D, int64_t S)
{
    __program_canon_cpu_internal(__state, a, b, LEN_1D, S);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_1D, int64_t S)
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
