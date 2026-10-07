/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0__scan_in_b;
};

DACE_EXPORTED gpuError_t __dace_scan_canon_gpu_1_14_c0(const double* __sc_in, double* __sc_out, const double* __sc_init, long long __sc_n, gpuStream_t __sc_stream);
DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_0_1_33(canon_gpu_state_t *__state, double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, gpuStream_t gpu_stream);
DACE_EXPORTED void __dace_runkernel_canon_gpu_size1_wrap_region_0_1_37(canon_gpu_state_t *__state, double * __restrict__ _scan_seed_b, const double * __restrict__ b, gpuStream_t gpu_stream);
static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_in_b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_seed_b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_0_map_0_1_35(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D, gpuStream_t gpu_stream);
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double * _scan_seed_b;
        DACE_GPU_CHECK(hipMalloc((void**)&_scan_seed_b, 1 * sizeof(double)));

        {
            gpuStream_t __dace_current_stream = gpu_streams[0];
            __dace_runkernel_canon_gpu_single_state_body_map_0_1_33(__state, __state->__0__scan_in_b, c, d, e, LEN_1D, __dace_current_stream);
        }
        {
            gpuStream_t __dace_current_stream = gpu_streams[0];
            __dace_runkernel_canon_gpu_size1_wrap_region_0_1_37(__state, _scan_seed_b, b, __dace_current_stream);
        }
        {  // for_16_0_scan_op
            DACE_GPU_CHECK(__dace_scan_canon_gpu_1_14_c0((__state->__0__scan_in_b), (b + 1), _scan_seed_b, ((LEN_1D - 1)), gpu_streams[gpu_streams_idx(0)]));
        }
        {
            gpuStream_t __dace_current_stream = gpu_streams[0];
            __dace_runkernel_canon_gpu_single_state_body_0_map_0_1_35(__state, a, b, c, d, LEN_1D, __dace_current_stream);
        }
        DACE_GPU_CHECK(hipFree(_scan_seed_b));

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, double * __restrict__ e, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, c, d, e, LEN_1D);
}
DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_experimental_cuda(__state, LEN_1D);

    if (__result) {
        delete __state;
        return nullptr;
    }
    {  // Environment: ScanScratch
        ::dace::cub::get_scratch<::dace::cub::ScanTag>(134217728ull, 0);
    }
    DACE_GPU_CHECK(hipMalloc((void**)&__state->__0__scan_in_b, (LEN_1D - 1) * sizeof(double)));

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;
    DACE_GPU_CHECK(hipFree(__state->__0__scan_in_b));

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
