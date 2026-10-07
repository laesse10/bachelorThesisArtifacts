/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
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
        double sum_val_0;

        {
            #pragma omp parallel for
            for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_1D; _loop_it_1 += 1) {
                double sum_val_plus_a_slice;
                double a_fwd;
                double b_fwd;
                double _fused_inc;
                double _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = d[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a_fwd = __out;
                }
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = e[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b_fwd = __out;
                }
                {
                    double __in1 = a_fwd;
                    double __in2 = b_fwd;
                    double __out;

                    ///////////////////
                    // Tasklet code (_fuse_red_1)
                    __out = (__in1 + __in2);
                    ///////////////////

                    _fused_inc = __out;
                }
                {
                    double __in2 = _fused_inc;
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
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = d[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a[_loop_it_1] = __out;
                }
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = e[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b[_loop_it_1] = __out;
                }
            }
        }

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &_priv_sum_val, &sum_val_0, 1);
        {
            double _in = sum_val_0;
            double _out;

            ///////////////////
            // Tasklet code (_assign_sum_val_0_to_b)
            _out = _in;
            ///////////////////

            b[0] = _out;
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
