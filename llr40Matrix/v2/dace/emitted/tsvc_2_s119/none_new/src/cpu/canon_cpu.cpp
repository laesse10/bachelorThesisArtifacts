/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double aa_index;
    double bb_index;
    double aa_slice_plus_bb_slice;


    for (int64_t i = 1; (i < LEN_2D); i = (i + 1)) {

        for (int64_t j = 1; (j < LEN_2D); j = (j + 1)) {
            {

                aa_index = aa[aa_idx((i - 1), (j - 1), LEN_2D)];  // copy_aa_to_aa_index
                bb_index = bb[bb_idx(i, j, LEN_2D)];  // copy_bb_to_bb_index
                aa_slice_plus_bb_slice = (aa_index + bb_index);  // _Add_
                aa[aa_idx(i, j, LEN_2D)] = aa_slice_plus_bb_slice;  // assign_18_12

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
