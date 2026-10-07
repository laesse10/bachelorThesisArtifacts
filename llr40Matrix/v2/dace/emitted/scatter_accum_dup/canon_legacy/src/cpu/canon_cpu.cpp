/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

inline void loop_body_0_1_0(canon_cpu_state_t *__state, const int* __restrict__ ip, const double* __restrict__ src, double* __restrict__ bins, int64_t _loop_it_0) {
    int ip_index;


    ip_index = ip[_loop_it_0];
    {
        double _wcr_priv__Add____out;

        {
            double __in2 = src[_loop_it_0];
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = __in2;
            ///////////////////

            _wcr_priv__Add____out = __out;
        }
        dace::wcr_fixed<dace::ReductionType::Sum, double>::reduce_atomic(bins + ip_index, *(&_wcr_priv__Add____out));

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ bins, int * __restrict__ ip, double * __restrict__ src, int64_t LEN_1D)
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
                loop_body_0_1_0(__state, &ip[0], &src[0], &bins[0], _loop_it_0);
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ bins, int * __restrict__ ip, double * __restrict__ src, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, bins, ip, src, LEN_1D);
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
