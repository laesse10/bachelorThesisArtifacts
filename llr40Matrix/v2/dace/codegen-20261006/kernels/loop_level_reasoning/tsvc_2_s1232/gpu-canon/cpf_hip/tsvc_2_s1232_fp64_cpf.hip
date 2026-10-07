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
#include <limits>
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


// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline T cpf_min(const T& value) {
    return value;
}
template <typename T, typename... Ts>
static constexpr inline typename std::common_type<T, Ts...>::type cpf_min(const T& a, const Ts&... rest) {
    return (cpf_min(rest...) < a) ? cpf_min(rest...) : a;
}
template <typename T>
static constexpr inline void py_divmod(const T& numerator, const T& denominator, T& quotient,
                                       T& remainder) {
    if constexpr (std::is_floating_point_v<T>) {
        const T ratio = numerator / denominator;
        remainder = std::fmod(numerator, denominator);
        if (denominator == 0) {
            quotient = ratio;
            return;
        }
        quotient = (numerator - remainder) / denominator;
        if (remainder == 0) {
            remainder = std::copysign(static_cast<T>(0), denominator);
        } else if ((denominator < 0) != (remainder < 0)) {
            remainder += denominator;
            quotient -= 1;
        }
        if (quotient == 0) {
            quotient = std::copysign(static_cast<T>(0), ratio);
        } else {
            const T floored = std::floor(quotient);
            quotient = quotient - floored > static_cast<T>(0.5) ? floored + 1 : floored;
        }
    } else if (denominator == 0) {
        quotient = 0;
        remainder = 0;
    } else if (numerator == std::numeric_limits<T>::min() && denominator == static_cast<T>(-1)) {
        quotient = numerator;
        remainder = 0;
    } else {
        quotient = static_cast<T>(numerator / denominator);
        remainder = static_cast<T>(numerator % denominator);
        if (remainder != 0 && ((remainder < 0) != (denominator < 0))) {
            quotient = static_cast<T>(quotient - 1);
            remainder = static_cast<T>(remainder + denominator);
        }
    }
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R py_floor(const T& numerator, const U& denominator) {
    R quotient = 0;
    R remainder = 0;
    py_divmod(static_cast<R>(numerator), static_cast<R>(denominator), quotient, remainder);
    return quotient;
}

struct tsvc_2_s1232_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_tsvc_2_s1232_fp64_single_state_body_map_0_1_3(tsvc_2_s1232_fp64_state_t *__state, double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, const int64_t VLEN, gpuStream_t gpu_stream);
static constexpr inline int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s1232_fp64(double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, int64_t VLEN, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s1232_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s1232_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }
        {  // check_assumption_1
            if ((VLEN < 0)) {
                std::abort();
            }
        }
        {  // check_assumption_2
            if ((VLEN < 1)) {
                std::abort();
            }
        }

    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_tsvc_2_s1232_fp64_single_state_body_map_0_1_3(__state, aa, bb, cc, LEN_2D, VLEN, __cpf_current_stream);
    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}


static inline __device__ __forceinline__ void nested_single_state_body_0_1_7(const double* __restrict__ bb, const double* __restrict__ cc, double* __restrict__ aa, int64_t LEN_2D, int64_t VLEN, int64_t __tid, int64_t _loop_it_1) {


    // parallel -- lane-strided: lane t of the block takes iterations t, t + B, ... (B lanes)
    // why: the map is parallel, and adjacent lanes touching adjacent elements coalesce the accesses
    for (int64_t _loop_it_0 = __tid; _loop_it_0 < (cpf_min((LEN_2D - 1), py_floor(_loop_it_1, VLEN)) + 1); _loop_it_0 += 256) {
        aa[aa_idx(_loop_it_1, _loop_it_0, LEN_2D)] = (bb[bb_idx(_loop_it_1, _loop_it_0, LEN_2D)] + cc[cc_idx(_loop_it_1, _loop_it_0, LEN_2D)]);  // _Add_
    }
}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
// parallel -- the iterations are independent
// settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
__global__ void __launch_bounds__(256) tsvc_2_s1232_fp64_single_state_body_map_0_1_3(double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, const int64_t VLEN) {
    int64_t _loop_it_1 = static_cast<int64_t>(blockIdx.x);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- block tile: this kernel runs one block per outer iteration, and these are the lanes of the block
    // why: the inner parallel maps are strided across the lanes, so one outer iteration uses a whole block instead of one thread; code outside them runs on every lane, or on lane 0 alone between two barriers when it updates data other lanes read
    {
        int __tid = threadIdx.x;
        nested_single_state_body_0_1_7(&bb[0], &cc[0], &aa[0], LEN_2D, VLEN, __tid, _loop_it_1);
    }
}


void __cpf_runkernel_tsvc_2_s1232_fp64_single_state_body_map_0_1_3(tsvc_2_s1232_fp64_state_t *__state, double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, const int64_t VLEN, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s1232_fp64_single_state_body_map_0_1_3(tsvc_2_s1232_fp64_state_t *__state, double * __restrict__ aa, const double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, const int64_t VLEN, gpuStream_t gpu_stream)

{

    void  *tsvc_2_s1232_fp64_single_state_body_map_0_1_3_args[] = { (void *)&aa, (void *)&bb, (void *)&cc, (void *)&LEN_2D, (void *)&VLEN };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s1232_fp64_single_state_body_map_0_1_3, dim3(LEN_2D, 1, 1), dim3(256, 1, 1), tsvc_2_s1232_fp64_single_state_body_map_0_1_3_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s1232_fp64_single_state_body_map_0_1_3", LEN_2D, 1, 1, 256, 1, 1);
}