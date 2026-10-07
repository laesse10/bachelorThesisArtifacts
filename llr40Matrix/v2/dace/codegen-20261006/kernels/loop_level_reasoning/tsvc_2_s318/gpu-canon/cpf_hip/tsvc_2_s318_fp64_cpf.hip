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


struct tsvc_2_s318_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

static consteval inline int64_t _argfi_val_for_20_size() { return 1; }
static consteval inline int64_t _argfi_idx_for_20_size() { return 1; }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void tsvc_2_s318_fp64(const double * __restrict__ a, double * __restrict__ result, int64_t LEN_1D, int64_t inc, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    tsvc_2_s318_fp64_state_t __cpf_state{&__cpf_context};
    tsvc_2_s318_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double* __restrict__ _argfi_val_for_20 = new (std::align_val_t(64)) double[_argfi_val_for_20_size()];
    int64_t* __restrict__ _argfi_idx_for_20 = new (std::align_val_t(64)) int64_t[_argfi_idx_for_20_size()];
    double scal_result;
    double result_host;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double maxv;
    int64_t index;

    {

        // argument reduction: index of the element the reduction selected
        {  // for_20_argfi_argreduce_openmp
            const double* __restrict__ _in = &a[0];
            int64_t _out_idx;
            double _out_val;
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = std::abs(_in[(0) * (inc)]); __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < (LEN_1D); ++__i) {
                const double __v = std::abs(_in[(__i) * (inc)]);
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            _argfi_idx_for_20[0] = _out_idx;
            _argfi_val_for_20[0] = _out_val;
        }

    }
    {

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
    maxv = _argfi_val_for_20[0];
    index = _argfi_idx_for_20[0];
    {
        double float_index;

        {  // _convert_to_float64_
            float_index = double(index);
        }
        result_host = (maxv + float_index);  // _Add_
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
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argfi_val_for_20, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argfi_idx_for_20, std::align_val_t(64));
    gpu_streams = nullptr;
}


