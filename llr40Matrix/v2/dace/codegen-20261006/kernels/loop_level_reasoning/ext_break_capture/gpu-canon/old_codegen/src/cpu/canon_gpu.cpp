/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_gpu_state_t {
    dace::cuda::Context *gpu_context;
    double * __restrict__ __0_a_host;
};

void __program_canon_gpu_internal(canon_gpu_state_t*__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    int64_t scal_out_index;
    double scal_out_value;
    double out_value_host;
    int64_t out_index_host;
    int64_t _loop_it_0;
    double a_index;

    {

        DACE_GPU_CHECK(hipMemcpyAsync(&scal_out_index, out_index, 1 * sizeof(int64_t), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));
        DACE_GPU_CHECK(hipMemcpyAsync(&scal_out_value, out_value, 1 * sizeof(double), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

    }
    out_index_host = scal_out_index;
    {


        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &scal_out_value, &out_value_host, 1);
        DACE_GPU_CHECK(hipMemcpyAsync(__state->__0_a_host, a, LEN_1D * sizeof(double), hipMemcpyDeviceToHost, nullptr));
        DACE_GPU_CHECK(hipStreamSynchronize(nullptr));

    }
    {

        {

            ///////////////////
            // Tasklet code (check_assumption_0)
            if ((LEN_1D < 0)) {
                std::abort();
            }
            ///////////////////

        }

    }
    out_index_host = -1;
    {

        {
            double __out;

            ///////////////////
            // Tasklet code (assign_19_4)
            __out = -1.0;
            ///////////////////

            out_value_host = __out;
        }

    }

    for (_loop_it_0 = 0; (_loop_it_0 < LEN_1D); _loop_it_0 = (_loop_it_0 + 1)) {

        a_index = __state->__0_a_host[_loop_it_0];

        if ((a_index > 1)) {

            out_index_host = _loop_it_0;
            {

                {
                    double _in = __state->__0_a_host[_loop_it_0];
                    double _out;

                    ///////////////////
                    // Tasklet code (_assign_a_to_out_value)
                    _out = _in;
                    ///////////////////

                    out_value_host = _out;
                }

            }
            break;
        }



    }

    {

        {
            int64_t __out;

            ///////////////////
            // Tasklet code (symassign)
            __out = out_index_host;
            ///////////////////

            scal_out_index = __out;
        }
        DACE_GPU_CHECK(hipMemcpyAsync(out_index, &scal_out_index, 1 * sizeof(int64_t), hipMemcpyHostToDevice, nullptr));

        dace::CopyND<double, 1, false, 1>::template ConstDst<1>::Copy(
        &out_value_host, &scal_out_value, 1);
        DACE_GPU_CHECK(hipMemcpyAsync(out_value, &scal_out_value, 1 * sizeof(double), hipMemcpyHostToDevice, nullptr));

    }
}

DACE_EXPORTED void __dace_gpu_drain_error(canon_gpu_state_t *__state);
DACE_EXPORTED void __program_canon_gpu(canon_gpu_state_t *__state, double * __restrict__ a, int64_t * __restrict__ out_index, double * __restrict__ out_value, int64_t LEN_1D)
{
    __dace_gpu_drain_error(__state);
    __program_canon_gpu_internal(__state, a, out_index, out_value, LEN_1D);
}
DACE_EXPORTED int __dace_init_cuda(canon_gpu_state_t *__state, int64_t LEN_1D);
DACE_EXPORTED int __dace_exit_cuda(canon_gpu_state_t *__state);

DACE_EXPORTED canon_gpu_state_t *__dace_init_canon_gpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_gpu_state_t *__state = new canon_gpu_state_t();
    __result |= __dace_init_cuda(__state, LEN_1D);

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

    int __err_cuda = __dace_exit_cuda(__state);
    if (__err_cuda) {
        __err = __err_cuda;
    }
    delete __state;
    return __err;
}
