
#include <hip/hip_runtime.h>
#include <dace/dace.h>

#include "dace/cuda/gpucub.cuh"
#include "dace/cub_scratch.cuh"
#include "dace/cub_compat.cuh"
#include "dace/cuda/scan_affine.cuh"
#include "dace/cuda/scan.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    int8_t * __restrict__ __0_compaction_mask_for_17;
    int64_t * __restrict__ __0_compaction_rank_for_17;
};

DACE_EXPORTED gpuError_t __dace_scan_canon_gpu_1_5_c0(const int8_t* __sc_in, int64_t* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
gpuError_t __dace_scan_canon_gpu_1_5_c0(const int8_t* __sc_in, int64_t* __sc_out, long long __sc_n, gpuStream_t __sc_stream) {
    size_t _sc_needed = 0;
    gpuError_t _sc_status = ::gpucub::DeviceScan::ExclusiveScan(nullptr, _sc_needed, __sc_in, __sc_out, DACE_CUB_SUM_OP, static_cast<int64_t>(0), __sc_n, __sc_stream);
    if (_sc_status != gpuSuccess) return _sc_status;
    void* _sc_scratch = ::dace::cub::get_scratch<::dace::cub::ScanTag>(_sc_needed, __sc_stream, &_sc_status);
    if (_sc_scratch == nullptr) return _sc_status != gpuSuccess ? _sc_status : gpuErrorMemoryAllocation;
    return ::gpucub::DeviceScan::ExclusiveScan(_sc_scratch, _sc_needed, __sc_in, __sc_out, DACE_CUB_SUM_OP, static_cast<int64_t>(0), __sc_n, __sc_stream);
}


DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

DACE_DFI void loop_body_0_1_0(const double* __restrict__ src, int8_t * __restrict__ compaction_mask_for_17, int64_t compaction_it_for_17_compaction_mask) {
    double src_index;


    src_index = src[compaction_it_for_17_compaction_mask];
    {

        {
            int8_t __out;

            ///////////////////
            // Tasklet code (for_17_compaction_mask_mask)
            __out = (src_index > 0.0);
            ///////////////////

            compaction_mask_for_17[compaction_it_for_17_compaction_mask] = __out;
        }

    }
}

DACE_DFI void loop_body_0_1_7(const int64_t * __restrict__ compaction_rank_for_17, const double* __restrict__ src, const double* __restrict__ weight, double* __restrict__ packed, int64_t compaction_it_for_17_compaction_scatter) {
    double src_index;
    int64_t n_0;


    src_index = src[compaction_it_for_17_compaction_scatter];
    n_0 = (0 + (1 * compaction_rank_for_17[compaction_it_for_17_compaction_scatter]));
    if ((src_index > 0.0)) {
        {

            {
                double __in1 = src[compaction_it_for_17_compaction_scatter];
                double __in2 = weight[compaction_it_for_17_compaction_scatter];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                packed[n_0] = __out;
            }

        }
    }

}



int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_1D) {
    int count;

    // Check that we are able to run hip code
    if (hipGetDeviceCount(&count) != hipSuccess)
    {
        printf("ERROR: GPU drivers are not configured or hip-capable device "
               "not found\n");
        return 1;
    }
    if (count == 0)
    {
        printf("ERROR: No hip-capable devices found\n");
        return 2;
    }

    // One GPU per process, selected here and never changed, so the memory pool, every kernel and
    // every library handle share it. Which physical GPU is the process's business: the visible-
    // devices variable renumbers what it exposes, so a rank's own GPU is device 0. An ordinal
    // fixed at codegen time cannot do that -- every rank shares one build.
    const int __dace_device = 0;
    if (hipSetDevice(__dace_device) != hipSuccess)
    {
        printf("ERROR: could not select hip device 0 out of %d visible\n", count);
        return 4;
    }

    __dace_gpu_drain_error(__state);

    // Initialize hip before we run the application
    float *dev_X;
    DACE_GPU_CHECK(hipMalloc((void **) &dev_X, 1));
    DACE_GPU_CHECK(hipFree(dev_X));

    __state->gpu_context = new dace::cuda::Context(1, 1);

    // After the context exists: DACE_GPU_CHECK records into it.
    

    // Create hip streams and events
    for(int i = 0; i < 1; ++i) {
        DACE_GPU_CHECK(hipStreamCreateWithFlags(&__state->gpu_context->internal_streams[i], hipStreamNonBlocking));
        __state->gpu_context->streams[i] = __state->gpu_context->internal_streams[i]; // Allow for externals to modify streams
    }
    for(int i = 0; i < 1; ++i) {
        DACE_GPU_CHECK(hipEventCreateWithFlags(&__state->gpu_context->events[i], hipEventDisableTiming));
    }

    

    return 0;
}

