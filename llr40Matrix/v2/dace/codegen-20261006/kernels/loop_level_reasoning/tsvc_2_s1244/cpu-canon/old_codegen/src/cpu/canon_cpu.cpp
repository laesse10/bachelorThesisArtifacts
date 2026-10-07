/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

#ifdef _OPENMP
#include <omp.h>
#endif
inline void nested_a_split_snapshot_0_1_7(canon_cpu_state_t *__state, const double* __restrict__ b, const double* __restrict__ c, double* __restrict__ a, double* __restrict__ d, int64_t LEN_1D, int __dace_num_threads, int64_t antidep_chunk__loop_it_1) {
    int64_t _loop_it_1;

    for (_loop_it_1 = antidep_chunk__loop_it_1; (_loop_it_1 < Min((LEN_1D - 2), ((antidep_chunk__loop_it_1 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1))); _loop_it_1 = (_loop_it_1 + 1)) {
        {
            double c_slice_times_c_slice;
            double b_slice_plus_c_slice_c_slice;
            double b_slice_times_b_slice;
            double b_slice_c_slice_c_slice_plus_b_slice_b_slice;
            double __map_fusion_a;

            {
                double __in1 = b[_loop_it_1];
                double __in2 = b[_loop_it_1];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                b_slice_times_b_slice = __out;
            }
            {
                double __in1 = c[_loop_it_1];
                double __in2 = c[_loop_it_1];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                c_slice_times_c_slice = __out;
            }
            {
                double __in2 = c_slice_times_c_slice;
                double __in1 = b[_loop_it_1];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice_plus_c_slice_c_slice = __out;
            }
            {
                double __in2 = b_slice_times_b_slice;
                double __in1 = b_slice_plus_c_slice_c_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b_slice_c_slice_c_slice_plus_b_slice_b_slice = __out;
            }
            {
                double __in1 = b_slice_c_slice_c_slice_plus_b_slice_b_slice;
                double __in2 = c[_loop_it_1];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                __map_fusion_a = __out;
            }

            dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
            &__map_fusion_a, a + _loop_it_1, 1);
            {
                double __in1 = __map_fusion_a;
                double __in2 = a[(_loop_it_1 + 1)];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                d[_loop_it_1] = __out;
            }

        }

    }

}

