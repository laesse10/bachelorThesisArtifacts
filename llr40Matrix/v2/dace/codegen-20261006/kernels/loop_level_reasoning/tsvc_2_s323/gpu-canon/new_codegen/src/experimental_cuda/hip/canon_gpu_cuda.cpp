
#include <hip/hip_runtime.h>
#include <dace/dace.h>

#include "dace/cuda/gpucub.cuh"
#include "dace/cub_scratch.cuh"
#include "dace/cub_compat.cuh"
#include "dace/cuda/scan_affine.cuh"
#include "dace/cuda/scan.cuh"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0__scan_in_b;
};

struct canon_gpu_1_14_c0_seeded {
    const double* in;
    const double* seed;
    __host__ __device__ __forceinline__ double operator()(long long i) const {
        double v = static_cast<double>(in[i]);
        return i == 0 ? DACE_CUB_SUM_OP(static_cast<double>(*seed), v) : v;
    }
};
#if !defined(__HIPCC__)
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#endif
DACE_EXPORTED gpuError_t __dace_scan_canon_gpu_1_14_c0(const double* __sc_in, double* __sc_out, const double* __sc_init, long long __sc_n, gpuStream_t __sc_stream);
gpuError_t __dace_scan_canon_gpu_1_14_c0(const double* __sc_in, double* __sc_out, const double* __sc_init, long long __sc_n, gpuStream_t __sc_stream) {
    #if defined(__HIPCC__)
    ::gpucub::TransformInputIterator<double, canon_gpu_1_14_c0_seeded, ::gpucub::CountingInputIterator<long long>> __sc_items(::gpucub::CountingInputIterator<long long>(0), canon_gpu_1_14_c0_seeded{__sc_in, __sc_init});
    #else
    thrust::transform_iterator<canon_gpu_1_14_c0_seeded, thrust::counting_iterator<long long>, double> __sc_items(thrust::counting_iterator<long long>(0), canon_gpu_1_14_c0_seeded{__sc_in, __sc_init});
    #endif
    size_t _sc_needed = 0;
    gpuError_t _sc_status = ::gpucub::DeviceScan::InclusiveScan(nullptr, _sc_needed, __sc_items, __sc_out, DACE_CUB_SUM_OP, __sc_n, __sc_stream);
    if (_sc_status != gpuSuccess) return _sc_status;
    void* _sc_scratch = ::dace::cub::get_scratch<::dace::cub::ScanTag>(_sc_needed, __sc_stream, &_sc_status);
    if (_sc_scratch == nullptr) return _sc_status != gpuSuccess ? _sc_status : gpuErrorMemoryAllocation;
    return ::gpucub::DeviceScan::InclusiveScan(_sc_scratch, _sc_needed, __sc_items, __sc_out, DACE_CUB_SUM_OP, __sc_n, __sc_stream);
}


DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);
DACE_EXPORTED int __dace_gpu_last_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED bool __dace_gpu_set_stream(canon_gpu_state_t *__state, int streamid, gpuStream_t stream);
DACE_EXPORTED void __dace_gpu_set_all_streams(canon_gpu_state_t *__state, gpuStream_t stream);

static DACE_HDFI constexpr int64_t c_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t d_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t e_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_in_b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t _scan_seed_b_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
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

