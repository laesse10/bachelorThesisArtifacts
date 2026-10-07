/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

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

            {
                double __in1 = b[_loop_it_0];
                double __in2 = d[_loop_it_0];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                b_slice_times_d_slice = __out;
            }
            {
                double __in2 = b_slice_times_d_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = __in2;
                ///////////////////

                _wcr_priv__Add____out = __out;
            }
            {
                double __in1 = a[_loop_it_0];
                double __in2 = _wcr_priv__Add____out;
                double __out;

                ///////////////////
                // Tasklet code (augassign)
                __out = (__in1 + __in2);
                ///////////////////

                a[_loop_it_0] = __out;
            }

        }
        if ((LEN_1D > 10)) {
            {
                double d_slice_times_d_slice;
                double _wcr_priv__Add____out_0;

                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = d[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    d_slice_times_d_slice = __out;
                }
                {
                    double __in2 = d_slice_times_d_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out_0 = __out;
                }
                {
                    double __in1 = c[_loop_it_0];
                    double __in2 = _wcr_priv__Add____out_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (augassign)
                    __out = (__in1 + __in2);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        } else {
            {
                double d_slice_times_e_slice;

                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = e[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    d_slice_times_e_slice = __out;
                }
                {
                    double __in1 = d_slice_times_e_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + 1.0);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        }
    } else {

        x_index = x[0];
        {
            double e_slice_times_e_slice;

            {
                double __in1 = e[_loop_it_0];
                double __in2 = e[_loop_it_0];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                e_slice_times_e_slice = __out;
            }
            {
                double __in2 = e_slice_times_e_slice;
                double __in1 = a[_loop_it_0];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b[_loop_it_0] = __out;
            }

        }
        if ((x_index > 0.0)) {
            {
                double d_slice_times_d_slice_0;

                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = d[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    d_slice_times_d_slice_0 = __out;
                }
                {
                    double __in2 = d_slice_times_d_slice_0;
                    double __in1 = a[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        } else {
            {
                double e_slice_times_e_slice_0;
                double _wcr_priv__Add____out_1;

                {
                    double __in1 = e[_loop_it_0];
                    double __in2 = e[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    e_slice_times_e_slice_0 = __out;
                }
                {
                    double __in2 = e_slice_times_e_slice_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out_1 = __out;
                }
                {
                    double __in1 = c[_loop_it_0];
                    double __in2 = _wcr_priv__Add____out_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (augassign)
                    __out = (__in1 + __in2);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        }
    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, double * __restrict__ x, int64_t LEN_1D)
{

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {

        {
            #pragma omp parallel for
            for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
                loop_body_0_1_0(__state, &d[0], &e[0], &x[0], &a[0], &b[0], &c[0], LEN_1D, _loop_it_0);
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
