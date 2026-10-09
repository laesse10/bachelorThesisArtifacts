/* extractedMatrix-style follow-up variant `dnoomp` of DaCe's canon/new code for wf_diff_skew
   (llr40Matrix/v2/dace/emitted/wf_diff_skew/canon_new): its 2 `#pragma omp` line(s) removed (the loops and blocks they annotate stay), so nothing
   is outlined into an OpenMP region and no worksharing loop has a barrier. */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, double* __restrict__ a, int64_t LEN_2D) {

    for (int64_t _loop_it_0 = 1; (_loop_it_0 < LEN_2D); _loop_it_0 = (_loop_it_0 + 1)) {

        for (int64_t _loop_it_1 = 0; _loop_it_1 < (LEN_2D - 1); _loop_it_1 += 1) {
            double a_index_1;
            double a_slice_a_slice_plus_a_slice;
            double a_slice_plus_a_slice;
            double a_index_0;
            double a_index;
            a_index = a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index
            a_index_0 = a[a_idx((_loop_it_0 - 1), _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index_0
            a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
            a_index_1 = a[a_idx((_loop_it_0 - 1), (_loop_it_1 + 1), LEN_2D)];  // _assign_in_a_to_a_index_1
            a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = a_slice_a_slice_plus_a_slice;  // _assign_out_a_slice_a_slice_plus_a_slice_to_a
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
