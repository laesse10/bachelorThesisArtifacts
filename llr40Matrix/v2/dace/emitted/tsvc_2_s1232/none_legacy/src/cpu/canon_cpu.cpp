/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D, int64_t VLEN)
{
    double bb_index;
    double cc_index;
    double aa_slice;
    int64_t j;
    int64_t i;


    for (j = 0; (j < LEN_2D); j = (j + 1)) {

        for (i = (VLEN * j); (i < LEN_2D); i = (i + 1)) {
            {

                {
                    double _cpy_in = bb[((LEN_2D * i) + j)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_bb_to_bb_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    bb_index = _cpy_out;
                }
                {
                    double _cpy_in = cc[((LEN_2D * i) + j)];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_cc_to_cc_index)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    cc_index = _cpy_out;
                }
                {
                    double __in2 = cc_index;
                    double __in1 = bb_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    aa_slice = __out;
                }
                {
                    double __inp = aa_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_19_12)
                    __out = __inp;
                    ///////////////////

                    aa[((LEN_2D * i) + j)] = __out;
                }

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D, int64_t VLEN)
{
    __program_canon_cpu_internal(__state, aa, bb, cc, LEN_2D, VLEN);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_2D, int64_t VLEN)
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
