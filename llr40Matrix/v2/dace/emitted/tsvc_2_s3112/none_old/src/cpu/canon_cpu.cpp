/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double __sum;
    double a_index;
    double __sum_plus_a_slice;
    int64_t i;

    {

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_18_4)
            __out = 0.0;
            ///////////////////

            __sum = __out;
        }

    }

    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            a + i, &a_index, 1);
            {
                double __in1 = __sum;
                double __in2 = a_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                __sum_plus_a_slice = __out;
            }
            {
                double __inp = __sum_plus_a_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_20_8)
                __out = __inp;
                ///////////////////

                __sum = __out;
            }
            {
                double __inp = __sum;
                double __out;

                ///////////////////
                // Tasklet code (assign_21_8)
                __out = __inp;
                ///////////////////

                b[i] = __out;
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
