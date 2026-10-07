/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0_bb_host;
};

DACE_EXPORTED gpuError_t __dace_argreduce_canon_gpu_2_2(const double *__ar_in, double *__ar_val, long long *__ar_idx, long long __ar_items, gpuStream_t __ar_stream);
static DACE_HDFI consteval int64_t bb_host_size() { return 4; }
static DACE_HDFI consteval int64_t _argmax2d_val_for_19_size() { return 1; }
static DACE_HDFI consteval int64_t _argmax2d_idx_for_19_size() { return 1; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _argmax2d_val_for_19_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _argmax2d_idx_for_19_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t bb_host_idx(int64_t __d0, int64_t __d1) { return ((2 * __d0) + __d1); }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double* __restrict__ _argmax2d_val_for_19 = new (std::align_val_t(64)) double[_argmax2d_val_for_19_size()];
    int64_t* __restrict__ _argmax2d_idx_for_19 = new (std::align_val_t(64)) int64_t[_argmax2d_idx_for_19_size()];
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double maxv;
    int64_t xindex;
    int64_t yindex;

    {

        {  // copy_bb_to_bb_host
            hipMemcpyAsync((__state->__0_bb_host), bb, 4 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }
    {

        {  // for_19_argreduce2d_cuda
            double __ar_val;
            long long __ar_idx;
            DACE_GPU_CHECK(__dace_argreduce_canon_gpu_2_2(aa, &__ar_val, &__ar_idx, (long long)((LEN_2D * LEN_2D)), gpu_streams[gpu_streams_idx(0)]));
            _argmax2d_val_for_19[_argmax2d_val_for_19_idx(0)] = __ar_val;
            _argmax2d_idx_for_19[_argmax2d_idx_for_19_idx(0)] = (int64_t)__ar_idx;
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    maxv = _argmax2d_val_for_19[0];
    xindex = py_floor(_argmax2d_idx_for_19[0], LEN_2D);
    yindex = py_mod(_argmax2d_idx_for_19[0], LEN_2D);
    {
        double float_xindex;
        double maxv_plus_expr;
        double float_yindex;

        float_yindex = double(yindex);  // _convert_to_float64_
        float_xindex = double(xindex);  // _convert_to_float64_
        maxv_plus_expr = (maxv + float_xindex);  // _Add_
        __state->__0_bb_host[bb_host_idx(0, 0)] = (maxv_plus_expr + float_yindex);  // _Add_

    }
    {

        {  // copy_bb_host_to_bb
            hipMemcpyAsync(bb, (__state->__0_bb_host), 4 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax2d_val_for_19, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax2d_idx_for_19, std::align_val_t(64));
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, aa, bb, LEN_2D);
}
DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_2D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_2D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_experimental_cuda(__state, LEN_2D);

    if (__result) {
        delete __state;
        return nullptr;
    }
    {  // Environment: DetectScratch
        ::dace::cub::get_scratch<::dace::cub::DetectFlagTag>(sizeof(unsigned long long), 0);
    }
    __state->__0_bb_host = new (std::align_val_t(64)) double[bb_host_size()];

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_bb_host, std::align_val_t(64));

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
