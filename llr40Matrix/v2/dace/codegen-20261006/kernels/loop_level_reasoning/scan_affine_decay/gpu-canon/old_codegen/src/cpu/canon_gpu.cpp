/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};

DACE_EXPORTED gpuError_t __dace_scan_affine_canon_gpu_0_0(const double* __sc_c, const double* __sc_d, const double* __sc_seed_ptr, double __sc_seed_val, double* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
DACE_EXPORTED void __dace_runkernel_size1_wrap_region_0_0_9(canon_gpu_state_t *__state, double * __restrict__ _scan_seed_y, const double * __restrict__ y);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{

    {
        double * _scan_seed_y;
        DACE_GPU_CHECK(hipMalloc((void**)&_scan_seed_y, 1 * sizeof(double)));

        __dace_runkernel_size1_wrap_region_0_0_9(__state, _scan_seed_y, y);
        {
            double * __restrict__ _scan_init = &_scan_seed_y[0];
            double* __restrict__ _scan_in = &x[1];
            double* __restrict__ _scan_coef = &c[1];
            double* __restrict__ _scan_out = y + 1;

            ///////////////////
            hipStream_t __dace_current_stream = nullptr;
            DACE_GPU_CHECK(__dace_scan_affine_canon_gpu_0_0(_scan_coef, _scan_in, _scan_init, static_cast<double>(0), _scan_out, ((LEN_1D - 1)), __dace_current_stream));
            ///////////////////

        }
        DACE_GPU_CHECK(hipFree(_scan_seed_y));

    }
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ c, double * __restrict__ x, double * __restrict__ y, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, c, x, y, LEN_1D);
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
