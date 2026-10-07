/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0_a_host;
};

static DACE_HDFI constexpr int64_t gpu_streams_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t a_host_idx(int64_t __d0) { return __d0; }
void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
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
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    out_index_host = scal_out_index;
    {

        out_value_host = scal_out_value;  // copy_scal_out_value_to_out_value_host

    }
    {

        {  // copy_a_to_a_host
            hipMemcpyAsync((__state->__0_a_host), a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, gpu_streams[gpu_streams_idx(0)]);
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

    for (int64_t _loop_it_0 = 0; (_loop_it_0 < LEN_1D); _loop_it_0 = (_loop_it_0 + 1)) {

        a_index = __state->__0_a_host[_loop_it_0];

        if ((a_index > 1)) {

            out_index_host = _loop_it_0;
            {

                out_value_host = __state->__0_a_host[a_host_idx(_loop_it_0)];  // _assign_a_to_out_value

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
            DACE_GPU_CHECK(hipStreamSynchronize(gpu_streams[gpu_streams_idx(0)]));
        }

    }
    gpu_streams = nullptr;
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, out_index, out_value, LEN_1D);
}
DACE_EXPORTED int __dace_init_experimental_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_experimental_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_experimental_cuda(__state, LEN_1D);

    if (__result) {
        delete __state;
        return nullptr;
    }
    __state->__0_a_host = new (std::align_val_t(64)) double[LEN_1D];

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_gpu(canon_gpu_state_t *__state)
{

    int __err = 0;
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](__state->__0_a_host, std::align_val_t(64));

    int __err_experimental_cuda = __dace_exit_experimental_cuda(__state);
    if (__err_experimental_cuda) {
        __err = __err_experimental_cuda;
    }
    delete __state;
    return __err;
}
