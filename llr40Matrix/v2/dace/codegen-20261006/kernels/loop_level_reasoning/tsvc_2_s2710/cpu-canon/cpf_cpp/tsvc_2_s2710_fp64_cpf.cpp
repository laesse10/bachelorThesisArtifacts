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
static constexpr inline int64_t b_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t d_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t a_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t c_idx(int64_t __d0) { return __d0; }
static constexpr inline int64_t e_idx(int64_t __d0) { return __d0; }
static inline void loop_body_0_1_0(const double* __restrict__ d, const double* __restrict__ e, const double* __restrict__ x, double* __restrict__ a, double* __restrict__ b, double* __restrict__ c, int64_t LEN_1D, int64_t _loop_it_0) {
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

extern "C" void tsvc_2_s2710_fp64(double * __restrict__ a, double * __restrict__ b, double * __restrict__ c, const double * __restrict__ d, const double * __restrict__ e, const double * __restrict__ x, int64_t LEN_1D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{

    {

        {  // check_assumption_0
            if ((LEN_1D < 0)) {
                std::abort();
            }
        }

    }

    // parallel -- the iterations are independent
    // settled: proven, no need to re-check; focus: schedule, tiling, vectorization, fusion, data layout
    #pragma omp parallel for
    for (int64_t _loop_it_0 = 0; _loop_it_0 < LEN_1D; _loop_it_0 += 1) {
        loop_body_0_1_0(&d[0], &e[0], &x[0], &a[0], &b[0], &c[0], LEN_1D, _loop_it_0);
    }
}
