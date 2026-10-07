/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t bins_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t src_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ bins, int * __restrict__ ip, double * __restrict__ src, int64_t LEN_1D)
{
    double bins_index;
    double src_index;
    double bins_slice_plus_src_slice;
    int ip_index;
    int bins_slice;


    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {

        ip_index = ip[i];
        bins_slice = ip[i];
        {

            bins_index = bins[bins_idx(ip_index)];  // copy_bins_to_bins_index
            src_index = src[src_idx(i)];  // copy_src_to_src_index
            bins_slice_plus_src_slice = (bins_index + src_index);  // _Add_
            bins[bins_idx(bins_slice)] = bins_slice_plus_src_slice;  // assign_17_8

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
