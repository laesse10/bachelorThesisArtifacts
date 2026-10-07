/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    double x;
    double x_0;
    int64_t idx;
    int64_t i;
    double a_index;
    bool __tmp0;

    {

        {
            double _cpy_in = a[0];
            double _cpy_out;

            ///////////////////
            // Tasklet code (copy_a_to_x)
            _cpy_out = _cpy_in;
            ///////////////////

            x = _cpy_out;
        }

    }
    idx = 0;

    for (i = 1; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];

        __tmp0 = (a_index > x);

        if (__tmp0) {
            {

                {
                    double _cpy_in = a[i];
                    double _cpy_out;

                    ///////////////////
                    // Tasklet code (copy_a_to_x_0)
                    _cpy_out = _cpy_in;
                    ///////////////////

                    x_0 = _cpy_out;
                }
                {
                    double __inp = x_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_20_12)
                    __out = __inp;
                    ///////////////////

                    x = __out;
                }

            }
            idx = i;

        }


    }

    {

        {
            double __inp = x;
            double __out;

            ///////////////////
            // Tasklet code (assign_22_4)
            __out = __inp;
            ///////////////////

            out_value[0] = __out;
        }
        {
            int64_t __out;

            ///////////////////
            // Tasklet code (assign_23_4)
            __out = idx;
            ///////////////////

            out_index[0] = __out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out_index, out_value, LEN_1D);
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
