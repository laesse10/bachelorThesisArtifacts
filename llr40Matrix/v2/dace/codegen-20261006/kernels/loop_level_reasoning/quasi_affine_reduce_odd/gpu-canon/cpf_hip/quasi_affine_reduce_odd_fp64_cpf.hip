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

struct quasi_affine_reduce_odd_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13(quasi_affine_reduce_odd_fp64_state_t *__state, double * __restrict__ out, gpuStream_t gpu_stream);
void __cpf_runkernel_quasi_affine_reduce_odd_fp64_reduce_values_0_0_15(quasi_affine_reduce_odd_fp64_state_t *__state, const double * __restrict__ _in, double * __restrict__ out, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t _in_idx(int64_t __d0) { return (2 * __d0); }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void quasi_affine_reduce_odd_fp64(const double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    quasi_affine_reduce_odd_fp64_state_t __cpf_state{&__cpf_context};
    quasi_affine_reduce_odd_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {
        const double* _in;
        _in = &a[1];

        {
            gpuStream_t __cpf_current_stream = gpu_streams[0];
            __cpf_runkernel_quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13(__state, out, __cpf_current_stream);
        }
        {
            gpuStream_t __cpf_current_stream = gpu_streams[0];
            __cpf_runkernel_quasi_affine_reduce_odd_fp64_reduce_values_0_0_15(__state, _in, out, LEN_1D, __cpf_current_stream);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}



// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13(double * __restrict__ out) {
    int b___wrap_i = (256 * blockIdx.x);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int __wrap_i = (threadIdx.x + b___wrap_i);
        if (__wrap_i >= b___wrap_i && __wrap_i < (cpf_min(0, (b___wrap_i + 255)) + 1)) {
            {  // assign_16_4
                double __out;
                __out = 0.0;
                out[0] = __out;
            }
        }
    }
}


void __cpf_runkernel_quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13(quasi_affine_reduce_odd_fp64_state_t *__state, double * __restrict__ out, gpuStream_t gpu_stream);
void __cpf_runkernel_quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13(quasi_affine_reduce_odd_fp64_state_t *__state, double * __restrict__ out, gpuStream_t gpu_stream)

{

    void  *quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13_args[] = { (void *)&out };
    gpuError_t __err = hipLaunchKernel((void*)quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13, dim3(1, 1, 1), dim3(256, 1, 1), quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "quasi_affine_reduce_odd_fp64_size1_wrap_region_0_0_13", 1, 1, 1, 256, 1, 1);
}
// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(128) quasi_affine_reduce_odd_fp64_reduce_values_0_0_15(const double * __restrict__ _in, double * __restrict__ out, int64_t LEN_1D) {
    int64_t b__i0 = (128 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _i0 = (threadIdx.x + b__i0);
        double __bpart_0_6_0[1];
        for (int __bi = 0; __bi < 1; ++__bi) __bpart_0_6_0[__bi] = double(0.0);
        if (_i0 >= b__i0 && _i0 < (cpf_min((b__i0 + 127), (int_ceil((LEN_1D - 1), 2) - 1)) + 1)) {
            // reduction over the given axes with the given operator
            {  // identity
                double __out;
                __out = _in[_in_idx(_i0)];
                __bpart_0_6_0[0] = __bpart_0_6_0[0] + (__out);
            }
        }
        {
            typedef gpucub::BlockReduce<double, 128, gpucub::BLOCK_REDUCE_WARP_REDUCTIONS, 1, 1> __brt_reduce_values_0;
            __shared__ typename __brt_reduce_values_0::TempStorage __brs_reduce_values_0;
            for (int __bk_reduce_values_0 = 0; __bk_reduce_values_0 < 1; ++__bk_reduce_values_0) {
                double __bres_reduce_values_0 = __brt_reduce_values_0(__brs_reduce_values_0).Reduce(__bpart_0_6_0[__bk_reduce_values_0], [] (const double &__cpf_acc, const double &__cpf_val) { return __cpf_acc + (__cpf_val); });
                if (threadIdx.x == 0 && threadIdx.y == 0 && threadIdx.z == 0) {
                    cpf_gpu_atomic(out + ((0) + __bk_reduce_values_0), __bres_reduce_values_0, [] (const double &__cpf_acc, const double &__cpf_val) { return __cpf_acc + (__cpf_val); });
                }
                __syncthreads();
            }
        }
    }
}


void __cpf_runkernel_quasi_affine_reduce_odd_fp64_reduce_values_0_0_15(quasi_affine_reduce_odd_fp64_state_t *__state, const double * __restrict__ _in, double * __restrict__ out, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_quasi_affine_reduce_odd_fp64_reduce_values_0_0_15(quasi_affine_reduce_odd_fp64_state_t *__state, const double * __restrict__ _in, double * __restrict__ out, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(int_ceil((LEN_1D - 1), 2), 128)) <= 0)) {

        return;
    }

    void  *quasi_affine_reduce_odd_fp64_reduce_values_0_0_15_args[] = { (void *)&_in, (void *)&out, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)quasi_affine_reduce_odd_fp64_reduce_values_0_0_15, dim3(int_ceil(int_ceil((LEN_1D - 1), 2), 128), 1, 1), dim3(128, 1, 1), quasi_affine_reduce_odd_fp64_reduce_values_0_0_15_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "quasi_affine_reduce_odd_fp64_reduce_values_0_0_15", int_ceil(int_ceil((LEN_1D - 1), 2), 128), 1, 1, 128, 1, 1);
}