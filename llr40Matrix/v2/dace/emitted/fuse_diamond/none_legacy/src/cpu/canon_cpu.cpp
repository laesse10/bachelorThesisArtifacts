/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    double *t;
    t = new (std::align_val_t(64)) double[LEN_1D];
    double *u;
    u = new (std::align_val_t(64)) double[LEN_1D];
    double *v;
    v = new (std::align_val_t(64)) double[LEN_1D];
    double a_index;
    double a_index_0;
    double t_slice;
    double t_index;
    double u_slice;
    double t_index_0;
    double v_slice;
    double u_index;
    double v_index;
    double out_slice;
    int64_t i;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            {
                double _cpy_in = a[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_a_to_a_index)
                _cpy_out = _cpy_in;
                ///////////////////

                a_index = _cpy_out;
            }
            {
                double _cpy_in = a[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_a_to_a_index_0)
                _cpy_out = _cpy_in;
                ///////////////////

                a_index_0 = _cpy_out;
            }
            {
                double __in2 = a_index_0;
                double __in1 = a_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                t_slice = __out;
            }
            {
                double __inp = t_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_20_8)
                __out = __inp;
                ///////////////////

                t[i] = __out;
            }

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            {
                double _cpy_in = t[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_t_to_t_index)
                _cpy_out = _cpy_in;
                ///////////////////

                t_index = _cpy_out;
            }
            {
                double __in1 = t_index;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + 1.0);
                ///////////////////

                u_slice = __out;
            }
            {
                double __inp = u_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_22_8)
                __out = __inp;
                ///////////////////

                u[i] = __out;
            }

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            {
                double _cpy_in = t[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_t_to_t_index_0)
                _cpy_out = _cpy_in;
                ///////////////////

                t_index_0 = _cpy_out;
            }
            {
                double __in1 = t_index_0;
                double __out;

                ///////////////////
                // Tasklet code (_Sub_)
                __out = (__in1 - 1.0);
                ///////////////////

                v_slice = __out;
            }
            {
                double __inp = v_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_24_8)
                __out = __inp;
                ///////////////////

                v[i] = __out;
            }

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            {
                double _cpy_in = u[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_u_to_u_index)
                _cpy_out = _cpy_in;
                ///////////////////

                u_index = _cpy_out;
            }
            {
                double _cpy_in = v[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_v_to_v_index)
                _cpy_out = _cpy_in;
                ///////////////////

                v_index = _cpy_out;
            }
            {
                double __in2 = v_index;
                double __in1 = u_index;
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                out_slice = __out;
            }
            {
                double __inp = out_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_26_8)
                __out = __inp;
                ///////////////////

                out[i] = __out;
            }

        }

    }


    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](t, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](u, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](v, std::align_val_t(64));
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out, LEN_1D);
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
