/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ bins, int * __restrict__ ip, double * __restrict__ src, int64_t LEN_1D)
{
    double bins_index;
    double src_index;
    double bins_slice_plus_src_slice;
    int64_t i;
    int ip_index;
    int bins_slice;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {

        ip_index = ip[i];
        bins_slice = ip[i];
        {

            {
                double _cpy_in = bins[ip_index];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_bins_to_bins_index)
                _cpy_out = _cpy_in;
                ///////////////////

                bins_index = _cpy_out;
            }
            {
                double _cpy_in = src[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_src_to_src_index)
                _cpy_out = _cpy_in;
                ///////////////////

                src_index = _cpy_out;
            }
            {
                double __in2 = src_index;
                double __in1 = bins_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                bins_slice_plus_src_slice = __out;
            }
            {
                double __inp = bins_slice_plus_src_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_17_8)
                __out = __inp;
                ///////////////////

                bins[bins_slice] = __out;
            }

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ bins, int * __restrict__ ip, double * __restrict__ src, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, bins, ip, src, LEN_1D);
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
