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

struct wf_diff_skew_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_wf_diff_skew_fp64_single_state_body_map_1_0_18(wf_diff_skew_fp64_state_t *__state, double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0, gpuStream_t gpu_stream);
extern "C" void wf_diff_skew_fp64(double * __restrict__ a, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    wf_diff_skew_fp64_state_t __cpf_state{&__cpf_context};
    wf_diff_skew_fp64_state_t *__state = &__cpf_state;
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

    // sequential -- carried: RAW on a[_loop_it_0 - 1, _loop_it_1]
    // settled: proven, an iteration reads what an earlier one wrote; focus: restructure around the dependence (reorder, block, rewrite as a scan or reduction), or optimize the work inside
    for (int64_t _loop_it_0 = 1; (_loop_it_0 < LEN_2D); _loop_it_0 = (_loop_it_0 + 1)) {
        {
            gpuStream_t* gpu_streams = __state->gpu_context->streams;

            {
                gpuStream_t __cpf_current_stream = gpu_streams[0];
                __cpf_runkernel_wf_diff_skew_fp64_single_state_body_map_1_0_18(__state, a, LEN_2D, _loop_it_0, __cpf_current_stream);
            }
            gpu_streams = nullptr;

        }

    }

}


static constexpr inline int64_t a_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }

// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) wf_diff_skew_fp64_single_state_body_map_1_0_18(double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0) {
    int64_t b__loop_it_1 = (256 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _loop_it_1 = (threadIdx.x + b__loop_it_1);
        double a_index;
        double a_index_0;
        double a_index_1;
        double a_slice_plus_a_slice;
        double a_slice_a_slice_plus_a_slice;
        if (_loop_it_1 >= b__loop_it_1 && _loop_it_1 < (cpf_min((LEN_2D - 2), (b__loop_it_1 + 255)) + 1)) {
            a_index = a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index
            a_index_0 = a[a_idx((_loop_it_0 - 1), _loop_it_1, LEN_2D)];  // _assign_in_a_to_a_index_0
            a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
            a_index_1 = a[a_idx((_loop_it_0 - 1), (_loop_it_1 + 1), LEN_2D)];  // _assign_in_a_to_a_index_1
            a_slice_a_slice_plus_a_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
            a[a_idx(_loop_it_0, _loop_it_1, LEN_2D)] = a_slice_a_slice_plus_a_slice;  // _assign_out_a_slice_a_slice_plus_a_slice_to_a
        }
    }
}


void __cpf_runkernel_wf_diff_skew_fp64_single_state_body_map_1_0_18(wf_diff_skew_fp64_state_t *__state, double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0, gpuStream_t gpu_stream);
void __cpf_runkernel_wf_diff_skew_fp64_single_state_body_map_1_0_18(wf_diff_skew_fp64_state_t *__state, double * __restrict__ a, int64_t LEN_2D, const int64_t _loop_it_0, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_2D - 1), 256)) <= 0)) {

        return;
    }

    void  *wf_diff_skew_fp64_single_state_body_map_1_0_18_args[] = { (void *)&a, (void *)&LEN_2D, (void *)&_loop_it_0 };
    gpuError_t __err = hipLaunchKernel((void*)wf_diff_skew_fp64_single_state_body_map_1_0_18, dim3(int_ceil((LEN_2D - 1), 256), 1, 1), dim3(256, 1, 1), wf_diff_skew_fp64_single_state_body_map_1_0_18_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "wf_diff_skew_fp64_single_state_body_map_1_0_18", int_ceil((LEN_2D - 1), 256), 1, 1, 256, 1, 1);
}