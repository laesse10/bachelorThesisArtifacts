/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, double * __restrict__ x, int64_t LEN_1D)
{
    double a_index_0;
    double b_index_0;
    double d_index;
    double b_slice_times_d_slice;
    double a_slice_plus_b_slice_d_slice;
    double c_index;
    double d_index_0;
    double d_index_1;
    double d_slice_times_d_slice;
    double c_slice_plus_d_slice_d_slice;
    double d_index_2;
    double e_index;
    double d_slice_times_e_slice;
    double c_slice;
    double a_index_1;
    double e_index_0;
    double e_index_1;
    double e_slice_times_e_slice;
    double b_slice;
    double a_index_2;
    double d_index_3;
    double d_index_4;
    double d_slice_times_d_slice_0;
    double c_slice_0;
    double c_index_0;
    double e_index_2;
    double e_index_3;
    double e_slice_times_e_slice_0;
    double c_slice_plus_e_slice_e_slice;
    int64_t i;
    double a_index;
    double b_index;
    double x_index;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {

        a_index = a[i];
        b_index = b[i];

        if ((a_index > b_index)) {
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + i, &a_index_0, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                b + i, &b_index_0, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                d + i, &d_index, 1);
                {
                    double __in1 = b_index_0;
                    double __in2 = d_index;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_d_slice = __out;
                }
                {
                    double __in2 = b_slice_times_d_slice;
                    double __in1 = a_index_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a_slice_plus_b_slice_d_slice = __out;
                }
                {
                    double __inp = a_slice_plus_b_slice_d_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (assign_18_12)
                    __out = __inp;
                    ///////////////////

                    a[i] = __out;
                }

            }

            if ((LEN_1D > 10)) {
                {


                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    c + i, &c_index, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    d + i, &d_index_0, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    d + i, &d_index_1, 1);
                    {
                        double __in1 = d_index_0;
                        double __in2 = d_index_1;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        d_slice_times_d_slice = __out;
                    }
                    {
                        double __in2 = d_slice_times_d_slice;
                        double __in1 = c_index;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        c_slice_plus_d_slice_d_slice = __out;
                    }
                    {
                        double __inp = c_slice_plus_d_slice_d_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (assign_20_16)
                        __out = __inp;
                        ///////////////////

                        c[i] = __out;
                    }

                }
            } else {
                {


                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    d + i, &d_index_2, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    e + i, &e_index, 1);
                    {
                        double __in2 = e_index;
                        double __in1 = d_index_2;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        d_slice_times_e_slice = __out;
                    }
                    {
                        double __in1 = d_slice_times_e_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + 1.0);
                        ///////////////////

                        c_slice = __out;
                    }
                    {
                        double __inp = c_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (assign_22_16)
                        __out = __inp;
                        ///////////////////

                        c[i] = __out;
                    }

                }
            }

        } else {

            x_index = x[0];
            {


                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                a + i, &a_index_1, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                e + i, &e_index_0, 1);

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                e + i, &e_index_1, 1);
                {
                    double __in1 = e_index_0;
                    double __in2 = e_index_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    e_slice_times_e_slice = __out;
                }
                {
                    double __in2 = e_slice_times_e_slice;
                    double __in1 = a_index_1;
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
                    // Tasklet code (assign_24_12)
                    __out = __inp;
                    ///////////////////

                    b[i] = __out;
                }

            }

            if ((x_index > 0.0)) {
                {


                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    a + i, &a_index_2, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    d + i, &d_index_3, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    d + i, &d_index_4, 1);
                    {
                        double __in1 = d_index_3;
                        double __in2 = d_index_4;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        d_slice_times_d_slice_0 = __out;
                    }
                    {
                        double __in2 = d_slice_times_d_slice_0;
                        double __in1 = a_index_2;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        c_slice_0 = __out;
                    }
                    {
                        double __inp = c_slice_0;
                        double __out;

                        ///////////////////
                        // Tasklet code (assign_26_16)
                        __out = __inp;
                        ///////////////////

                        c[i] = __out;
                    }

                }
            } else {
                {


                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    c + i, &c_index_0, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    e + i, &e_index_2, 1);

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    e + i, &e_index_3, 1);
                    {
                        double __in1 = e_index_2;
                        double __in2 = e_index_3;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        e_slice_times_e_slice_0 = __out;
                    }
                    {
                        double __in2 = e_slice_times_e_slice_0;
                        double __in1 = c_index_0;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        c_slice_plus_e_slice_e_slice = __out;
                    }
                    {
                        double __inp = c_slice_plus_e_slice_e_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (assign_28_16)
                        __out = __inp;
                        ///////////////////

                        c[i] = __out;
                    }

                }
            }

        }


    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, double * __restrict__ x, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, e, x, LEN_1D);
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
