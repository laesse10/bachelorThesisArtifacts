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


struct ext_break_capture_fp64_state_t {
    cpf_gpu_context *gpu_context;
};

static constexpr inline int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_host_idx(int64_t __d0) { return __d0; }
extern "C" void ext_break_capture_fp64(const double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    // The device handshake, which a single entry point has nowhere else to put.
    cpf_gpu_context __cpf_context{};
    __cpf_context.streams[0] = nullptr;
    ext_break_capture_fp64_state_t __cpf_state{&__cpf_context};
    ext_break_capture_fp64_state_t *__state = &__cpf_state;
    struct __cpf_drain {
        ~__cpf_drain() { cpf_gpu_check(hipDeviceSynchronize()); }
    } __cpf_drain_guard;
    double* __restrict__ a_host = new (std::align_val_t(64)) double[LEN_1D];
    int64_t scal_out_index;
    double scal_out_value;
    double out_value_host;
    gpuStream_t* gpu_streams = __state->gpu_context->streams;
    int64_t out_index_host;
    double a_index;

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
    out_index_host = scal_out_index;
    {

        out_value_host = scal_out_value;  // copy_scal_out_value_to_out_value_host

    }
    {

        {  // copy_a_to_a_host
            hipMemcpyAsync(a_host, a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }
    out_index_host = -1;
    {

        out_value_host = -1.0;  // assign_19_4

    }

    // unsure -- loop body contains a BreakBlock
    // open: not proven either way; kept sequential to be safe, and may well be parallel -- a fact about the indices (a bound, a permutation) settles it
    for (int64_t _loop_it_0 = 0; (_loop_it_0 < LEN_1D); _loop_it_0 = (_loop_it_0 + 1)) {

        a_index = a_host[_loop_it_0];

        if ((a_index > 1)) {

            out_index_host = _loop_it_0;
            {

                out_value_host = a_host[a_host_idx(_loop_it_0)];  // _assign_a_to_out_value

            }
            break;
        }



    }

    {

        scal_out_value = out_value_host;  // copy_out_value_host_to_scal_out_value
        scal_out_index = out_index_host;  // symassign

    }
    {

        {  // copy_scal_out_index_to_out_index
            hipMemcpyAsync(out_index, (&scal_out_index), 1 * sizeof(int64_t), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }
        {  // copy_scal_out_value_to_out_value
            hipMemcpyAsync(out_value, (&scal_out_value), 1 * sizeof(double), hipMemcpyHostToDevice, gpu_streams[gpu_streams_idx(0)]);
        }

    }
    {

        {  // gpu_streams_synchronization
            cpf_gpu_check(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](a_host, std::align_val_t(64));
    gpu_streams = nullptr;
}


