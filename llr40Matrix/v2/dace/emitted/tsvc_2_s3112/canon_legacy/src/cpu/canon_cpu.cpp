/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "cstring"
#include "numeric"
#include "functional"
#include "algorithm"
#include "dace/scan.hpp"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{

    {
        double __sum;
        double _scan_seed_b;

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_18_4)
            __out = 0.0;
            ///////////////////

            __sum = __out;
        }
        {
            double _in = __sum;
            double _out;

            ///////////////////
            // Tasklet code (_assign___sum_to__scan_seed_b)
            _out = _in;
            ///////////////////

            _scan_seed_b = _out;
        }
        {
            double _scan_init = _scan_seed_b;
            double* __restrict__ _scan_in = &a[0];
            double* __restrict__ _scan_out = b;

            ///////////////////
            ::dace::scan::inclusive_sum(_scan_in, _scan_in + (LEN_1D), _scan_out, _scan_init);
            ///////////////////

        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, LEN_1D);
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
