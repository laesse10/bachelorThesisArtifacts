/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    double aa_index;
    double cc_index;
    double aa_slice_plus_cc_slice;
    double bb_index;
    double cc_index_0;
    double bb_slice_plus_cc_slice;
    int64_t j;


    for (int64_t i = 8; (i < LEN_2D); i = (i + 1)) {

        for (j = 8; (j < LEN_2D); j = (j + 1)) {
            {

                aa_index = aa[aa_idx((j - 1), i, LEN_2D)];  // copy_aa_to_aa_index
                cc_index = cc[cc_idx(j, i, LEN_2D)];  // copy_cc_to_cc_index
                aa_slice_plus_cc_slice = (aa_index + cc_index);  // _Add_
                aa[aa_idx(j, i, LEN_2D)] = aa_slice_plus_cc_slice;  // assign_18_12

            }

        }


        for (j = 8; (j < LEN_2D); j = (j + 1)) {
            {

                bb_index = bb[bb_idx((i - 1), j, LEN_2D)];  // copy_bb_to_bb_index
                cc_index_0 = cc[cc_idx(i, j, LEN_2D)];  // copy_cc_to_cc_index_0
                bb_slice_plus_cc_slice = (bb_index + cc_index_0);  // _Add_
                bb[bb_idx(i, j, LEN_2D)] = bb_slice_plus_cc_slice;  // assign_20_12

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
