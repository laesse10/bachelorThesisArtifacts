/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0__scan_seed_a;
    double * __restrict__ __0_a_host;
    double * __restrict__ __0_c_host;
    double * __restrict__ __0_b_host;
};

DACE_EXPORTED gpuError_t __dace_scan_affine_canon_gpu_1r0r0_8(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
DACE_EXPORTED void __dace_runkernel_single_state_body_map_2_0_12(canon_gpu_state_t *__state, double * __restrict__ _scan_coef_a, double * __restrict__ _scan_in_a, const double * __restrict__ b, const double * __restrict__ c, int K, int64_t LEN_1D);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    double a_index_0;
    double a_index_1;
    double __tmp0_0;
    double __tmp0_1;
    double b_slice_times_c_slice_0;
    double b_slice_times_c_slice_1;
    double __tmp1_0;
    double __tmp1_1;
    int64_t _loop_it_2;
    int64_t _loop_it_3;

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((K < 0)) {
                std::abort();
            }
            ///////////////////

        }
        {

            ///////////////////
            // Tasklet code (check_assumption_1)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }

    if ((K >= 1)) {
        {
            double * _scan_in_a;
            DACE_GPU_CHECK(hipMalloc((void**)&_scan_in_a, ((- K) + LEN_1D) * sizeof(double)));
            double * _scan_coef_a;
            DACE_GPU_CHECK(hipMalloc((void**)&_scan_coef_a, ((- K) + LEN_1D) * sizeof(double)));

            __dace_runkernel_single_state_body_map_2_0_12(__state, _scan_coef_a, _scan_in_a, b, c, K, LEN_1D);
            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0__scan_seed_a, a, K * sizeof(double), hipMemcpyDeviceToDevice, nullptr));
            {
                double * __restrict__ _scan_init = &__state->__0__scan_seed_a[0];
                double * __restrict__ _scan_in = &_scan_in_a[0];
                double * __restrict__ _scan_coef = &_scan_coef_a[0];
                double* __restrict__ _scan_out = a + K;

                ///////////////////
                hipStream_t __dace_current_stream = nullptr;
                DACE_GPU_CHECK(__dace_scan_affine_canon_gpu_1r0r0_8(_scan_coef, _scan_in, _scan_init, static_cast<double>(0), _scan_out, (((- K) + LEN_1D)), __dace_current_stream));
                ///////////////////

            }
            DACE_GPU_CHECK(hipFree(_scan_in_a));
            DACE_GPU_CHECK(hipFree(_scan_coef_a));

        }
    } else if (((! (K >= 1)) && (K == 0))) {
        {

            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_a_host, a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
            DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_c_host, c, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
            DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_b_host, b, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
            DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

        }

        for (_loop_it_2 = K; (_loop_it_2 < LEN_1D); _loop_it_2 = (_loop_it_2 + 1)) {
            {

                {
                    double _in = __state->__0_a_host[((- K) + _loop_it_2)];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_a_to_a_index_0)
                    _out = _in;
                    ///////////////////

                    a_index_0 = _out;
                }
                {
                    double __in2 = a_index_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (0.75 * __in2);
                    ///////////////////

                    __tmp0_0 = __out;
                }
                {
                    double __in1 = __state->__0_b_host[_loop_it_2];
                    double __in2 = __state->__0_c_host[_loop_it_2];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_c_slice_0 = __out;
                }
                {
                    double __in2 = b_slice_times_c_slice_0;
                    double __in1 = __tmp0_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    __tmp1_0 = __out;
                }
                {
                    double _in = __tmp1_0;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign___tmp1_0_to_a)
                    _out = _in;
                    ///////////////////

                    __state->__0_a_host[_loop_it_2] = _out;
                }

            }

        }

        {

            DACE_GPU_CHECK(hipMemcpyAsync(a, __state->__0_a_host, LEN_1D * sizeof(double), hipMemcpyHostToDevice, nullptr));

        }
    } else {
        {

            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_a_host, a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
            DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_c_host, c, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
            DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
            DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_b_host, b, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
            DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

        }

        for (_loop_it_3 = K; (_loop_it_3 < LEN_1D); _loop_it_3 = (_loop_it_3 + 1)) {
            {

                {
                    double _in = __state->__0_a_host[((- K) + _loop_it_3)];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_a_to_a_index_1)
                    _out = _in;
                    ///////////////////

                    a_index_1 = _out;
                }
                {
                    double __in2 = a_index_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (0.75 * __in2);
                    ///////////////////

                    __tmp0_1 = __out;
                }
                {
                    double __in1 = __state->__0_b_host[_loop_it_3];
                    double __in2 = __state->__0_c_host[_loop_it_3];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    b_slice_times_c_slice_1 = __out;
                }
                {
                    double __in2 = b_slice_times_c_slice_1;
                    double __in1 = __tmp0_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    __tmp1_1 = __out;
                }
                {
                    double _in = __tmp1_1;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign___tmp1_1_to_a)
                    _out = _in;
                    ///////////////////

                    __state->__0_a_host[_loop_it_3] = _out;
                }

            }

        }

        {

            DACE_GPU_CHECK(hipMemcpyAsync(a, __state->__0_a_host, LEN_1D * sizeof(double), hipMemcpyHostToDevice, nullptr));

        }
    }


}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, c, K, LEN_1D);
}
DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t K, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t K, int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_cuda(__state, K, LEN_1D);

    if (__result) {
        delete __state;
        return nullptr;
    }
    {  // Environment: ScanScratch
        ::dace::cub::get_scratch<::dace::cub::ScanTag>(134217728ull, 0);
    }
    DACE_GPU_CHECK(hipMalloc((void**)&__state->__0__scan_seed_a, K * sizeof(double)));
    __state->__0_a_host = new (std::align_val_t(64)) double[LEN_1D];
    __state->__0_c_host = new (std::align_val_t(64)) double[LEN_1D];
    __state->__0_b_host = new (std::align_val_t(64)) double[LEN_1D];

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;
    DACE_GPU_CHECK(hipFree(__state->__0__scan_seed_a));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_a_host, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_c_host, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_b_host, std::align_val_t(64));

    int __err_cuda = __dace_exit_cuda(__state);
    if (__err_cuda) {
        __err = __err_cuda;
    }
    {  // Environment: ScanScratch
        ::dace::cub::release_scratch<::dace::cub::ScanTag>();
    }
    delete __state;
    return __err;
}
