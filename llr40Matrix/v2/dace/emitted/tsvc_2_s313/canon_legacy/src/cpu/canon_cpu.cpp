/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    double _priv_dot;

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
            double __out;

            ///////////////////
            // Tasklet code (assign_16_4)
            __out = 0.0;
            ///////////////////

            dot[0] = __out;
        }
        {
            double _cpy_in = dot[0];
            double _cpy_out;

            ///////////////////
            // Tasklet code (copy_dot_to__priv_dot)
            _cpy_out = _cpy_in;
            ///////////////////

            _priv_dot = _cpy_out;
        }

    }
    {

        {
            #pragma omp parallel for reduction(+:_priv_dot)
            for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
                double a_slice_times_b_slice;
                double _wcr_priv__Add____out;
                {
                    double __in1 = a[_loop_it_0];
                    double __in2 = b[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    a_slice_times_b_slice = __out;
                }
                {
                    double __in2 = a_slice_times_b_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out = __out;
                }
                {
                    double _cpy_in = _wcr_priv__Add____out;
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy__wcr_priv__Add____out_to__priv_dot)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    *(&_priv_dot) = *(&_priv_dot) + (_cpy_out);
                }
            }
        }
        {
            double _cpy_in = _priv_dot;
            double _cpy_out;

            ///////////////////
            // Tasklet code (copy__priv_dot_to_dot)
            _cpy_out = _cpy_in;
            ///////////////////

            dot[0] = _cpy_out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, dot, LEN_1D);
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
