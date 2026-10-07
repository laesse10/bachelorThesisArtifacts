/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    double b_index;
    double c_index;
    double d_index;
    double c_slice_times_d_slice;
    double a_slice;
    double a_index;
    double c_index_0;
    double e_index;
    double c_slice_times_e_slice;
    double b_slice;
    int64_t i;


    for (i = 1; (i < LEN_1D); i = (i + 1)) {
        {

            {
                double _cpy_in = b[(i - 1)];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_b_to_b_index)
                _cpy_out = _cpy_in;
                ///////////////////

                b_index = _cpy_out;
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
                double _cpy_in = c[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_c_to_c_index_0)
                _cpy_out = _cpy_in;
                ///////////////////

                c_index_0 = _cpy_out;
            }
            {
                double _cpy_in = d[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_d_to_d_index)
                _cpy_out = _cpy_in;
                ///////////////////

                d_index = _cpy_out;
            }
            {
                double __in2 = d_index;
                double __in1 = c_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                c_slice_times_d_slice = __out;
            }
            {
                double __in2 = c_slice_times_d_slice;
                double __in1 = b_index;
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
                double _cpy_in = e[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_e_to_e_index)
                _cpy_out = _cpy_in;
                ///////////////////

                e_index = _cpy_out;
            }
            {
                double __in2 = e_index;
                double __in1 = c_index_0;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                c_slice_times_e_slice = __out;
            }
            {
                double __in1 = a_index;
                double __in2 = c_slice_times_e_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice = __out;
            }
            {
                double __inp = b_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_18_8)
                __out = __inp;
                ///////////////////

                b[i] = __out;
            }

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, e, LEN_1D);
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
