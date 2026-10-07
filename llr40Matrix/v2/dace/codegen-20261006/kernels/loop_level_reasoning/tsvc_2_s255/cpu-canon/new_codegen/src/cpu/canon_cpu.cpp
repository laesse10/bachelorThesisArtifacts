/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double y_2;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double x;
        double y_1;
        double b_slice_plus_x_0;
        double b_slice_x_plus_y_0;

        x = b[b_idx((LEN_1D - 1))];  // _assign_b_to_x
        b_slice_plus_x_0 = (b[b_idx(0)] + x);  // _Add_
        y_1 = b[b_idx((LEN_1D - 2))];  // _assign_b_to_y_1
        b_slice_x_plus_y_0 = (b_slice_plus_x_0 + y_1);  // _Add_
        a[a_idx(0)] = (b_slice_x_plus_y_0 * 0.333);  // _Mult_
        y_2 = (x + 0.0);  // _Add_

    }
    {
        double b_index_1;
        double b_slice_plus_x_1;
        double b_slice_x_plus_y_1;

        b_index_1 = b[b_idx(1)];  // _assign_b_to_b_index_1
        b_slice_plus_x_1 = (b_index_1 + b[b_idx(0)]);  // _Add_
        b_slice_x_plus_y_1 = (b_slice_plus_x_1 + y_2);  // _Add_
        a[a_idx(1)] = (b_slice_x_plus_y_1 * 0.333);  // _Mult_

    }

    #pragma omp parallel for
    for (int64_t _loop_it_0 = 2; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
        double b_index_2;
        double b_slice_plus_x_2;
        double b_slice_x_plus_y_2;
        double y_remat;
        b_index_2 = b[b_idx(_loop_it_0)];  // _assign_b_to_b_index_2
        b_slice_plus_x_2 = (b_index_2 + b[b_idx((_loop_it_0 - 1))]);  // _Add_
        y_remat = (b[b_idx((_loop_it_0 - 2))] + 0.0);  // _Add__remat
        b_slice_x_plus_y_2 = (b_slice_plus_x_2 + y_remat);  // _Add_
        a[a_idx(_loop_it_0)] = (b_slice_x_plus_y_2 * 0.333);  // _Mult_
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
