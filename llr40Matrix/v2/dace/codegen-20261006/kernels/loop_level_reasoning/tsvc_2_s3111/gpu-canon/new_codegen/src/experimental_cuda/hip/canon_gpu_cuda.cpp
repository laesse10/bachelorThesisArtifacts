
#include <hip/hip_runtime.h>
#include <dace/dace.h>

constexpr double _priv_sum_val = 0.0;

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
};



DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _priv_sum_val_gpu_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }


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

__global__ void __launch_bounds__(512) canon_gpu_single_state_body_map_0_2_17(double * __restrict__ _priv_sum_val_gpu, const double * __restrict__ a, int64_t LEN_1D) {
    int64_t b__loop_it_0 = (512 * static_cast<int64_t>(blockIdx.x));
    {
        int64_t _loop_it_0 = (threadIdx.x + b__loop_it_0);
        double sum_val_plus_a_slice;
        double sum_val_masked_val;
        double _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out;
        double __bpart_2_2_0[1];
        for (int __bi = 0; __bi < 1; ++__bi) __bpart_2_2_0[__bi] = double(0.0);
        if (_loop_it_0 >= b__loop_it_0 && _loop_it_0 < (Min((LEN_1D - 1), (b__loop_it_0 + 511)) + 1)) {
            sum_val_masked_val = ((a[a_idx(_loop_it_0)] > 0.0) ? a[a_idx(_loop_it_0)] : 0.0);  // sum_val_mask
            sum_val_plus_a_slice = sum_val_masked_val;  // _Add_
            _wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out = sum_val_plus_a_slice;  // _assign_out_sum_val_plus_a_slice_to__priv_sum_val
            __bpart_2_2_0[0] = dace::_wcr_fixed<dace::ReductionType::Sum, double>()(__bpart_2_2_0[0], *(&_wcr_priv__assign_out_sum_val_plus_a_slice_to__priv_sum_val__out));
        }
        {
            typedef gpucub::BlockReduce<double, 512, gpucub::BLOCK_REDUCE_WARP_REDUCTIONS, 1, 1> __brt_single_state_body_map_0;
            __shared__ typename __brt_single_state_body_map_0::TempStorage __brs_single_state_body_map_0;
            for (int __bk_single_state_body_map_0 = 0; __bk_single_state_body_map_0 < 1; ++__bk_single_state_body_map_0) {
                double __bres_single_state_body_map_0 = __brt_single_state_body_map_0(__brs_single_state_body_map_0).Reduce(__bpart_2_2_0[__bk_single_state_body_map_0], dace::_wcr_fixed<dace::ReductionType::Sum, double>());
                if (threadIdx.x == 0 && threadIdx.y == 0 && threadIdx.z == 0) {
                    dace::_wcr_fixed<dace::ReductionType::Sum, double>::reduce_atomic(_priv_sum_val_gpu + ((0) + __bk_single_state_body_map_0), __bres_single_state_body_map_0);
                }
                __syncthreads();
            }
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_0_2_17(canon_gpu_state_t *__state, double * __restrict__ _priv_sum_val_gpu, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_single_state_body_map_0_2_17(canon_gpu_state_t *__state, double * __restrict__ _priv_sum_val_gpu, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 512)) <= 0)) {

        return;
    }

    void  *canon_gpu_single_state_body_map_0_2_17_args[] = { (void *)&_priv_sum_val_gpu, (void *)&a, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_single_state_body_map_0_2_17, dim3(int_ceil(LEN_1D, 512), 1, 1), dim3(512, 1, 1), canon_gpu_single_state_body_map_0_2_17_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_single_state_body_map_0_2_17", int_ceil(LEN_1D, 512), 1, 1, 512, 1, 1);
}
__global__ void __launch_bounds__(256) canon_gpu_size1_wrap_region_0_2_19(const double * __restrict__ _priv_sum_val_gpu, double * __restrict__ b) {
    int b___wrap_i = (256 * blockIdx.x);
    {
        int __wrap_i = (threadIdx.x + b___wrap_i);
        if (__wrap_i >= b___wrap_i && __wrap_i < (Min(0, (b___wrap_i + 255)) + 1)) {
            b[b_idx(0)] = _priv_sum_val_gpu[_priv_sum_val_gpu_idx(0)];  // _assign_sum_val_0_to_b
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_size1_wrap_region_0_2_19(canon_gpu_state_t *__state, const double * __restrict__ _priv_sum_val_gpu, double * __restrict__ b, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_size1_wrap_region_0_2_19(canon_gpu_state_t *__state, const double * __restrict__ _priv_sum_val_gpu, double * __restrict__ b, gpuStream_t gpu_stream)

{

    void  *canon_gpu_size1_wrap_region_0_2_19_args[] = { (void *)&_priv_sum_val_gpu, (void *)&b };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_size1_wrap_region_0_2_19, dim3(1, 1, 1), dim3(256, 1, 1), canon_gpu_size1_wrap_region_0_2_19_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_size1_wrap_region_0_2_19", 1, 1, 1, 256, 1, 1);
}

