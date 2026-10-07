/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, double * __restrict__ cc, double * __restrict__ d, int64_t LEN_2D)
{

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
            #pragma omp parallel for
            for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
                for (int64_t _loop_it_2 = 0; _loop_it_2 < LEN_2D; _loop_it_2 += 1) {
                    double bb_slice_times_cc_slice;
                    double _wcr_priv__Add____out;
                    {
                        double __in1 = bb[((LEN_2D * _loop_it_1) + _loop_it_2)];
                        double __in2 = cc[((LEN_2D * _loop_it_1) + _loop_it_2)];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        bb_slice_times_cc_slice = __out;
                    }
                    {
                        double __in2 = bb_slice_times_cc_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = __in2;
                        ///////////////////

                        _wcr_priv__Add____out = __out;
                    }
                    {
                        double __in1 = aa[((LEN_2D * _loop_it_1) + _loop_it_2)];
                        double __in2 = _wcr_priv__Add____out;
                        double __out;

                        ///////////////////
                        // Tasklet code (augassign)
                        __out = (__in1 + __in2);
                        ///////////////////

                        aa[((LEN_2D * _loop_it_1) + _loop_it_2)] = __out;
                    }
                }
            }
        }
        {
            #pragma omp parallel for
            for (int64_t _loop_it_3 = 0; _loop_it_3 < LEN_2D; _loop_it_3 += 1) {
                double c_slice_times_d_slice;
                {
                    double __in1 = c[_loop_it_3];
                    double __in2 = d[_loop_it_3];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    c_slice_times_d_slice = __out;
                }
                {
                    double __in2 = c_slice_times_d_slice;
                    double __in1 = b[_loop_it_3];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a[_loop_it_3] = __out;
                }
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, double * __restrict__ cc, double * __restrict__ d, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, b, bb, c, cc, d, LEN_2D);
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
