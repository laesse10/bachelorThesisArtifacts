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


// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline T cpf_max(const T& value) {
    return value;
}
template <typename T, typename... Ts>
static constexpr inline typename std::common_type<T, Ts...>::type cpf_max(const T& a, const Ts&... rest) {
    return (a < cpf_max(rest...)) ? cpf_max(rest...) : a;
}
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

struct wf_triangular_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_wf_triangular_fp64_single_state_body_map_1_0_3(wf_triangular_fp64_state_t *__state, double * __restrict__ a, int64_t LEN_2D, int64_t _skew_t_0, gpuStream_t gpu_stream);
extern "C" void wf_triangular_fp64(double * __restrict__ a, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    wf_triangular_fp64_state_t __cpf_state{&__cpf_context};
    wf_triangular_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }

    // sequential -- wavefront tile diagonal (t = _loop_it_0 + j) [64x64]: the tile diagonal carries every dependence
    // alternatives: the element diagonal, or the unskewed nest -- all three bit-identical; a bigger tile trades kernel launches for block-local barriers
    for (int64_t _skew_t_0 = 0; (_skew_t_0 <= ((2 * int_ceil((LEN_2D - 1), 64)) - 2)); _skew_t_0 = (_skew_t_0 + 1)) {
        {
            gpuStream_t* gpu_streams = __state->gpu_context->streams;

            {
                gpuStream_t __cpf_current_stream = gpu_streams[0];
                __cpf_runkernel_wf_triangular_fp64_single_state_body_map_1_0_3(__state, a, LEN_2D, _skew_t_0, __cpf_current_stream);
            }
            gpu_streams = nullptr;

        }

    }

}


static constexpr inline int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static inline __device__ __forceinline__ void loop_body_3_0_0(double* __restrict__ a, int64_t LEN_2D, int64_t _skew_p_0, int64_t _skew_p_1, int64_t _skew_t_0, int64_t _skew_t_1) {

    if ((((((64 * _skew_p_0) + _skew_p_1) + 1) >= cpf_max((((- LEN_2D) + _skew_t_1) + 1), ((((64 * _skew_p_0) - (64 * _skew_t_0)) + _skew_t_1) - 64), int_ceil(_skew_t_1, 2), ((64 * _skew_p_0) + 1))) && ((((64 * _skew_p_0) + _skew_p_1) + 1) <= cpf_min(((((64 * _skew_p_0) - (64 * _skew_t_0)) + _skew_t_1) - 1), (LEN_2D - 1), ((64 * _skew_p_0) + 64))))) {
        {
            double a_slice_a_slice_plus_a_slice;
            double a_slice_a_slice_a_slice_div_3_0;
            double a_index_0;
            double a_index;
            double a_index_1;
            double a_slice_plus_a_slice;

            a_index = a[a_idx(((((-64 * _skew_p_0) - _skew_p_1) + _skew_t_1) - 1), (((64 * _skew_p_0) + _skew_p_1) + 1), LEN_2D)];  // _assign_a_to_a_index
            a_index_0 = a[a_idx(((((-64 * _skew_p_0) - _skew_p_1) + _skew_t_1) - 2), (((64 * _skew_p_0) + _skew_p_1) + 1), LEN_2D)];  // _assign_a_to_a_index_0
            a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
            a_index_1 = a[a_idx(((((-64 * _skew_p_0) - _skew_p_1) + _skew_t_1) - 1), ((64 * _skew_p_0) + _skew_p_1), LEN_2D)];  // _assign_a_to_a_index_1
            a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
            a_slice_a_slice_a_slice_div_3_0 = (a_slice_a_slice_plus_a_slice / 3.0);  // _Div_
            a[a_idx(((((-64 * _skew_p_0) - _skew_p_1) + _skew_t_1) - 1), (((64 * _skew_p_0) + _skew_p_1) + 1), LEN_2D)] = a_slice_a_slice_a_slice_div_3_0;  // _assign_a_slice_a_slice_a_slice_div_3_0_to_a

        }
    }
}

static inline __device__ __forceinline__ void loop_body_1_0_0(double* __restrict__ a, int64_t LEN_2D, int64_t _skew_p_0, int64_t _skew_t_0) {

    // sequential -- wavefront diagonal (t = _loop_it_0 + j): the skew put every dependence on this axis
    // alternative: the unskewed nest, sequential in both axes -- worth timing on CPU, rarely on GPU
    for (int64_t _skew_t_1 = ((64 * _skew_t_0) + 2); (_skew_t_1 <= ((64 * _skew_t_0) + 128)); _skew_t_1 = (_skew_t_1 + 1)) {
        {

            // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
            // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
            // parallel -- warp tile: a map its producer proved parallel (tagged is_warp_tile), spread over the threads of the block instead of one thread walking it
            // why: the device offload serializes every map nested in a kernel; the producer's proof lets this one use the block
            // a block barrier follows the map: the enclosing loop runs it again, and without the barrier the next run could read what this one has not finished writing
            // parallel -- wavefront front (t = _loop_it_0 + j): at a fixed diagonal the points are independent
            {
                int _skew_p_1 = threadIdx.x;
                loop_body_3_0_0(&a[0], LEN_2D, _skew_p_0, _skew_p_1, _skew_t_0, _skew_t_1);
            }
            {  // sync_threads
                __syncthreads();
            }

        }

    }
}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
// parallel -- wavefront tile column [64x64]: the tiles on one diagonal are independent
__global__ void __launch_bounds__(64) wf_triangular_fp64_single_state_body_map_1_0_3(double * __restrict__ a, int64_t LEN_2D, int64_t _skew_t_0) {
    int64_t _skew_p_0 = (static_cast<int64_t>(blockIdx.x) + cpf_max(0, ((_skew_t_0 - int_ceil((LEN_2D - 1), 64)) + 1)));
    loop_body_1_0_0(&a[0], LEN_2D, _skew_p_0, _skew_t_0);
}


void __cpf_runkernel_wf_triangular_fp64_single_state_body_map_1_0_3(wf_triangular_fp64_state_t *__state, double * __restrict__ a, int64_t LEN_2D, int64_t _skew_t_0, gpuStream_t gpu_stream);
void __cpf_runkernel_wf_triangular_fp64_single_state_body_map_1_0_3(wf_triangular_fp64_state_t *__state, double * __restrict__ a, int64_t LEN_2D, int64_t _skew_t_0, gpuStream_t gpu_stream)

{

    if ((((((- cpf_max(0, ((_skew_t_0 - int_ceil((LEN_2D - 1), 64)) + 1))) + cpf_min(_skew_t_0, (int_ceil((LEN_2D - 1), 64) - 1))) + 1)) <= 0)) {

        return;
    }

    void  *wf_triangular_fp64_single_state_body_map_1_0_3_args[] = { (void *)&a, (void *)&LEN_2D, (void *)&_skew_t_0 };
    gpuError_t __err = hipLaunchKernel((void*)wf_triangular_fp64_single_state_body_map_1_0_3, dim3((((- cpf_max(0, ((_skew_t_0 - int_ceil((LEN_2D - 1), 64)) + 1))) + cpf_min(_skew_t_0, (int_ceil((LEN_2D - 1), 64) - 1))) + 1), 1, 1), dim3(64, 1, 1), wf_triangular_fp64_single_state_body_map_1_0_3_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "wf_triangular_fp64_single_state_body_map_1_0_3", (((- cpf_max(0, ((_skew_t_0 - int_ceil((LEN_2D - 1), 64)) + 1))) + cpf_min(_skew_t_0, (int_ceil((LEN_2D - 1), 64) - 1))) + 1), 1, 1, 64, 1, 1);
}