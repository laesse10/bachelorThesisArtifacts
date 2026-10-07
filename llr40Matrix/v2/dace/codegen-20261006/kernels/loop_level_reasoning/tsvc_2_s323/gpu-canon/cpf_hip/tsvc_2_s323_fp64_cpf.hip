// Rendered by DaCe CPF (canonical parallel form): self-contained, no DaCe runtime.
// Already parallelized, with basic heuristics applied. Every loop names its class first:
//   parallel, sequential -- settled; their parallelism needs no further reasoning.
//   unsure               -- open; the only loops whose parallelism is worth reasoning about.
// Spend the effort on heuristic optimizations and restructuring.
#include <cstdint>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <complex>
#include <numeric>
#include <new>
#include <type_traits>
#include <hip/hip_runtime.h>
#include <hipcub/hipcub.hpp>
#include <utility>

// What dace/dace.h would define for the device side.
using gpuStream_t = hipStream_t;
using gpuEvent_t = hipEvent_t;
using gpuError_t = hipError_t;
static constexpr gpuError_t gpuSuccess = hipSuccess;
static constexpr gpuError_t gpuErrorMemoryAllocation = hipErrorOutOfMemory;

//: A failed GPU call ends the program, naming the call site.
static inline void cpf_gpu_check(gpuError_t status, const char *file = __builtin_FILE(), int line = __builtin_LINE()) {
    if (status != gpuSuccess) {
        fprintf(stderr, "%s:%d: GPU error %d (%s)\n", file, line, (int)status, hipGetErrorString(status));
        abort();
    }
}

//: The one stream canon offloads onto, in the shape the generated frame indexes.
struct cpf_gpu_context {
    gpuStream_t streams[1];
    gpuEvent_t events[1];
    gpuError_t lasterror;
};

//: A failed kernel launch ends the program, naming the kernel and its launch geometry.
static inline void cpf_kernel_launch_check(gpuError_t err, const char *name, long long gx, long long gy, long long gz,
                                           long long bx, long long by, long long bz) {
    if (err != gpuSuccess) {
        fprintf(stderr, "%s launch failed (grid %lld,%lld,%lld block %lld,%lld,%lld): %s\n", name, gx, gy, gz, bx, by,
                bz, hipGetErrorString(err));
        abort();
    }
}

//: The backend cub, under the name the code generator writes -- what gpucub.cuh aliases. hipCUB
//: keeps the functor structs CCCL 3 dropped, and a unit built by hipcc is never built against CCCL.
namespace gpucub = hipcub;


// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline T cpf_min(const T& value) {
    return value;
}
template <typename T, typename... Ts>
static constexpr inline typename std::common_type<T, Ts...>::type cpf_min(const T& a, const Ts&... rest) {
    return (cpf_min(rest...) < a) ? cpf_min(rest...) : a;
}
//: The CUB scratch pool, as much of it as one entry point can hold: one buffer per tag,
//: allocated on first use and grown, never shrunk, so a repeated call pays no allocation.
//: The runtime pre-allocates and releases it from the entry points a repeated invocation
//: shares; one self-contained call has neither, so the buffer owns itself and frees at
//: static destruction. It is also not keyed by stream, as the runtime pool is: a CPF unit
//: issues every launch on the one stream (see cpf_gpu_context), so its calls are ordered.
struct ScanTag {};
struct DetectFlagTag {};
struct cpf_scratch_block {
    void *ptr = nullptr;
    size_t capacity = 0;
    ~cpf_scratch_block() { if (ptr != nullptr) { (void)hipFree(ptr); } }
};
template <typename Tag>
static inline void *get_scratch(size_t bytes, gpuStream_t stream, gpuError_t *status) {
    static cpf_scratch_block block;
    *status = gpuSuccess;
    if (bytes <= block.capacity) return block.ptr;
    if (block.ptr != nullptr) {
        // In flight work may still be reading the old buffer.
        *status = hipStreamSynchronize(stream);
        if (*status != gpuSuccess) return nullptr;
        *status = hipFree(block.ptr);
        block.ptr = nullptr;
        block.capacity = 0;
        if (*status != gpuSuccess) return nullptr;
    }
    *status = hipMalloc(&block.ptr, bytes);
    if (*status != gpuSuccess) { block.ptr = nullptr; return nullptr; }
    block.capacity = bytes;
    return block.ptr;
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R int_ceil(const T& numerator, const U& denominator) {
    return (numerator + denominator - 1) / denominator;
}

struct tsvc_2_s323_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

gpuError_t __cpf_scan_tsvc_2_s323_fp64_1_14_c0(const double* __sc_in, double* __sc_out, const double* __sc_init, long long __sc_n, gpuStream_t __sc_stream);
void __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_map_0_1_33(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s323_fp64_size1_wrap_region_0_1_37(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ _scan_seed_b, const double * __restrict__ b, gpuStream_t gpu_stream);
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t e_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _scan_in_b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t _scan_seed_b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
void __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_0_map_0_1_35(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s323_fp64(double * __restrict__ a, double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s323_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s323_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double * _scan_in_b;
    cpf_gpu_check(hipMalloc((void**)&_scan_in_b, (LEN_1D - 1) * sizeof(double)));
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    {
        double * _scan_seed_b;
        cpf_gpu_check(hipMalloc((void**)&_scan_seed_b, 1 * sizeof(double)));

        {
            gpuStream_t __cpf_current_stream = gpu_streams[0];
            __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_map_0_1_33(__state, _scan_in_b, c, d, e, LEN_1D, __cpf_current_stream);
        }
        {
            gpuStream_t __cpf_current_stream = gpu_streams[0];
            __cpf_runkernel_tsvc_2_s323_fp64_size1_wrap_region_0_1_37(__state, _scan_seed_b, b, __cpf_current_stream);
        }
        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_16_0_scan_op
            cpf_gpu_check(__cpf_scan_tsvc_2_s323_fp64_1_14_c0(_scan_in_b, (b + 1), _scan_seed_b, ((LEN_1D - 1)), gpu_streams[gpu_streams_idx(0)]));
        }
        {
            gpuStream_t __cpf_current_stream = gpu_streams[0];
            __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_0_map_0_1_35(__state, a, b, c, d, LEN_1D, __cpf_current_stream);
        }
        cpf_gpu_check(hipFree(_scan_seed_b));

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    cpf_gpu_check(hipFree(_scan_in_b));
    gpu_streams = nullptr;
}

struct tsvc_2_s323_fp64_1_14_c0_seeded {
    const double* in;
    const double* seed;
    __host__ __device__ __forceinline__ double operator()(long long i) const {
        double v = static_cast<double>(in[i]);
        return i == 0 ? ::gpucub::Sum()(static_cast<double>(*seed), v) : v;
    }
};
#if !defined(__HIPCC__)
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#endif
gpuError_t __cpf_scan_tsvc_2_s323_fp64_1_14_c0(const double* __sc_in, double* __sc_out, const double* __sc_init, long long __sc_n, gpuStream_t __sc_stream);
gpuError_t __cpf_scan_tsvc_2_s323_fp64_1_14_c0(const double* __sc_in, double* __sc_out, const double* __sc_init, long long __sc_n, gpuStream_t __sc_stream) {
    #if defined(__HIPCC__)
    ::gpucub::TransformInputIterator<double, tsvc_2_s323_fp64_1_14_c0_seeded, ::gpucub::CountingInputIterator<long long>> __sc_items(::gpucub::CountingInputIterator<long long>(0), tsvc_2_s323_fp64_1_14_c0_seeded{__sc_in, __sc_init});
    #else
    thrust::transform_iterator<tsvc_2_s323_fp64_1_14_c0_seeded, thrust::counting_iterator<long long>, double> __sc_items(thrust::counting_iterator<long long>(0), tsvc_2_s323_fp64_1_14_c0_seeded{__sc_in, __sc_init});
    #endif
    size_t _sc_needed = 0;
    gpuError_t _sc_status = ::gpucub::DeviceScan::InclusiveScan(nullptr, _sc_needed, __sc_items, __sc_out, ::gpucub::Sum(), __sc_n, __sc_stream);
    if (_sc_status != gpuSuccess) return _sc_status;
    void* _sc_scratch = get_scratch<ScanTag>(_sc_needed, __sc_stream, &_sc_status);
    if (_sc_scratch == nullptr) return _sc_status != gpuSuccess ? _sc_status : gpuErrorMemoryAllocation;
    return ::gpucub::DeviceScan::InclusiveScan(_sc_scratch, _sc_needed, __sc_items, __sc_out, ::gpucub::Sum(), __sc_n, __sc_stream);
}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) tsvc_2_s323_fp64_single_state_body_map_0_1_33(double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D) {
    int64_t b__loop_it_1 = ((256 * static_cast<int64_t>(blockIdx.x)) + 1);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _loop_it_1 = (threadIdx.x + b__loop_it_1);
        double c_slice_times_d_slice;
        double c_slice_times_e_slice;
        double b_slice;
        double a_fwd;
        if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (cpf_min((LEN_1D - 1), (b__loop_it_1 + 255)) + 1)) {
            c_slice_times_d_slice = (c[c_idx(_loop_it_1)] * d[d_idx(_loop_it_1)]);  // _Mult_
            a_fwd = c_slice_times_d_slice;  // _assign_c_slice_times_d_slice_to_a_fwd
            c_slice_times_e_slice = (c[c_idx(_loop_it_1)] * e[e_idx(_loop_it_1)]);  // _Mult_
            b_slice = (a_fwd + c_slice_times_e_slice);  // _Add_
            _scan_in_b[_scan_in_b_idx((_loop_it_1 - 1))] = b_slice;  // _assign_out_b_slice_to__scan_in_b
        }
    }
}


