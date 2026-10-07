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

inline void copy_a_to__scan_seed_a_sdfg_2_0_11(canon_cpu_state_t *__state, const double* __restrict__ a, double* __restrict__ _cpy_out, int64_t K) {

    {
        const double* _cpy_in;
        _cpy_in = &a[0];

        {
            #pragma omp parallel for
            for (int64_t __i0 = 0; __i0 < K; __i0 += 1) {
                {
                    double _in = _cpy_in[__i0];
                    double _out;

                    ///////////////////
                    // Tasklet code (copy_a_to__scan_seed_a_tasklet)
                    _out = _in;
                    ///////////////////

                    _cpy_out[__i0] = _out;
                }
            }
        }

    }
}

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
    int64_t _loop_it_2;
    int64_t _loop_it_3;

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((K < 0)) {
                std::abort();
            }
            ///////////////////

        }
        {

            ///////////////////
            // Tasklet code (check_assumption_1)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }

    if ((K >= 1)) {
        {
            double *_scan_in_a;
            _scan_in_a = new (std::align_val_t(64)) double[((- K) + LEN_1D)];
            double *_scan_coef_a;
            _scan_coef_a = new (std::align_val_t(64)) double[((- K) + LEN_1D)];

            {
                #pragma omp parallel for
                for (int64_t _loop_it_1 = K; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
                    {
                        double __aff_in1 = b[_loop_it_1];
                        double __aff_in2 = c[_loop_it_1];
                        double __out;

                        ///////////////////
                        // Tasklet code (affine_delta)
                        __out = (__aff_in1 * __aff_in2);
                        ///////////////////

                        _scan_in_a[((- K) + _loop_it_1)] = __out;
                    }
                    {
                        double __out;

                        ///////////////////
                        // Tasklet code (affine_coef)
                        __out = 0.75;
                        ///////////////////

                        _scan_coef_a[((- K) + _loop_it_1)] = __out;
                    }
                }
            }
            copy_a_to__scan_seed_a_sdfg_2_0_11(__state, &a[0], &__state->__0__scan_seed_a[0], K);
            {
                double* __restrict__ _scan_init = &__state->__0__scan_seed_a[0];
                double* __restrict__ _scan_in = &_scan_in_a[0];
                double* __restrict__ _scan_coef = &_scan_coef_a[0];
                double* __restrict__ _scan_out = a + K;

                ///////////////////
                ::dace::scan::inclusive_affine_strided(_scan_coef, _scan_in, _scan_out, static_cast<long>(((- K) + LEN_1D)), static_cast<long>(K), _scan_init);
                ///////////////////

            }
            static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
            ::operator delete[](_scan_in_a, std::align_val_t(64));
            static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
            ::operator delete[](_scan_coef_a, std::align_val_t(64));

        }
    } else if (((! (K >= 1)) && (K == 0))) {

        for (_loop_it_2 = K; (_loop_it_2 < LEN_1D); _loop_it_2 = (_loop_it_2 + 1)) {
            {

                {
                    double _in = a[((- K) + _loop_it_2)];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_a_to_a_index_0)
                    _out = _in;
                    ///////////////////

                    a_index_0 = _out;
                }
                {
                    double __in2 = a_index_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (0.75 * __in2);
                    ///////////////////

                    __tmp0_0 = __out;
                }
                {
                    double __in1 = b[_loop_it_2];
                    double __in2 = c[_loop_it_2];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_c_slice_0 = __out;
                }
                {
                    double __in2 = b_slice_times_c_slice_0;
                    double __in1 = __tmp0_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    __tmp1_0 = __out;
                }
                {
                    double _in = __tmp1_0;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign___tmp1_0_to_a)
                    _out = _in;
                    ///////////////////

                    a[_loop_it_2] = _out;
                }

            }

        }


    } else {

        for (_loop_it_3 = K; (_loop_it_3 < LEN_1D); _loop_it_3 = (_loop_it_3 + 1)) {
            {

                {
                    double _in = a[((- K) + _loop_it_3)];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_a_to_a_index_1)
                    _out = _in;
                    ///////////////////

                    a_index_1 = _out;
                }
                {
                    double __in2 = a_index_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (0.75 * __in2);
                    ///////////////////

                    __tmp0_1 = __out;
                }
                {
                    double __in1 = b[_loop_it_3];
                    double __in2 = c[_loop_it_3];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_c_slice_1 = __out;
                }
                {
                    double __in2 = b_slice_times_c_slice_1;
                    double __in1 = __tmp0_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    __tmp1_1 = __out;
                }
                {
                    double _in = __tmp1_1;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign___tmp1_1_to_a)
                    _out = _in;
                    ///////////////////

                    a[_loop_it_3] = _out;
                }

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
