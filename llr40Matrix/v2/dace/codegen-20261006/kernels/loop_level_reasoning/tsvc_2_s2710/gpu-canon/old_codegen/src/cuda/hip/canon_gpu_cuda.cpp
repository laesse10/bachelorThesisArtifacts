
#include <hip/hip_runtime.h>
#include <dace/dace.h>


struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};



DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

DACE_DFI void loop_body_0_1_0(const double* __restrict__ d, const double* __restrict__ e, const double* __restrict__ x, double* __restrict__ a, double* __restrict__ b, double* __restrict__ c, int64_t LEN_1D, int64_t _loop_it_0) {
    double a_index;
    double b_index;
    double x_index;


    a_index = a[_loop_it_0];
    b_index = b[_loop_it_0];
    if ((a_index > b_index)) {
        {
            double b_slice_times_d_slice;
            double _wcr_priv__Add____out;

            {
                double __in1 = b[_loop_it_0];
                double __in2 = d[_loop_it_0];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                b_slice_times_d_slice = __out;
            }
            {
                double __in2 = b_slice_times_d_slice;
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = __in2;
                ///////////////////

                _wcr_priv__Add____out = __out;
            }
            {
                double __in1 = a[_loop_it_0];
                double __in2 = _wcr_priv__Add____out;
                double __out;

                ///////////////////
                // Tasklet code (augassign)
                __out = (__in1 + __in2);
                ///////////////////

                a[_loop_it_0] = __out;
            }

        }
        if ((LEN_1D > 10)) {
            {
                double d_slice_times_d_slice;
                double _wcr_priv__Add____out_0;

                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = d[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    d_slice_times_d_slice = __out;
                }
                {
                    double __in2 = d_slice_times_d_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out_0 = __out;
                }
                {
                    double __in1 = c[_loop_it_0];
                    double __in2 = _wcr_priv__Add____out_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (augassign)
                    __out = (__in1 + __in2);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        } else {
            {
                double d_slice_times_e_slice;

                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = e[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    d_slice_times_e_slice = __out;
                }
                {
                    double __in1 = d_slice_times_e_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + 1.0);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        }
    } else {

        x_index = x[0];
        {
            double e_slice_times_e_slice;

            {
                double __in1 = e[_loop_it_0];
                double __in2 = e[_loop_it_0];
                double __out;

                ///////////////////
                // Tasklet code (_Mult_)
                __out = (__in1 * __in2);
                ///////////////////

                e_slice_times_e_slice = __out;
            }
            {
                double __in2 = e_slice_times_e_slice;
                double __in1 = a[_loop_it_0];
                double __out;

                ///////////////////
                // Tasklet code (_Add_)
                __out = (__in1 + __in2);
                ///////////////////

                b[_loop_it_0] = __out;
            }

        }
        if ((x_index > 0.0)) {
            {
                double d_slice_times_d_slice_0;

                {
                    double __in1 = d[_loop_it_0];
                    double __in2 = d[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    d_slice_times_d_slice_0 = __out;
                }
                {
                    double __in2 = d_slice_times_d_slice_0;
                    double __in1 = a[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = (__in1 + __in2);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

            }
        } else {
            {
                double e_slice_times_e_slice_0;
                double _wcr_priv__Add____out_1;

                {
                    double __in1 = e[_loop_it_0];
                    double __in2 = e[_loop_it_0];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    e_slice_times_e_slice_0 = __out;
                }
                {
                    double __in2 = e_slice_times_e_slice_0;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    _wcr_priv__Add____out_1 = __out;
                }
                {
                    double __in1 = c[_loop_it_0];
                    double __in2 = _wcr_priv__Add____out_1;
                    double __out;

                    ///////////////////
                    // Tasklet code (augassign)
                    __out = (__in1 + __in2);
                    ///////////////////

                    c[_loop_it_0] = __out;
                }

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

__global__ void  __launch_bounds__(256) single_state_body_map_0_1_12(double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D) {
    {
        int64_t b__loop_it_0 = (256 * static_cast<int64_t>(blockIdx.x));
        {
            {
                int64_t _loop_it_0 = (threadIdx.x + b__loop_it_0);
                if (_loop_it_0 >= b__loop_it_0 && _loop_it_0 < (Min((LEN_1D - 1), (b__loop_it_0 + 255)) + 1)) {
                    loop_body_0_1_0(&d[0], &e[0], &x[0], &a[0], &b[0], &c[0], LEN_1D, _loop_it_0);
                }
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_single_state_body_map_0_1_12(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D);
void __dace_runkernel_single_state_body_map_0_1_12(canon_gpu_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D)
{

    if (((int_ceil(LEN_1D, 256)) <= 0)) {

        return;
    }

    void  *single_state_body_map_0_1_12_args[] = { (void *)&a, (void *)&b, (void *)&c, (void *)&d, (void *)&e, (void *)&x, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)single_state_body_map_0_1_12, dim3(int_ceil(LEN_1D, 256), 1, 1), dim3(256, 1, 1), single_state_body_map_0_1_12_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "single_state_body_map_0_1_12", int_ceil(LEN_1D, 256), 1, 1, 256, 1, 1);
}

