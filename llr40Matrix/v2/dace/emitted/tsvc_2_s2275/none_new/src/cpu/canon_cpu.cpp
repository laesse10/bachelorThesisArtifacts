/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
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


    for (int64_t i = 0; (i < LEN_2D); i = (i + 1)) {

        for (int64_t j = 0; (j < LEN_2D); j = (j + 1)) {
            {

                aa_index = aa[aa_idx(j, i, LEN_2D)];  // copy_aa_to_aa_index
                bb_index = bb[bb_idx(j, i, LEN_2D)];  // copy_bb_to_bb_index
                cc_index = cc[cc_idx(j, i, LEN_2D)];  // copy_cc_to_cc_index
                bb_slice_times_cc_slice = (bb_index * cc_index);  // _Mult_
                aa_slice_plus_bb_slice_cc_slice = (aa_index + bb_slice_times_cc_slice);  // _Add_
                aa[aa_idx(j, i, LEN_2D)] = aa_slice_plus_bb_slice_cc_slice;  // assign_18_12

            }

        }

        {

            b_index = b[b_idx(i)];  // copy_b_to_b_index
            c_index = c[c_idx(i)];  // copy_c_to_c_index
            d_index = d[d_idx(i)];  // copy_d_to_d_index
            c_slice_times_d_slice = (c_index * d_index);  // _Mult_
            a_slice = (b_index + c_slice_times_d_slice);  // _Add_
            a[a_idx(i)] = a_slice;  // assign_19_8

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
