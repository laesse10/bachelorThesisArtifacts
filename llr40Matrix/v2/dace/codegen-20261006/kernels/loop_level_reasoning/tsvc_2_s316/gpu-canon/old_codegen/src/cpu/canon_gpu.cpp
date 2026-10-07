/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED void __dace_runkernel_reduce_init_map_1_1_5(canon_gpu_state_t *__state, double * __restrict__ _out);
DACE_EXPORTED void __dace_runkernel_grid_1_0_0(canon_gpu_state_t *__state, const double * __restrict__ _in, double * __restrict__ _out, int64_t LEN_1D);
inline void reduce_0_1_6(canon_gpu_state_t *__state, const double* __restrict__ _in, double * __restrict__ _out, int64_t LEN_1D) {

    {

        __dace_runkernel_reduce_init_map_1_1_5(__state, _out);

    }
    {

        __dace_runkernel_grid_1_0_0(__state, _in, _out, LEN_1D);

    }
}

void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    double scal_result;
    double result_host;
    double _arg_max_buf_for_17_host;
    double x;

    {
        double * _arg_max_buf_for_17;
        DACE_GPU_CHECK(hipMalloc((void**)&_arg_max_buf_for_17, 1 * sizeof(double)));

        DACE_GPU_CHECK(hipMemcpyAsync(&scal_result, result, 1 * sizeof(double), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &scal_result, &result_host, 1);
        reduce_0_1_6(__state, &a[0], &_arg_max_buf_for_17[0], LEN_1D);
        DACE_GPU_CHECK(hipMemcpyAsync(&_arg_max_buf_for_17_host, _arg_max_buf_for_17, 1 * sizeof(double), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
        DACE_GPU_CHECK(hipFree(_arg_max_buf_for_17));

    }
    x = _arg_max_buf_for_17_host;
    {

        {
            double __out;

            ///////////////////
            // Tasklet code (symassign)
            __out = x;
            ///////////////////

            result_host = __out;
        }

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &result_host, &scal_result, 1);
        DACE_GPU_CHECK(hipMemcpyAsync(result, &scal_result, 1 * sizeof(double), hipMemcpyHostToDevice, nullptr));

    }
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, result, LEN_1D);
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
    delete __state;
    return __err;
}
