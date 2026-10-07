/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

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
    int64_t i;

    {


        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        b + (LEN_1D - 1), &x, 1);

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        b + (LEN_1D - 2), &y, 1);

    }

    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &b_index, 1);
            {
                double __in2 = x;
                double __in1 = b_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice_plus_x = __out;
            }
            {
                double __in2 = y;
                double __in1 = b_slice_plus_x;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice_x_plus_y = __out;
            }
            {
                double __in1 = b_slice_x_plus_y;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * 0.333);
                ///////////////////

                a_slice = __out;
            }
            {
                double __inp = a_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_19_8)
                __out = __inp;
                ///////////////////

                a[i] = __out;
            }
            {
                double __in1 = x;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + 0.0);
                ///////////////////

                y_0 = __out;
            }
            {
                double __inp = y_0;
                double __out;

                ///////////////////
                // Tasklet code (assign_20_8)
                __out = __inp;
                ///////////////////

                y = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &x_0, 1);
            {
                double __inp = x_0;
                double __out;

                ///////////////////
                // Tasklet code (assign_21_8)
                __out = __inp;
                ///////////////////

                x = __out;
            }

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
