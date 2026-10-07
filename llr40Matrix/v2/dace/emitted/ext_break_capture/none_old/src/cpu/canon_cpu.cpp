/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    double out_value_slice;
    int64_t i;
    double a_index;

    {

        {
            int64_t __out;

            ///////////////////
            // Tasklet code (assign_18_4)
            __out = -1;
            ///////////////////

            out_index[0] = __out;
        }
        {
            double __out;

            ///////////////////
            // Tasklet code (assign_19_4)
            __out = -1.0;
            ///////////////////

            out_value[0] = __out;
        }

    }

    for (i = 0; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];

        if ((a_index > 1)) {
            {

                {
                    int64_t __out;

                    ///////////////////
                    // Tasklet code (assign_22_12)
                    __out = i;
                    ///////////////////

                    out_index[0] = __out;
                }

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + i, &out_value_slice, 1);
                {
                    double __inp = out_value_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_23_12)
                    __out = __inp;
                    ///////////////////

                    out_value[0] = __out;
                }

            }
            break;
        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out_index, out_value, LEN_1D);
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
