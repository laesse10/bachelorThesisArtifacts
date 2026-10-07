/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_scan_canon_gpu_0_5_c0(const double* __sc_in, double* __sc_out, double __sc_init, long long __sc_n, gpuStream_t __sc_stream);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{

    {
        double __sum;
        double _scan_seed_b;

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_18_4)
            __out = 0.0;
            ///////////////////

            __sum = __out;
        }
        {
            double _in = __sum;
            double _out;

            ///////////////////
            // Tasklet code (_assign___sum_to__scan_seed_b)
            _out = _in;
            ///////////////////

            _scan_seed_b = _out;
        }
        {
            double _scan_init = _scan_seed_b;
            double* __restrict__ _scan_in = &a[0];
            double* __restrict__ _scan_out = b;

            ///////////////////
            hipStream_t __dace_current_stream = nullptr;
            DACE_GPU_CHECK(__dace_scan_canon_gpu_0_5_c0(_scan_in, _scan_out, _scan_init, (LEN_1D), __dace_current_stream));
            ///////////////////

        }

    }
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
    {  // Environment: ScanScratch
        ::dace::cub::get_scratch<::dace::cub::ScanTag>(134217728ull, 0);
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
    {  // Environment: ScanScratch
        ::dace::cub::release_scratch<::dace::cub::ScanTag>();
    }
    delete __state;
    return __err;
}
