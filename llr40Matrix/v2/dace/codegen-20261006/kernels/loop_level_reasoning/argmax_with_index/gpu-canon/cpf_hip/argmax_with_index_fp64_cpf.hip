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


struct argmax_with_index_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

static consteval inline int64_t _argmax_val_for_18_size() { return 1; }
static consteval inline int64_t _argmax_idx_for_18_size() { return 1; }
static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
extern "C" void argmax_with_index_fp64(const double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    argmax_with_index_fp64_state_t __cpf_state{&__cpf_context};
    argmax_with_index_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double* __restrict__ _argmax_val_for_18 = new (std::align_val_t(64)) double[_argmax_val_for_18_size()];
    int64_t* __restrict__ _argmax_idx_for_18 = new (std::align_val_t(64)) int64_t[_argmax_idx_for_18_size()];
    double scal_out_value;
    int64_t scal_out_index;
    double out_value_host;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    double x;
    int64_t idx;

    {

        // argument reduction: index of the element the reduction selected
        {  // for_18_argreduce_openmp
            const double* __restrict__ _in = &a[0];
            int64_t _out_idx;
            double _out_val;
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = _in[0]; __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < (LEN_1D); ++__i) {
                const double __v = _in[__i];
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            _argmax_idx_for_18[0] = _out_idx;
            _argmax_val_for_18[0] = _out_val;
        }

    }
    {

        {  // copy_out_index_to_scal_out_index
            hipMemcpyAsync((&scal_out_index), out_index, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_out_value_to_scal_out_value
            hipMemcpyAsync((&scal_out_value), out_value, 1 * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    {

        out_value_host = scal_out_value;  // copy_scal_out_value_to_out_value_host

    }
    x = _argmax_val_for_18[0];
    idx = _argmax_idx_for_18[0];
    {

        scal_out_index = idx;  // symassign
        out_value_host = x;  // symassign
        scal_out_value = out_value_host;  // copy_out_value_host_to_scal_out_value

    }
    {

        {  // copy_scal_out_value_to_out_value
            hipMemcpyAsync(out_value, (&scal_out_value), 1 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_scal_out_index_to_out_index
            hipMemcpyAsync(out_index, (&scal_out_index), 1 * sizeof(int64_t), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax_val_for_18, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<int64_t>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](_argmax_idx_for_18, std::align_val_t(64));
    gpu_streams = nullptr;
}


