/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, int64_t LEN_2D)
{
    double a_index;
    double b_index;
    double c_index;
    double b_slice_times_c_slice;
    double a_slice_plus_b_slice_c_slice;
    double aa_index;
    double bb_index;
    double a_index_0;
    double bb_slice_times_a_slice;
    double aa_slice_plus_bb_slice_a_slice;


    for (int64_t i = 0; (i < LEN_2D); i = (i + 1)) {
        {

            a_index = a[a_idx(i)];  // copy_a_to_a_index
            b_index = b[b_idx(i)];  // copy_b_to_b_index
            c_index = c[c_idx(i)];  // copy_c_to_c_index
            b_slice_times_c_slice = (b_index * c_index);  // _Mult_
            a_slice_plus_b_slice_c_slice = (a_index + b_slice_times_c_slice);  // _Add_
            a[a_idx(i)] = a_slice_plus_b_slice_c_slice;  // assign_17_8

        }

        for (int64_t j = 1; (j < LEN_2D); j = (j + 1)) {
            {

                aa_index = aa[aa_idx((j - 1), i, LEN_2D)];  // copy_aa_to_aa_index
                bb_index = bb[bb_idx(j, i, LEN_2D)];  // copy_bb_to_bb_index
                a_index_0 = a[a_idx(i)];  // copy_a_to_a_index_0
                bb_slice_times_a_slice = (bb_index * a_index_0);  // _Mult_
                aa_slice_plus_bb_slice_a_slice = (aa_index + bb_slice_times_a_slice);  // _Add_
                aa[aa_idx(j, i, LEN_2D)] = aa_slice_plus_bb_slice_a_slice;  // assign_19_12

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, b, bb, c, LEN_2D);
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
