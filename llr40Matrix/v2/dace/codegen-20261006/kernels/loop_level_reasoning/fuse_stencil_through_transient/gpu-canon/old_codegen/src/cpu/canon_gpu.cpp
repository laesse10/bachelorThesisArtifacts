/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED void __dace_runkernel_single_state_body_0_map_0_1_13(canon_gpu_state_t *__state, const double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
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

        __dace_runkernel_single_state_body_0_map_0_1_13(__state, a, out, LEN_1D);

    }
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, out, LEN_1D);
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
