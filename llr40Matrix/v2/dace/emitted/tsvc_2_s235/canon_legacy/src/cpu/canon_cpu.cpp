/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

#ifdef _OPENMP
#include <omp.h>
#endif
inline void nested_sdfg_0_2_0(canon_cpu_state_t *__state, const double* __restrict__ a, const double* __restrict__ bb, double* __restrict__ aa, int64_t LEN_2D, int __dace_band, int __dace_num_threads) {
    int64_t _loop_it_1;

    for (_loop_it_1 = 1; (_loop_it_1 < LEN_2D); _loop_it_1 = (_loop_it_1 + 1)) {
        {

            {
                for (int64_t _loop_it_3 = py_floor((LEN_2D * __dace_band), __dace_num_threads); _loop_it_3 < py_floor((LEN_2D * (__dace_band + 1)), __dace_num_threads); _loop_it_3 += 1) {
                    double aa_index;
                    double bb_slice_times_a_slice;
                    double aa_slice_plus_bb_slice_a_slice;
                    {
                        double __in1 = bb[((LEN_2D * _loop_it_1) + _loop_it_3)];
                        double __in2 = a[_loop_it_3];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        bb_slice_times_a_slice = __out;
                    }
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
                        double __in2 = bb_slice_times_a_slice;
                        double __in1 = aa_index;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        aa_slice_plus_bb_slice_a_slice = __out;
                    }
                    {
                        double _in = aa_slice_plus_bb_slice_a_slice;
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_out_aa_slice_plus_bb_slice_a_slice_to_aa)
                        _out = _in;
                        ///////////////////

                        aa[((LEN_2D * _loop_it_1) + _loop_it_3)] = _out;
                    }
                }
            }

        }

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, int64_t LEN_2D)
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
            for (int64_t _loop_it_2 = 0; _loop_it_2 < LEN_2D; _loop_it_2 += 1) {
                double b_slice_times_c_slice;
                double _wcr_priv__Add____out;
                {
                    double __in1 = b[_loop_it_2];
                    double __in2 = c[_loop_it_2];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_c_slice = __out;
                }
                {
                    double __in2 = b_slice_times_c_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out = __out;
                }
                {
                    double __in1 = a[_loop_it_2];
                    double __in2 = _wcr_priv__Add____out;
                    double __out;

                    ///////////////////
                    // Tasklet code (augassign)
                    __out = (__in1 + __in2);
                    ///////////////////

                    a[_loop_it_2] = __out;
                }
            }
        }

    }
    {

        {
            #pragma omp parallel for
            for (int __dace_band = 0; __dace_band < __dace_num_threads; __dace_band += 1) {
                nested_sdfg_0_2_0(__state, &a[0], &bb[0], &aa[0], LEN_2D, __dace_band, __dace_num_threads);
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, double * __restrict__ b, double * __restrict__ bb, double * __restrict__ c, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, b, bb, c, LEN_2D);
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
