
#include <hip/hip_runtime.h>
#include <dace/dace.h>


struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};



DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }


int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D) {
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

    __state->gpu_context = new dace::cuda::Context(1, 0);

    // After the context exists: DACE_GPU_CHECK records into it.
    

    // Create hip streams and events
    for(int i = 0; i < 1; ++i) {
        __state->gpu_context->internal_streams[i] = nullptr;
        __state->gpu_context->streams[i] = __state->gpu_context->internal_streams[i]; // Allow for externals to modify streams
    }
    for(int i = 0; i < 0; ++i) {
        DACE_GPU_CHECK(hipEventCreateWithFlags(&__state->gpu_context->events[i], hipEventDisableTiming));
    }

    

    return 0;
}

int __dace_exit_experimental_cuda(canon_gpu_state_t *__state) {
    

    // Synchronize and check for CUDA errors
    int __err = static_cast<int>(__state->gpu_context->lasterror);
    if (__err == 0)
        __err = static_cast<int>(hipDeviceSynchronize());

    // Destroy hip streams and events
    for(int i = 0; i < 1; ++i) {
        { /* no action needed */ };
    }
    for(int i = 0; i < 0; ++i) {
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

__global__ void __launch_bounds__(128) canon_gpu_reduce_init_map_0_4_6(double * __restrict__ _arg_max_buf_for_17) {
    int b__o = (128 * blockIdx.x);
    {
        int _o = (threadIdx.x + b__o);
        if (_o >= b__o && _o < (Min(0, (b__o + 127)) + 1)) {
            {  // reduce_init
                double __out;
                __out = 1.7976931348623157e+308;
                _arg_max_buf_for_17[0] = __out;
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_reduce_init_map_0_4_6(canon_gpu_state_t *__state, double * __restrict__ _arg_max_buf_for_17, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_reduce_init_map_0_4_6(canon_gpu_state_t *__state, double * __restrict__ _arg_max_buf_for_17, gpuStream_t gpu_stream)

{

    void  *canon_gpu_reduce_init_map_0_4_6_args[] = { (void *)&_arg_max_buf_for_17 };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_reduce_init_map_0_4_6, dim3(1, 1, 1), dim3(128, 1, 1), canon_gpu_reduce_init_map_0_4_6_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_reduce_init_map_0_4_6", 1, 1, 1, 128, 1, 1);
}
__global__ void __launch_bounds__(64) canon_gpu_grid_0_3_0(double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D) {
    int64_t _o0 = static_cast<int64_t>(blockIdx.x);
    double acc;
    {
        int tid = threadIdx.x;
        {  // init_scalar
            double __o_out;
            __o_out = 1.7976931348623157e+308;
            acc = __o_out;
        }
        assert(((1024 * int_ceil(LEN_1D, 1024))) > 0 && "Map reduce_values requires a positive step");
        for (int64_t _j0 = (1024 * _o0); _j0 < LEN_1D; _j0 += (1024 * int_ceil(LEN_1D, 1024))) {
            for (int64_t _i0 = (_j0 + tid); _i0 < (Min((LEN_1D - 1), (_j0 + 1023)) + 1); _i0 += 64) {
                {  // identity
                    double __o_out;
                    __o_out = a[a_idx(_i0)];
                    dace::wcr_fixed<dace::ReductionType::Min, double>::reduce(&acc, __o_out);
                }
            }
        }
        {  // warp_reduce
            acc = dace::warpReduce<dace::ReductionType::Min, double>::reduce(acc);
        }
    }
    {  // cond_write
        double _input = acc;
        double _output;
        if ((threadIdx.x == 0)) {
            _output = dace::wcr_fixed<dace::ReductionType::Min, double>::reduce_atomic(_arg_max_buf_for_17, _input);
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_grid_0_3_0(canon_gpu_state_t *__state, double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_grid_0_3_0(canon_gpu_state_t *__state, double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 1024)) <= 0)) {

        return;
    }

    void  *canon_gpu_grid_0_3_0_args[] = { (void *)&_arg_max_buf_for_17, (void *)&a, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_grid_0_3_0, dim3(int_ceil(LEN_1D, 1024), 1, 1), dim3(64, 1, 1), canon_gpu_grid_0_3_0_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_grid_0_3_0", int_ceil(LEN_1D, 1024), 1, 1, 64, 1, 1);
}

