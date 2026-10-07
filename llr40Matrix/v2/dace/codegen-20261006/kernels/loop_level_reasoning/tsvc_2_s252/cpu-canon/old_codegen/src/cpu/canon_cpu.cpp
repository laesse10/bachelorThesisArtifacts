/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
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
        double t;
        double s_0;

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_16_4)
            __out = 0.0;
            ///////////////////

            t = __out;
        }
        {
            double __in1 = b[0];
            double __in2 = c[0];
            double __out;

            ///////////////////
            // Tasklet code (_Mult_)
            __out = (__in1 * __in2);
            ///////////////////

            s_0 = __out;
        }
        {
            double __in1 = s_0;
            double __in2 = t;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            a[0] = __out;
        }
        {
            #pragma omp parallel for
            for (int64_t _loop_it_0 = 1; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
                double s_1;
                double t_remat;
                double t_remat_0;
                {
                    double __in1 = b[_loop_it_0];
                    double __in2 = c[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    s_1 = __out;
                }
                {
                    double __in1 = b[(_loop_it_0 - 1)];
                    double __in2 = c[(_loop_it_0 - 1)];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult__remat)
                    __out = (__in1 * __in2);
                    ///////////////////

                    t_remat = __out;
                }
                {
                    double __in1 = t_remat;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add__remat)
                    __out = (__in1 + 0.0);
                    ///////////////////

                    t_remat_0 = __out;
                }
                {
                    double __in1 = s_1;
                    double __in2 = t_remat_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a[_loop_it_0] = __out;
                }
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, LEN_1D);
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
