/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t val_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t w_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_idx(int64_t __d0) { return __d0; }
inline void loop_body_0_1_0(canon_cpu_state_t *__state, const int64_t* __restrict__ row_ptr, const double* __restrict__ val, const double* __restrict__ w, double* __restrict__ out, int64_t _loop_it_0) {
    double acc;
    double _priv_acc;
    int64_t row_ptr_index;
    int64_t row_ptr_index_0;

    {

        acc = 0.0;  // assign_17_8

    }
    row_ptr_index = row_ptr[_loop_it_0];
    row_ptr_index_0 = row_ptr[(_loop_it_0 + 1)];
    {

        {  // copy_acc_to__priv_acc
            double _cpy_out;
            _cpy_out = acc;
            _priv_acc = _cpy_out;
        }

    }
    {

        double __acc_1_0__priv_acc = *(&_priv_acc);
        for (int64_t _loop_it_1 = row_ptr_index; _loop_it_1 < row_ptr_index_0; _loop_it_1 += 1) {
            double val_slice_times_w_slice;
            double acc_plus_val_slice_w_slice;
            double _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out;
            val_slice_times_w_slice = (val[val_idx(_loop_it_1)] * w[w_idx(_loop_it_1)]);  // _Mult_
            acc_plus_val_slice_w_slice = val_slice_times_w_slice;  // _Add_
            _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out = acc_plus_val_slice_w_slice;  // _assign_out_acc_plus_val_slice_w_slice_to__priv_acc
            {  // copy__wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out_to__priv_acc
                double _cpy_out;
                _cpy_out = _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out;
                __acc_1_0__priv_acc = __acc_1_0__priv_acc + (_cpy_out);
            }
        }
        *(&_priv_acc) = __acc_1_0__priv_acc;
        {  // copy__priv_acc_to_acc
            double _cpy_in = _priv_acc;
            acc = _cpy_in;
        }
        out[out_idx(_loop_it_0)] = acc;  // _assign_acc_to_out

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{

    {

        {  // check_assumption_0
            if ((NSEG < 0)) {
                std::abort();
            }
        }

    }

    #pragma omp parallel for
    for (int64_t _loop_it_0 = 0; _loop_it_0 < NSEG; _loop_it_0 += 1) {
        loop_body_0_1_0(__state, &row_ptr[0], &val[0], &w[0], &out[0], _loop_it_0);
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