int __dace_exit_cuda(canon_gpu_state_t *__state) {
    

    // Synchronize and check for CUDA errors
    int __err = static_cast<int>(__state->gpu_context->lasterror);
    if (__err == 0)
        __err = static_cast<int>(hipDeviceSynchronize());

    // Destroy hip streams and events
    for(int i = 0; i < 1; ++i) {
        DACE_GPU_CHECK(hipStreamDestroy(__state->gpu_context->internal_streams[i]));
    }
    for(int i = 0; i < 1; ++i) {
        DACE_GPU_CHECK(hipEventDestroy(__state->gpu_context->events[i]));
    }

    delete __state->gpu_context;
    return __err;
}

// Discard a pending error left by another GPU user in this process, so the next checked call does
// not report it as its own. Sticky errors survive this and are reported normally.
// Must not touch __state->gpu_context: init calls this before the context exists.
void __dace_gpu_drain_error(canon_gpu_state_t *__state) {
    (void)__state;
    gpuError_t __pre_existing = hipGetLastError();
    if (__pre_existing != (gpuError_t)0) {
        printf("WARNING: a GPU error was already pending on entry to a DaCe program and has been "
               "discarded: %s (%d). It was not caused by this SDFG.\n",
               gpuGetErrorString(__pre_existing), __pre_existing);
    }
}

// Returns what the generated code recorded, not the runtime's shared slot, and clears it.
int __dace_gpu_last_error(canon_gpu_state_t *__state) {
    int __err = static_cast<int>(__state->gpu_context->lasterror);
    __state->gpu_context->lasterror = (gpuError_t)0;
    return __err;
}

bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream)
{
    if (streamid < 0 || streamid >= 1)
        return false;

    __state->gpu_context->streams[streamid] = stream;

    return true;
}

void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream)
{
    for (int i = 0; i < 1; ++i)
        __state->gpu_context->streams[i] = stream;
}

