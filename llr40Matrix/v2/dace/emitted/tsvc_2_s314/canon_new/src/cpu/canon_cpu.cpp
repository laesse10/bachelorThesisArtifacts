/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t result_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    double _arg_max_buf_for_17;
    double x;

    {

        {  // reduce
            (&_arg_max_buf_for_17)[0] = -1.7976931348623157e+308;
            (&_arg_max_buf_for_17)[0] = ::dace::reduce::max(a, (long)(LEN_1D), (long)(1), (&_arg_max_buf_for_17)[0]);
        }

    }
    x = _arg_max_buf_for_17;
    {

        result[result_idx(0)] = x;  // symassign

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, result, LEN_1D);
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
