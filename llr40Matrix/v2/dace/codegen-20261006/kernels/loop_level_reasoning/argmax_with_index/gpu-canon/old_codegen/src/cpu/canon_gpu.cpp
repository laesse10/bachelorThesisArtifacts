/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_argreduce_canon_gpu_1_7(const double *__ar_in, double *__ar_val, long long *__ar_idx, long long __ar_items, gpuStream_t __ar_stream);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    double *_argmax_val_for_18;
    _argmax_val_for_18 = new (std::align_val_t(64)) double[1];
    int64_t *_argmax_idx_for_18;
    _argmax_idx_for_18 = new (std::align_val_t(64)) int64_t[1];
    double scal_out_value;
    int64_t scal_out_index;
    double out_value_host;
    double x;
    int64_t idx;

    {

        DACE_GPU_CHECK(hipMemcpyAsync(&scal_out_value, out_value, 1 * sizeof(double), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &scal_out_value, &out_value_host, 1);
        DACE_GPU_CHECK(hipMemcpyAsync(&scal_out_index, out_index, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
        {
            double* __restrict__ _in = &a[0];
            double _out_val;
            int64_t _out_idx;

            ///////////////////
            hipStream_t __dace_current_stream = nullptr;
            double __ar_val;
            long long __ar_idx;
            DACE_GPU_CHECK(__dace_argreduce_canon_gpu_1_7(_in, &__ar_val, &__ar_idx, (long long)(LEN_1D), __dace_current_stream));
            _out_val = __ar_val;
            _out_idx = (int64_t)__ar_idx;
            ///////////////////

            _argmax_val_for_18[0] = _out_val;
            _argmax_idx_for_18[0] = _out_idx;
        }

    }
    x = _argmax_val_for_18[0];
    idx = _argmax_idx_for_18[0];
    {

        {
            double __out;

            ///////////////////
            // Tasklet code (symassign)
            __out = x;
            ///////////////////

            out_value_host = __out;
        }

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &out_value_host, &scal_out_value, 1);
        DACE_GPU_CHECK(hipMemcpyAsync(out_value, &scal_out_value, 1 * sizeof(double), hipMemcpyHostToDevice, nullptr));
        {
            int64_t __out;

            ///////////////////
            // Tasklet code (symassign)
            __out = idx;
            ///////////////////

            scal_out_index = __out;
        }
        DACE_GPU_CHECK(hipMemcpyAsync(out_index, &scal_out_index, 1 * sizeof(int64_t), hipMemcpyHostToDevice, nullptr));

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax_val_for_18, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax_idx_for_18, std::align_val_t(64));
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, out_index, out_value, LEN_1D);
}
DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_cuda(__state, LEN_1D);

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

    int __err_cuda = __dace_exit_cuda(__state);
    if (__err_cuda) {
        __err = __err_cuda;
    }
    {  // Environment: DetectScratch
        ::dace::cub::release_scratch<::dace::cub::DetectFlagTag>();
        ::dace::cub::release_scratch<::dace::cub::DetectOwnerTag>();
    }
    delete __state;
    return __err;
}
