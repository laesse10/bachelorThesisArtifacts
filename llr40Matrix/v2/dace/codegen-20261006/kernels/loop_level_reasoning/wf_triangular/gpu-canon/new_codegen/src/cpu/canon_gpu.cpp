/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_1_0_3(canon_gpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D, int64_t _skew_t_0, gpuStream_t gpu_stream);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, int64_t LEN_2D)
{

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    for (int64_t _skew_t_0 = 0; (_skew_t_0 <= ((2 * int_ceil((LEN_2D - 1), 64)) - 2)); _skew_t_0 = (_skew_t_0 + 1)) {
        {
            gpuStream_t* gpu_streams = __state->gpu_context->streams;

            {
                gpuStream_t __dace_current_stream = gpu_streams[0];
                __dace_runkernel_canon_gpu_single_state_body_map_1_0_3(__state, a, LEN_2D, _skew_t_0, __dace_current_stream);
            }
            gpu_streams = nullptr;

        }

    }

}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, LEN_2D);
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
