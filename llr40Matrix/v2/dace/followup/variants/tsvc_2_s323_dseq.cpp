/* extractedMatrix-style follow-up variant `dseq` of DaCe's canon/new code for tsvc_2_s323
   (llr40Matrix/v2/dace/emitted/tsvc_2_s323/canon_new): the map, the library scan and the second map replaced by one sequential loop, as the
   translated C++ computes it (a, then b, per element). */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "cstring"
#include "numeric"
#include "functional"
#include "algorithm"
#include "dace/scan.hpp"

struct canon_cpu_state_t {
    double * __restrict__ __0__scan_in_b;
};

static DACE_HDFI constexpr int64_t _scan_in_b_size(int64_t LEN_1D) { return (LEN_1D - 1); }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_in_b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        /* follow-up `dseq`: the recurrence as one sequential loop, the translated code's order */
        for (int64_t i_ = 1; i_ < LEN_1D; ++i_) {
            a[i_] = (b[(i_ - 1)] + (c[i_] * d[i_]));
            b[i_] = (a[i_] + (c[i_] * e[i_]));
        }
    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, e, LEN_1D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_cpu_state_t *__state = new canon_cpu_state_t();

    if (__result) {
        delete __state;
        return nullptr;
    }
    __state->__0__scan_in_b = new (std::align_val_t(64)) double[_scan_in_b_size(LEN_1D)];

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_cpu(canon_cpu_state_t *__state)
{

    int __err = 0;
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0__scan_in_b, std::align_val_t(64));
    delete __state;
    return __err;
}
