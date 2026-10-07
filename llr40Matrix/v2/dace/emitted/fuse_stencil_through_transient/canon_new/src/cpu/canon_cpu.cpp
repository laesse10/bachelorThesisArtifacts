/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    #pragma omp parallel for
    for (int64_t _loop_it_1 = 1; _loop_it_1 < (LEN_1D - 2); _loop_it_1 += 1) {
        double __tmp1;
        double __tmp3;
        double __tmp0;
        double __tmp2;
        __tmp1 = (a[a_idx((_loop_it_1 - 1))] + a[a_idx(_loop_it_1)]);  // _Add_
        __tmp0 = (__tmp1 + a[a_idx((_loop_it_1 + 1))]);  // _Add_
        __tmp3 = (a[a_idx(_loop_it_1)] + a[a_idx((_loop_it_1 + 1))]);  // _Add_
        __tmp2 = (__tmp3 + a[a_idx((_loop_it_1 + 2))]);  // _Add_
        out[out_idx(_loop_it_1)] = (__tmp0 * __tmp2);  // _Mult_
    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out, LEN_1D);
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
