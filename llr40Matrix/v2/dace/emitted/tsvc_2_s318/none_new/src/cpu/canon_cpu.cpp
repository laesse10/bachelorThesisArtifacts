/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t result_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    int64_t k;
    int64_t index;
    double a_index;
    double maxv;
    double a_index_0;
    int64_t k_plus_inc_0;


    k = 0;
    index = 0;
    a_index = a[0];

    maxv = abs(a_index);
    k = (0 + inc);

    for (int64_t i = 1; (i < LEN_1D); i = (i + 1)) {

        a_index_0 = a[k];

        if ((abs(a_index_0) > maxv)) {

            index = i;
            maxv = (abs(a_index_0) + 0.0);

        }

        k_plus_inc_0 = (k + inc);

        k = k_plus_inc_0;


    }

    {
        double float_index;
        double result_slice;

        float_index = double(index);  // _convert_to_float64_
        result_slice = (maxv + float_index);  // _Add_
        result[result_idx(0)] = result_slice;  // assign_26_4

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    __program_canon_cpu_internal(__state, a, result, LEN_1D, inc);
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
