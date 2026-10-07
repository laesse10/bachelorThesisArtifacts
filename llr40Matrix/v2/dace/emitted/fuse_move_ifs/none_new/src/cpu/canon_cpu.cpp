/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t src_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ cond, double * __restrict__ src, int64_t K, int64_t LEN_2D)
{
    double src_index;
    double a_slice;
    double src_index_0;
    double b_slice;
    int64_t i;
    double cond_index;
    int64_t j;


    for (i = 0; (i < LEN_2D); i = (i + 1)) {

        cond_index = cond[i];

        if ((cond_index > 0.0)) {

            for (j = 0; (j < LEN_2D); j = (j + 1)) {
                {

                    src_index = src[src_idx(i, j, LEN_2D)];  // copy_src_to_src_index
                    a_slice = (src_index * 2.0);  // _Mult_
                    a[a_idx(i, j, LEN_2D)] = a_slice;  // assign_20_16

                }

            }

        }


    }


    if ((K > 0)) {

        for (i = 0; (i < LEN_2D); i = (i + 1)) {

            for (j = 0; (j < LEN_2D); j = (j + 1)) {
                {

                    src_index_0 = src[src_idx(i, j, LEN_2D)];  // copy_src_to_src_index_0
                    b_slice = (src_index_0 + 1.0);  // _Add_
                    b[b_idx(i, j, LEN_2D)] = b_slice;  // assign_24_16

                }

            }


        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ cond, double * __restrict__ src, int64_t K, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, b, cond, src, K, LEN_2D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t K, int64_t LEN_2D)
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
