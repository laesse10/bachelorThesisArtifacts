/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    double sum_val;
    double c_index;
    double d_index;
    double a_slice;
    double a_index;
    double sum_val_plus_a_slice;
    double c_index_0;
    double e_index;
    double b_slice;
    double b_index;
    double sum_val_plus_b_slice;
    int64_t i;

    {

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_16_4)
            __out = 0.0;
            ///////////////////

            sum_val = __out;
        }

    }

    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {


            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            c + i, &c_index_0, 1);

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            d + i, &d_index, 1);
            {
                double __in2 = d_index;
                double __in1 = c_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                a_slice = __out;
            }
            {
                double __inp = a_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_18_8)
                __out = __inp;
                ///////////////////

                a[i] = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            a + i, &a_index, 1);
            {
                double __in1 = sum_val;
                double __in2 = a_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                sum_val_plus_a_slice = __out;
            }
            {
                double __inp = sum_val_plus_a_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_19_8)
                __out = __inp;
                ///////////////////

                sum_val = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            e + i, &e_index, 1);
            {
                double __in1 = c_index_0;
                double __in2 = e_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice = __out;
            }
            {
                double __inp = b_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_20_8)
                __out = __inp;
                ///////////////////

                b[i] = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            b + i, &b_index, 1);
            {
                double __in2 = b_index;
                double __in1 = sum_val;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                sum_val_plus_b_slice = __out;
            }
            {
                double __inp = sum_val_plus_b_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_21_8)
                __out = __inp;
                ///////////////////

                sum_val = __out;
            }

        }

    }

    {

        {
            double __inp = sum_val;
            double __out;

            ///////////////////
            // Tasklet code (assign_22_4)
            __out = __inp;
            ///////////////////

            b[0] = __out;
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
