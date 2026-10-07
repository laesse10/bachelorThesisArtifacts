/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"
#include "dace/cub_scratch.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    int8_t * __restrict__ __0_compaction_mask_for_17;
    int64_t * __restrict__ __0_compaction_rank_for_17;
};

DACE_EXPORTED gpuError_t __dace_scan_canon_gpu_1_5_c0(const int8_t* __sc_in, int64_t* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
DACE_EXPORTED void __dace_runkernel_single_state_body_map_0_1_15(canon_gpu_state_t *__state, int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D);
DACE_EXPORTED void __dace_runkernel_reduce_init_map_5_1_5(canon_gpu_state_t *__state, int64_t * __restrict__ _out);
DACE_EXPORTED void __dace_runkernel_grid_5_0_0(canon_gpu_state_t *__state, const int8_t * __restrict__ _in, int64_t * __restrict__ _out, int64_t LEN_1D);
inline void reduce_0_1_14(canon_gpu_state_t *__state, const int8_t * __restrict__ _in, int64_t * __restrict__ _out, int64_t LEN_1D) {

    {

        __dace_runkernel_reduce_init_map_5_1_5(__state, _out);

    }
    {

        __dace_runkernel_grid_5_0_0(__state, _in, _out, LEN_1D);

    }
}

DACE_EXPORTED void __dace_runkernel_single_state_body_0_map_0_1_17(canon_gpu_state_t *__state, const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D);
void __program_canon_gpu_internal(canon_gpu_state_t*__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    int64_t scal_out_count;
    int64_t compaction_total_for_17_host;
    int64_t n;

    {

        DACE_GPU_CHECK(hipMemcpyAsync(&scal_out_count, out_count, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

    }
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
        int64_t * compaction_total_for_17;
        DACE_GPU_CHECK(hipMalloc((void**)&compaction_total_for_17, 1 * sizeof(int64_t)));

        __dace_runkernel_single_state_body_map_0_1_15(__state, __state->__0_compaction_mask_for_17, src, LEN_1D);
        {
            int8_t * __restrict__ _scan_in = &__state->__0_compaction_mask_for_17[0];
            int64_t* __restrict__ _scan_out = __state->__0_compaction_rank_for_17;

            ///////////////////
            hipStream_t __dace_current_stream = nullptr;
            DACE_GPU_CHECK(__dace_scan_canon_gpu_1_5_c0(_scan_in, _scan_out, (LEN_1D), __dace_current_stream));
            ///////////////////

        }
        reduce_0_1_14(__state, &__state->__0_compaction_mask_for_17[0], &compaction_total_for_17[0], LEN_1D);
        DACE_GPU_CHECK(hipMemcpyAsync(&compaction_total_for_17_host, compaction_total_for_17, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
        __dace_runkernel_single_state_body_0_map_0_1_17(__state, __state->__0_compaction_rank_for_17, packed, src, weight, LEN_1D);
        DACE_GPU_CHECK(hipFree(compaction_total_for_17));

    }
    n = (0 + (1 * compaction_total_for_17_host));
    {

        {
            int64_t __out;

            ///////////////////
            // Tasklet code (symassign)
            __out = n;
            ///////////////////

            scal_out_count = __out;
        }
        DACE_GPU_CHECK(hipMemcpyAsync(out_count, &scal_out_count, 1 * sizeof(int64_t), hipMemcpyHostToDevice, nullptr));

    }
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, int64_t * __restrict__ out_count, double * __restrict__ packed, double * __restrict__ src, double * __restrict__ weight, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, out_count, packed, src, weight, LEN_1D);
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
    DACE_GPU_CHECK(hipMalloc((void**)&__state->__0_compaction_mask_for_17, LEN_1D * sizeof(int8_t)));
    DACE_GPU_CHECK(hipMalloc((void**)&__state->__0_compaction_rank_for_17, LEN_1D * sizeof(int64_t)));

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;
    DACE_GPU_CHECK(hipFree(__state->__0_compaction_mask_for_17));
    DACE_GPU_CHECK(hipFree(__state->__0_compaction_rank_for_17));

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
