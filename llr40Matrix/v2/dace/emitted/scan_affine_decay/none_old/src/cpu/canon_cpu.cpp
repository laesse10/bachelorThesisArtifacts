/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    double c_index;
    double y_index;
    double c_slice_times_y_slice;
    double x_index;
    double c_slice_y_slice_plus_x_slice;
    int64_t i;


    for (i = 1; (i < LEN_1D); i = (i + 1)) {
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            y + (i - 1), &y_index, 1);
            {
                double __in1 = c_index;
                double __in2 = y_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                c_slice_times_y_slice = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            x + i, &x_index, 1);
            {
                double __in1 = c_slice_times_y_slice;
                double __in2 = x_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                c_slice_y_slice_plus_x_slice = __out;
            }
            {
                double __inp = c_slice_y_slice_plus_x_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_17_8)
                __out = __inp;
                ///////////////////

                y[i] = __out;
            }

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
