/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0_a_split_snap;
};

DACE_EXPORTED void __dace_runkernel_single_state_body_0_map_0_1_19(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ a_split_snap, const double * __restrict__ b, const double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
{

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

        DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_a_split_snap + 1, a + 1, (LEN_1D - 1) * sizeof(double), hipMemcpyDeviceToDevice, nullptr));
        __dace_runkernel_single_state_body_0_map_0_1_19(__state, a, __state->__0_a_split_snap, b, c, d, LEN_1D);

    }
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, c, d, LEN_1D);
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
    DACE_GPU_CHECK(hipMalloc((void**)&__state->__0_a_split_snap, LEN_1D * sizeof(double)));

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;
    DACE_GPU_CHECK(hipFree(__state->__0_a_split_snap));

    int __err_cuda = __dace_exit_cuda(__state);
    if (__err_cuda) {
        __err = __err_cuda;
    }
    delete __state;
    return __err;
}
