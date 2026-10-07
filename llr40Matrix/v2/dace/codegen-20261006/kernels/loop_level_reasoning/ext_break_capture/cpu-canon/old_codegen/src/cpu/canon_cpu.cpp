/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    int64_t _loop_it_0;
    double a_index;

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
            int64_t __out;

            ///////////////////
            // Tasklet code (assign_18_4)
            __out = -1;
            ///////////////////

            out_index[0] = __out;
        }
        {
            double __out;

            ///////////////////
            // Tasklet code (assign_19_4)
            __out = -1.0;
            ///////////////////

            out_value[0] = __out;
        }

    }

    for (_loop_it_0 = 0; (_loop_it_0 < LEN_1D); _loop_it_0 = (_loop_it_0 + 1)) {

        a_index = a[_loop_it_0];

        if ((a_index > 1)) {
            {

                {
                    int64_t __out;

                    ///////////////////
                    // Tasklet code (assign_22_12)
                    __out = _loop_it_0;
                    ///////////////////

                    out_index[0] = __out;
                }
                {
                    double _in = a[_loop_it_0];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_a_to_out_value)
                    _out = _in;
                    ///////////////////

                    out_value[0] = _out;
                }

            }
            break;
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
