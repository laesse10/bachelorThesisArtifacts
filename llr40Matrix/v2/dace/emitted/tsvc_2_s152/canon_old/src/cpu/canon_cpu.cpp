/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

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

        {
            #pragma omp parallel for
            for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
                double b_slice_times_c_slice;
                double _wcr_priv__Add____out;
                double __map_fusion_b;
                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = e[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    __map_fusion_b = __out;
                }

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                &__map_fusion_b, b + _loop_it_0, 1);
                {
                    double __in1 = __map_fusion_b;
                    double __in2 = c[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_c_slice = __out;
                }
                {
                    double __in2 = b_slice_times_c_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out = __out;
                }
                {
                    double __in1 = a[_loop_it_0];
                    double __in2 = _wcr_priv__Add____out;
                    double __out;

                    ///////////////////
                    // Tasklet code (augassign)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a[_loop_it_0] = __out;
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
