/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_scan_affine_canon_gpu_0_0(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
DACE_EXPORTED void __dace_runkernel_canon_gpu_size1_wrap_region_0_0_12(canon_gpu_state_t *__state, double * __restrict__ _scan_seed_y, const double * __restrict__ y, gpuStream_t gpu_stream);
static DACE_HDFI constexpr int64_t y_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_seed_y_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {
        double * _scan_seed_y;
        DACE_GPU_CHECK(hipMalloc((void**)&_scan_seed_y, 1 * sizeof(double)));

        {
            gpuStream_t __dace_current_stream = gpu_streams[0];
            __dace_runkernel_canon_gpu_size1_wrap_region_0_0_12(__state, _scan_seed_y, y, __dace_current_stream);
        }
        {  // for_16_affine_scan_op
            DACE_GPU_CHECK(__dace_scan_affine_canon_gpu_0_0((c + 1), (x + 1), _scan_seed_y, static_cast<double>(0), (y + 1), ((LEN_1D - 1)), gpu_streams[gpu_streams_idx(0)]));
        }
        DACE_GPU_CHECK(hipFree(_scan_seed_y));

    }
    {

        {  // gpu_streams_synchronization
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, c, x, y, LEN_1D);
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
