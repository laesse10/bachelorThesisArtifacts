/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    double src_index_0;
    double weight_index;
    double packed_slice;
    int64_t n;
    int64_t i;
    double src_index;


    n = 0;

    for (i = 0; (i < LEN_1D); i = (i + 1)) {

        src_index = src[i];

        if ((src_index > 0.0)) {
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                src + i, &src_index_0, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                weight + i, &weight_index, 1);
                {
                    double __in2 = weight_index;
                    double __in1 = src_index_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    packed_slice = __out;
                }
                {
                    double __inp = packed_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_19_12)
                    __out = __inp;
                    ///////////////////

                    packed[n] = __out;
                }

            }
            n = (n + 1);

        }


    }

    {

        {
            int64_t __out;

            ///////////////////
            // Tasklet code (assign_21_4)
            __out = n;
            ///////////////////

            out_count[0] = __out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, out_count, packed, src, weight, LEN_1D);
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
