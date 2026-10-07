/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    double aa_index;
    double cc_index;
    double aa_slice_plus_cc_slice;
    double bb_index;
    double cc_index_0;
    double bb_slice_plus_cc_slice;
    int64_t i;
    int64_t j;


    for (i = 8; (i < LEN_2D); i = (i + 1)) {

        for (j = 8; (j < LEN_2D); j = (j + 1)) {
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
                    double _cpy_in = cc[((LEN_2D * j) + i)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_cc_to_cc_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    cc_index = _cpy_out;
                }
                {
                    double __in1 = aa_index;
                    double __in2 = cc_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    aa_slice_plus_cc_slice = __out;
                }
                {
                    double __inp = aa_slice_plus_cc_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_18_12)
                    __out = __inp;
                    ///////////////////

                    aa[((LEN_2D * j) + i)] = __out;
                }

            }

        }


        for (j = 8; (j < LEN_2D); j = (j + 1)) {
            {

                {
                    double _cpy_in = bb[(((LEN_2D * j) + i) - 1)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_bb_to_bb_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    bb_index = _cpy_out;
                }
                {
                    double _cpy_in = cc[((LEN_2D * j) + i)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_cc_to_cc_index_0)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    cc_index_0 = _cpy_out;
                }
                {
                    double __in1 = bb_index;
                    double __in2 = cc_index_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    bb_slice_plus_cc_slice = __out;
                }
                {
                    double __inp = bb_slice_plus_cc_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_20_12)
                    __out = __inp;
                    ///////////////////

                    bb[((LEN_2D * j) + i)] = __out;
                }

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, aa, bb, cc, LEN_2D);
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
