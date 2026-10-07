/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int * __restrict__ ip, int64_t LEN_1D)
{
    double a_index;
    double b_index;
    double b_slice_times_2_0;
    double a_slice_plus_b_slice_2_0;
    int ip_index;


    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            a_index = a[a_idx(i)];  // copy_a_to_a_index

        }
        ip_index = ip[i];
        {

            b_index = b[b_idx(ip_index)];  // copy_b_to_b_index
            b_slice_times_2_0 = (b_index * 2.0);  // _Mult_
            a_slice_plus_b_slice_2_0 = (a_index + b_slice_times_2_0);  // _Add_
            a[a_idx(i)] = a_slice_plus_b_slice_2_0;  // assign_17_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int * __restrict__ ip, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, ip, LEN_1D);
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
