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
#include <limits>
#include <utility>

// Functions the DaCe runtime headers would otherwise provide.
template <typename T>
static constexpr inline void py_divmod(const T& numerator, const T& denominator, T& quotient,
                                       T& remainder) {
    if constexpr (std::is_floating_point_v<T>) {
        const T ratio = numerator / denominator;
        remainder = std::fmod(numerator, denominator);
        if (denominator == 0) {
            quotient = ratio;
            return;
        }
        quotient = (numerator - remainder) / denominator;
        if (remainder == 0) {
            remainder = std::copysign(static_cast<T>(0), denominator);
        } else if ((denominator < 0) != (remainder < 0)) {
            remainder += denominator;
            quotient -= 1;
        }
        if (quotient == 0) {
            quotient = std::copysign(static_cast<T>(0), ratio);
        } else {
            const T floored = std::floor(quotient);
            quotient = quotient - floored > static_cast<T>(0.5) ? floored + 1 : floored;
        }
    } else if (denominator == 0) {
        quotient = 0;
        remainder = 0;
    } else if (numerator == std::numeric_limits<T>::min() && denominator == static_cast<T>(-1)) {
        quotient = numerator;
        remainder = 0;
    } else {
        quotient = static_cast<T>(numerator / denominator);
        remainder = static_cast<T>(numerator % denominator);
        if (remainder != 0 && ((remainder < 0) != (denominator < 0))) {
            quotient = static_cast<T>(quotient - 1);
            remainder = static_cast<T>(remainder + denominator);
        }
    }
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R py_floor(const T& numerator, const U& denominator) {
    R quotient = 0;
    R remainder = 0;
    py_divmod(static_cast<R>(numerator), static_cast<R>(denominator), quotient, remainder);
    return quotient;
}
template <typename T, typename U, typename R = decltype(std::declval<T>() / std::declval<U>())>
static constexpr inline R py_mod(const T& numerator, const U& denominator) {
    R quotient = 0;
    R remainder = 0;
    py_divmod(static_cast<R>(numerator), static_cast<R>(denominator), quotient, remainder);
    return remainder;
}
static constexpr inline int64_t bb_idx(int64_t __d0, int64_t __d1) { return ((2 * __d0) + __d1); }
extern "C" void tsvc_2_s3110_fp64(const double * __restrict__ aa, double * __restrict__ bb, int64_t LEN_2D, const uint8_t * __restrict__ workspace, int64_t workspace_size)
{
    double _argmax2d_val_for_19;
    int64_t _argmax2d_idx_for_19;
    double maxv;
    int64_t xindex;
    int64_t yindex;

    {

        {  // check_assumption_0
            if ((LEN_2D < 0)) {
                std::abort();
            }
        }

    }
    {

        // argument reduction: index of the element the reduction selected
        {  // for_19_argreduce2d_openmp
            const double* __restrict__ _in = &aa[0];
            int64_t _out_idx;
            double _out_val;
            struct __ar_pair { double __ar_v; int64_t __ar_i; };
            #pragma omp declare reduction(__ar_best_op : struct __ar_pair : \
            omp_out = (omp_in.__ar_v > omp_out.__ar_v || (omp_in.__ar_v == omp_out.__ar_v && omp_in.__ar_i < omp_out.__ar_i)) ? omp_in : omp_out) \
            initializer(omp_priv = omp_orig)
            struct __ar_pair __ar_best;
            __ar_best.__ar_v = _in[0]; __ar_best.__ar_i = 0;
            #pragma omp parallel for reduction(__ar_best_op : __ar_best)
            for (int64_t __i = 1; __i < ((LEN_2D * LEN_2D)); ++__i) {
                const double __v = _in[__i];
                if (__v > __ar_best.__ar_v) { __ar_best.__ar_v = __v; __ar_best.__ar_i = __i; }
            }
            _out_val = __ar_best.__ar_v;
            _out_idx = __ar_best.__ar_i;
            _argmax2d_idx_for_19 = _out_idx;
            _argmax2d_val_for_19 = _out_val;
        }

    }
    maxv = _argmax2d_val_for_19;
    xindex = py_floor(_argmax2d_idx_for_19, LEN_2D);
    yindex = py_mod(_argmax2d_idx_for_19, LEN_2D);
    {
        double float_xindex;
        double maxv_plus_expr;
        double float_yindex;

        float_xindex = double(xindex);  // _convert_to_float64_
        maxv_plus_expr = (maxv + float_xindex);  // _Add_
        float_yindex = double(yindex);  // _convert_to_float64_
        bb[bb_idx(0, 0)] = (maxv_plus_expr + float_yindex);  // _Add_

    }
}
