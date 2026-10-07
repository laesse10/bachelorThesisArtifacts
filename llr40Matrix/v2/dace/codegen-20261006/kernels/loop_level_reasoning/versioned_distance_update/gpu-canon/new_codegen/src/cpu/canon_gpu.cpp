/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0__scan_seed_a;
    double * __restrict__ __0_c_host;
    double * __restrict__ __0_b_host;
    double * __restrict__ __0_a_host;
};

DACE_EXPORTED gpuError_t __dace_scan_affine_canon_gpu_1r0r0_8(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_2_0_17(canon_gpu_state_t *__state, double * __restrict__ _scan_coef_a, double * __restrict__ _scan_in_a, const double * __restrict__ b, const double * __restrict__ c, int K, int64_t LEN_1D, gpuStream_t gpu_stream);
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_in_a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_coef_a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_host_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_host_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t c_host_idx(int64_t __d0) { return __d0; }
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
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((K < 0)) {
                std::abort();
            }
        }
        {  // check_assumption_1
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    if ((K >= 1)) {
        {
            double * _scan_in_a;
            DACE_GPU_CHECK(hipMalloc((void**)&_scan_in_a, ((- K) + LEN_1D) * sizeof(double)));
            double * _scan_coef_a;
            DACE_GPU_CHECK(hipMalloc((void**)&_scan_coef_a, ((- K) + LEN_1D) * sizeof(double)));

            {
                gpuStream_t __dace_current_stream = gpu_streams[0];
                __dace_runkernel_canon_gpu_single_state_body_map_2_0_17(__state, _scan_coef_a, _scan_in_a, b, c, K, LEN_1D, __dace_current_stream);
            }
            {  // copy_a_to__scan_seed_a
                hipMemcpyAsync((__state->__0__scan_seed_a), a, K * sizeof(double), hipMemcpyDeviceToDevice, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // for_17_affine_scan_op
                DACE_GPU_CHECK(__dace_scan_affine_canon_gpu_1r0r0_8(_scan_coef_a, _scan_in_a, (__state->__0__scan_seed_a), static_cast<double>(0), (a + K), (((- K) + LEN_1D)), gpu_streams[gpu_streams_idx(0)]));
            }
            DACE_GPU_CHECK(hipFree(_scan_in_a));
            DACE_GPU_CHECK(hipFree(_scan_coef_a));

        }
    } else if (((! (K >= 1)) && (K == 0))) {
        {

            {  // copy_c_to_c_host
                hipMemcpyAsync((__state->__0_c_host), c, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_b_to_b_host
                hipMemcpyAsync((__state->__0_b_host), b, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_a_to_a_host
                hipMemcpyAsync((__state->__0_a_host), a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }

        }

        for (int64_t _loop_it_2 = K; (_loop_it_2 < LEN_1D); _loop_it_2 = (_loop_it_2 + 1)) {
            {

                a_index_0 = __state->__0_a_host[a_host_idx(((- K) + _loop_it_2))];  // _assign_a_to_a_index_0
                __tmp0_0 = (0.75 * a_index_0);  // _Mult_
                b_slice_times_c_slice_0 = (__state->__0_b_host[b_host_idx(_loop_it_2)] * __state->__0_c_host[c_host_idx(_loop_it_2)]);  // _Mult_
                __tmp1_0 = (__tmp0_0 + b_slice_times_c_slice_0);  // _Add_
                __state->__0_a_host[a_host_idx(_loop_it_2)] = __tmp1_0;  // _assign___tmp1_0_to_a

            }

        }

        {

            {  // copy_a_host_to_a
                hipMemcpyAsync(a, (__state->__0_a_host), LEN_1D * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
            }

        }
    } else {
        {

            {  // copy_c_to_c_host
                hipMemcpyAsync((__state->__0_c_host), c, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_b_to_b_host
                hipMemcpyAsync((__state->__0_b_host), b, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }
            {  // copy_a_to_a_host
                hipMemcpyAsync((__state->__0_a_host), a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
            }

        }

        for (int64_t _loop_it_3 = K; (_loop_it_3 < LEN_1D); _loop_it_3 = (_loop_it_3 + 1)) {
            {

                a_index_1 = __state->__0_a_host[a_host_idx(((- K) + _loop_it_3))];  // _assign_a_to_a_index_1
                __tmp0_1 = (0.75 * a_index_1);  // _Mult_
                b_slice_times_c_slice_1 = (__state->__0_b_host[b_host_idx(_loop_it_3)] * __state->__0_c_host[c_host_idx(_loop_it_3)]);  // _Mult_
                __tmp1_1 = (__tmp0_1 + b_slice_times_c_slice_1);  // _Add_
                __state->__0_a_host[a_host_idx(_loop_it_3)] = __tmp1_1;  // _assign___tmp1_1_to_a

            }

        }

        {

            {  // copy_a_host_to_a
                hipMemcpyAsync(a, (__state->__0_a_host), LEN_1D * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
            }

        }
    }


    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, int64_t K, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, c, K, LEN_1D);
}
DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t K, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t K, int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_experimental_cuda(__state, K, LEN_1D);

    if (__result) {
        delete __state;
        return nullptr;
    }
    {  // Environment: ScanScratch
        ::dace::cub::get_scratch<::dace::cub::ScanTag>(134217728ull, 0);
    }
    DACE_GPU_CHECK(hipMalloc((void**)&__state->__0__scan_seed_a, K * sizeof(double)));
    __state->__0_c_host = new (std::align_val_t(64)) double[LEN_1D];
    __state->__0_b_host = new (std::align_val_t(64)) double[LEN_1D];
    __state->__0_a_host = new (std::align_val_t(64)) double[LEN_1D];

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
    ::operator delete[](__state->__0_c_host, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_b_host, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_a_host, std::align_val_t(64));

    int __err_experimental_cuda = __dace_exit_experimental_cuda(__state);
    if (__err_experimental_cuda) {
        __err = __err_experimental_cuda;
    }
    {  // Environment: ScanScratch
        ::dace::cub::release_scratch<::dace::cub::ScanTag>();
    }
    delete __state;
    return __err;
}
