/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED void __dace_runkernel_single_state_body_map_0_2_14(canon_gpu_state_t *__state, double * __restrict__ _priv_sum_val_gpu, const double * __restrict__ a, int64_t LEN_1D);
DACE_EXPORTED void __dace_runkernel_size1_wrap_region_0_2_16(canon_gpu_state_t *__state, const double * __restrict__ _priv_sum_val_gpu, double * __restrict__ b);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double * _priv_sum_val_gpu;
    DACE_GPU_CHECK(hipMalloc((void**)&_priv_sum_val_gpu, 1 * sizeof(double)));

    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    {
        double _priv_sum_val;

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_16_4)
            __out = 0.0;
            ///////////////////

            _priv_sum_val = __out;
        }
        DACE_GPU_CHECK(hipMemcpyAsync(_priv_sum_val_gpu, &_priv_sum_val, 1 * sizeof(double), hipMemcpyHostToDevice, nullptr));

    }
    {

        __dace_runkernel_single_state_body_map_0_2_14(__state, _priv_sum_val_gpu, a, LEN_1D);
        __dace_runkernel_size1_wrap_region_0_2_16(__state, _priv_sum_val_gpu, b);

    }
    DACE_GPU_CHECK(hipFree(_priv_sum_val_gpu));
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, LEN_1D);
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
