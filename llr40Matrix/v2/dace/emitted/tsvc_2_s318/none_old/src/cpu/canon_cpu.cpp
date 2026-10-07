/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    int64_t k;
    int64_t index;
    double a_index;
    double maxv;
    int64_t i;
    double a_index_0;
    int64_t k_plus_inc_0;


    k = 0;
    index = 0;
    a_index = a[0];

    maxv = abs(a_index);
    k = (0 + inc);

    for (i = 1; (i < LEN_1D); i = (i + 1)) {

        a_index_0 = a[k];

        if ((abs(a_index_0) > maxv)) {

            index = i;
            maxv = (abs(a_index_0) + 0.0);

        }

        k_plus_inc_0 = (k + inc);

        k = k_plus_inc_0;


    }

    {
        double float_index;
        double result_slice;

        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(index);
            ///////////////////

            float_index = __out;
        }
        {
            double __in2 = float_index;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (maxv + __in2);
            ///////////////////

            result_slice = __out;
        }
        {
            double __inp = result_slice;
            double __out;

            ///////////////////
            // Tasklet code (assign_26_4)
            __out = __inp;
            ///////////////////

            result[0] = __out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    __program_canon_cpu_internal(__state, a, result, LEN_1D, inc);
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
