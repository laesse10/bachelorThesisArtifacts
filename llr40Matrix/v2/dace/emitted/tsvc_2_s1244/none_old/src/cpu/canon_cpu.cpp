/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

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
    int64_t i;


    for (i = 0; (i < (LEN_1D - 1)); i = (i + 1)) {
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &b_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &b_index_0, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &b_index_1, 1);
            {
                double __in1 = b_index_0;
                double __in2 = b_index_1;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                b_slice_times_b_slice = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index_0, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index_1, 1);
            {
                double __in1 = c_index;
                double __in2 = c_index_0;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                c_slice_times_c_slice = __out;
            }
            {
                double __in2 = c_slice_times_c_slice;
                double __in1 = b_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice_plus_c_slice_c_slice = __out;
            }
            {
                double __in2 = b_slice_times_b_slice;
                double __in1 = b_slice_plus_c_slice_c_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice_c_slice_c_slice_plus_b_slice_b_slice = __out;
            }
            {
                double __in2 = c_index_1;
                double __in1 = b_slice_c_slice_c_slice_plus_b_slice_b_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                a_slice = __out;
            }
            {
                double __inp = a_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_17_8)
                __out = __inp;
                ///////////////////

                a[i] = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            a + i, &a_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            a + (i + 1), &a_index_0, 1);
            {
                double __in1 = a_index;
                double __in2 = a_index_0;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                d_slice = __out;
            }
            {
                double __inp = d_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_18_8)
                __out = __inp;
                ///////////////////

                d[i] = __out;
            }

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
