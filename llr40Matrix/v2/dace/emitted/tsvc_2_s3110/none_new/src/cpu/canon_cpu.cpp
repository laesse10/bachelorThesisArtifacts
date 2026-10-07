/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1) { return ((2 * __d0) + __d1); }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double maxv;
    double maxv_0;
    int64_t xindex;
    int64_t yindex;
    double aa_index;
    bool __tmp0;

    {

        maxv = aa[aa_idx(0, 0, LEN_2D)];  // copy_aa_to_maxv

    }
    xindex = 0;
    yindex = 0;

    for (int64_t i = 0; (i < LEN_2D); i = (i + 1)) {

        for (int64_t j = 0; (j < LEN_2D); j = (j + 1)) {

            aa_index = aa[((LEN_2D * i) + j)];

            __tmp0 = (aa_index > maxv);

            if (__tmp0) {
                {

                    maxv_0 = aa[aa_idx(i, j, LEN_2D)];  // copy_aa_to_maxv_0
                    maxv = maxv_0;  // assign_22_16

                }
                xindex = i;
                yindex = j;

            }


        }


    }

    {
        double float_xindex;
        double maxv_plus_expr;
        double float_yindex;
        double chksum;

        float_xindex = double(xindex);  // _convert_to_float64_
        maxv_plus_expr = (maxv + float_xindex);  // _Add_
        float_yindex = double(yindex);  // _convert_to_float64_
        chksum = (maxv_plus_expr + float_yindex);  // _Add_
        bb[bb_idx(0, 0)] = chksum;  // assign_28_4

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