void __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_map_0_1_33(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_map_0_1_33(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ _scan_in_b, const double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_1D - 1), 256)) <= 0)) {

        return;
    }

    void  *tsvc_2_s323_fp64_single_state_body_map_0_1_33_args[] = { (void *)&_scan_in_b, (void *)&c, (void *)&d, (void *)&e, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s323_fp64_single_state_body_map_0_1_33, dim3(int_ceil((LEN_1D - 1), 256), 1, 1), dim3(256, 1, 1), tsvc_2_s323_fp64_single_state_body_map_0_1_33_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s323_fp64_single_state_body_map_0_1_33", int_ceil((LEN_1D - 1), 256), 1, 1, 256, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) tsvc_2_s323_fp64_size1_wrap_region_0_1_37(double * __restrict__ _scan_seed_b, const double * __restrict__ b) {
    int b___wrap_i = (256 * blockIdx.x);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int __wrap_i = (threadIdx.x + b___wrap_i);
        if (__wrap_i >= b___wrap_i && __wrap_i < (cpf_min(0, (b___wrap_i + 255)) + 1)) {
            _scan_seed_b[_scan_seed_b_idx(0)] = b[b_idx(0)];  // _assign_b_to__scan_seed_b
        }
    }
}


void __cpf_runkernel_tsvc_2_s323_fp64_size1_wrap_region_0_1_37(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ _scan_seed_b, const double * __restrict__ b, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s323_fp64_size1_wrap_region_0_1_37(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ _scan_seed_b, const double * __restrict__ b, gpuStream_t gpu_stream)

{

    void  *tsvc_2_s323_fp64_size1_wrap_region_0_1_37_args[] = { (void *)&_scan_seed_b, (void *)&b };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s323_fp64_size1_wrap_region_0_1_37, dim3(1, 1, 1), dim3(256, 1, 1), tsvc_2_s323_fp64_size1_wrap_region_0_1_37_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s323_fp64_size1_wrap_region_0_1_37", 1, 1, 1, 256, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) tsvc_2_s323_fp64_single_state_body_0_map_0_1_35(double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D) {
    int64_t b__loop_it_2 = ((256 * static_cast<int64_t>(blockIdx.x)) + 1);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _loop_it_2 = (threadIdx.x + b__loop_it_2);
        double nested_sdfg_c_slice_times_d_slice;
        if (_loop_it_2 >= b__loop_it_2 && _loop_it_2 < (cpf_min((LEN_1D - 1), (b__loop_it_2 + 255)) + 1)) {
            nested_sdfg_c_slice_times_d_slice = (c[c_idx(_loop_it_2)] * d[d_idx(_loop_it_2)]);  // _Mult_
            a[a_idx(_loop_it_2)] = (b[b_idx((_loop_it_2 - 1))] + nested_sdfg_c_slice_times_d_slice);  // _Add_
        }
    }
}


void __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_0_map_0_1_35(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s323_fp64_single_state_body_0_map_0_1_35(tsvc_2_s323_fp64_state_t *__state, double * __restrict__ a, const double * __restrict__ b, const double * __restrict__ c, const double * __restrict__ d, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_1D - 1), 256)) <= 0)) {

        return;
    }

    void  *tsvc_2_s323_fp64_single_state_body_0_map_0_1_35_args[] = { (void *)&a, (void *)&b, (void *)&c, (void *)&d, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s323_fp64_single_state_body_0_map_0_1_35, dim3(int_ceil((LEN_1D - 1), 256), 1, 1), dim3(256, 1, 1), tsvc_2_s323_fp64_single_state_body_0_map_0_1_35_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s323_fp64_single_state_body_0_map_0_1_35", int_ceil((LEN_1D - 1), 256), 1, 1, 256, 1, 1);
}