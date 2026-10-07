/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"
constexpr double __sum = 0.0;

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_scan_canon_gpu_0_5_c0(const double* __sc_in, double* __sc_out, double __sc_init, long long __sc_n, gpuStream_t __sc_stream);
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    double _scan_seed_b;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // _assign___sum_to__scan_seed_b
            double _in = __sum;
            double _out;
            _out = _in;
            _scan_seed_b = _out;
        }

    }
    {

        {  // for_19_scan_op_op
            DACE_GPU_CHECK(__dace_scan_canon_gpu_0_5_c0(a, b, _scan_seed_b, (LEN_1D), gpu_streams[gpu_streams_idx(0)]));
        }

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, b, LEN_1D);
}
DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_experimental_cuda(__state, LEN_1D);

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

    int __err_experimental_cuda = __dace_exit_experimental_cuda(__state);
    if (__err_experimental_cuda) {
        __err = __err_experimental_cuda;
    }
    {  // Environment: ScanScratch
        ::dace::cub::release_scratch<::dace::cub::ScanTag>();
    }
    delete __state;
    return __err;
}
