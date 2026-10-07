/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t out_index_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_value_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    double _argmax_val_for_18;
    int64_t _argmax_idx_for_18;
    double x;
    int64_t idx;

    {

        {  // for_18_argreduce_openmp
            double* __restrict__ _in = &a[0];
            double _out_val;
            int64_t _out_idx;
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = _in[0]; __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < (LEN_1D); ++__i) {
                const double __v = _in[__i];
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            _argmax_val_for_18 = _out_val;
            _argmax_idx_for_18 = _out_idx;
        }

    }
    x = _argmax_val_for_18;
    idx = _argmax_idx_for_18;
    {

        out_index[out_index_idx(0)] = idx;  // assign_23_4
        out_value[out_value_idx(0)] = x;  // symassign

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out_index, out_value, LEN_1D);
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
