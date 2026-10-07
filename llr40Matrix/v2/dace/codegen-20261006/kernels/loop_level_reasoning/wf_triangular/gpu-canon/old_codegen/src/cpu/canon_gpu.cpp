/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED void __dace_runkernel_single_state_body_map_1_0_3(canon_gpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D, int64_t _skew_t_0);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, int64_t LEN_2D)
{
    int64_t _skew_t_0;

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

    for (_skew_t_0 = 0; (_skew_t_0 <= ((2 * int_ceil((LEN_2D - 1), 64)) - 2)); _skew_t_0 = (_skew_t_0 + 1)) {
        {

            __dace_runkernel_single_state_body_map_1_0_3(__state, a, LEN_2D, _skew_t_0);

        }

    }

}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, LEN_2D);
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
