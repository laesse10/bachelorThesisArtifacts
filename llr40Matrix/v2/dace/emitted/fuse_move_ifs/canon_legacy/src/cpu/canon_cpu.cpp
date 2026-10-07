/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

inline void loop_body_0_1_3(canon_cpu_state_t *__state, const double* __restrict__ cond, const double* __restrict__ src, double* __restrict__ a, double* __restrict__ b, int64_t K, int64_t LEN_2D, int64_t _loop_it_0, int64_t _loop_it_1) {
    double cond_index;


    cond_index = cond[_loop_it_0];
    if (((cond_index > 0.0) && (K > 0))) {
        {

            {
                double __in1 = src[((LEN_2D * _loop_it_0) + _loop_it_1)];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * 2.0);
                ///////////////////

                a[((LEN_2D * _loop_it_0) + _loop_it_1)] = __out;
            }
            {
                double __in1 = src[((LEN_2D * _loop_it_0) + _loop_it_1)];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + 1.0);
                ///////////////////

                b[((LEN_2D * _loop_it_0) + _loop_it_1)] = __out;
            }

        }
    } else if (((! (cond_index > 0.0)) && (K > 0))) {
        {

            {
                double __in1 = src[((LEN_2D * _loop_it_0) + _loop_it_1)];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + 1.0);
                ///////////////////

                b[((LEN_2D * _loop_it_0) + _loop_it_1)] = __out;
            }

        }
    } else if (((cond_index > 0.0) && (! (K > 0)))) {
        {

            {
                double __in1 = src[((LEN_2D * _loop_it_0) + _loop_it_1)];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * 2.0);
                ///////////////////

                a[((LEN_2D * _loop_it_0) + _loop_it_1)] = __out;
            }

        }
    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ cond, double * __restrict__ src, int64_t K, int64_t LEN_2D)
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
            for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_2D; _loop_it_0 += 1) {
                for (int64_t _loop_it_1 = 0; _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
                    loop_body_0_1_3(__state, &cond[0], &src[0], &a[0], &b[0], K, LEN_2D, _loop_it_0, _loop_it_1);
                }
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ cond, double * __restrict__ src, int64_t K, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, b, cond, src, K, LEN_2D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t K, int64_t LEN_2D)
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
