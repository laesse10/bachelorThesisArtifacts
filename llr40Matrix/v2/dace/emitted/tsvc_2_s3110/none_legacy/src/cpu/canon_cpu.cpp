/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double maxv;
    double maxv_0;
    int64_t xindex;
    int64_t yindex;
    int64_t i;
    int64_t j;
    double aa_index;
    bool __tmp0;

    {

        {
            double _cpy_in = aa[0];
            double _cpy_out;

            ///////////////////
            // Tasklet code (copy_aa_to_maxv)
            _cpy_out = _cpy_in;
            ///////////////////

            maxv = _cpy_out;
        }

    }
    xindex = 0;
    yindex = 0;

    for (i = 0; (i < LEN_2D); i = (i + 1)) {

        for (j = 0; (j < LEN_2D); j = (j + 1)) {

            aa_index = aa[((LEN_2D * i) + j)];

            __tmp0 = (aa_index > maxv);

            if (__tmp0) {
                {

                    {
                        double _cpy_in = aa[((LEN_2D * i) + j)];
                        double _cpy_out;

                        ///////////////////
                        // Tasklet code (copy_aa_to_maxv_0)
                        _cpy_out = _cpy_in;
                        ///////////////////

                        maxv_0 = _cpy_out;
                    }
                    {
                        double __inp = maxv_0;
                        double __out;

                        ///////////////////
                        // Tasklet code (assign_22_16)
                        __out = __inp;
                        ///////////////////

                        maxv = __out;
                    }

                }
                xindex = i;
                yindex = j;

            }


        }


    }

    {
        double float_xindex;
        double maxv_plus_expr;
        double float_yindex;
        double chksum;

        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(xindex);
            ///////////////////

            float_xindex = __out;
        }
        {
            double __in1 = maxv;
            double __in2 = float_xindex;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
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

            chksum = __out;
        }
        {
            double __inp = chksum;
            double __out;

            ///////////////////
            // Tasklet code (assign_28_4)
            __out = __inp;
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
