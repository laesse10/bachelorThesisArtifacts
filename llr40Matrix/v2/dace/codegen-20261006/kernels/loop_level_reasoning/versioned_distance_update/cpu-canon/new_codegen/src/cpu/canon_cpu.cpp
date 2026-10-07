/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "cstring"
#include "numeric"
#include "functional"
#include "algorithm"
#include "dace/scan.hpp"

struct canon_cpu_state_t {
    double * __restrict__ __0__scan_seed_a;
};

static DACE_HDFI constexpr int64_t _scan_in_a_size(int64_t K, int64_t LEN_1D) { return ((- K) + LEN_1D); }
static DACE_HDFI constexpr int64_t _scan_coef_a_size(int64_t K, int64_t LEN_1D) { return ((- K) + LEN_1D); }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_in_a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_coef_a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _cpy_in_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _cpy_out_idx(int64_t __d0) { return __d0; }
inline void copy_a_to__scan_seed_a_sdfg_2_0_12(canon_cpu_state_t *__state, const double* __restrict__ a, double* __restrict__ _cpy_out, int64_t K) {

    {
        const double* _cpy_in;
        _cpy_in = &a[0];

        #pragma omp parallel for
        for (int64_t __i0 = 0; __i0 < K; __i0 += 1) {
            _cpy_out[_cpy_out_idx(__i0)] = _cpy_in[_cpy_in_idx(__i0)];  // copy_a_to__scan_seed_a_tasklet
        }

    }
}

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    double a_index_0;
    double a_index_1;
    double __tmp0_0;
    double __tmp0_1;
    double b_slice_times_c_slice_0;
    double b_slice_times_c_slice_1;
    double __tmp1_0;
    double __tmp1_1;

    {

        {  // check_assumption_0
            if ((K < 0)) {
                std::abort();
            }
        }
        {  // check_assumption_1
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    if ((K >= 1)) {
        {
            double* __restrict__ _scan_in_a = new (std::align_val_t(64)) double[_scan_in_a_size(K, LEN_1D)];
            double* __restrict__ _scan_coef_a = new (std::align_val_t(64)) double[_scan_coef_a_size(K, LEN_1D)];

            #pragma omp parallel for
            for (int64_t _loop_it_1 = K; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
                _scan_in_a[_scan_in_a_idx(((- K) + _loop_it_1))] = (b[b_idx(_loop_it_1)] * c[c_idx(_loop_it_1)]);  // affine_delta
                _scan_coef_a[_scan_coef_a_idx(((- K) + _loop_it_1))] = 0.75;  // affine_coef
            }
            copy_a_to__scan_seed_a_sdfg_2_0_12(__state, &a[0], &__state->__0__scan_seed_a[0], K);
            {  // for_17_affine_scan_op
                ::dace::scan::inclusive_affine_strided(_scan_coef_a, _scan_in_a, (a + K), static_cast<long>(((- K) + LEN_1D)), static_cast<long>(K), (__state->__0__scan_seed_a));
            }
            static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
            ::operator delete[](_scan_in_a, std::align_val_t(64));
            static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
            ::operator delete[](_scan_coef_a, std::align_val_t(64));

        }
    } else if (((! (K >= 1)) && (K == 0))) {

        for (int64_t _loop_it_2 = K; (_loop_it_2 < LEN_1D); _loop_it_2 = (_loop_it_2 + 1)) {
            {

                a_index_0 = a[a_idx(((- K) + _loop_it_2))];  // _assign_a_to_a_index_0
                __tmp0_0 = (0.75 * a_index_0);  // _Mult_
                b_slice_times_c_slice_0 = (b[b_idx(_loop_it_2)] * c[c_idx(_loop_it_2)]);  // _Mult_
                __tmp1_0 = (__tmp0_0 + b_slice_times_c_slice_0);  // _Add_
                a[a_idx(_loop_it_2)] = __tmp1_0;  // _assign___tmp1_0_to_a

            }

        }


    } else {

        for (int64_t _loop_it_3 = K; (_loop_it_3 < LEN_1D); _loop_it_3 = (_loop_it_3 + 1)) {
            {

                a_index_1 = a[a_idx(((- K) + _loop_it_3))];  // _assign_a_to_a_index_1
                __tmp0_1 = (0.75 * a_index_1);  // _Mult_
                b_slice_times_c_slice_1 = (b[b_idx(_loop_it_3)] * c[c_idx(_loop_it_3)]);  // _Mult_
                __tmp1_1 = (__tmp0_1 + b_slice_times_c_slice_1);  // _Add_
                a[a_idx(_loop_it_3)] = __tmp1_1;  // _assign___tmp1_1_to_a

            }

        }


    }


}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, K, LEN_1D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t K, int64_t LEN_1D)
{

    int __result = 0;
    canon_cpu_state_t *__state = new canon_cpu_state_t();

    if (__result) {
        delete __state;
        return nullptr;
    }
    __state->__0__scan_seed_a = new (std::align_val_t(64)) double[K];

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
    ::operator delete[](__state->__0__scan_seed_a, std::align_val_t(64));
    delete __state;
    return __err;
}
