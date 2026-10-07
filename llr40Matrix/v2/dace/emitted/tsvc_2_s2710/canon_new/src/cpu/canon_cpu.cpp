/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
inline void loop_body_0_1_0(canon_cpu_state_t *__state, const double* __restrict__ d, const double* __restrict__ e, const double* __restrict__ x, double* __restrict__ a, double* __restrict__ b, double* __restrict__ c, int64_t LEN_1D, int64_t _loop_it_0) {
    double a_index;
    double b_index;
    double x_index;


    a_index = a[_loop_it_0];
    b_index = b[_loop_it_0];
    if ((a_index > b_index)) {
        {
            double b_slice_times_d_slice;
            double _wcr_priv__Add____out;

            b_slice_times_d_slice = (b[b_idx(_loop_it_0)] * d[d_idx(_loop_it_0)]);  // _Mult_
            _wcr_priv__Add____out = b_slice_times_d_slice;  // _Add_
            a[a_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + _wcr_priv__Add____out);  // augassign

        }
        if ((LEN_1D > 10)) {
            {
                double d_slice_times_d_slice;
                double _wcr_priv__Add____out_0;

                d_slice_times_d_slice = (d[d_idx(_loop_it_0)] * d[d_idx(_loop_it_0)]);  // _Mult_
                _wcr_priv__Add____out_0 = d_slice_times_d_slice;  // _Add_
                c[c_idx(_loop_it_0)] = (c[c_idx(_loop_it_0)] + _wcr_priv__Add____out_0);  // augassign

            }
        } else {
            {
                double d_slice_times_e_slice;

                d_slice_times_e_slice = (d[d_idx(_loop_it_0)] * e[e_idx(_loop_it_0)]);  // _Mult_
                c[c_idx(_loop_it_0)] = (d_slice_times_e_slice + 1.0);  // _Add_

            }
        }
    } else {

        x_index = x[0];
        {
            double e_slice_times_e_slice;

            e_slice_times_e_slice = (e[e_idx(_loop_it_0)] * e[e_idx(_loop_it_0)]);  // _Mult_
            b[b_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + e_slice_times_e_slice);  // _Add_

        }
        if ((x_index > 0.0)) {
            {
                double d_slice_times_d_slice_0;

                d_slice_times_d_slice_0 = (d[d_idx(_loop_it_0)] * d[d_idx(_loop_it_0)]);  // _Mult_
                c[c_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + d_slice_times_d_slice_0);  // _Add_

            }
        } else {
            {
                double e_slice_times_e_slice_0;
                double _wcr_priv__Add____out_1;

                e_slice_times_e_slice_0 = (e[e_idx(_loop_it_0)] * e[e_idx(_loop_it_0)]);  // _Mult_
                _wcr_priv__Add____out_1 = e_slice_times_e_slice_0;  // _Add_
                c[c_idx(_loop_it_0)] = (c[c_idx(_loop_it_0)] + _wcr_priv__Add____out_1);  // augassign

            }
        }
    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, double * __restrict__ x, int64_t LEN_1D)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    #pragma omp parallel for
    for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
        loop_body_0_1_0(__state, &d[0], &e[0], &x[0], &a[0], &b[0], &c[0], LEN_1D, _loop_it_0);
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