__global__ void  __launch_bounds__(256) single_state_body_map_0_1_15(int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D) {
    {
        int64_t b_compaction_it_for_17_compaction_mask = (256 * static_cast<int64_t>(blockIdx.x));
        {
            {
                int64_t compaction_it_for_17_compaction_mask = (threadIdx.x + b_compaction_it_for_17_compaction_mask);
                if (compaction_it_for_17_compaction_mask >= b_compaction_it_for_17_compaction_mask && compaction_it_for_17_compaction_mask < (Min((LEN_1D - 1), (b_compaction_it_for_17_compaction_mask + 255)) + 1)) {
                    loop_body_0_1_0(&src[0], &compaction_mask_for_17[0], compaction_it_for_17_compaction_mask);
                }
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_single_state_body_map_0_1_15(canon_gpu_state_t *__state, int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D);
void __dace_runkernel_single_state_body_map_0_1_15(canon_gpu_state_t *__state, int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D)
{

    if (((int_ceil(LEN_1D, 256)) <= 0)) {

        return;
    }

    void  *single_state_body_map_0_1_15_args[] = { (void *)&compaction_mask_for_17, (void *)&src, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)single_state_body_map_0_1_15, dim3(int_ceil(LEN_1D, 256), 1, 1), dim3(256, 1, 1), single_state_body_map_0_1_15_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "single_state_body_map_0_1_15", int_ceil(LEN_1D, 256), 1, 1, 256, 1, 1);
}
__global__ void  __launch_bounds__(128) reduce_init_map_5_1_5(int64_t * __restrict__ _out) {
    {
        int b__o = (128 * blockIdx.x);
        {
            {
                int _o = (threadIdx.x + b__o);
                if (_o >= b__o && _o < (Min(0, (b__o + 127)) + 1)) {
                    {
                        int64_t __out;

                        ///////////////////
                        // Tasklet code (reduce_init)
                        __out = 0;
                        ///////////////////

                        _out[0] = __out;
                    }
                }
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_reduce_init_map_5_1_5(canon_gpu_state_t *__state, int64_t * __restrict__ _out);
void __dace_runkernel_reduce_init_map_5_1_5(canon_gpu_state_t *__state, int64_t * __restrict__ _out)
{

    void  *reduce_init_map_5_1_5_args[] = { (void *)&_out };
    gpuError_t __err = hipLaunchKernel((void*)reduce_init_map_5_1_5, dim3(1, 1, 1), dim3(128, 1, 1), reduce_init_map_5_1_5_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "reduce_init_map_5_1_5", 1, 1, 1, 128, 1, 1);
}
__global__ void  __launch_bounds__(64) grid_5_0_0(const int8_t * __restrict__ _in, int64_t * __restrict__ _out, int64_t LEN_1D) {
    {
        int64_t _o0 = static_cast<int64_t>(blockIdx.x);
        int64_t acc;
        {
            {
                int tid = threadIdx.x;
                {
                    {
                        int64_t __o_out;

                        ///////////////////
                        // Tasklet code (init_scalar)
                        __o_out = 0;
                        ///////////////////

                        acc = __o_out;
                    }
                    {
                        assert(((1024 * int_ceil(LEN_1D, 1024))) > 0 && "Map reduce_values requires a positive step");
                        for (int64_t _j0 = (1024 * _o0); _j0 < LEN_1D; _j0 += (1024 * int_ceil(LEN_1D, 1024))) {
                            for (int64_t _i0 = (_j0 + tid); _i0 < (Min((LEN_1D - 1), (_j0 + 1023)) + 1); _i0 += 64) {
                                {
                                    int8_t __b_in = _in[_i0];
                                    int64_t __a_in = acc;
                                    int64_t __o_out;

                                    ///////////////////
                                    // Tasklet code (identity)
                                    __o_out = __b_in;
                                    ///////////////////

                                    dace::wcr_fixed<dace::ReductionType::Sum, int64_t>::reduce(&acc, __o_out);
                                }
                            }
                        }
                    }
                    {
                        int64_t __a = acc;
                        int64_t __out;

                        ///////////////////
                        __out = dace::warpReduce<dace::ReductionType::Sum, int64_t>::reduce(__a);
                        ///////////////////

                        acc = __out;
                    }
                }
            }
        }
        {
            int64_t _input = acc;
            int64_t _output;

            ///////////////////
            // Tasklet code (cond_write)
            if ((threadIdx.x == 0)) {
                _output = dace::wcr_fixed<dace::ReductionType::Sum, int64_t>::reduce_atomic(_out, _input);
            }
            ///////////////////

        }
    }
}


DACE_EXPORTED void __dace_runkernel_grid_5_0_0(canon_gpu_state_t *__state, const int8_t * __restrict__ _in, int64_t * __restrict__ _out, int64_t LEN_1D);
void __dace_runkernel_grid_5_0_0(canon_gpu_state_t *__state, const int8_t * __restrict__ _in, int64_t * __restrict__ _out, int64_t LEN_1D)
{

    if (((int_ceil(LEN_1D, 1024)) <= 0)) {

        return;
    }

    void  *grid_5_0_0_args[] = { (void *)&_in, (void *)&_out, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)grid_5_0_0, dim3(int_ceil(LEN_1D, 1024), 1, 1), dim3(64, 1, 1), grid_5_0_0_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "grid_5_0_0", int_ceil(LEN_1D, 1024), 1, 1, 64, 1, 1);
}
__global__ void  __launch_bounds__(256) single_state_body_0_map_0_1_17(const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D) {
    {
        int64_t b_compaction_it_for_17_compaction_scatter = (256 * static_cast<int64_t>(blockIdx.x));
        {
            {
                int64_t compaction_it_for_17_compaction_scatter = (threadIdx.x + b_compaction_it_for_17_compaction_scatter);
                if (compaction_it_for_17_compaction_scatter >= b_compaction_it_for_17_compaction_scatter && compaction_it_for_17_compaction_scatter < (Min((LEN_1D - 1), (b_compaction_it_for_17_compaction_scatter + 255)) + 1)) {
                    loop_body_0_1_7(&compaction_rank_for_17[0], &src[0], &weight[0], &packed[0], compaction_it_for_17_compaction_scatter);
                }
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_single_state_body_0_map_0_1_17(canon_gpu_state_t *__state, const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D);
void __dace_runkernel_single_state_body_0_map_0_1_17(canon_gpu_state_t *__state, const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D)
{

    if (((int_ceil(LEN_1D, 256)) <= 0)) {

        return;
    }

    void  *single_state_body_0_map_0_1_17_args[] = { (void *)&compaction_rank_for_17, (void *)&packed, (void *)&src, (void *)&weight, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)single_state_body_0_map_0_1_17, dim3(int_ceil(LEN_1D, 256), 1, 1), dim3(256, 1, 1), single_state_body_0_map_0_1_17_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "single_state_body_0_map_0_1_17", int_ceil(LEN_1D, 256), 1, 1, 256, 1, 1);
}

