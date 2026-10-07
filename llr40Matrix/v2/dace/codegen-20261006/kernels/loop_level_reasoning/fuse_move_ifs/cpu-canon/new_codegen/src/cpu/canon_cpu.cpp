/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t src_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
inline void loop_body_0_1_3(canon_cpu_state_t *__state, const double* __restrict__ cond, const double* __restrict__ src, double* __restrict__ a, double* __restrict__ b, int64_t K, int64_t LEN_2D, int64_t _loop_it_0, int64_t _loop_it_1) {
    double cond_index;


    cond_index = cond[_loop_it_0];
    if (((cond_index > 0.0) && (K > 0))) {
        {

            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] * 2.0);  // _Mult_
            b[b_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] + 1.0);  // _Add_

        }
    } else if (((! (cond_index > 0.0)) && (K > 0))) {
        {

            b[b_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] + 1.0);  // _Add_

        }
    } else if (((cond_index > 0.0) && (! (K > 0)))) {
        {

            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = (src[src_idx(_loop_it_0, _loop_it_1, LEN_2D)] * 2.0);  // _Mult_

        }
    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ cond, double * __restrict__ src, int64_t K, int64_t LEN_2D)
{

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    #pragma omp parallel for
    for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_2D; _loop_it_0 += 1) {
        for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
            loop_body_0_1_3(__state, &cond[0], &src[0], &a[0], &b[0], K, LEN_2D, _loop_it_0, _loop_it_1);
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
