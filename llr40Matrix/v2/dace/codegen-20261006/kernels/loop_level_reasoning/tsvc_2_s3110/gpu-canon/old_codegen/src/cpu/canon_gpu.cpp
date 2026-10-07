/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0_bb_host;
};

DACE_EXPORTED gpuError_t __dace_argreduce_canon_gpu_2_2(const double *__ar_in, double *__ar_val, long long *__ar_idx, long long __ar_items, gpuStream_t __ar_stream);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    double *_argmax2d_val_for_19;
    _argmax2d_val_for_19 = new (std::align_val_t(64)) double[1];
    int64_t *_argmax2d_idx_for_19;
    _argmax2d_idx_for_19 = new (std::align_val_t(64)) int64_t[1];
    double maxv;
    int64_t xindex;
    int64_t yindex;

    {

        DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_bb_host, bb, 4 * sizeof(double), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

    }
    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_2D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {

        {
            double* __restrict__ _in = &aa[0];
            double _out_val;
            int64_t _out_idx;

            ///////////////////
            hipStream_t __dace_current_stream = nullptr;
            double __ar_val;
            long long __ar_idx;
            DACE_GPU_CHECK(__dace_argreduce_canon_gpu_2_2(_in, &__ar_val, &__ar_idx, (long long)((LEN_2D * LEN_2D)), __dace_current_stream));
            _out_val = __ar_val;
            _out_idx = (int64_t)__ar_idx;
            ///////////////////

            _argmax2d_val_for_19[0] = _out_val;
            _argmax2d_idx_for_19[0] = _out_idx;
        }

    }
    maxv = _argmax2d_val_for_19[0];
    xindex = py_floor(_argmax2d_idx_for_19[0], LEN_2D);
    yindex = py_mod(_argmax2d_idx_for_19[0], LEN_2D);
    {
        double float_xindex;
        double maxv_plus_expr;
        double float_yindex;

        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(xindex);
            ///////////////////

            float_xindex = __out;
        }
        {
            double __in2 = float_xindex;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (maxv + __in2);
            ///////////////////

            maxv_plus_expr = __out;
        }
        {
            double __out;

            ///////////////////
            // Tasklet code (_convert_to_float64_)
            __out = double(yindex);
            ///////////////////

            float_yindex = __out;
        }
        {
            double __in2 = float_yindex;
            double __in1 = maxv_plus_expr;
            double __out;

            ///////////////////
            // Tasklet code (_Add_)
            __out = (__in1 + __in2);
            ///////////////////

            __state->__0_bb_host[0] = __out;
        }
        DACE_GPU_CHECK(hipMemcpyAsync(bb, __state->__0_bb_host, 4 * sizeof(double), hipMemcpyHostToDevice, nullptr));

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax2d_val_for_19, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax2d_idx_for_19, std::align_val_t(64));
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, aa, bb, LEN_2D);
}
DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_2D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_2D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_cuda(__state, LEN_2D);

    if (__result) {
        delete __state;
        return nullptr;
    }
    {  // Environment: DetectScratch
        ::dace::cub::get_scratch<::dace::cub::DetectFlagTag>(sizeof(unsigned long long), 0);
    }
    __state->__0_bb_host = new (std::align_val_t(64)) double[4];

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
