/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, double * __restrict__ cc, double * __restrict__ d, int64_t LEN_2D)
{

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    #pragma omp parallel for
    for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
        for (int64_t _loop_it_2 = 0; _loop_it_2 < LEN_2D; _loop_it_2 += 1) {
            double bb_slice_times_cc_slice;
            double _wcr_priv__Add____out;
            bb_slice_times_cc_slice = (bb[bb_idx(_loop_it_1, _loop_it_2, LEN_2D)] * cc[cc_idx(_loop_it_1, _loop_it_2, LEN_2D)]);  // _Mult_
            _wcr_priv__Add____out = bb_slice_times_cc_slice;  // _Add_
            aa[aa_idx(_loop_it_1, _loop_it_2, LEN_2D)] = (aa[aa_idx(_loop_it_1, _loop_it_2, LEN_2D)] + _wcr_priv__Add____out);  // augassign
        }
    }
    #pragma omp parallel for
    for (int64_t _loop_it_3 = 0; _loop_it_3 < LEN_2D; _loop_it_3 += 1) {
        double c_slice_times_d_slice;
        c_slice_times_d_slice = (c[c_idx(_loop_it_3)] * d[d_idx(_loop_it_3)]);  // _Mult_
        a[a_idx(_loop_it_3)] = (b[b_idx(_loop_it_3)] + c_slice_times_d_slice);  // _Add_
    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, double * __restrict__ cc, double * __restrict__ d, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, b, bb, c, cc, d, LEN_2D);
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
