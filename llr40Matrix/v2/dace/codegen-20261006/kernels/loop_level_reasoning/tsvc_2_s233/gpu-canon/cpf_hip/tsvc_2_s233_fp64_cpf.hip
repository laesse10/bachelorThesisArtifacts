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

struct tsvc_2_s233_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10(tsvc_2_s233_fp64_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, gpuStream_t gpu_stream);
static constexpr inline int64_t aa_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t cc_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t bb_idx(int64_t __d0, int64_t __d1, int64_t LEN_2D) { return ((LEN_2D * __d0) + __d1); }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s233_fp64(double * __restrict__ aa, double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s233_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s233_fp64_state_t *__state = &__cpf_state;
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

    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10(__state, aa, bb, cc, LEN_2D, __cpf_current_stream);
    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}


static inline __device__ __forceinline__ void nested_sdfg_0_1_0(const double* __restrict__ cc, double* __restrict__ aa, double* __restrict__ bb, int64_t LEN_2D, int64_t _loop_it_3_lane) {

    // sequential -- carried: RAW on aa[_loop_it_1 - 1, _loop_it_3]
    // settled: proven, an iteration reads what an earlier one wrote; focus: restructure around the dependence (reorder, block, rewrite as a scan or reduction), or optimize the work inside
    for (int64_t _loop_it_1 = 8; (_loop_it_1 < LEN_2D); _loop_it_1 = (_loop_it_1 + 1)) {
        {
            double aa_index;
            double bb_index;
            double aa_slice_plus_cc_slice;
            double bb_slice_plus_cc_slice;

            aa_index = aa[aa_idx((_loop_it_1 - 1), _loop_it_3_lane, LEN_2D)];  // _assign_in_aa_to_aa_index
            aa_slice_plus_cc_slice = (aa_index + cc[cc_idx(_loop_it_1, _loop_it_3_lane, LEN_2D)]);  // _Add_
            aa[aa_idx(_loop_it_1, _loop_it_3_lane, LEN_2D)] = aa_slice_plus_cc_slice;  // _assign_out_aa_slice_plus_cc_slice_to_aa
            bb_index = bb[bb_idx(_loop_it_3_lane, (_loop_it_1 - 1), LEN_2D)];  // _assign_in_bb_to_bb_index
            bb_slice_plus_cc_slice = (bb_index + cc[cc_idx(_loop_it_3_lane, _loop_it_1, LEN_2D)]);  // _Add_
            bb[bb_idx(_loop_it_3_lane, _loop_it_1, LEN_2D)] = bb_slice_plus_cc_slice;  // _assign_out_bb_slice_plus_cc_slice_to_bb

        }

    }
}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10(double * __restrict__ aa, double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D) {
    int64_t b__loop_it_3_lane = ((256 * static_cast<int64_t>(blockIdx.x)) + 8);
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _loop_it_3_lane = (threadIdx.x + b__loop_it_3_lane);
        if (_loop_it_3_lane >= b__loop_it_3_lane && _loop_it_3_lane < (cpf_min((LEN_2D - 1), (b__loop_it_3_lane + 255)) + 1)) {
            nested_sdfg_0_1_0(&cc[0], &aa[0], &bb[0], LEN_2D, _loop_it_3_lane);
        }
    }
}


void __cpf_runkernel_tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10(tsvc_2_s233_fp64_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10(tsvc_2_s233_fp64_state_t *__state, double * __restrict__ aa, double * __restrict__ bb, const double * __restrict__ cc, int64_t LEN_2D, gpuStream_t gpu_stream)

{

    if (((int_ceil((LEN_2D - 8), 256)) <= 0)) {

        return;
    }

    void  *tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10_args[] = { (void *)&aa, (void *)&bb, (void *)&cc, (void *)&LEN_2D };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10, dim3(int_ceil((LEN_2D - 8), 256), 1, 1), dim3(256, 1, 1), tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s233_fp64_for_16_fis0_lanes_0_1_10", int_ceil((LEN_2D - 8), 256), 1, 1, 256, 1, 1);
}