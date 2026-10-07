/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    double aa_index_0;
    double bb_index;
    double cc_index;
    double bb_slice_times_cc_slice;
    double aa_slice_plus_bb_slice_cc_slice;
    int64_t i;
    double aa_index;
    int64_t j;


    for (i = 0; (i < LEN_2D); i = (i + 1)) {

        aa_index = aa[i];

        if ((aa_index > 0.0)) {

            for (j = 1; (j < LEN_2D); j = (j + 1)) {
                {


                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    aa + ((LEN_2D * (j - 1)) + i), &aa_index_0, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    bb + ((LEN_2D * j) + i), &bb_index, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    cc + ((LEN_2D * j) + i), &cc_index, 1);
                    {
                        double __in2 = cc_index;
                        double __in1 = bb_index;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        bb_slice_times_cc_slice = __out;
                    }
                    {
                        double __in2 = bb_slice_times_cc_slice;
                        double __in1 = aa_index_0;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        aa_slice_plus_bb_slice_cc_slice = __out;
                    }
                    {
                        double __inp = aa_slice_plus_bb_slice_cc_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (assign_19_16)
                        __out = __inp;
                        ///////////////////

                        aa[((LEN_2D * j) + i)] = __out;
                    }

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
