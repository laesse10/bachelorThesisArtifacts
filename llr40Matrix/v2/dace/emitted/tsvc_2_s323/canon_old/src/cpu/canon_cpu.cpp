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

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {
        double _scan_seed_b;

        {
            #pragma omp parallel for
            for (int64_t _loop_it_1 = 1; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
                double c_slice_times_d_slice;
                double c_slice_times_e_slice;
                double b_slice;
                double a_fwd;
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = d[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    c_slice_times_d_slice = __out;
                }
                {
                    double _in = c_slice_times_d_slice;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_c_slice_times_d_slice_to_a_fwd)
                    _out = _in;
                    ///////////////////

                    a_fwd = _out;
                }
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = e[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    c_slice_times_e_slice = __out;
                }
                {
                    double __in2 = c_slice_times_e_slice;
                    double __in1 = a_fwd;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b_slice = __out;
                }
                {
                    double _in = b_slice;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_out_b_slice_to__scan_in_b)
                    _out = _in;
                    ///////////////////

                    __state->__0__scan_in_b[(_loop_it_1 - 1)] = _out;
                }
            }
        }
        {
            double _in = b[0];
            double _out;

            ///////////////////
            // Tasklet code (_assign_b_to__scan_seed_b)
            _out = _in;
            ///////////////////

            _scan_seed_b = _out;
        }
        {
            double _scan_init = _scan_seed_b;
            double* __restrict__ _scan_in = &__state->__0__scan_in_b[0];
            double* __restrict__ _scan_out = b + 1;

            ///////////////////
            ::dace::scan::inclusive_sum(_scan_in, _scan_in + ((LEN_1D - 1)), _scan_out, _scan_init);
            ///////////////////

        }
        {
            #pragma omp parallel for
            for (int64_t _loop_it_2 = 1; _loop_it_2 < LEN_1D; _loop_it_2 += 1) {
                double nested_sdfg_c_slice_times_d_slice;
                {
                    double __in1 = c[_loop_it_2];
                    double __in2 = d[_loop_it_2];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    nested_sdfg_c_slice_times_d_slice = __out;
                }
                {
                    double __in2 = nested_sdfg_c_slice_times_d_slice;
                    double __in1 = b[(_loop_it_2 - 1)];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a[_loop_it_2] = __out;
                }
            }
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
    __state->__0__scan_in_b = new (std::align_val_t(64)) double[(LEN_1D - 1)];

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
