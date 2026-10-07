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

struct tsvc_2_s2710_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

void __cpf_runkernel_tsvc_2_s2710_fp64_single_state_body_map_0_1_14(tsvc_2_s2710_fp64_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D, gpuStream_t gpu_stream);
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t e_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s2710_fp64(double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s2710_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s2710_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    {
        gpuStream_t __cpf_current_stream = gpu_streams[0];
        __cpf_runkernel_tsvc_2_s2710_fp64_single_state_body_map_0_1_14(__state, a, b, c, d, e, x, LEN_1D, __cpf_current_stream);
    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}


static inline __device__ __forceinline__ void loop_body_0_1_0(const double* __restrict__ d, const double* __restrict__ e, const double* __restrict__ x, double* __restrict__ a, double* __restrict__ b, double* __restrict__ c, int64_t LEN_1D, int64_t _loop_it_0) {
    double a_index;
    double b_index;
    double x_index;


    a_index = a[_loop_it_0];
    b_index = b[_loop_it_0];
    if ((a_index > b_index)) {
        {
            double b_slice_times_d_slice;
            double _wcr_priv__Add____out;

            b_slice_times_d_slice = (b[b_idx(_loop_it_0)] * d[d_idx(_loop_it_0)]);  // _Mult_
            _wcr_priv__Add____out = b_slice_times_d_slice;  // _Add_
            a[a_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + _wcr_priv__Add____out);  // augassign

        }
        if ((LEN_1D > 10)) {
            {
                double d_slice_times_d_slice;
                double _wcr_priv__Add____out_0;

                d_slice_times_d_slice = (d[d_idx(_loop_it_0)] * d[d_idx(_loop_it_0)]);  // _Mult_
                _wcr_priv__Add____out_0 = d_slice_times_d_slice;  // _Add_
                c[c_idx(_loop_it_0)] = (c[c_idx(_loop_it_0)] + _wcr_priv__Add____out_0);  // augassign

            }
        } else {
            {
                double d_slice_times_e_slice;

                d_slice_times_e_slice = (d[d_idx(_loop_it_0)] * e[e_idx(_loop_it_0)]);  // _Mult_
                c[c_idx(_loop_it_0)] = (d_slice_times_e_slice + 1.0);  // _Add_

            }
        }
    } else {

        x_index = x[0];
        {
            double e_slice_times_e_slice;

            e_slice_times_e_slice = (e[e_idx(_loop_it_0)] * e[e_idx(_loop_it_0)]);  // _Mult_
            b[b_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + e_slice_times_e_slice);  // _Add_

        }
        if ((x_index > 0.0)) {
            {
                double d_slice_times_d_slice_0;

                d_slice_times_d_slice_0 = (d[d_idx(_loop_it_0)] * d[d_idx(_loop_it_0)]);  // _Mult_
                c[c_idx(_loop_it_0)] = (a[a_idx(_loop_it_0)] + d_slice_times_d_slice_0);  // _Add_

            }
        } else {
            {
                double e_slice_times_e_slice_0;
                double _wcr_priv__Add____out_1;

                e_slice_times_e_slice_0 = (e[e_idx(_loop_it_0)] * e[e_idx(_loop_it_0)]);  // _Mult_
                _wcr_priv__Add____out_1 = e_slice_times_e_slice_0;  // _Add_
                c[c_idx(_loop_it_0)] = (c[c_idx(_loop_it_0)] + _wcr_priv__Add____out_1);  // augassign

            }
        }
    }
}


// parallel -- kernel: one GPU launch; the iterations of this map make up the launch grid (blockIdx)
// proven parallel; settled -- focus: block size, tiling, memory access pattern
__global__ void __launch_bounds__(256) tsvc_2_s2710_fp64_single_state_body_map_0_1_14(double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D) {
    int64_t b__loop_it_0 = (256 * static_cast<int64_t>(blockIdx.x));
    // parallel -- thread block: the iterations of this map run on the threads of one block (threadIdx), one each
    // proven parallel; settled -- a thread past the bounds of a trailing block is masked off
    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    {
        int64_t _loop_it_0 = (threadIdx.x + b__loop_it_0);
        if (_loop_it_0 >= b__loop_it_0 && _loop_it_0 < (cpf_min((LEN_1D - 1), (b__loop_it_0 + 255)) + 1)) {
            loop_body_0_1_0(&d[0], &e[0], &x[0], &a[0], &b[0], &c[0], LEN_1D, _loop_it_0);
        }
    }
}


void __cpf_runkernel_tsvc_2_s2710_fp64_single_state_body_map_0_1_14(tsvc_2_s2710_fp64_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D, gpuStream_t gpu_stream);
void __cpf_runkernel_tsvc_2_s2710_fp64_single_state_body_map_0_1_14(tsvc_2_s2710_fp64_state_t *__state, double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D, gpuStream_t gpu_stream)

{

    if (((int_ceil(LEN_1D, 256)) <= 0)) {

        return;
    }

    void  *tsvc_2_s2710_fp64_single_state_body_map_0_1_14_args[] = { (void *)&a, (void *)&b, (void *)&c, (void *)&d, (void *)&e, (void *)&x, (void *)&LEN_1D };
    gpuError_t __err = hipLaunchKernel((void*)tsvc_2_s2710_fp64_single_state_body_map_0_1_14, dim3(int_ceil(LEN_1D, 256), 1, 1), dim3(256, 1, 1), tsvc_2_s2710_fp64_single_state_body_map_0_1_14_args, 0, gpu_stream);

    cpf_kernel_launch_check(__err, "tsvc_2_s2710_fp64_single_state_body_map_0_1_14", int_ceil(LEN_1D, 256), 1, 1, 256, 1, 1);
}