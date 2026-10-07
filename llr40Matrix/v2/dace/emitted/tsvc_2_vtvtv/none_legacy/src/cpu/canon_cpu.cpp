/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
{
    double a_index;
    double b_index;
    double a_slice_times_b_slice;
    double c_index;
    double a_slice_b_slice_times_c_slice;
    int64_t i;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

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
                double __in2 = b_index;
                double __in1 = a_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                a_slice_times_b_slice = __out;
            }
            {
                double _cpy_in = c[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_c_to_c_index)
                _cpy_out = _cpy_in;
                ///////////////////

                c_index = _cpy_out;
            }
            {
                double __in2 = c_index;
                double __in1 = a_slice_times_b_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                a_slice_b_slice_times_c_slice = __out;
            }
            {
                double __inp = a_slice_b_slice_times_c_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_17_8)
                __out = __inp;
                ///////////////////

                a[i] = __out;
            }

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, LEN_1D);
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
