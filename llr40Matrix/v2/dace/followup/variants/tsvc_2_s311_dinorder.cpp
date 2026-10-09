/* extractedMatrix-style follow-up variant `dinorder` of DaCe's canon/new code for tsvc_2_s311
   (llr40Matrix/v2/dace/emitted/tsvc_2_s311/canon_new): dace::reduce::sum (an `omp parallel for simd reduction`, which may reorder the additions)
   written as a plain in-order loop. */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t sum_out_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ sum_out, int64_t LEN_1D)
{

    {

        sum_out[sum_out_idx(0)] = 0.0;  // assign_16_4
        {  // reduce
            { double acc_ = sum_out[0]; for (long k_ = 0; k_ < (long)(LEN_1D); ++k_) acc_ = acc_ + a[k_]; sum_out[0] = acc_; }  /* follow-up `dinorder` */
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ sum_out, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, sum_out, LEN_1D);
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
