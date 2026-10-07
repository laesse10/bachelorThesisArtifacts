/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    double a_index;
    double __tmp0;
    double b_index;
    double c_index;
    double b_slice_times_c_slice;
    double __tmp1;


    for (int64_t i = K; (i < LEN_1D); i = (i + 1)) {
        {

            a_index = a[a_idx(((- K) + i))];  // copy_a_to_a_index
            __tmp0 = (0.75 * a_index);  // _Mult_
            b_index = b[b_idx(i)];  // copy_b_to_b_index
            c_index = c[c_idx(i)];  // copy_c_to_c_index
            b_slice_times_c_slice = (b_index * c_index);  // _Mult_
            __tmp1 = (__tmp0 + b_slice_times_c_slice);  // _Add_
            a[a_idx(i)] = __tmp1;  // assign_18_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, K, LEN_1D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t K, int64_t LEN_1D)
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
