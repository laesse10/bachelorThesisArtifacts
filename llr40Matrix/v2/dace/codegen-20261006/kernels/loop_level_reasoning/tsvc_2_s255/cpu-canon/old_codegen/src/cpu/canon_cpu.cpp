/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double y_2;

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
        double x;
        double y_1;
        double b_slice_plus_x_0;
        double b_slice_x_plus_y_0;

        {
            double _in = b[(LEN_1D - 1)];
            double _out;

            ///////////////////
            // Tasklet code (_assign_b_to_x)
            _out = _in;
            ///////////////////

            x = _out;
        }
        {
            double __in1 = b[0];
            double __in2 = x;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            b_slice_plus_x_0 = __out;
        }
        {
            double _in = b[(LEN_1D - 2)];
            double _out;

            ///////////////////
            // Tasklet code (_assign_b_to_y_1)
            _out = _in;
            ///////////////////

            y_1 = _out;
        }
        {
            double __in1 = b_slice_plus_x_0;
            double __in2 = y_1;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            b_slice_x_plus_y_0 = __out;
        }
        {
            double __in1 = b_slice_x_plus_y_0;
            double __out;

            ///////////////////
            // Tasklet code (_Mult_)
            __out = (__in1 * 0.333);
            ///////////////////

            a[0] = __out;
        }
        {
            double __in1 = x;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + 0.0);
            ///////////////////

            y_2 = __out;
        }

    }
    {
        double b_index_1;
        double b_slice_plus_x_1;
        double b_slice_x_plus_y_1;

        {
            double _in = b[1];
            double _out;

            ///////////////////
            // Tasklet code (_assign_b_to_b_index_1)
            _out = _in;
            ///////////////////

            b_index_1 = _out;
        }
        {
            double __in1 = b_index_1;
            double __in2 = b[0];
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            b_slice_plus_x_1 = __out;
        }
        {
            double __in2 = y_2;
            double __in1 = b_slice_plus_x_1;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            b_slice_x_plus_y_1 = __out;
        }
        {
            double __in1 = b_slice_x_plus_y_1;
            double __out;

            ///////////////////
            // Tasklet code (_Mult_)
            __out = (__in1 * 0.333);
            ///////////////////

            a[1] = __out;
        }

    }
    {

        {
            #pragma omp parallel for
            for (int64_t _loop_it_0 = 2; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
                double b_index_2;
                double b_slice_plus_x_2;
                double b_slice_x_plus_y_2;
                double y_remat;
                {
                    double _in = b[_loop_it_0];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_b_to_b_index_2)
                    _out = _in;
                    ///////////////////

                    b_index_2 = _out;
                }
                {
                    double __in1 = b_index_2;
                    double __in2 = b[(_loop_it_0 - 1)];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b_slice_plus_x_2 = __out;
                }
                {
                    double __in1 = b[(_loop_it_0 - 2)];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add__remat)
                    __out = (__in1 + 0.0);
                    ///////////////////

                    y_remat = __out;
                }
                {
                    double __in1 = b_slice_plus_x_2;
                    double __in2 = y_remat;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b_slice_x_plus_y_2 = __out;
                }
                {
                    double __in1 = b_slice_x_plus_y_2;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * 0.333);
                    ///////////////////

                    a[_loop_it_0] = __out;
                }
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, LEN_1D);
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
