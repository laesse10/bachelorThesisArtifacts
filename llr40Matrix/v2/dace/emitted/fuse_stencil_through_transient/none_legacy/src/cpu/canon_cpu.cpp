/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    double *tmp;
    tmp = new (std::align_val_t(64)) double[LEN_1D];
    double a_index;
    double a_index_0;
    double a_slice_plus_a_slice;
    double a_index_1;
    double tmp_slice;
    double tmp_index;
    double tmp_index_0;
    double out_slice;
    int64_t i;


    for (i = 1; (i < (LEN_1D - 1)); i = (i + 1)) {
        {

            {
                double _cpy_in = a[(i - 1)];
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
                double __in1 = a_index;
                double __in2 = a_index_0;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                a_slice_plus_a_slice = __out;
            }
            {
                double _cpy_in = a[(i + 1)];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_a_to_a_index_1)
                _cpy_out = _cpy_in;
                ///////////////////

                a_index_1 = _cpy_out;
            }
            {
                double __in2 = a_index_1;
                double __in1 = a_slice_plus_a_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                tmp_slice = __out;
            }
            {
                double __inp = tmp_slice;
                double __out;

                ///////////////////
                // Tasklet code (assign_18_8)
                __out = __inp;
                ///////////////////

                tmp[i] = __out;
            }

        }

    }


    for (i = 1; (i < (LEN_1D - 2)); i = (i + 1)) {
        {

            {
                double _cpy_in = tmp[i];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_tmp_to_tmp_index)
                _cpy_out = _cpy_in;
                ///////////////////

                tmp_index = _cpy_out;
            }
            {
                double _cpy_in = tmp[(i + 1)];
                double _cpy_out;

                ///////////////////
                // Tasklet code (copy_tmp_to_tmp_index_0)
                _cpy_out = _cpy_in;
                ///////////////////

                tmp_index_0 = _cpy_out;
            }
            {
                double __in1 = tmp_index;
                double __in2 = tmp_index_0;
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
                // Tasklet code (assign_20_8)
                __out = __inp;
                ///////////////////

                out[i] = __out;
            }

        }

    }


    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](tmp, std::align_val_t(64));
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
