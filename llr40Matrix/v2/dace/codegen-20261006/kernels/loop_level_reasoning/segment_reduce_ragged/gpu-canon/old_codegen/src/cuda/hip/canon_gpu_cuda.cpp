
#include <hip/hip_runtime.h>
#include <dace/dace.h>


struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};



DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t NSEG);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

DACE_DFI void loop_body_1_0_0(const int64_t* __restrict__ row_ptr, const double* __restrict__ val, const double* __restrict__ w, double* __restrict__ out, int __tid, int64_t _loop_it_0) {
    double acc;
    double _priv_acc;
    int64_t row_ptr_index;
    int64_t row_ptr_index_0;

    {

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_17_8)
            __out = 0.0;
            ///////////////////

            acc = __out;
        }

    }
    row_ptr_index = row_ptr[_loop_it_0];
    row_ptr_index_0 = row_ptr[(_loop_it_0 + 1)];
    {


        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &acc, &_priv_acc, 1);

    }
    {
        double _priv_acc_0;

        {
            double __out;

            ///////////////////
            __out = double(0.0);
            ///////////////////

            _priv_acc_0 = __out;
        }
        {
            for (int64_t _loop_it_1 = (__tid + row_ptr_index); _loop_it_1 < row_ptr_index_0; _loop_it_1 += 256) {
                double val_slice_times_w_slice;
                double acc_plus_val_slice_w_slice;
                double _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out;
                {
                    double __in1 = val[_loop_it_1];
                    double __in2 = w[_loop_it_1];
                    double __out;

                    ///////////////////
                    // Tasklet code (_Mult_)
                    __out = (__in1 * __in2);
                    ///////////////////

                    val_slice_times_w_slice = __out;
                }
                {
                    double __in2 = val_slice_times_w_slice;
                    double __out;

                    ///////////////////
                    // Tasklet code (_Add_)
                    __out = __in2;
                    ///////////////////

                    acc_plus_val_slice_w_slice = __out;
                }
                {
                    double _in = acc_plus_val_slice_w_slice;
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_out_acc_plus_val_slice_w_slice_to__priv_acc)
                    _out = _in;
                    ///////////////////

                    _wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out = _out;
                }

                dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Accumulate(
                &_wcr_priv__assign_out_acc_plus_val_slice_w_slice_to__priv_acc__out, &_priv_acc_0, [] (const double& a, const double& b) { return (a + b); }, 1);
            }
        }
        {
            double __a = _priv_acc_0;
            double __acc = _priv_acc;
            double __out;

            ///////////////////
            double __lanes__priv_acc_0;
            {
                typedef gpucub::BlockReduce<double, 256> BlockReduceT__priv_acc_0;
                __shared__ typename BlockReduceT__priv_acc_0::TempStorage tmp__priv_acc_0;
                __shared__ double bcast__priv_acc_0;
                double __brtot__priv_acc_0 = BlockReduceT__priv_acc_0(tmp__priv_acc_0).Reduce((__a), dace::_wcr_fixed<dace::ReductionType::Sum, double>());
                if (threadIdx.x == 0) bcast__priv_acc_0 = __brtot__priv_acc_0;
                __syncthreads();
                __lanes__priv_acc_0 = bcast__priv_acc_0;
                __syncthreads();
            }
            __out = dace::_wcr_fixed<dace::ReductionType::Sum, double>()(__acc, __lanes__priv_acc_0);
            ///////////////////

            _priv_acc = __out;
        }

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &_priv_acc, &acc, 1);
        {
            double _in = acc;
            double _out;

            ///////////////////
            // Tasklet code (_assign_acc_to_out)
            _out = _in;
            ///////////////////

            out[_loop_it_0] = _out;
        }

    }
}

DACE_DFI void nested_single_state_body_0_1_8(const int64_t* __restrict__ row_ptr, const double* __restrict__ val, const double* __restrict__ w, double* __restrict__ out, int __tid, int64_t _loop_it_0) {

    {

        loop_body_1_0_0(&row_ptr[0], &val[0], &w[0], &out[0], __tid, _loop_it_0);

    }
}



int __dace_init_cuda(canon_gpu_state_t *__state, int64_t NSEG) {
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

__global__ void  __launch_bounds__(256) single_state_body_map_0_1_4(double * __restrict__ out, const int64_t * __restrict__ row_ptr, const double * __restrict__ val, const double * __restrict__ w, int64_t NSEG) {
    {
        int64_t _loop_it_0 = static_cast<int64_t>(blockIdx.x);
        {
            {
                int __tid = threadIdx.x;
                {
                    nested_single_state_body_0_1_8(&row_ptr[0], &val[0], &w[0], &out[0], __tid, _loop_it_0);
                }
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_single_state_body_map_0_1_4(canon_gpu_state_t *__state, double * __restrict__ out, const int64_t * __restrict__ row_ptr, const double * __restrict__ val, const double * __restrict__ w, int64_t NSEG);
void __dace_runkernel_single_state_body_map_0_1_4(canon_gpu_state_t *__state, double * __restrict__ out, const int64_t * __restrict__ row_ptr, const double * __restrict__ val, const double * __restrict__ w, int64_t NSEG)
{

    void  *single_state_body_map_0_1_4_args[] = { (void *)&out, (void *)&row_ptr, (void *)&val, (void *)&w, (void *)&NSEG };
    gpuError_t __err = hipLaunchKernel((void*)single_state_body_map_0_1_4, dim3(NSEG, 1, 1), dim3(256, 1, 1), single_state_body_map_0_1_4_args, 0, nullptr);
    DACE_KERNEL_LAUNCH_CHECK(__err, "single_state_body_map_0_1_4", NSEG, 1, 1, 256, 1, 1);
}

