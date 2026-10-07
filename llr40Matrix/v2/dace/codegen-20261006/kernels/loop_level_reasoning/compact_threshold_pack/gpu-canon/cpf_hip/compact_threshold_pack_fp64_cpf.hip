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

//: A conflicting accumulation, applied atomically under any binary operator.
//:
//: HIP spells only a few operator/type pairs as an intrinsic (atomicAdd on the arithmetic types,
//: atomicMin/atomicMax on the integers), and an SDFG's write-conflict resolution is any of nine
//: reductions over any element type, so one CAS loop covers the set where a table of intrinsics
//: would leave holes. On a tree-reduced accumulator this runs ONCE PER BLOCK, after
//: gpucub::BlockReduce has folded the block's partials, so the loop is off the per-element path.
template <typename T, typename V, typename Op>
__device__ inline void cpf_gpu_atomic(T *address, V value, Op op) {
    using cpf_atomic_word =
        typename std::conditional<sizeof(T) == sizeof(unsigned int), unsigned int, unsigned long long>::type;
    static_assert(sizeof(T) == sizeof(cpf_atomic_word), "no atomic word as wide as the accumulator");
    cpf_atomic_word *word = reinterpret_cast<cpf_atomic_word *>(address);
    cpf_atomic_word old = *word;
    cpf_atomic_word assumed;
    do {
        assumed = old;
        T updated = op(__builtin_bit_cast(T, assumed), static_cast<T>(value));
        old = atomicCAS(word, assumed, __builtin_bit_cast(cpf_atomic_word, updated));
    } while (assumed != old);
}


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

struct compact_threshold_pack_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

