/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t a_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t tmp_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ a, double * __restrict__ out, int64_t LEN_1D)
{
    double* __restrict__ tmp = new (std::align_val_t(64)) double[LEN_1D];
    double a_index;
    double a_index_0;
    double a_slice_plus_a_slice;
    double a_index_1;
    double tmp_slice;
    double tmp_index;
    double tmp_index_0;
    double out_slice;
    int64_t i;


    for (i = 1; (i < (LEN_1D - 1)); i = (i + 1)) {
        {

            a_index = a[a_idx((i - 1))];  // copy_a_to_a_index
            a_index_0 = a[a_idx(i)];  // copy_a_to_a_index_0
            a_slice_plus_a_slice = (a_index + a_index_0);  // _Add_
            a_index_1 = a[a_idx((i + 1))];  // copy_a_to_a_index_1
            tmp_slice = (a_slice_plus_a_slice + a_index_1);  // _Add_
            tmp[tmp_idx(i)] = tmp_slice;  // assign_18_8

        }

    }


    for (i = 1; (i < (LEN_1D - 2)); i = (i + 1)) {
        {

            tmp_index = tmp[tmp_idx(i)];  // copy_tmp_to_tmp_index
            tmp_index_0 = tmp[tmp_idx((i + 1))];  // copy_tmp_to_tmp_index_0
            out_slice = (tmp_index * tmp_index_0);  // _Mult_
            out[out_idx(i)] = out_slice;  // assign_20_8

        }

    }


    static_assert(std::is_trivially_destructible<double>::value, "aligned heap deallocation skips destructors");
    ::operator delete[](tmp, std::align_val_t(64));
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
