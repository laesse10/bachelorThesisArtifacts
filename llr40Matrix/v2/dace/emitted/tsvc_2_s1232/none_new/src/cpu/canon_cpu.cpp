/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D, int64_t VLEN)
{
    double bb_index;
    double cc_index;
    double aa_slice;


    for (int64_t j = 0; (j < LEN_2D); j = (j + 1)) {

        for (int64_t i = (VLEN * j); (i < LEN_2D); i = (i + 1)) {
            {

                bb_index = bb[bb_idx(i, j, LEN_2D)];  // copy_bb_to_bb_index
                cc_index = cc[cc_idx(i, j, LEN_2D)];  // copy_cc_to_cc_index
                aa_slice = (bb_index + cc_index);  // _Add_
                aa[aa_idx(i, j, LEN_2D)] = aa_slice;  // assign_19_12

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
