/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_argreduce_canon_gpu_1_6(const double *__ar_in, long long __ar_stride, double *__ar_val, long long *__ar_idx, long long __ar_items, gpuStream_t __ar_stream);
static DACE_HDFI consteval int64_t _argfi_val_for_20_size() { return 1; }
static DACE_HDFI consteval int64_t _argfi_idx_for_20_size() { return 1; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _argfi_val_for_20_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _argfi_idx_for_20_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    double* __restrict__ _argfi_val_for_20 = new (std::align_val_t(64)) double[_argfi_val_for_20_size()];
    int64_t* __restrict__ _argfi_idx_for_20 = new (std::align_val_t(64)) int64_t[_argfi_idx_for_20_size()];
    double scal_result;
    double result_host;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double maxv;
    int64_t index;

    {

        {  // for_20_argfi_argreduce_cuda
            double __ar_val;
            long long __ar_idx;
            DACE_GPU_CHECK(__dace_argreduce_canon_gpu_1_6(a, (long long)(inc), &__ar_val, &__ar_idx, (long long)(LEN_1D), gpu_streams[gpu_streams_idx(0)]));
            _argfi_val_for_20[_argfi_val_for_20_idx(0)] = __ar_val;
            _argfi_idx_for_20[_argfi_idx_for_20_idx(0)] = (int64_t)__ar_idx;
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
    maxv = _argfi_val_for_20[0];
    index = _argfi_idx_for_20[0];
    {
        double float_index;

        float_index = double(index);  // _convert_to_float64_
        result_host = (maxv + float_index);  // _Add_
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
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argfi_val_for_20, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argfi_idx_for_20, std::align_val_t(64));
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, result, LEN_1D, inc);
}
DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D, int64_t inc);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_1D, int64_t inc)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_experimental_cuda(__state, LEN_1D, inc);

    if (__result) {
        delete __state;
        return nullptr;
    }
    {  // Environment: DetectScratch
        ::dace::cub::get_scratch<::dace::cub::DetectFlagTag>(sizeof(unsigned long long), 0);
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
    {  // Environment: DetectScratch
        ::dace::cub::release_scratch<::dace::cub::DetectFlagTag>();
        ::dace::cub::release_scratch<::dace::cub::DetectOwnerTag>();
    }
    delete __state;
    return __err;
}
