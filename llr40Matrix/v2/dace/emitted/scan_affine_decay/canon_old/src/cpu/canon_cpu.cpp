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

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{

    {
        double _scan_seed_y;

        {
            double _in = y[0];
            double _out;

            ///////////////////
            // Tasklet code (_assign_y_to__scan_seed_y)
            _out = _in;
            ///////////////////

            _scan_seed_y = _out;
        }
        {
            double _scan_init = _scan_seed_y;
            double* __restrict__ _scan_in = &x[1];
            double* __restrict__ _scan_coef = &c[1];
            double* __restrict__ _scan_out = y + 1;

            ///////////////////
            ::dace::scan::inclusive_affine(_scan_coef, _scan_in, _scan_out, static_cast<long>((LEN_1D - 1)), _scan_init);
            ///////////////////

        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, c, x, y, LEN_1D);
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
