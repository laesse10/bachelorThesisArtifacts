
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

static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static DACE_HDFI constexpr int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }


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

__global__ void __launch_bounds__(256) canon_gpu_single_state_body_0_map_0_1_23(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_2D) {
    int64_t b__loop_it_3 = (256 * static_cast<int64_t>(blockIdx.x));
    {
        int64_t _loop_it_3 = (threadIdx.x + b__loop_it_3);
        double c_slice_times_d_slice;
        if (_loop_it_3 >= b__loop_it_3 && _loop_it_3 < (Min((LEN_2D - 1), (b__loop_it_3 + 255)) + 1)) {
            c_slice_times_d_slice = (c[c_idx(_loop_it_3)] * d[d_idx(_loop_it_3)]);  // _Mult_
            a[a_idx(_loop_it_3)] = (b[b_idx(_loop_it_3)] + c_slice_times_d_slice);  // _Add_
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_0_map_0_1_23(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_2D, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_single_state_body_0_map_0_1_23(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_2D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_2D, 256)) <= 0)) {

        return;
    }

    void  *canon_gpu_single_state_body_0_map_0_1_23_args[] = { (void *)&a, (void *)&b, (void *)&c, (void *)&d, (void *)&LEN_2D };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_single_state_body_0_map_0_1_23, dim3(int_ceil(LEN_2D, 256), 1, 1), dim3(256, 1, 1), canon_gpu_single_state_body_0_map_0_1_23_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_single_state_body_0_map_0_1_23", int_ceil(LEN_2D, 256), 1, 1, 256, 1, 1);
}
__global__ void __launch_bounds__(256) canon_gpu_single_state_body_map_0_1_25(double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D) {
    int64_t b__loop_it_2 = (64 * static_cast<int64_t>(blockIdx.x));
    int64_t b__loop_it_1 = (4 * static_cast<int64_t>(blockIdx.y));
    {
        int64_t _loop_it_2 = (threadIdx.x + b__loop_it_2);
        int64_t _loop_it_1 = (threadIdx.y + b__loop_it_1);
        double bb_slice_times_cc_slice;
        double _wcr_priv__Add____out;
        if (_loop_it_2 >= b__loop_it_2 && _loop_it_2 < (Min((LEN_2D - 1), (b__loop_it_2 + 63)) + 1)) {
            if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (Min((LEN_2D - 1), (b__loop_it_1 + 3)) + 1)) {
                bb_slice_times_cc_slice = (bb[bb_idx(_loop_it_1, _loop_it_2, LEN_2D)] * cc[cc_idx(_loop_it_1, _loop_it_2, LEN_2D)]);  // _Mult_
                _wcr_priv__Add____out = bb_slice_times_cc_slice;  // _Add_
                aa[aa_idx(_loop_it_1, _loop_it_2, LEN_2D)] = (aa[aa_idx(_loop_it_1, _loop_it_2, LEN_2D)] + _wcr_priv__Add____out);  // augassign
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_0_1_25(canon_gpu_state_t *__state, double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_single_state_body_map_0_1_25(canon_gpu_state_t *__state, double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_2D, 64)) <= 0) || ((int_ceil(LEN_2D, 4)) <= 0)) {

        return;
    }

    void  *canon_gpu_single_state_body_map_0_1_25_args[] = { (void *)&aa, (void *)&bb, (void *)&cc, (void *)&LEN_2D };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_single_state_body_map_0_1_25, dim3(int_ceil(LEN_2D, 64), int_ceil(LEN_2D, 4), 1), dim3(64, 4, 1), canon_gpu_single_state_body_map_0_1_25_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_single_state_body_map_0_1_25", int_ceil(LEN_2D, 64), int_ceil(LEN_2D, 4), 1, 64, 4, 1);
}

