/* extractedMatrix-style follow-up variant `dnolib` of DaCe's canon/new code for tsvc_2_s3112
   (llr40Matrix/v2/dace/emitted/tsvc_2_s3112/canon_new): dace::scan::inclusive_sum written as a plain sequential loop (same additions, same order). */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "cstring"
#include "numeric"
#include "functional"
#include "algorithm"
#include "dace/scan.hpp"
constexpr double __sum = 0.0;

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{

    {
        double _scan_seed_b;

        {  // _assign___sum_to__scan_seed_b
            double _in = __sum;
            double _out;
            _out = _in;
            _scan_seed_b = _out;
        }
        {  // for_19_scan_op_op
            { double acc_ = _scan_seed_b; for (int64_t k_ = 0; k_ < LEN_1D; ++k_) { acc_ = acc_ + a[k_]; b[k_] = acc_; } }  /* follow-up `dnolib` */
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, LEN_1D);
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
