/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{
    double acc;
    double val_index;
    double w_index;
    double val_slice_times_w_slice;
    double acc_plus_val_slice_w_slice;
    int64_t s;
    int64_t row_ptr_index;
    int64_t row_ptr_index_0;
    int64_t e;


    for (s = 0; (s < NSEG); s = (s + 1)) {
        {

            {
                double __out;

                ///////////////////
                // Tasklet code (assign_17_8)
                __out = 0.0;
                ///////////////////

                acc = __out;
            }

        }
        row_ptr_index = row_ptr[s];
        row_ptr_index_0 = row_ptr[(s + 1)];

        for (e = row_ptr_index; (e < row_ptr_index_0); e = (e + 1)) {
            {

                {
                    double _cpy_in = val[e];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_val_to_val_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    val_index = _cpy_out;
                }
                {
                    double _cpy_in = w[e];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_w_to_w_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    w_index = _cpy_out;
                }
                {
                    double __in2 = w_index;
                    double __in1 = val_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    val_slice_times_w_slice = __out;
                }
                {
                    double __in1 = acc;
                    double __in2 = val_slice_times_w_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    acc_plus_val_slice_w_slice = __out;
                }
                {
                    double __inp = acc_plus_val_slice_w_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_19_12)
                    __out = __inp;
                    ///////////////////

                    acc = __out;
                }

            }

        }

        {

            {
                double __inp = acc;
                double __out;

                ///////////////////
                // Tasklet code (assign_20_8)
                __out = __inp;
                ///////////////////

                out[s] = __out;
            }

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{
    __program_canon_cpu_internal(__state, out, row_ptr, val, w, NSEG);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t NSEG)
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
