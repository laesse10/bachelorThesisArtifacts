/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, double * __restrict__ x, int64_t LEN_1D)
{
    double a_index_0;
    double b_index_0;
    double d_index;
    double b_slice_times_d_slice;
    double a_slice_plus_b_slice_d_slice;
    double c_index;
    double d_index_0;
    double d_index_1;
    double d_slice_times_d_slice;
    double c_slice_plus_d_slice_d_slice;
    double d_index_2;
    double e_index;
    double d_slice_times_e_slice;
    double c_slice;
    double a_index_1;
    double e_index_0;
    double e_index_1;
    double e_slice_times_e_slice;
    double b_slice;
    double a_index_2;
    double d_index_3;
    double d_index_4;
    double d_slice_times_d_slice_0;
    double c_slice_0;
    double c_index_0;
    double e_index_2;
    double e_index_3;
    double e_slice_times_e_slice_0;
    double c_slice_plus_e_slice_e_slice;
    double a_index;
    double b_index;
    double x_index;


    for (int64_t i = 0; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];
        b_index = b[i];

        if ((a_index > b_index)) {
            {

                a_index_0 = a[a_idx(i)];  // copy_a_to_a_index_0
                b_index_0 = b[b_idx(i)];  // copy_b_to_b_index_0
                d_index = d[d_idx(i)];  // copy_d_to_d_index
                b_slice_times_d_slice = (b_index_0 * d_index);  // _Mult_
                a_slice_plus_b_slice_d_slice = (a_index_0 + b_slice_times_d_slice);  // _Add_
                a[a_idx(i)] = a_slice_plus_b_slice_d_slice;  // assign_18_12

            }

            if ((LEN_1D > 10)) {
                {

                    c_index = c[c_idx(i)];  // copy_c_to_c_index
                    d_index_0 = d[d_idx(i)];  // copy_d_to_d_index_0
                    d_index_1 = d[d_idx(i)];  // copy_d_to_d_index_1
                    d_slice_times_d_slice = (d_index_0 * d_index_1);  // _Mult_
                    c_slice_plus_d_slice_d_slice = (c_index + d_slice_times_d_slice);  // _Add_
                    c[c_idx(i)] = c_slice_plus_d_slice_d_slice;  // assign_20_16

                }
            } else {
                {

                    d_index_2 = d[d_idx(i)];  // copy_d_to_d_index_2
                    e_index = e[e_idx(i)];  // copy_e_to_e_index
                    d_slice_times_e_slice = (d_index_2 * e_index);  // _Mult_
                    c_slice = (d_slice_times_e_slice + 1.0);  // _Add_
                    c[c_idx(i)] = c_slice;  // assign_22_16

                }
            }

        } else {

            x_index = x[0];
            {

                a_index_1 = a[a_idx(i)];  // copy_a_to_a_index_1
                e_index_0 = e[e_idx(i)];  // copy_e_to_e_index_0
                e_index_1 = e[e_idx(i)];  // copy_e_to_e_index_1
                e_slice_times_e_slice = (e_index_0 * e_index_1);  // _Mult_
                b_slice = (a_index_1 + e_slice_times_e_slice);  // _Add_
                b[b_idx(i)] = b_slice;  // assign_24_12

            }

            if ((x_index > 0.0)) {
                {

                    a_index_2 = a[a_idx(i)];  // copy_a_to_a_index_2
                    d_index_3 = d[d_idx(i)];  // copy_d_to_d_index_3
                    d_index_4 = d[d_idx(i)];  // copy_d_to_d_index_4
                    d_slice_times_d_slice_0 = (d_index_3 * d_index_4);  // _Mult_
                    c_slice_0 = (a_index_2 + d_slice_times_d_slice_0);  // _Add_
                    c[c_idx(i)] = c_slice_0;  // assign_26_16

                }
            } else {
                {

                    c_index_0 = c[c_idx(i)];  // copy_c_to_c_index_0
                    e_index_2 = e[e_idx(i)];  // copy_e_to_e_index_2
                    e_index_3 = e[e_idx(i)];  // copy_e_to_e_index_3
                    e_slice_times_e_slice_0 = (e_index_2 * e_index_3);  // _Mult_
                    c_slice_plus_e_slice_e_slice = (c_index_0 + e_slice_times_e_slice_0);  // _Add_
                    c[c_idx(i)] = c_slice_plus_e_slice_e_slice;  // assign_28_16

                }
            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, double * __restrict__ x, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, e, x, LEN_1D);
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
