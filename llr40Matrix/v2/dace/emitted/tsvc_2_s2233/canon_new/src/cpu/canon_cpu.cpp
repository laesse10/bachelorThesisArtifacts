/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

#ifdef _OPENMP
#include <omp.h>
#endif
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, const double* __restrict__ cc, double* __restrict__ aa, double* __restrict__ bb, int64_t LEN_2D, int __dace_band, int __dace_num_threads) {

    for (int64_t _loop_it_1 = 8; (_loop_it_1 < LEN_2D); _loop_it_1 = (_loop_it_1 + 1)) {

        for (int64_t _loop_it_3 = (py_floor((__dace_band * (LEN_2D - 8)), __dace_num_threads) + 8); _loop_it_3 < (py_floor(((LEN_2D - 8) * (__dace_band + 1)), __dace_num_threads) + 8); _loop_it_3 += 1) {
            double aa_slice_plus_cc_slice;
            double bb_slice_plus_cc_slice;
            double aa_index;
            double bb_index;
            aa_index = aa[aa_idx((_loop_it_1 - 1), _loop_it_3, LEN_2D)];  // _assign_in_aa_to_aa_index
            aa_slice_plus_cc_slice = (aa_index + cc[cc_idx(_loop_it_1, _loop_it_3, LEN_2D)]);  // _Add_
            aa[aa_idx(_loop_it_1, _loop_it_3, LEN_2D)] = aa_slice_plus_cc_slice;  // _assign_out_aa_slice_plus_cc_slice_to_aa
            bb_index = bb[bb_idx((_loop_it_1 - 1), _loop_it_3, LEN_2D)];  // _assign_in_bb_to_bb_index
            bb_slice_plus_cc_slice = (bb_index + cc[cc_idx(_loop_it_1, _loop_it_3, LEN_2D)]);  // _Add_
            bb[bb_idx(_loop_it_1, _loop_it_3, LEN_2D)] = bb_slice_plus_cc_slice;  // _assign_out_bb_slice_plus_cc_slice_to_bb
        }

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    #ifdef _OPENMP
    const int64_t __dace_num_threads = omp_get_max_threads();
    #else
    const int64_t __dace_num_threads = 1;
    #endif

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    #pragma omp parallel for
    for (int __dace_band = 0; __dace_band < __dace_num_threads; __dace_band += 1) {
        nested_sdfg_0_1_0(__state, &cc[0], &aa[0], &bb[0], LEN_2D, __dace_band, __dace_num_threads);
    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, aa, bb, cc, LEN_2D);
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
