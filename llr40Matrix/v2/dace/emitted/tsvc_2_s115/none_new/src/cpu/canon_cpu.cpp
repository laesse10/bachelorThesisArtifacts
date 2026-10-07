/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, int64_t LEN_2D)
{
    double a_index;
    double aa_index;
    double a_index_0;
    double aa_slice_times_a_slice;
    double a_slice_minus_aa_slice_a_slice;


    for (int64_t j = 0; (j < LEN_2D); j = (j + 1)) {

        for (int64_t i = (j + 1); (i < LEN_2D); i = (i + 1)) {
            {

                a_index = a[a_idx(i)];  // copy_a_to_a_index
                a_index_0 = a[a_idx(j)];  // copy_a_to_a_index_0
                aa_index = aa[aa_idx(j, i, LEN_2D)];  // copy_aa_to_aa_index
                aa_slice_times_a_slice = (aa_index * a_index_0);  // _Mult_
                a_slice_minus_aa_slice_a_slice = (a_index - aa_slice_times_a_slice);  // _Sub_
                a[a_idx(i)] = a_slice_minus_aa_slice_a_slice;  // assign_18_12

            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, LEN_2D);
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
