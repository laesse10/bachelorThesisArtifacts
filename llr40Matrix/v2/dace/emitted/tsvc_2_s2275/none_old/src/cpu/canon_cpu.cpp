/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, double * __restrict__ cc, double * __restrict__ d, int64_t LEN_2D)
{
    double aa_index;
    double bb_index;
    double cc_index;
    double bb_slice_times_cc_slice;
    double aa_slice_plus_bb_slice_cc_slice;
    double b_index;
    double c_index;
    double d_index;
    double c_slice_times_d_slice;
    double a_slice;
    int64_t i;
    int64_t j;


    for (i = 0; (i < LEN_2D); i = (i + 1)) {

        for (j = 0; (j < LEN_2D); j = (j + 1)) {
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                aa + ((LEN_2D * j) + i), &aa_index, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                bb + ((LEN_2D * j) + i), &bb_index, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                cc + ((LEN_2D * j) + i), &cc_index, 1);
                {
                    double __in1 = bb_index;
                    double __in2 = cc_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    bb_slice_times_cc_slice = __out;
                }
                {
                    double __in2 = bb_slice_times_cc_slice;
                    double __in1 = aa_index;
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
                    // Tasklet code (assign_18_12)
                    __out = __inp;
                    ///////////////////

                    aa[((LEN_2D * j) + i)] = __out;
                }

            }

        }

        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &b_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            d + i, &d_index, 1);
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
                double __in1 = b_index;
                double __in2 = c_slice_times_d_slice;
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
                // Tasklet code (assign_19_8)
                __out = __inp;
                ///////////////////

                a[i] = __out;
            }

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, double * __restrict__ cc, double * __restrict__ d, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, b, bb, c, cc, d, LEN_2D);
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
