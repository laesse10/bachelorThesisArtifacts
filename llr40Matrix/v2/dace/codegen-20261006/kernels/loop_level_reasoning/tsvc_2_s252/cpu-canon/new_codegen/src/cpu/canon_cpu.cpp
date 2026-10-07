/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
constexpr double t = 0.0;

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double s_0;

        s_0 = (b[b_idx(0)] * c[c_idx(0)]);  // _Mult_
        {  // _Add_
            double __in2 = t;
            a[a_idx(0)] = (s_0 + __in2);
        }
        #pragma omp parallel for
        for (int64_t _loop_it_0 = 1; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
            double s_1;
            double t_remat;
            double t_remat_0;
            s_1 = (b[b_idx(_loop_it_0)] * c[c_idx(_loop_it_0)]);  // _Mult_
            t_remat = (b[b_idx((_loop_it_0 - 1))] * c[c_idx((_loop_it_0 - 1))]);  // _Mult__remat
            t_remat_0 = (t_remat + 0.0);  // _Add__remat
            a[a_idx(_loop_it_0)] = (s_1 + t_remat_0);  // _Add_
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, LEN_1D);
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
