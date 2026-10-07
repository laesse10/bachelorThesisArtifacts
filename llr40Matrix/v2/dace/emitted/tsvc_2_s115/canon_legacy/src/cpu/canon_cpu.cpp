/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

inline void nested_sdfg_0_1_0(canon_cpu_state_t *__state, const double* __restrict__ aa, double* __restrict__ a, int64_t LEN_2D) {
    int64_t _loop_it_0;

    for (_loop_it_0 = 0; (_loop_it_0 < LEN_2D); _loop_it_0 = (_loop_it_0 + 1)) {
        {

            {
                #pragma omp for
                for (int64_t _loop_it_1 = (_loop_it_0 + 1); _loop_it_1 < LEN_2D; _loop_it_1 += 1) {
                    double a_index_0;
                    double aa_index;
                    double a_index;
                    double a_slice_minus_aa_slice_a_slice;
                    double aa_slice_times_a_slice;
                    {
                        double _in = a[_loop_it_1];
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_in_a_to_a_index)
                        _out = _in;
                        ///////////////////

                        a_index = _out;
                    }
                    {
                        double _in = a[_loop_it_0];
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_in_a_to_a_index_0)
                        _out = _in;
                        ///////////////////

                        a_index_0 = _out;
                    }
                    {
                        double _in = aa[((LEN_2D * _loop_it_0) + _loop_it_1)];
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_in_aa_to_aa_index)
                        _out = _in;
                        ///////////////////

                        aa_index = _out;
                    }
                    {
                        double __in1 = aa_index;
                        double __in2 = a_index_0;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        aa_slice_times_a_slice = __out;
                    }
                    {
                        double __in1 = a_index;
                        double __in2 = aa_slice_times_a_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Sub_)
                        __out = (__in1 - __in2);
                        ///////////////////

                        a_slice_minus_aa_slice_a_slice = __out;
                    }
                    {
                        double _in = a_slice_minus_aa_slice_a_slice;
                        double _out;

                        ///////////////////
                        // Tasklet code (_assign_out_a_slice_minus_aa_slice_a_slice_to_a)
                        _out = _in;
                        ///////////////////

                        a[_loop_it_1] = _out;
                    }
                }
            }

        }

    }
}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ aa, int64_t LEN_2D)
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
            #pragma omp parallel
            {
                nested_sdfg_0_1_0(__state, &aa[0], &a[0], LEN_2D);
            }
        }

    }
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ aa, int64_t LEN_2D)
{
    __program_canon_cpu_internal(__state, a, aa, LEN_2D);
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
