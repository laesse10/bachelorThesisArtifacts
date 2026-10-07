/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
constexpr double _priv_sum_val = 0.0;

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_0_2_17(canon_gpu_state_t *__state, double * __restrict__ _priv_sum_val_gpu, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream);
DACE_EXPORTED void __dace_runkernel_canon_gpu_size1_wrap_region_0_2_19(canon_gpu_state_t *__state, const double * __restrict__ _priv_sum_val_gpu, double * __restrict__ b, gpuStream_t gpu_stream);
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _priv_sum_val_gpu_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double * _priv_sum_val_gpu;
    DACE_GPU_CHECK(hipMalloc((void**)&_priv_sum_val_gpu, 1 * sizeof(double)));
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {

        {  // copy__priv_sum_val_to__priv_sum_val_gpu
            hipMemcpyAsync(_priv_sum_val_gpu, (&_priv_sum_val), 1 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }

    {
        gpuStream_t __dace_current_stream = gpu_streams[0];
        __dace_runkernel_canon_gpu_single_state_body_map_0_2_17(__state, _priv_sum_val_gpu, a, LEN_1D, __dace_current_stream);
    }
    {
        gpuStream_t __dace_current_stream = gpu_streams[0];
        __dace_runkernel_canon_gpu_size1_wrap_region_0_2_19(__state, _priv_sum_val_gpu, b, __dace_current_stream);
    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    DACE_GPU_CHECK(hipFree(_priv_sum_val_gpu));
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, LEN_1D);
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

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;

    int __err_experimental_cuda = __dace_exit_experimental_cuda(__state);
    if (__err_experimental_cuda) {
        __err = __err_experimental_cuda;
    }
    delete __state;
    return __err;
}
