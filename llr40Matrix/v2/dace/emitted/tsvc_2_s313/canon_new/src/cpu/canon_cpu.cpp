/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t dot_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ dot, int64_t LEN_1D)
{
    double _priv_dot;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {

        dot[dot_idx(0)] = 0.0;  // assign_16_4
        {  // copy_dot_to__priv_dot
            double _cpy_out;
            _cpy_out = dot[dot_idx(0)];
            _priv_dot = _cpy_out;
        }

    }
    {

        #pragma omp parallel for reduction(+:_priv_dot)
        for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
            double a_slice_times_b_slice;
            double _wcr_priv__Add____out;
            a_slice_times_b_slice = (a[a_idx(_loop_it_0)] * b[b_idx(_loop_it_0)]);  // _Mult_
            _wcr_priv__Add____out = a_slice_times_b_slice;  // _Add_
            {  // copy__wcr_priv__Add____out_to__priv_dot
                double _cpy_out;
                _cpy_out = _wcr_priv__Add____out;
                *(&_priv_dot) = *(&_priv_dot) + (_cpy_out);
            }
        }
        {  // copy__priv_dot_to_dot
            double _cpy_in = _priv_dot;
            dot[dot_idx(0)] = _cpy_in;
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
