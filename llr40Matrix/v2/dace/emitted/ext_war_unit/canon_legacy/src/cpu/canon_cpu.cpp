/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

#ifdef _OPENMP
#include <omp.h>
#endif
inline void copy_a_to_a_antidep_seam_sdfg_0_3_4(canon_cpu_state_t *__state, const double* __restrict__ a, double* __restrict__ a_antidep_seam, int64_t LEN_1D, int __dace_num_threads) {

    {
        const double* _cpy_in;
        _cpy_in = &a[1];
        double* _cpy_out;
        _cpy_out = &a_antidep_seam[0];

        {
            #pragma omp parallel for
            for (int __i0 = 0; __i0 < int_ceil((LEN_1D - 2), int_ceil((LEN_1D - 2), __dace_num_threads)); __i0 += 1) {
                {
                    double _in = _cpy_in[(__i0 * int_ceil((LEN_1D - 2), __dace_num_threads))];
                    double _out;

                    ///////////////////
                    // Tasklet code (copy_a_to_a_antidep_seam_tasklet)
                    _out = _in;
                    ///////////////////

                    _cpy_out[__i0] = _out;
                }
            }
        }

    }
}

inline void nested_a_split_snapshot_0_1_5(canon_cpu_state_t *__state, const double* __restrict__ b, double* __restrict__ a, int64_t LEN_1D, int __dace_num_threads, int64_t antidep_chunk__loop_it_0) {
    int64_t _loop_it_0;

    for (_loop_it_0 = antidep_chunk__loop_it_0; (_loop_it_0 < Min((LEN_1D - 2), ((antidep_chunk__loop_it_0 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1))); _loop_it_0 = (_loop_it_0 + 1)) {
        {

            {
                double __in2 = b[_loop_it_0];
                double __in1 = a[(_loop_it_0 + 1)];
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

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    #ifdef _OPENMP
    const int __dace_num_threads = omp_get_max_threads();
    #else
    const int __dace_num_threads = 1;
    #endif
    double *a_antidep_seam;
    a_antidep_seam = new (std::align_val_t(64)) double[(__dace_num_threads + 1)];
    int64_t __dace_rng_0;
    int64_t __dace_rng_1;

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((__dace_num_threads < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
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

        copy_a_to_a_antidep_seam_sdfg_0_3_4(__state, &a[0], &a_antidep_seam[0], LEN_1D, __dace_num_threads);
        {
            double _cpy_in = a[(LEN_1D - 1)];
            double _cpy_out;

            ///////////////////
            // Tasklet code (copy_a_to_a_antidep_seam)
            _cpy_out = _cpy_in;
            ///////////////////

            a_antidep_seam[int_ceil((LEN_1D - 2), int_ceil((LEN_1D - 2), __dace_num_threads))] = _cpy_out;
        }

    }
    {

        {
            for (int64_t _loop_it_0 = 0; _loop_it_0 < (Min(0, (LEN_1D - 2)) + 1); _loop_it_0 += 1) {
                {
                    double __in2 = b[_loop_it_0];
                    double __in1 = a_antidep_seam[0];
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
    __dace_rng_0 = int_ceil((LEN_1D - 2), __dace_num_threads);
    {

        {
            assert((__dace_rng_0) > 0 && "Map single_state_body_map requires a positive step");
            #pragma omp parallel for
            for (int64_t antidep_chunk__loop_it_0 = 1; antidep_chunk__loop_it_0 < (LEN_1D - 1); antidep_chunk__loop_it_0 += __dace_rng_0) {
                nested_a_split_snapshot_0_1_5(__state, &b[0], &a[0], LEN_1D, __dace_num_threads, antidep_chunk__loop_it_0);
            }
        }

    }
    __dace_rng_1 = int_ceil((LEN_1D - 2), __dace_num_threads);
    {

        {
            assert((__dace_rng_1) > 0 && "Map single_state_body_map requires a positive step");
            #pragma omp parallel for
            for (int64_t antidep_chunk__loop_it_0 = 1; antidep_chunk__loop_it_0 < (LEN_1D - 1); antidep_chunk__loop_it_0 += __dace_rng_1) {
                {
                    for (int64_t _loop_it_0 = Min((LEN_1D - 2), ((antidep_chunk__loop_it_0 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1)); _loop_it_0 < (Min((LEN_1D - 2), ((antidep_chunk__loop_it_0 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1)) + 1); _loop_it_0 += 1) {
                        {
                            double __in2 = b[_loop_it_0];
                            double __in1 = a_antidep_seam[(py_floor((antidep_chunk__loop_it_0 - 1), int_ceil((LEN_1D - 2), __dace_num_threads)) + 1)];
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

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](a_antidep_seam, std::align_val_t(64));
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
