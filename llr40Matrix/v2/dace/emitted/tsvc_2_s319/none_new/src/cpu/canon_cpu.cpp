/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    double sum_val;
    double c_index;
    double d_index;
    double a_slice;
    double a_index;
    double sum_val_plus_a_slice;
    double c_index_0;
    double e_index;
    double b_slice;
    double b_index;
    double sum_val_plus_b_slice;

    {

        sum_val = 0.0;  // assign_16_4

    }

    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            c_index = c[c_idx(i)];  // copy_c_to_c_index
            c_index_0 = c[c_idx(i)];  // copy_c_to_c_index_0
            d_index = d[d_idx(i)];  // copy_d_to_d_index
            a_slice = (c_index + d_index);  // _Add_
            a[a_idx(i)] = a_slice;  // assign_18_8
            a_index = a[a_idx(i)];  // copy_a_to_a_index
            sum_val_plus_a_slice = (sum_val + a_index);  // _Add_
            sum_val = sum_val_plus_a_slice;  // assign_19_8
            e_index = e[e_idx(i)];  // copy_e_to_e_index
            b_slice = (c_index_0 + e_index);  // _Add_
            b[b_idx(i)] = b_slice;  // assign_20_8
            b_index = b[b_idx(i)];  // copy_b_to_b_index
            sum_val_plus_b_slice = (sum_val + b_index);  // _Add_
            sum_val = sum_val_plus_b_slice;  // assign_21_8

        }

    }

    {

        b[b_idx(0)] = sum_val;  // assign_22_4

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, e, LEN_1D);
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
