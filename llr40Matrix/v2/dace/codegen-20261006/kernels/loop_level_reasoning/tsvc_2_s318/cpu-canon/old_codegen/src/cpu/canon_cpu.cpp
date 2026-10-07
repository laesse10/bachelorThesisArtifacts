/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    double _argfi_val_for_20;
    int64_t _argfi_idx_for_20;
    double maxv;
    int64_t index;

    {

        {
            double* __restrict__ _in = &a[0];
            double _out_val;
            int64_t _out_idx;

            ///////////////////
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = std::abs(_in[(0) * (inc)]); __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < (LEN_1D); ++__i) {
                const double __v = std::abs(_in[(__i) * (inc)]);
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            ///////////////////

            _argfi_val_for_20 = _out_val;
            _argfi_idx_for_20 = _out_idx;
        }

    }
    maxv = _argfi_val_for_20;
    index = _argfi_idx_for_20;
    {
        double float_index;

        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(index);
            ///////////////////

            float_index = __out;
        }
        {
            double __in2 = float_index;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (maxv + __in2);
            ///////////////////

            result[0] = __out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    __program_canon_cpu_internal(__state, a, result, LEN_1D, inc);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_1D, int64_t inc)
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
