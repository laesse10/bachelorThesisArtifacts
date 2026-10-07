/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    double dot_index;
    double a_index;
    double b_index;
    double a_slice_times_b_slice;
    double dot_slice_plus_a_slice_b_slice;
    int64_t i;

    {

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_16_4)
            __out = 0.0;
            ///////////////////

            dot[0] = __out;
        }

    }

    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            {
                double _cpy_in = dot[0];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_dot_to_dot_index)
                _cpy_out = _cpy_in;
                ///////////////////

                dot_index = _cpy_out;
            }
            {
                double _cpy_in = a[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_a_to_a_index)
                _cpy_out = _cpy_in;
                ///////////////////

                a_index = _cpy_out;
            }
            {
                double _cpy_in = b[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_b_to_b_index)
                _cpy_out = _cpy_in;
                ///////////////////

                b_index = _cpy_out;
            }
            {
                double __in1 = a_index;
                double __in2 = b_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                a_slice_times_b_slice = __out;
            }
            {
                double __in2 = a_slice_times_b_slice;
                double __in1 = dot_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                dot_slice_plus_a_slice_b_slice = __out;
            }
            {
                double __inp = dot_slice_plus_a_slice_b_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_18_8)
                __out = __inp;
                ///////////////////

                dot[0] = __out;
            }

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, dot, LEN_1D);
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
