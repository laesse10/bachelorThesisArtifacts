
#include <hip/hip_runtime.h>
#include <dace/dace.h>


struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0_a_split_snap;
};



DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);



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

__global__ void  __launch_bounds__(256) single_state_body_0_map_0_1_19(double * __restrict__ a, const double * __restrict__ a_split_snap, const double * __restrict__ b, const double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D) {
    {
        int64_t b__loop_it_1 = (256 * static_cast<int64_t>(blockIdx.x));
        {
            {
                int64_t _loop_it_1 = (threadIdx.x + b__loop_it_1);
                double c_slice_times_c_slice;
                double b_slice_plus_c_slice_c_slice;
                double b_slice_times_b_slice;
                double b_slice_c_slice_c_slice_plus_b_slice_b_slice;
                double __map_fusion_a;
                if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (Min((LEN_1D - 2), (b__loop_it_1 + 255)) + 1)) {
                    {
                        double __in1 = b[_loop_it_1];
                        double __in2 = b[_loop_it_1];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        b_slice_times_b_slice = __out;
                    }
                    {
                        double __in1 = c[_loop_it_1];
                        double __in2 = c[_loop_it_1];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Mult_)
                        __out = (__in1 * __in2);
                        ///////////////////

                        c_slice_times_c_slice = __out;
                    }
                    {
                        double __in2 = c_slice_times_c_slice;
                        double __in1 = b[_loop_it_1];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        b_slice_plus_c_slice_c_slice = __out;
                    }
                    {
                        double __in2 = b_slice_times_b_slice;
                        double __in1 = b_slice_plus_c_slice_c_slice;
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        b_slice_c_slice_c_slice_plus_b_slice_b_slice = __out;
                    }
                    {
                        double __in1 = b_slice_c_slice_c_slice_plus_b_slice_b_slice;
                        double __in2 = c[_loop_it_1];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        __map_fusion_a = __out;
                    }

                    dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
                    &__map_fusion_a, a + _loop_it_1, 1);
                    {
                        double __in1 = __map_fusion_a;
                        double __in2 = a_split_snap[(_loop_it_1 + 1)];
                        double __out;

                        ///////////////////
                        // Tasklet code (_Add_)
                        __out = (__in1 + __in2);
                        ///////////////////

                        d[_loop_it_1] = __out;
                    }
                }
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_single_state_body_0_map_0_1_19(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ a_split_snap, const double * __restrict__ b, const double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D);
void __dace_runkernel_single_state_body_0_map_0_1_19(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ a_split_snap, const double * __restrict__ b, const double * __restrict__ c, double * __restrict__ d, int64_t LEN_1D)
{

    if (((int_ceil((LEN_1D - 1), 256)) <= 0)) {

        return;
    }

    void  *single_state_body_0_map_0_1_19_args[] = { (void *)&a, (void *)&a_split_snap, (void *)&b, (void *)&c, (void *)&d, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)single_state_body_0_map_0_1_19, dim3(int_ceil((LEN_1D - 1), 256), 1, 1), dim3(256, 1, 1), single_state_body_0_map_0_1_19_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "single_state_body_0_map_0_1_19", int_ceil((LEN_1D - 1), 256), 1, 1, 256, 1, 1);
}

