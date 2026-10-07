/* DaCe AUTO-GENERATED FILE. DO NOT MODIFY */
#include <dace/dace.h>
#include "../../include/hash.h"

struct canon_cpu_state_t {

};

static DACE_HDFI constexpr int64_t val_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t w_idx(int64_t __d0) { return __d0; }
static DACE_HDFI constexpr int64_t out_idx(int64_t __d0) { return __d0; }
void __program_canon_cpu_internal(canon_cpu_state_t*__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{
    double acc;
    double val_index;
    double w_index;
    double val_slice_times_w_slice;
    double acc_plus_val_slice_w_slice;
    int64_t row_ptr_index;
    int64_t row_ptr_index_0;


    for (int64_t s = 0; (s < NSEG); s = (s + 1)) {
        {

            acc = 0.0;  // assign_17_8

        }
        row_ptr_index = row_ptr[s];
        row_ptr_index_0 = row_ptr[(s + 1)];

        for (int64_t e = row_ptr_index; (e < row_ptr_index_0); e = (e + 1)) {
            {

                val_index = val[val_idx(e)];  // copy_val_to_val_index
                w_index = w[w_idx(e)];  // copy_w_to_w_index
                val_slice_times_w_slice = (val_index * w_index);  // _Mult_
                acc_plus_val_slice_w_slice = (acc + val_slice_times_w_slice);  // _Add_
                acc = acc_plus_val_slice_w_slice;  // assign_19_12

            }

        }

        {

            out[out_idx(s)] = acc;  // assign_20_8

        }

    }

}

DACE_EXPORTED void __program_canon_cpu(canon_cpu_state_t *__state, double * __restrict__ out, int64_t * __restrict__ row_ptr, double * __restrict__ val, double * __restrict__ w, int64_t NSEG)
{
    __program_canon_cpu_internal(__state, out, row_ptr, val, w, NSEG);
}

DACE_EXPORTED canon_cpu_state_t *__dace_init_canon_cpu(int64_t NSEG)
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
