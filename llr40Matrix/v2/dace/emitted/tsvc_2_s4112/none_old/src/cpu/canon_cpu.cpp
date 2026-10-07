/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int * __restrict__ ip, int64_t LEN_1D)
{
    double a_index;
    double b_index;
    double b_slice_times_2_0;
    double a_slice_plus_b_slice_2_0;
    int64_t i;
    int ip_index;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            a + i, &a_index, 1);

        }
        ip_index = ip[i];
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + ip_index, &b_index, 1);
            {
                double __in1 = b_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * 2.0);
                ///////////////////

                b_slice_times_2_0 = __out;
            }
            {
                double __in1 = a_index;
                double __in2 = b_slice_times_2_0;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                a_slice_plus_b_slice_2_0 = __out;
            }
            {
                double __inp = a_slice_plus_b_slice_2_0;
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

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int * __restrict__ ip, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, ip, LEN_1D);
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
