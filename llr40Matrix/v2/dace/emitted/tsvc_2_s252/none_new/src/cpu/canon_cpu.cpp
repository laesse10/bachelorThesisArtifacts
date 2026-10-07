/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
{
    double t;
    double b_index;
    double c_index;
    double s;
    double a_slice;
    double t_0;

    {

        t = 0.0;  // assign_16_4

    }

    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            b_index = b[b_idx(i)];  // copy_b_to_b_index
            c_index = c[c_idx(i)];  // copy_c_to_c_index
            s = (b_index * c_index);  // _Mult_
            a_slice = (s + t);  // _Add_
            a[a_idx(i)] = a_slice;  // assign_19_8
            t_0 = (s + 0.0);  // _Add_
            t = t_0;  // assign_20_8

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
