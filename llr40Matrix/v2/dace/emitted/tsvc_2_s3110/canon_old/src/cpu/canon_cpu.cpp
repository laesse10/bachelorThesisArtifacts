/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double _argmax2d_val_for_19;
    int64_t _argmax2d_idx_for_19;
    double maxv;
    int64_t xindex;
    int64_t yindex;

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_2D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {

        {
            double* __restrict__ _in = &aa[0];
            double _out_val;
            int64_t _out_idx;

            ///////////////////
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = _in[0]; __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < ((LEN_2D * LEN_2D)); ++__i) {
                const double __v = _in[__i];
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            ///////////////////

            _argmax2d_val_for_19 = _out_val;
            _argmax2d_idx_for_19 = _out_idx;
        }

    }
    maxv = _argmax2d_val_for_19;
    xindex = py_floor(_argmax2d_idx_for_19, LEN_2D);
    yindex = py_mod(_argmax2d_idx_for_19, LEN_2D);
    {
        double float_xindex;
        double maxv_plus_expr;
        double float_yindex;

        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(xindex);
            ///////////////////

            float_xindex = __out;
        }
        {
            double __in2 = float_xindex;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (maxv + __in2);
            ///////////////////

            maxv_plus_expr = __out;
        }
        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(yindex);
            ///////////////////

            float_yindex = __out;
        }
        {
            double __in2 = float_yindex;
            double __in1 = maxv_plus_expr;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            bb[0] = __out;
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, aa, bb, LEN_2D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_2D)
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
