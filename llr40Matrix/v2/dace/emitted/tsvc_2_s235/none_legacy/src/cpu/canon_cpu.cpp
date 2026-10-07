/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, int64_t LEN_2D)
{
    double a_index;
    double b_index;
    double c_index;
    double b_slice_times_c_slice;
    double a_slice_plus_b_slice_c_slice;
    double aa_index;
    double bb_index;
    double a_index_0;
    double bb_slice_times_a_slice;
    double aa_slice_plus_bb_slice_a_slice;
    int64_t i;
    int64_t j;


    for (i = 0; (i < LEN_2D); i = (i + 1)) {
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
                double _cpy_in = c[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_c_to_c_index)
                _cpy_out = _cpy_in;
                ///////////////////

                c_index = _cpy_out;
            }
            {
                double __in1 = b_index;
                double __in2 = c_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                b_slice_times_c_slice = __out;
            }
            {
                double __in2 = b_slice_times_c_slice;
                double __in1 = a_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                a_slice_plus_b_slice_c_slice = __out;
            }
            {
                double __inp = a_slice_plus_b_slice_c_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_17_8)
                __out = __inp;
                ///////////////////

                a[i] = __out;
            }

        }

        for (j = 1; (j < LEN_2D); j = (j + 1)) {
            {

                {
                    double _cpy_in = aa[((LEN_2D * (j - 1)) + i)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_aa_to_aa_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    aa_index = _cpy_out;
                }
                {
                    double _cpy_in = bb[((LEN_2D * j) + i)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_bb_to_bb_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    bb_index = _cpy_out;
                }
                {
                    double _cpy_in = a[i];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_a_to_a_index_0)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    a_index_0 = _cpy_out;
                }
                {
                    double __in2 = a_index_0;
                    double __in1 = bb_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    bb_slice_times_a_slice = __out;
                }
                {
                    double __in2 = bb_slice_times_a_slice;
                    double __in1 = aa_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    aa_slice_plus_bb_slice_a_slice = __out;
                }
                {
                    double __inp = aa_slice_plus_bb_slice_a_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_19_12)
                    __out = __inp;
                    ///////////////////

                    aa[((LEN_2D * j) + i)] = __out;
                }

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, b, bb, c, LEN_2D);
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