gpuError_t __cpf_scan_compact_threshold_pack_fp64_1_5_c0(const int8_t* __sc_in, int64_t* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
void __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_map_0_3_7(compact_threshold_pack_fp64_state_t *__state, int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_compact_threshold_pack_fp64_reduce_init_map_0_5_6(compact_threshold_pack_fp64_state_t *__state, int64_t * __restrict__ compaction_total_for_17, gpuStream_t gpu_stream);
void __cpf_runkernel_compact_threshold_pack_fp64_reduce_values_0_6_7(compact_threshold_pack_fp64_state_t *__state, const int8_t * __restrict__ compaction_mask_for_17, int64_t * __restrict__ compaction_total_for_17, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t compaction_mask_for_17_idx(int64_t __d0) { return __d0; }
void __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_0_map_0_4_16(compact_threshold_pack_fp64_state_t *__state, const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t src_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t weight_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t packed_idx(int64_t __d0) { return __d0; }
extern "C" void compact_threshold_pack_fp64(int64_t * __restrict__ out_count, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    compact_threshold_pack_fp64_state_t __cpf_state{&__cpf_context};
    compact_threshold_pack_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    int8_t * compaction_mask_for_17;
    cpf_gpu_check(hipMalloc((void**)&compaction_mask_for_17, LEN_1D * sizeof(int8_t)));
    int64_t * compaction_rank_for_17;
    cpf_gpu_check(hipMalloc((void**)&compaction_rank_for_17, LEN_1D * sizeof(int64_t)));
    int64_t scal_out_count;
    int64_t compaction_total_for_17_host;
    int64_t * compaction_total_for_17;
    cpf_gpu_check(hipMalloc((void**)&compaction_total_for_17, 1 * sizeof(int64_t)));
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    int64_t n;

    {

        {  // copy_out_count_to_scal_out_count
            hipMemcpyAsync((&scal_out_count), out_count, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_map_0_3_7(__state, compaction_mask_for_17, src, LEN_1D, __cpf_current_stream);
    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_compact_threshold_pack_fp64_reduce_init_map_0_5_6(__state, compaction_total_for_17, __cpf_current_stream);
    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_compact_threshold_pack_fp64_reduce_values_0_6_7(__state, compaction_mask_for_17, compaction_total_for_17, LEN_1D, __cpf_current_stream);
    }
    {

        {  // copy_compaction_total_for_17_to_compaction_total_for_17_host
            hipMemcpyAsync((&compaction_total_for_17_host), compaction_total_for_17, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }
        // scan: running (prefix) fold along an axis
        // parallel scan; canonicalization takes the parallel form.
        // Alternative: a sequential loop over parallel maps.
        // CPU: the loop is worth trying -- the scan does more work, and the loop may already saturate the memory system.
        // GPU: the scan is usually the better of the two.
        // Both are correct. Measure before choosing.
        {  // for_17_compaction_scan_scan
            cpf_gpu_check(__cpf_scan_compact_threshold_pack_fp64_1_5_c0(compaction_mask_for_17, compaction_rank_for_17, (LEN_1D), gpu_streams[gpu_streams_idx(0)]));
        }
        {
            gpuStream_t __cpf_current_stream = gpu_streams[0];
            __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_0_map_0_4_16(__state, compaction_rank_for_17, packed, src, weight, LEN_1D, __cpf_current_stream);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    n = compaction_total_for_17_host;
    {

        scal_out_count = n;  // symassign

    }
    {

        {  // copy_scal_out_count_to_out_count
            hipMemcpyAsync(out_count, (&scal_out_count), 1 * sizeof(int64_t), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    cpf_gpu_check(hipFree(compaction_mask_for_17));
    cpf_gpu_check(hipFree(compaction_rank_for_17));
    cpf_gpu_check(hipFree(compaction_total_for_17));
    gpu_streams = nullptr;
}

gpuError_t __cpf_scan_compact_threshold_pack_fp64_1_5_c0(const int8_t* __sc_in, int64_t* __sc_out, long long __sc_n, gpuStream_t __sc_stream);
gpuError_t __cpf_scan_compact_threshold_pack_fp64_1_5_c0(const int8_t* __sc_in, int64_t* __sc_out, long long __sc_n, gpuStream_t __sc_stream) {
    size_t _sc_needed = 0;
    gpuError_t _sc_status = ::gpucub::DeviceScan::ExclusiveScan(nullptr, _sc_needed, __sc_in, __sc_out, ::gpucub::Sum(), static_cast<int64_t>(0), __sc_n, __sc_stream);
    if (_sc_status != gpuSuccess) return _sc_status;
    void* _sc_scratch = get_scratch<ScanTag>(_sc_needed, __sc_stream, &_sc_status);
    if (_sc_scratch == nullptr) return _sc_status != gpuSuccess ? _sc_status : gpuErrorMemoryAllocation;
    return ::gpucub::DeviceScan::ExclusiveScan(_sc_scratch, _sc_needed, __sc_in, __sc_out, ::gpucub::Sum(), static_cast<int64_t>(0), __sc_n, __sc_stream);
}

static inline __device__ __forceinline__ void loop_body_0_3_2(const double* __restrict__ src, int8_t * __restrict__ compaction_mask_for_17, int64_t compaction_it_for_17_compaction_mask) {
    double src_index;


    src_index = src[compaction_it_for_17_compaction_mask];
    {

        compaction_mask_for_17[compaction_mask_for_17_idx(compaction_it_for_17_compaction_mask)] = static_cast<int8_t>((src_index > 0.0));  // for_17_compaction_mask_mask

    }
}

static inline __device__ __forceinline__ void loop_body_0_4_1(const int64_t * __restrict__ compaction_rank_for_17, const double* __restrict__ src, const double* __restrict__ weight, double* __restrict__ packed, int64_t compaction_it_for_17_compaction_scatter) {
    double src_index;
    int64_t n_0;


    src_index = src[compaction_it_for_17_compaction_scatter];
    n_0 = compaction_rank_for_17[compaction_it_for_17_compaction_scatter];
    if ((src_index > 0.0)) {
        {

            packed[packed_idx(n_0)] = (src[src_idx(compaction_it_for_17_compaction_scatter)] * weight[weight_idx(compaction_it_for_17_compaction_scatter)]);  // _Mult_

        }
    }

}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) compact_threshold_pack_fp64_single_state_body_map_0_3_7(int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D) {
    int64_t b_compaction_it_for_17_compaction_mask = (256 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t compaction_it_for_17_compaction_mask = (threadIdx.x + b_compaction_it_for_17_compaction_mask);
        if (compaction_it_for_17_compaction_mask >= b_compaction_it_for_17_compaction_mask && compaction_it_for_17_compaction_mask < (cpf_min((LEN_1D - 1), (b_compaction_it_for_17_compaction_mask + 255)) + 1)) {
            loop_body_0_3_2(&src[0], &compaction_mask_for_17[0], compaction_it_for_17_compaction_mask);
        }
    }
}


void __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_map_0_3_7(compact_threshold_pack_fp64_state_t *__state, int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_map_0_3_7(compact_threshold_pack_fp64_state_t *__state, int8_t * __restrict__ compaction_mask_for_17, const double * __restrict__ src, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 256)) <= 0)) {

        return;
    }

    void  *compact_threshold_pack_fp64_single_state_body_map_0_3_7_args[] = { (void *)&compaction_mask_for_17, (void *)&src, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)compact_threshold_pack_fp64_single_state_body_map_0_3_7, dim3(int_ceil(LEN_1D, 256), 1, 1), dim3(256, 1, 1), compact_threshold_pack_fp64_single_state_body_map_0_3_7_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "compact_threshold_pack_fp64_single_state_body_map_0_3_7", int_ceil(LEN_1D, 256), 1, 1, 256, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(128) compact_threshold_pack_fp64_reduce_init_map_0_5_6(int64_t * __restrict__ compaction_total_for_17) {
    int b__o0 = (128 * blockIdx.x);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int _o0 = (threadIdx.x + b__o0);
        if (_o0 >= b__o0 && _o0 < (cpf_min(0, (b__o0 + 127)) + 1)) {
            // reduction over the given axes with the given operator
            {  // reduce_init
                int64_t __out;
                __out = 0;
                compaction_total_for_17[0] = __out;
            }
        }
    }
}


void __cpf_runkernel_compact_threshold_pack_fp64_reduce_init_map_0_5_6(compact_threshold_pack_fp64_state_t *__state, int64_t * __restrict__ compaction_total_for_17, gpuStream_t gpu_stream);
void __cpf_runkernel_compact_threshold_pack_fp64_reduce_init_map_0_5_6(compact_threshold_pack_fp64_state_t *__state, int64_t * __restrict__ compaction_total_for_17, gpuStream_t gpu_stream)

{

    void  *compact_threshold_pack_fp64_reduce_init_map_0_5_6_args[] = { (void *)&compaction_total_for_17 };
    gpuError_t __err = hipLaunchKernel((void*)compact_threshold_pack_fp64_reduce_init_map_0_5_6, dim3(1, 1, 1), dim3(128, 1, 1), compact_threshold_pack_fp64_reduce_init_map_0_5_6_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "compact_threshold_pack_fp64_reduce_init_map_0_5_6", 1, 1, 1, 128, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(128) compact_threshold_pack_fp64_reduce_values_0_6_7(const int8_t * __restrict__ compaction_mask_for_17, int64_t * __restrict__ compaction_total_for_17, int64_t LEN_1D) {
    int64_t b__i0 = (128 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _i0 = (threadIdx.x + b__i0);
        int64_t __bpart_6_0_0[1];
        for (int __bi = 0; __bi < 1; ++__bi) __bpart_6_0_0[__bi] = int64_t(0);
        if (_i0 >= b__i0 && _i0 < (cpf_min((LEN_1D - 1), (b__i0 + 127)) + 1)) {
            {  // identity
                int64_t __out;
                __out = compaction_mask_for_17[compaction_mask_for_17_idx(_i0)];
                __bpart_6_0_0[0] = __bpart_6_0_0[0] + (__out);
            }
        }
        {
            typedef gpucub::BlockReduce<int64_t, 128, gpucub::BLOCK_REDUCE_WARP_REDUCTIONS, 1, 1> __brt_reduce_values_0;
            __shared__ typename __brt_reduce_values_0::TempStorage __brs_reduce_values_0;
            for (int __bk_reduce_values_0 = 0; __bk_reduce_values_0 < 1; ++__bk_reduce_values_0) {
                int64_t __bres_reduce_values_0 = __brt_reduce_values_0(__brs_reduce_values_0).Reduce(__bpart_6_0_0[__bk_reduce_values_0], [] (const int64_t &__cpf_acc, const int64_t &__cpf_val) { return __cpf_acc + (__cpf_val); });
                if (threadIdx.x == 0 && threadIdx.y == 0 && threadIdx.z == 0) {
                    cpf_gpu_atomic(compaction_total_for_17 + ((0) + __bk_reduce_values_0), __bres_reduce_values_0, [] (const int64_t &__cpf_acc, const int64_t &__cpf_val) { return __cpf_acc + (__cpf_val); });
                }
                __syncthreads();
            }
        }
    }
}


void __cpf_runkernel_compact_threshold_pack_fp64_reduce_values_0_6_7(compact_threshold_pack_fp64_state_t *__state, const int8_t * __restrict__ compaction_mask_for_17, int64_t * __restrict__ compaction_total_for_17, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_compact_threshold_pack_fp64_reduce_values_0_6_7(compact_threshold_pack_fp64_state_t *__state, const int8_t * __restrict__ compaction_mask_for_17, int64_t * __restrict__ compaction_total_for_17, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 128)) <= 0)) {

        return;
    }

    void  *compact_threshold_pack_fp64_reduce_values_0_6_7_args[] = { (void *)&compaction_mask_for_17, (void *)&compaction_total_for_17, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)compact_threshold_pack_fp64_reduce_values_0_6_7, dim3(int_ceil(LEN_1D, 128), 1, 1), dim3(128, 1, 1), compact_threshold_pack_fp64_reduce_values_0_6_7_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "compact_threshold_pack_fp64_reduce_values_0_6_7", int_ceil(LEN_1D, 128), 1, 1, 128, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) compact_threshold_pack_fp64_single_state_body_0_map_0_4_16(const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D) {
    int64_t b_compaction_it_for_17_compaction_scatter = (256 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t compaction_it_for_17_compaction_scatter = (threadIdx.x + b_compaction_it_for_17_compaction_scatter);
        if (compaction_it_for_17_compaction_scatter >= b_compaction_it_for_17_compaction_scatter && compaction_it_for_17_compaction_scatter < (cpf_min((LEN_1D - 1), (b_compaction_it_for_17_compaction_scatter + 255)) + 1)) {
            loop_body_0_4_1(&compaction_rank_for_17[0], &src[0], &weight[0], &packed[0], compaction_it_for_17_compaction_scatter);
        }
    }
}


void __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_0_map_0_4_16(compact_threshold_pack_fp64_state_t *__state, const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_compact_threshold_pack_fp64_single_state_body_0_map_0_4_16(compact_threshold_pack_fp64_state_t *__state, const int64_t * __restrict__ compaction_rank_for_17, double * __restrict__ packed, const double * __restrict__ src, const double * __restrict__ weight, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 256)) <= 0)) {

        return;
    }

    void  *compact_threshold_pack_fp64_single_state_body_0_map_0_4_16_args[] = { (void *)&compaction_rank_for_17, (void *)&packed, (void *)&src, (void *)&weight, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)compact_threshold_pack_fp64_single_state_body_0_map_0_4_16, dim3(int_ceil(LEN_1D, 256), 1, 1), dim3(256, 1, 1), compact_threshold_pack_fp64_single_state_body_0_map_0_4_16_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "compact_threshold_pack_fp64_single_state_body_0_map_0_4_16", int_ceil(LEN_1D, 256), 1, 1, 256, 1, 1);
}