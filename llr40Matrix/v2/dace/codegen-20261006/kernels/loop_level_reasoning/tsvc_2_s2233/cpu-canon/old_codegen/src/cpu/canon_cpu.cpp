/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

#ifdef _OPENMP
#include <omp.h>
#endif
inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, const double* __restrict__ cc, double* __restrict__ aa, double* __restrict__ bb, int64_t LEN_2D, int __dace_band, int __dace_num_threads) {
    int64_t _loop_it_1;

    for (_loop_it_1 = 8; (_loop_it_1 < LEN_2D); _loop_it_1 = (_loop_it_1 + 1)) {
        {

            {
                for (int64_t _loop_it_3 = (py_floor((__dace_band * (LEN_2D - 8)), __dace_num_threads) + 8); _loop_it_3 < (py_floor(((LEN_2D - 8) * (__dace_band + 1)), __dace_num_threads) + 8); _loop_it_3 += 1) {
                    double aa_index;
                    double aa_slice_plus_cc_slice;
                    double bb_slice_plus_cc_slice;
                    double bb_index;
                    {
                        double _in = aa[((LEN_2D * (_loop_it_1 - 1)) + _loop_it_3)];
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_in_aa_to_aa_index)
                        _out = _in;
                        ///////////////////

                        aa_index = _out;
                    }
                    {
                        double __in1 = aa_index;
                        double __in2 = cc[((LEN_2D * _loop_it_1) + _loop_it_3)];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        aa_slice_plus_cc_slice = __out;
                    }
                    {
                        double _in = aa_slice_plus_cc_slice;
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_out_aa_slice_plus_cc_slice_to_aa)
                        _out = _in;
                        ///////////////////

                        aa[((LEN_2D * _loop_it_1) + _loop_it_3)] = _out;
                    }
                    {
                        double _in = bb[((LEN_2D * (_loop_it_1 - 1)) + _loop_it_3)];
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_in_bb_to_bb_index)
                        _out = _in;
                        ///////////////////

                        bb_index = _out;
                    }
                    {
                        double __in1 = bb_index;
                        double __in2 = cc[((LEN_2D * _loop_it_1) + _loop_it_3)];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        bb_slice_plus_cc_slice = __out;
                    }
                    {
                        double _in = bb_slice_plus_cc_slice;
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_out_bb_slice_plus_cc_slice_to_bb)
                        _out = _in;
                        ///////////////////

                        bb[((LEN_2D * _loop_it_1) + _loop_it_3)] = _out;
                    }
                }
            }

        }

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    #ifdef _OPENMP
    const int64_t __dace_num_threads = omp_get_max_threads();
    #else
    const int64_t __dace_num_threads = 1;
    #endif

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
            for (int __dace_band = 0; __dace_band < __dace_num_threads; __dace_band += 1) {
                nested_sdfg_0_1_0(__state, &cc[0], &aa[0], &bb[0], LEN_2D, __dace_band, __dace_num_threads);
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, double * __restrict__ cc, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, aa, bb, cc, LEN_2D);
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
