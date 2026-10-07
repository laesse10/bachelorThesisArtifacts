/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED void __dace_runkernel_canon_gpu_reduce_init_map_0_4_6(canon_gpu_state_t *__state, double * __restrict__ _arg_max_buf_for_17, gpuStream_t gpu_stream);
DACE_EXPORTED void __dace_runkernel_canon_gpu_grid_0_3_0(canon_gpu_state_t *__state, double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream);
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    double scal_result;
    double result_host;
    double _arg_max_buf_for_17_host;
    double * _arg_max_buf_for_17;
    DACE_GPU_CHECK(hipMalloc((void**)&_arg_max_buf_for_17, 1 * sizeof(double)));
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double x;



    {
        gpuStream_t __dace_current_stream = gpu_streams[0];
        __dace_runkernel_canon_gpu_reduce_init_map_0_4_6(__state, _arg_max_buf_for_17, __dace_current_stream);
    }

    {
        gpuStream_t __dace_current_stream = gpu_streams[0];
        __dace_runkernel_canon_gpu_grid_0_3_0(__state, _arg_max_buf_for_17, a, LEN_1D, __dace_current_stream);
    }
    {

        {  // copy__arg_max_buf_for_17_to__arg_max_buf_for_17_host
            hipMemcpyAsync((&_arg_max_buf_for_17_host), _arg_max_buf_for_17, 1 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_result_to_scal_result
            hipMemcpyAsync((&scal_result), result, 1 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    {

        result_host = scal_result;  // copy_scal_result_to_result_host

    }
    x = _arg_max_buf_for_17_host;
    {

        result_host = x;  // symassign
        scal_result = result_host;  // copy_result_host_to_scal_result

    }
    {

        {  // copy_scal_result_to_result
            hipMemcpyAsync(result, (&scal_result), 1 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    DACE_GPU_CHECK(hipFree(_arg_max_buf_for_17));
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, result, LEN_1D);
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
