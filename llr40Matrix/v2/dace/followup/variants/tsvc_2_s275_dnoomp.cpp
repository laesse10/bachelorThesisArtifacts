/* extractedMatrix-style follow-up variant `dnoomp` of DaCe's canon/new code for tsvc_2_s275
   (llr40Matrix/v2/dace/emitted/tsvc_2_s275/canon_new): its 1 `#pragma omp` line(s) removed (the band loop runs once, band 0 of 1). */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

#ifdef _OPENMP
#include <omp.h>
#endif
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
inline void loop_body_2_0_0(canon_cpu_state_t *__state, const double* __restrict__ bb, const double* __restrict__ cc, double* __restrict__ aa, int64_t LEN_2D, int64_t _loop_it_0, int64_t _loop_it_1) {
    double aa_index;


    aa_index = aa[_loop_it_0];
    if ((aa_index > 0.0)) {
        {
            double bb_slice_times_cc_slice;
            double aa_slice_plus_bb_slice_cc_slice;
            double aa_index_0;

            aa_index_0 = aa[aa_idx((_loop_it_1 - 1), _loop_it_0, LEN_2D)];  // _assign_aa_to_aa_index_0
            bb_slice_times_cc_slice = (bb[bb_idx(_loop_it_1, _loop_it_0, LEN_2D)] * cc[cc_idx(_loop_it_1, _loop_it_0, LEN_2D)]);  // _Mult_
            aa_slice_plus_bb_slice_cc_slice = (aa_index_0 + bb_slice_times_cc_slice);  // _Add_
            aa[aa_idx(_loop_it_1, _loop_it_0, LEN_2D)] = aa_slice_plus_bb_slice_cc_slice;  // _assign_aa_slice_plus_bb_slice_cc_slice_to_aa

        }
    }
}

inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, const double* __restrict__ bb, const double* __restrict__ cc, double* __restrict__ aa, int64_t LEN_2D, int __dace_band, int __dace_num_threads) {

    for (int64_t _loop_it_1 = 1; (_loop_it_1 < LEN_2D); _loop_it_1 = (_loop_it_1 + 1)) {

        for (int64_t _loop_it_0 = py_floor((LEN_2D * __dace_band), __dace_num_threads); _loop_it_0 < py_floor((LEN_2D * (__dace_band + 1)), __dace_num_threads); _loop_it_0 += 1) {
            loop_body_2_0_0(__state, &bb[0], &cc[0], &aa[0], LEN_2D, _loop_it_0, _loop_it_1);
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

    for (int __dace_band = 0; __dace_band < __dace_num_threads; __dace_band += 1) {
        nested_sdfg_0_1_0(__state, &bb[0], &cc[0], &aa[0], LEN_2D, __dace_band, __dace_num_threads);
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
