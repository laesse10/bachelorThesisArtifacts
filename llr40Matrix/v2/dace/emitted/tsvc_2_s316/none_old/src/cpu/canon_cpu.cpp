/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    double x;
    double x_0;
    int64_t i;
    double a_index;
    bool __tmp0;

    {


        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        a, &x, 1);

    }

    for (i = 1; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];

        __tmp0 = (a_index < x);

        if (__tmp0) {
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + i, &x_0, 1);
                {
                    double __inp = x_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_19_12)
                    __out = __inp;
                    ///////////////////

                    x = __out;
                }

            }
        }


    }

    {

        {
            double __inp = x;
            double __out;

            ///////////////////
            // Tasklet code (assign_20_4)
            __out = __inp;
            ///////////////////

            result[0] = __out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, result, LEN_1D);
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
