/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

inline void loop_body_0_1_0(canon_cpu_state_t *__state, const int64_t* __restrict__ row_ptr, const double* __restrict__ val, const double* __restrict__ w, double* __restrict__ out, int64_t _loop_it_0) {
    double acc;
    double _priv_acc;
    int64_t row_ptr_index;
    int64_t row_ptr_index_0;

    {

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_17_8)
            __out = 0.0;
            ///////////////////

            acc = __out;
        }

    }
    row_ptr_index = row_ptr[_loop_it_0];
    row_ptr_index_0 = row_ptr[(_loop_it_0 + 1)];
    {


        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &acc, &_priv_acc, 1);

    }
    {

        {
            for (int64_t _loop_it_1 = row_ptr_index; _loop_it_1 < row_ptr_index_0; _loop_it_1 += 1) {
                double val_slice_times_w_slice;
                double acc_plus_val_slice_w_slice;
                double _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out;
                {
                    double __in1 = val[_loop_it_1];
                    double __in2 = w[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    val_slice_times_w_slice = __out;
                }
                {
                    double __in2 = val_slice_times_w_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    acc_plus_val_slice_w_slice = __out;
                }
                {
                    double _in = acc_plus_val_slice_w_slice;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_out_acc_plus_val_slice_w_slice_to__priv_acc)
                    _out = _in;
                    ///////////////////

                    _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out = _out;
                }

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Accumulate(
                &_wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out, &_priv_acc, [] (const double& a, const double& b) { return (a + b); }, 1);
            }
        }

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &_priv_acc, &acc, 1);
        {
            double _in = acc;
            double _out;

            ///////////////////
            // Tasklet code (_assign_acc_to_out)
            _out = _in;
            ///////////////////

            out[_loop_it_0] = _out;
        }

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((NSEG < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {

        {
            #pragma omp parallel for
            for (int64_t _loop_it_0 = 0; _loop_it_0 < NSEG; _loop_it_0 += 1) {
                loop_body_0_1_0(__state, &row_ptr[0], &val[0], &w[0], &out[0], _loop_it_0);
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{
    __program_canon_cpu_internal(__state, out, row_ptr, val, w, NSEG);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t NSEG)
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
