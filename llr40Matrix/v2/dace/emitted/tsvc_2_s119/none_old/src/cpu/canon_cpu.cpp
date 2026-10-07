/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double aa_index;
    double bb_index;
    double aa_slice_plus_bb_slice;
    int64_t i;
    int64_t j;


    for (i = 1; (i < LEN_2D); i = (i + 1)) {

        for (j = 1; (j < LEN_2D); j = (j + 1)) {
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                aa + (((LEN_2D * (i - 1)) + j) - 1), &aa_index, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                bb + ((LEN_2D * i) + j), &bb_index, 1);
                {
                    double __in2 = bb_index;
                    double __in1 = aa_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    aa_slice_plus_bb_slice = __out;
                }
                {
                    double __inp = aa_slice_plus_bb_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_18_12)
                    __out = __inp;
                    ///////////////////

                    aa[((LEN_2D * i) + j)] = __out;
                }

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, aa, bb, LEN_2D);
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
