/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double _priv_sum_val;

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

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_16_4)
            __out = 0.0;
            ///////////////////

            _priv_sum_val = __out;
        }

    }
    {

        {
            #pragma omp parallel for
            for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
                double sum_val_plus_a_slice;
                double sum_val_masked_val;
                double _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
                {
                    double __addend = a[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (sum_val_mask)
                    __out = ((__addend > 0.0) ? __addend : 0.0);
                    ///////////////////

                    sum_val_masked_val = __out;
                }
                {
                    double __in2 = sum_val_masked_val;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    sum_val_plus_a_slice = __out;
                }
                {
                    double _in = sum_val_plus_a_slice;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_out_sum_val_plus_a_slice_to__priv_sum_val)
                    _out = _in;
                    ///////////////////

                    _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out = _out;
                }
                dace::wcr_fixed<dace::ReductionType::Sum, double>::reduce_atomic(&_priv_sum_val, *(&_wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out));
            }
        }
        {
            double _in = _priv_sum_val;
            double _out;

            ///////////////////
            // Tasklet code (_assign_sum_val_0_to_b)
            _out = _in;
            ///////////////////

            b[0] = _out;
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
