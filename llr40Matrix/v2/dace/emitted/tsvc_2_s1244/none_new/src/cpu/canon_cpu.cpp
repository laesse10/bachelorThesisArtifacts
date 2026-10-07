/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
{
    double b_index;
    double c_index;
    double c_index_0;
    double c_slice_times_c_slice;
    double b_slice_plus_c_slice_c_slice;
    double b_index_0;
    double b_index_1;
    double b_slice_times_b_slice;
    double b_slice_c_slice_c_slice_plus_b_slice_b_slice;
    double c_index_1;
    double a_slice;
    double a_index;
    double a_index_0;
    double d_slice;


    for (int64_t i = 0; (i < (LEN_1D - 1)); i = (i + 1)) {
        {

            b_index = b[b_idx(i)];  // copy_b_to_b_index
            b_index_0 = b[b_idx(i)];  // copy_b_to_b_index_0
            b_index_1 = b[b_idx(i)];  // copy_b_to_b_index_1
            b_slice_times_b_slice = (b_index_0 * b_index_1);  // _Mult_
            c_index = c[c_idx(i)];  // copy_c_to_c_index
            c_index_0 = c[c_idx(i)];  // copy_c_to_c_index_0
            c_slice_times_c_slice = (c_index * c_index_0);  // _Mult_
            b_slice_plus_c_slice_c_slice = (b_index + c_slice_times_c_slice);  // _Add_
            b_slice_c_slice_c_slice_plus_b_slice_b_slice = (b_slice_plus_c_slice_c_slice + b_slice_times_b_slice);  // _Add_
            c_index_1 = c[c_idx(i)];  // copy_c_to_c_index_1
            a_slice = (b_slice_c_slice_c_slice_plus_b_slice_b_slice + c_index_1);  // _Add_
            a[a_idx(i)] = a_slice;  // assign_17_8
            a_index = a[a_idx(i)];  // copy_a_to_a_index
            a_index_0 = a[a_idx((i + 1))];  // copy_a_to_a_index_0
            d_slice = (a_index + a_index_0);  // _Add_
            d[d_idx(i)] = d_slice;  // assign_18_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, LEN_1D);
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