__global__ void __launch_bounds__(256) canon_gpu_single_state_body_map_0_1_33(double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D) {
    int64_t b__loop_it_1 = ((256 * static_cast<int64_t>(blockIdx.x)) + 1);
    {
        int64_t _loop_it_1 = (threadIdx.x + b__loop_it_1);
        double c_slice_times_d_slice;
        double c_slice_times_e_slice;
        double b_slice;
        double a_fwd;
        if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (Min((LEN_1D - 1), (b__loop_it_1 + 255)) + 1)) {
            c_slice_times_d_slice = (c[c_idx(_loop_it_1)] * d[d_idx(_loop_it_1)]);  // _Mult_
            a_fwd = c_slice_times_d_slice;  // _assign_c_slice_times_d_slice_to_a_fwd
            c_slice_times_e_slice = (c[c_idx(_loop_it_1)] * e[e_idx(_loop_it_1)]);  // _Mult_
            b_slice = (a_fwd + c_slice_times_e_slice);  // _Add_
            _scan_in_b[_scan_in_b_idx((_loop_it_1 - 1))] = b_slice;  // _assign_out_b_slice_to__scan_in_b
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_map_0_1_33(canon_gpu_state_t *__state, double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_single_state_body_map_0_1_33(canon_gpu_state_t *__state, double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_1D - 1), 256)) <= 0)) {

        return;
    }

    void  *canon_gpu_single_state_body_map_0_1_33_args[] = { (void *)&_scan_in_b, (void *)&c, (void *)&d, (void *)&e, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_single_state_body_map_0_1_33, dim3(int_ceil((LEN_1D - 1), 256), 1, 1), dim3(256, 1, 1), canon_gpu_single_state_body_map_0_1_33_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_single_state_body_map_0_1_33", int_ceil((LEN_1D - 1), 256), 1, 1, 256, 1, 1);
}
__global__ void __launch_bounds__(256) canon_gpu_size1_wrap_region_0_1_37(double * __restrict__ _scan_seed_b, const double * __restrict__ b) {
    int b___wrap_i = (256 * blockIdx.x);
    {
        int __wrap_i = (threadIdx.x + b___wrap_i);
        if (__wrap_i >= b___wrap_i && __wrap_i < (Min(0, (b___wrap_i + 255)) + 1)) {
            _scan_seed_b[_scan_seed_b_idx(0)] = b[b_idx(0)];  // _assign_b_to__scan_seed_b
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_size1_wrap_region_0_1_37(canon_gpu_state_t *__state, double * __restrict__ _scan_seed_b, const double * __restrict__ b, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_size1_wrap_region_0_1_37(canon_gpu_state_t *__state, double * __restrict__ _scan_seed_b, const double * __restrict__ b, gpuStream_t gpu_stream)

{

    void  *canon_gpu_size1_wrap_region_0_1_37_args[] = { (void *)&_scan_seed_b, (void *)&b };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_size1_wrap_region_0_1_37, dim3(1, 1, 1), dim3(256, 1, 1), canon_gpu_size1_wrap_region_0_1_37_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_size1_wrap_region_0_1_37", 1, 1, 1, 256, 1, 1);
}
__global__ void __launch_bounds__(256) canon_gpu_single_state_body_0_map_0_1_35(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D) {
    int64_t b__loop_it_2 = ((256 * static_cast<int64_t>(blockIdx.x)) + 1);
    {
        int64_t _loop_it_2 = (threadIdx.x + b__loop_it_2);
        double nested_sdfg_c_slice_times_d_slice;
        if (_loop_it_2 >= b__loop_it_2 && _loop_it_2 < (Min((LEN_1D - 1), (b__loop_it_2 + 255)) + 1)) {
            nested_sdfg_c_slice_times_d_slice = (c[c_idx(_loop_it_2)] * d[d_idx(_loop_it_2)]);  // _Mult_
            a[a_idx(_loop_it_2)] = (b[b_idx((_loop_it_2 - 1))] + nested_sdfg_c_slice_times_d_slice);  // _Add_
        }
    }
}


DACE_EXPORTED void __dace_runkernel_canon_gpu_single_state_body_0_map_0_1_35(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D, gpuStream_t gpu_stream);
void __dace_runkernel_canon_gpu_single_state_body_0_map_0_1_35(canon_gpu_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_1D - 1), 256)) <= 0)) {

        return;
    }

    void  *canon_gpu_single_state_body_0_map_0_1_35_args[] = { (void *)&a, (void *)&b, (void *)&c, (void *)&d, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)canon_gpu_single_state_body_0_map_0_1_35, dim3(int_ceil((LEN_1D - 1), 256), 1, 1), dim3(256, 1, 1), canon_gpu_single_state_body_0_map_0_1_35_args, 0, gpu_stream);

    DACE_KERNEL_LAUNCH_CHECK(__err, "canon_gpu_single_state_body_0_map_0_1_35", int_ceil((LEN_1D - 1), 256), 1, 1, 256, 1, 1);
}

