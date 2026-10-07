
#include <hip/hip_runtime.h>
#include <dace/dace.h>


struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};



DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_2D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }


int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_2D) {
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

__global__ void __launch_bounds__(256) canon_gpu_single_state_body_map_1_0_18(double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0) {
    int64_t b__loop_it_1 = (256 * static_cast<int64_t>(blockIdx.x));
    {
        int64_t _loop_it_1 = (threadIdx.x + b__loop_it_1);
        double a_index;
        double a_index_0;
        double a_index_1;
        double a_slice_plus_a_slice;
        double a_slice_a_slice_plus_a_slice;
        if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (Min((LEN_2D - 2), (b__loop_it_1 + 255)) + 1)) {
            a_index = a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index
            a_index_0 = a[a_idx((_loop_it_0 - 1), _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index_0
            a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
            a_index_1 = a[a_idx((_loop_it_0 - 1), (_loop_it_1 + 1), LEN_2D)];  // _assign_in_a_to_a_index_1
            a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = a_slice_a_slice_plus_a_slice;  // _assign_out_a_slice_a_slice_plus_a_slice_to_a
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_1_0_18(canon_gpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_single_state_body_map_1_0_18(canon_gpu_state_t *__state, double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_2D - 1), 256)) <= 0)) {

        return;
    }

    void  *canon_gpu_single_state_body_map_1_0_18_args[] = { (void *)&a, (void *)&LEN_2D, (void *)&_loop_it_0 };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_single_state_body_map_1_0_18, dim3(int_ceil((LEN_2D - 1), 256), 1, 1), dim3(256, 1, 1), canon_gpu_single_state_body_map_1_0_18_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_single_state_body_map_1_0_18", int_ceil((LEN_2D - 1), 256), 1, 1, 256, 1, 1);
}

