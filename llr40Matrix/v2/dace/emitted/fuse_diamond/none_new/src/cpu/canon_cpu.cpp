/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t t_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t u_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t v_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    double* __restrict__ t = new (std::align_val_t(64)) double[LEN_1D];
    double* __restrict__ u = new (std::align_val_t(64)) double[LEN_1D];
    double* __restrict__ v = new (std::align_val_t(64)) double[LEN_1D];
    double a_index;
    double a_index_0;
    double t_slice;
    double t_index;
    double u_slice;
    double t_index_0;
    double v_slice;
    double u_index;
    double v_index;
    double out_slice;
    int64_t i;


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            a_index = a[a_idx(i)];  // copy_a_to_a_index
            a_index_0 = a[a_idx(i)];  // copy_a_to_a_index_0
            t_slice = (a_index * a_index_0);  // _Mult_
            t[t_idx(i)] = t_slice;  // assign_20_8

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            t_index = t[t_idx(i)];  // copy_t_to_t_index
            u_slice = (t_index + 1.0);  // _Add_
            u[u_idx(i)] = u_slice;  // assign_22_8

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            t_index_0 = t[t_idx(i)];  // copy_t_to_t_index_0
            v_slice = (t_index_0 - 1.0);  // _Sub_
            v[v_idx(i)] = v_slice;  // assign_24_8

        }

    }


    for (i = 0; (i < LEN_1D); i = (i + 1)) {
        {

            u_index = u[u_idx(i)];  // copy_u_to_u_index
            v_index = v[v_idx(i)];  // copy_v_to_v_index
            out_slice = (u_index * v_index);  // _Mult_
            out[out_idx(i)] = out_slice;  // assign_26_8

        }

    }


    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](t, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](u, std::align_val_t(64));
    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](v, std::align_val_t(64));
}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    __program_canon_cpu_internal(__state, a, out, LEN_1D);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t LEN_1D)
{

    int __result = 0;
    canon_cpu_state_t *__state = new canon_cpu_state_t();

    if (__result) {
        delete __state;
        return nullptr;
    }

    if (__result) {
        delete __state;
        return nullptr;
    }

    return __state;
}

DACE_EXPORTED int __dace_exit_canon_cpu(canon_cpu_state_t *__state)
{

    int __err = 0;
    delete __state;
    return __err;
}
