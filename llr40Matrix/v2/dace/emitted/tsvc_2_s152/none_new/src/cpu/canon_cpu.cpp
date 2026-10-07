/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    double d_index;
    double e_index;
    double b_slice;
    double a_index;
    double b_index;
    double c_index;
    double b_slice_times_c_slice;
    double a_slice_plus_b_slice_c_slice;
    int64_t i;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            d_index = d[d_idx(i)];  // copy_d_to_d_index
            e_index = e[e_idx(i)];  // copy_e_to_e_index
            b_slice = (d_index * e_index);  // _Mult_
            b[b_idx(i)] = b_slice;  // assign_17_8

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            a_index = a[a_idx(i)];  // copy_a_to_a_index
            b_index = b[b_idx(i)];  // copy_b_to_b_index
            c_index = c[c_idx(i)];  // copy_c_to_c_index
            b_slice_times_c_slice = (b_index * c_index);  // _Mult_
            a_slice_plus_b_slice_c_slice = (a_index + b_slice_times_c_slice);  // _Add_
            a[a_idx(i)] = a_slice_plus_b_slice_c_slice;  // assign_19_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, e, LEN_1D);
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
