/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
inline void loop_body_2_0_0(canon_cpu_state_t *__state, double* __restrict__ a, int64_t LEN_2D, int64_t _skew_p_0, int64_t _skew_t_0) {

    for (int64_t _loop_it_0 = (((-64 * _skew_p_0) + (64 * _skew_t_0)) + 1); (_loop_it_0 <= min((LEN_2D - 1), (((-64 * _skew_p_0) + (64 * _skew_t_0)) + 64))); _loop_it_0 = (_loop_it_0 + 1)) {
        for (int64_t _loop_it_1 = max(_loop_it_0, ((64 * _skew_p_0) + 1)); (_loop_it_1 <= min((LEN_2D - 1), ((64 * _skew_p_0) + 64))); _loop_it_1 = (_loop_it_1 + 1)) {
            {
                double a_index;
                double a_index_0;
                double a_slice_plus_a_slice;
                double a_index_1;
                double a_slice_a_slice_plus_a_slice;
                double a_slice_a_slice_a_slice_div_3_0;

                a_index = a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)];  // _assign_a_to_a_index
                a_index_0 = a[a_idx((_loop_it_0 - 1), _loop_it_1, LEN_2D)];  // _assign_a_to_a_index_0
                a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
                a_index_1 = a[a_idx(_loop_it_0, (_loop_it_1 - 1), LEN_2D)];  // _assign_a_to_a_index_1
                a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
                a_slice_a_slice_a_slice_div_3_0 = (a_slice_a_slice_plus_a_slice / 3.0);  // _Div_
                a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = a_slice_a_slice_a_slice_div_3_0;  // _assign_a_slice_a_slice_a_slice_div_3_0_to_a

            }

        }

    }
}

inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, double* __restrict__ a, int64_t LEN_2D) {

    for (int64_t _skew_t_0 = 0; (_skew_t_0 <= ((2 * int_ceil((LEN_2D - 1), 64)) - 2)); _skew_t_0 = (_skew_t_0 + 1)) {

        #pragma omp for
        for (int64_t _skew_p_0 = Max(0, ((_skew_t_0 - int_ceil((LEN_2D - 1), 64)) + 1)); _skew_p_0 < (Min(_skew_t_0, (int_ceil((LEN_2D - 1), 64) - 1)) + 1); _skew_p_0 += 1) {
            loop_body_2_0_0(__state, &a[0], LEN_2D, _skew_p_0, _skew_t_0);
        }

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t LEN_2D)
{

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    {
        #pragma omp parallel
        {
            nested_sdfg_0_1_0(__state, &a[0], LEN_2D);
        }
    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, LEN_2D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_2D)
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
