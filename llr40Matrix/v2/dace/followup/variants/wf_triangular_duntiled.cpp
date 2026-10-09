/* extractedMatrix-style follow-up variant `duntiled` of DaCe's canon/new code for wf_triangular
   (llr40Matrix/v2/dace/emitted/wf_triangular/canon_new): nested_sdfg_0_1_0, the skewed and 64x64-tiled wavefront, replaced by the plain row-by-row
   loop of the translated code (same operations, same association). The parallel region around it stays. */
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
    /* follow-up `duntiled`: the skewed 64x64 tiles undone, the translated code's row-by-row order */
    for (int64_t i_ = 1; i_ < LEN_2D; ++i_) {
        for (int64_t j_ = i_; j_ < LEN_2D; ++j_) {
            a[(i_ * LEN_2D) + j_] = (((a[(i_ * LEN_2D) + j_] + a[((i_ - 1) * LEN_2D) + j_]) + a[(i_ * LEN_2D) + (j_ - 1)]) / 3.0);
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
