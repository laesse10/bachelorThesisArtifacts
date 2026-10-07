/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t LEN_2D)
{
    double a_index;
    double a_index_0;
    double a_slice_plus_a_slice;
    double a_index_1;
    double a_slice_a_slice_plus_a_slice;
    double a_slice_a_slice_a_slice_div_3_0;
    int64_t i;
    int64_t j;


    for (i = 1; (i < LEN_2D); i = (i + 1)) {

        for (j = i; (j < LEN_2D); j = (j + 1)) {
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + ((LEN_2D * i) + j), &a_index, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + ((LEN_2D * (i - 1)) + j), &a_index_0, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + (((LEN_2D * i) + j) - 1), &a_index_1, 1);
                {
                    double __in1 = a_index;
                    double __in2 = a_index_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a_slice_plus_a_slice = __out;
                }
                {
                    double __in2 = a_index_1;
                    double __in1 = a_slice_plus_a_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a_slice_a_slice_plus_a_slice = __out;
                }
                {
                    double __in1 = a_slice_a_slice_plus_a_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Div_)
                    __out = (__in1 / 3.0);
                    ///////////////////

                    a_slice_a_slice_a_slice_div_3_0 = __out;
                }
                {
                    double __inp = a_slice_a_slice_a_slice_div_3_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_18_12)
                    __out = __inp;
                    ///////////////////

                    a[((LEN_2D * i) + j)] = __out;
                }

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, LEN_2D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_2D)
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
