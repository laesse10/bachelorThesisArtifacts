/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
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
                double a_index;
                double a_slice_times_b_slice;
                double a_slice_b_slice_times_c_slice;
                {
                    double _in = a[_loop_it_0];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_in_a_to_a_index)
                    _out = _in;
                    ///////////////////

                    a_index = _out;
                }
                {
                    double __in1 = a_index;
                    double __in2 = b[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    a_slice_times_b_slice = __out;
                }
                {
                    double __in1 = a_slice_times_b_slice;
                    double __in2 = c[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    a_slice_b_slice_times_c_slice = __out;
                }
                {
                    double _in = a_slice_b_slice_times_c_slice;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_out_a_slice_b_slice_times_c_slice_to_a)
                    _out = _in;
                    ///////////////////

                    a[_loop_it_0] = _out;
                }
            }
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
