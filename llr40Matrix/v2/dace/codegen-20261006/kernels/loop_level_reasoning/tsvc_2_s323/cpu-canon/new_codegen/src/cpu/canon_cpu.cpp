/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
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
        double _scan_seed_b;

        #pragma omp parallel for
        for (int64_t _loop_it_1 = 1; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
            double c_slice_times_d_slice;
            double c_slice_times_e_slice;
            double b_slice;
            double a_fwd;
            c_slice_times_d_slice = (c[c_idx(_loop_it_1)] * d[d_idx(_loop_it_1)]);  // _Mult_
            a_fwd = c_slice_times_d_slice;  // _assign_c_slice_times_d_slice_to_a_fwd
            c_slice_times_e_slice = (c[c_idx(_loop_it_1)] * e[e_idx(_loop_it_1)]);  // _Mult_
            b_slice = (a_fwd + c_slice_times_e_slice);  // _Add_
            __state->__0__scan_in_b[_scan_in_b_idx((_loop_it_1 - 1))] = b_slice;  // _assign_out_b_slice_to__scan_in_b
        }
        {  // _assign_b_to__scan_seed_b
            double _out;
            _out = b[b_idx(0)];
            _scan_seed_b = _out;
        }
        {  // for_16_0_scan_op
            ::dace::scan::inclusive_sum((__state->__0__scan_in_b), (__state->__0__scan_in_b) + ((LEN_1D - 1)), (b + 1), _scan_seed_b);
        }
        #pragma omp parallel for
        for (int64_t _loop_it_2 = 1; _loop_it_2 < LEN_1D; _loop_it_2 += 1) {
            double nested_sdfg_c_slice_times_d_slice;
            nested_sdfg_c_slice_times_d_slice = (c[c_idx(_loop_it_2)] * d[d_idx(_loop_it_2)]);  // _Mult_
            a[a_idx(_loop_it_2)] = (b[b_idx((_loop_it_2 - 1))] + nested_sdfg_c_slice_times_d_slice);  // _Add_
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
