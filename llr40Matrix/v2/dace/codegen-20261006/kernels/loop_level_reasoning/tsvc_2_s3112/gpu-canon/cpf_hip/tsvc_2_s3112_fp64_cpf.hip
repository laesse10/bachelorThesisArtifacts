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

//: The backend cub, under the name the code generator writes -- what gpucub.cuh aliases. hipCUB
//: keeps the functor structs CCCL 3 dropped, and a unit built by hipcc is never built against CCCL.
namespace gpucub = hipcub;


// Functions the DaCe runtime headers would otherwise provide.
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
constexpr double __sum = 0.0;

struct tsvc_2_s3112_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

gpuError_t __cpf_scan_tsvc_2_s3112_fp64_0_5_c0(const double* __sc_in, double* __sc_out, double __sc_init, long long __sc_n, gpuStream_t __sc_stream);
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s3112_fp64(const double * __restrict__ a, double * __restrict__ b, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s3112_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s3112_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double _scan_seed_b;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // _assign___sum_to__scan_seed_b
            double _in = __sum;
            double _out;
            _out = _in;
            _scan_seed_b = _out;
        }

    }
    {

        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_19_scan_op_op
            cpf_gpu_check(__cpf_scan_tsvc_2_s3112_fp64_0_5_c0(a, b, _scan_seed_b, (LEN_1D), gpu_streams[gpu_streams_idx(0)]));
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}

struct tsvc_2_s3112_fp64_0_5_c0_seeded {
    const double* in;
    double seed;
    __host__ __device__ __forceinline__ double operator()(long long i) const {
        double v = static_cast<double>(in[i]);
        return i == 0 ? ::gpucub::Sum()(static_cast<double>(seed), v) : v;
    }
};
#if !defined(__HIPCC__)
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#endif
gpuError_t __cpf_scan_tsvc_2_s3112_fp64_0_5_c0(const double* __sc_in, double* __sc_out, double __sc_init, long long __sc_n, gpuStream_t __sc_stream);
gpuError_t __cpf_scan_tsvc_2_s3112_fp64_0_5_c0(const double* __sc_in, double* __sc_out, double __sc_init, long long __sc_n, gpuStream_t __sc_stream) {
    #if defined(__HIPCC__)
    ::gpucub::TransformInputIterator<double, tsvc_2_s3112_fp64_0_5_c0_seeded, ::gpucub::CountingInputIterator<long long>> __sc_items(::gpucub::CountingInputIterator<long long>(0), tsvc_2_s3112_fp64_0_5_c0_seeded{__sc_in, __sc_init});
    #else
    thrust::transform_iterator<tsvc_2_s3112_fp64_0_5_c0_seeded, thrust::counting_iterator<long long>, double> __sc_items(thrust::counting_iterator<long long>(0), tsvc_2_s3112_fp64_0_5_c0_seeded{__sc_in, __sc_init});
    #endif
    size_t _sc_needed = 0;
    gpuError_t _sc_status = ::gpucub::DeviceScan::InclusiveScan(nullptr, _sc_needed, __sc_items, __sc_out, ::gpucub::Sum(), __sc_n, __sc_stream);
    if (_sc_status != gpuSuccess) return _sc_status;
    void* _sc_scratch = get_scratch<ScanTag>(_sc_needed, __sc_stream, &_sc_status);
    if (_sc_scratch == nullptr) return _sc_status != gpuSuccess ? _sc_status : gpuErrorMemoryAllocation;
    return ::gpucub::DeviceScan::InclusiveScan(_sc_scratch, _sc_needed, __sc_items, __sc_out, ::gpucub::Sum(), __sc_n, __sc_stream);
}

