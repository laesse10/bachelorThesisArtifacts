/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t y_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t x_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    double c_index;
    double y_index;
    double c_slice_times_y_slice;
    double x_index;
    double c_slice_y_slice_plus_x_slice;


    for (int64_t i = 1; (i < LEN_1D); i = (i + 1)) {
        {

            c_index = c[c_idx(i)];  // copy_c_to_c_index
            y_index = y[y_idx((i - 1))];  // copy_y_to_y_index
            c_slice_times_y_slice = (c_index * y_index);  // _Mult_
            x_index = x[x_idx(i)];  // copy_x_to_x_index
            c_slice_y_slice_plus_x_slice = (c_slice_times_y_slice + x_index);  // _Add_
            y[y_idx(i)] = c_slice_y_slice_plus_x_slice;  // assign_17_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, c, x, y, LEN_1D);
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