void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
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


        dace::CopyNDDynamic<double, 1, false, 1>::template ConstDst<1>::Copy(
        a + 1, a_antidep_seam, int_ceil((LEN_1D - 2), int_ceil((LEN_1D - 2), __dace_num_threads)), int_ceil((LEN_1D - 2), __dace_num_threads));

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        a + (LEN_1D - 1), a_antidep_seam + int_ceil((LEN_1D - 2), int_ceil((LEN_1D - 2), __dace_num_threads)), 1);

    }
    {

        {
            for (int64_t _loop_it_1 = 0; _loop_it_1 < (Min(0, (LEN_1D - 2)) + 1); _loop_it_1 += 1) {
                double c_slice_times_c_slice_0;
                double b_slice_plus_c_slice_c_slice_0;
                double b_slice_times_b_slice_0;
                double b_slice_c_slice_c_slice_plus_b_slice_b_slice_0;
                double __map_fusion_a_0;
                {
                    double __in1 = b[_loop_it_1];
                    double __in2 = b[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_b_slice_0 = __out;
                }
                {
                    double __in1 = c[_loop_it_1];
                    double __in2 = c[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    c_slice_times_c_slice_0 = __out;
                }
                {
                    double __in2 = c_slice_times_c_slice_0;
                    double __in1 = b[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b_slice_plus_c_slice_c_slice_0 = __out;
                }
                {
                    double __in2 = b_slice_times_b_slice_0;
                    double __in1 = b_slice_plus_c_slice_c_slice_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    b_slice_c_slice_c_slice_plus_b_slice_b_slice_0 = __out;
                }
                {
                    double __in1 = b_slice_c_slice_c_slice_plus_b_slice_b_slice_0;
                    double __in2 = c[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    __map_fusion_a_0 = __out;
                }

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                &__map_fusion_a_0, a + _loop_it_1, 1);
                {
                    double __in1 = __map_fusion_a_0;
                    double __in2 = a_antidep_seam[0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    d[_loop_it_1] = __out;
                }
            }
        }

    }
    __dace_rng_0 = int_ceil((LEN_1D - 2), __dace_num_threads);
    {

        {
            assert((__dace_rng_0) > 0 && "Map single_state_body_0_map requires a positive step");
            #pragma omp parallel for
            for (int64_t antidep_chunk__loop_it_1 = 1; antidep_chunk__loop_it_1 < (LEN_1D - 1); antidep_chunk__loop_it_1 += __dace_rng_0) {
                nested_a_split_snapshot_0_1_7(__state, &b[0], &c[0], &a[0], &d[0], LEN_1D, __dace_num_threads, antidep_chunk__loop_it_1);
            }
        }

    }
    __dace_rng_1 = int_ceil((LEN_1D - 2), __dace_num_threads);
    {

        {
            assert((__dace_rng_1) > 0 && "Map single_state_body_0_map requires a positive step");
            #pragma omp parallel for
            for (int64_t antidep_chunk__loop_it_1 = 1; antidep_chunk__loop_it_1 < (LEN_1D - 1); antidep_chunk__loop_it_1 += __dace_rng_1) {
                {
                    for (int64_t _loop_it_1 = Min((LEN_1D - 2), ((antidep_chunk__loop_it_1 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1)); _loop_it_1 < (Min((LEN_1D - 2), ((antidep_chunk__loop_it_1 + int_ceil((LEN_1D - 2), __dace_num_threads)) - 1)) + 1); _loop_it_1 += 1) {
                        double c_slice_times_c_slice_1;
                        double b_slice_plus_c_slice_c_slice_1;
                        double b_slice_times_b_slice_1;
                        double b_slice_c_slice_c_slice_plus_b_slice_b_slice_1;
                        double __map_fusion_a_1;
                        {
                            double __in1 = b[_loop_it_1];
                            double __in2 = b[_loop_it_1];
                            double __out;

                            ///////////////////
                            // Tasklet code (_Mult_)
                            __out = (__in1 * __in2);
                            ///////////////////

                            b_slice_times_b_slice_1 = __out;
                        }
                        {
                            double __in1 = c[_loop_it_1];
                            double __in2 = c[_loop_it_1];
                            double __out;

                            ///////////////////
                            // Tasklet code (_Mult_)
                            __out = (__in1 * __in2);
                            ///////////////////

                            c_slice_times_c_slice_1 = __out;
                        }
                        {
                            double __in2 = c_slice_times_c_slice_1;
                            double __in1 = b[_loop_it_1];
                            double __out;

                            ///////////////////
                            // Tasklet code (_Add_)
                            __out = (__in1 + __in2);
                            ///////////////////

                            b_slice_plus_c_slice_c_slice_1 = __out;
                        }
                        {
                            double __in2 = b_slice_times_b_slice_1;
                            double __in1 = b_slice_plus_c_slice_c_slice_1;
                            double __out;

                            ///////////////////
                            // Tasklet code (_Add_)
                            __out = (__in1 + __in2);
                            ///////////////////

                            b_slice_c_slice_c_slice_plus_b_slice_b_slice_1 = __out;
                        }
                        {
                            double __in1 = b_slice_c_slice_c_slice_plus_b_slice_b_slice_1;
                            double __in2 = c[_loop_it_1];
                            double __out;

                            ///////////////////
                            // Tasklet code (_Add_)
                            __out = (__in1 + __in2);
                            ///////////////////

                            __map_fusion_a_1 = __out;
                        }

                        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                        &__map_fusion_a_1, a + _loop_it_1, 1);
                        {
                            double __in1 = __map_fusion_a_1;
                            double __in2 = a_antidep_seam[(py_floor((antidep_chunk__loop_it_1 - 1), int_ceil((LEN_1D - 2), __dace_num_threads)) + 1)];
                            double __out;

                            ///////////////////
                            // Tasklet code (_Add_)
                            __out = (__in1 + __in2);
                            ///////////////////

                            d[_loop_it_1] = __out;
                        }
                    }
                }
            }
        }

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](a_antidep_seam, std::align_val_t(64));
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, b, c, d, LEN_1D);
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
