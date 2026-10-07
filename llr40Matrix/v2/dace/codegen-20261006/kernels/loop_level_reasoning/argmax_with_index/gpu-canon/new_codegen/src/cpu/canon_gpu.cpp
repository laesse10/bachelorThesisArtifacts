/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_argreduce_canon_gpu_1_7(const double *__ar_in, double *__ar_val, long long *__ar_idx, long long __ar_items, gpuStream_t __ar_stream);
static DACE_HDFI consteval int64_t _argmax_val_for_18_size() { return 1; }
static DACE_HDFI consteval int64_t _argmax_idx_for_18_size() { return 1; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _argmax_val_for_18_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _argmax_idx_for_18_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    double* __restrict__ _argmax_val_for_18 = new (std::align_val_t(64)) double[_argmax_val_for_18_size()];
    int64_t* __restrict__ _argmax_idx_for_18 = new (std::align_val_t(64)) int64_t[_argmax_idx_for_18_size()];
    double scal_out_value;
    int64_t scal_out_index;
    double out_value_host;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double x;
    int64_t idx;

    {

        {  // for_18_argreduce_cuda
            double __ar_val;
            long long __ar_idx;
            DACE_GPU_CHECK(__dace_argreduce_canon_gpu_1_7(a, &__ar_val, &__ar_idx, (long long)(LEN_1D), gpu_streams[gpu_streams_idx(0)]));
            _argmax_val_for_18[_argmax_val_for_18_idx(0)] = __ar_val;
            _argmax_idx_for_18[_argmax_idx_for_18_idx(0)] = (int64_t)__ar_idx;
        }
        {  // copy_out_index_to_scal_out_index
            hipMemcpyAsync((&scal_out_index), out_index, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_out_value_to_scal_out_value
            hipMemcpyAsync((&scal_out_value), out_value, 1 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    {

        out_value_host = scal_out_value;  // copy_scal_out_value_to_out_value_host

    }
    x = _argmax_val_for_18[0];
    idx = _argmax_idx_for_18[0];
    {

        scal_out_index = idx;  // symassign
        out_value_host = x;  // symassign
        scal_out_value = out_value_host;  // copy_out_value_host_to_scal_out_value

    }
    {

        {  // copy_scal_out_value_to_out_value
            hipMemcpyAsync(out_value, (&scal_out_value), 1 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_scal_out_index_to_out_index
            hipMemcpyAsync(out_index, (&scal_out_index), 1 * sizeof(int64_t), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax_val_for_18, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax_idx_for_18, std::align_val_t(64));
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, out_index, out_value, LEN_1D);
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
