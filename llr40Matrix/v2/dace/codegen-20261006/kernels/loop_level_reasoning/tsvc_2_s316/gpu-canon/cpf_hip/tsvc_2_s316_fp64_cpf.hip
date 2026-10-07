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
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R int_ceil(const T& numerator, const U& denominator) {
    return (numerator + denominator - 1) / denominator;
}

struct tsvc_2_s316_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_tsvc_2_s316_fp64_reduce_init_map_0_3_6(tsvc_2_s316_fp64_state_t *__state, double * __restrict__ _arg_max_buf_for_17, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s316_fp64_reduce_values_0_4_7(tsvc_2_s316_fp64_state_t *__state, double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s316_fp64(const double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s316_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s316_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double scal_result;
    double result_host;
    double _arg_max_buf_for_17_host;
    double * _arg_max_buf_for_17;
    cpf_gpu_check(hipMalloc((void**)&_arg_max_buf_for_17, 1 * sizeof(double)));
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double x;



    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_tsvc_2_s316_fp64_reduce_init_map_0_3_6(__state, _arg_max_buf_for_17, __cpf_current_stream);
    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_tsvc_2_s316_fp64_reduce_values_0_4_7(__state, _arg_max_buf_for_17, a, LEN_1D, __cpf_current_stream);
    }
    {

        {  // copy__arg_max_buf_for_17_to__arg_max_buf_for_17_host
            hipMemcpyAsync((&_arg_max_buf_for_17_host), _arg_max_buf_for_17, 1 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_result_to_scal_result
            hipMemcpyAsync((&scal_result), result, 1 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    {

        result_host = scal_result;  // copy_scal_result_to_result_host

    }
    x = _arg_max_buf_for_17_host;
    {

        result_host = x;  // symassign
        scal_result = result_host;  // copy_result_host_to_scal_result

    }
    {

        {  // copy_scal_result_to_result
            hipMemcpyAsync(result, (&scal_result), 1 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    cpf_gpu_check(hipFree(_arg_max_buf_for_17));
    gpu_streams = nullptr;
}



// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(128) tsvc_2_s316_fp64_reduce_init_map_0_3_6(double * __restrict__ _arg_max_buf_for_17) {
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
                double __out;
                __out = 1.7976931348623157e+308;
                _arg_max_buf_for_17[0] = __out;
            }
        }
    }
}


void __cpf_runkernel_tsvc_2_s316_fp64_reduce_init_map_0_3_6(tsvc_2_s316_fp64_state_t *__state, double * __restrict__ _arg_max_buf_for_17, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s316_fp64_reduce_init_map_0_3_6(tsvc_2_s316_fp64_state_t *__state, double * __restrict__ _arg_max_buf_for_17, gpuStream_t gpu_stream)

{

    void  *tsvc_2_s316_fp64_reduce_init_map_0_3_6_args[] = { (void *)&_arg_max_buf_for_17 };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s316_fp64_reduce_init_map_0_3_6, dim3(1, 1, 1), dim3(128, 1, 1), tsvc_2_s316_fp64_reduce_init_map_0_3_6_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s316_fp64_reduce_init_map_0_3_6", 1, 1, 1, 128, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(128) tsvc_2_s316_fp64_reduce_values_0_4_7(double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D) {
    int64_t b__i0 = (128 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _i0 = (threadIdx.x + b__i0);
        double __bpart_4_0_0[1];
        for (int __bi = 0; __bi < 1; ++__bi) __bpart_4_0_0[__bi] = double(1.7976931348623157e+308);
        if (_i0 >= b__i0 && _i0 < (cpf_min((LEN_1D - 1), (b__i0 + 127)) + 1)) {
            {  // identity
                double __out;
                __out = a[a_idx(_i0)];
                __bpart_4_0_0[0] = std::min(__bpart_4_0_0[0], __out);
            }
        }
        {
            typedef gpucub::BlockReduce<double, 128, gpucub::BLOCK_REDUCE_WARP_REDUCTIONS, 1, 1> __brt_reduce_values_0;
            __shared__ typename __brt_reduce_values_0::TempStorage __brs_reduce_values_0;
            for (int __bk_reduce_values_0 = 0; __bk_reduce_values_0 < 1; ++__bk_reduce_values_0) {
                double __bres_reduce_values_0 = __brt_reduce_values_0(__brs_reduce_values_0).Reduce(__bpart_4_0_0[__bk_reduce_values_0], [] (const double &__cpf_acc, const double &__cpf_val) { return std::min(__cpf_acc, __cpf_val); });
                if (threadIdx.x == 0 && threadIdx.y == 0 && threadIdx.z == 0) {
                    cpf_gpu_atomic(_arg_max_buf_for_17 + ((0) + __bk_reduce_values_0), __bres_reduce_values_0, [] (const double &__cpf_acc, const double &__cpf_val) { return std::min(__cpf_acc, __cpf_val); });
                }
                __syncthreads();
            }
        }
    }
}


void __cpf_runkernel_tsvc_2_s316_fp64_reduce_values_0_4_7(tsvc_2_s316_fp64_state_t *__state, double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s316_fp64_reduce_values_0_4_7(tsvc_2_s316_fp64_state_t *__state, double * __restrict__ _arg_max_buf_for_17, const double * __restrict__ a, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 128)) <= 0)) {

        return;
    }

    void  *tsvc_2_s316_fp64_reduce_values_0_4_7_args[] = { (void *)&_arg_max_buf_for_17, (void *)&a, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s316_fp64_reduce_values_0_4_7, dim3(int_ceil(LEN_1D, 128), 1, 1), dim3(128, 1, 1), tsvc_2_s316_fp64_reduce_values_0_4_7_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s316_fp64_reduce_values_0_4_7", int_ceil(LEN_1D, 128), 1, 1, 128, 1, 1);
}