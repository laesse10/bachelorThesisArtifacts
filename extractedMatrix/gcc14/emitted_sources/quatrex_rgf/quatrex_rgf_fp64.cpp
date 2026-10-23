// hpcagent_bench-autogen -- generated from quatrex_rgf_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
#include <cstdint>
#include <cmath>
#include <type_traits>
#include <cstring>
#include <cstdlib>
#ifndef NPB_HD
#if defined(__HIPCC__) || defined(__CUDACC__)
#define NPB_HD __host__ __device__
#else
#define NPB_HD
#endif
#endif
// Math constants as typed constexpr values. ``<cmath>`` may
// predefine M_PI / M_E as macros (glibc __USE_MISC); undefine
// them so the names rebind to our constexpr values -- we emit no
// macro DEFINITION, only remove the platform ones.
// [[maybe_unused]]: namespace-scope constexpr has internal linkage, so a
// kernel that references neither draws -Wunused-const-variable from clang
// (the C prelude spells these as macros and never does). They are prelude
// vocabulary offered to every kernel, which is exactly this attribute.
#ifdef M_PI
#undef M_PI
#endif
#ifdef M_E
#undef M_E
#endif
[[maybe_unused]] constexpr double M_PI = 3.14159265358979323846;
[[maybe_unused]] constexpr double M_E  = 2.71828182845904523536;
// Complex support via the GCC/Clang ``double _Complex`` extension
// (no <complex.h>, so no name clashes). The imaginary unit and
// the C99-named helpers are constexpr/inline FUNCTIONS, not macros.
constexpr NPB_HD double creal(double _Complex z) { return __real__ z; }
constexpr NPB_HD double cimag(double _Complex z) { return __imag__ z; }
inline NPB_HD double _Complex __npb_make_complex(double re, double im) {
    double _Complex z; __real__ z = re; __imag__ z = im; return z;
}
static const double _Complex _Complex_I = __npb_make_complex(0.0, 1.0);
inline NPB_HD double cabs(double _Complex z) {
    return sqrt(creal(z)*creal(z) + cimag(z)*cimag(z));
}
inline NPB_HD double carg(double _Complex z) { return atan2(cimag(z), creal(z)); }
/* ``cexp(z) = exp(re) * (cos(im) + i*sin(im))``. */
inline NPB_HD double _Complex cexp(double _Complex z) {
    return __npb_make_complex(exp(creal(z))*cos(cimag(z)),
                             exp(creal(z))*sin(cimag(z)));
}
/* ``clog(z) = log(|z|) + i*arg(z)``. */
inline NPB_HD double _Complex clog(double _Complex z) {
    return __npb_make_complex(log(cabs(z)), carg(z));
}
/* ``csqrt(z) = exp((1/2) * log(z))`` -- principal branch. */
inline NPB_HD double _Complex csqrt(double _Complex z) {
    double _Complex l = clog(z);
    return cexp(__npb_make_complex(0.5*creal(l), 0.5*cimag(l)));
}
/* ``cpow(z, w) = exp(w * log(z))`` -- general complex pow. */
inline NPB_HD double _Complex cpow(double _Complex z, double _Complex w) {
    double _Complex l = clog(z);
    return cexp(__npb_make_complex(
        creal(w)*creal(l) - cimag(w)*cimag(l),
        creal(w)*cimag(l) + cimag(w)*creal(l)));
}
/* ``z.conjugate()`` -- complex-conjugate scalar helper. */
inline NPB_HD double _Complex __npb_conj(double _Complex z) {
    return __npb_make_complex(creal(z), -cimag(z));
}
/* Integer power for VLA shape bounds. */
constexpr NPB_HD int64_t __npb_int_pow(int64_t base, int64_t exp) {
    int64_t result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}
/* Ternary-form ``max`` / ``min`` as constexpr function templates
 * so a mixed call like ``max(double, int)`` promotes the int
 * operand via the usual arithmetic conversions (``std::max``
 * would require both args to share a type). They PROPAGATE NaN (a
 * NaN in EITHER operand yields NaN): these serve the elementwise
 * ``np.maximum``/``np.minimum`` broadcast and the ``np.maximum.at`` /
 * ``np.minimum.at`` scatter folds, which follow numpy (propagate),
 * not Python builtin max. For finite operands the result is the
 * larger/smaller -- so the 3-way builtin max (needleman_wunsch,
 * always finite) is unchanged; integer NaN tests are dead. */
template <class A, class B>
constexpr NPB_HD auto max(A a, B b) { return a != a ? a : (b != b ? b : (b > a ? b : a)); }
template <class A, class B>
constexpr NPB_HD auto min(A a, B b) { return a != a ? a : (b != b ? b : (b < a ? b : a)); }
/* Elementwise ``np.maximum``/``np.minimum`` lower to ``fmax``/``fmin``;
 * libm ``fmax``/``fmin`` SUPPRESS NaN but numpy PROPAGATES it. These
 * single-evaluation helpers return NaN when either operand is NaN.
 * Integral operands take the exact integer compare (the same INTEGRAL/floating
 * split int_floor makes): converting them to double rounds anything above 2**53,
 * so min(2**53 + 1, 2**53 + 2) came back 2**53 -- a value neither operand had. */
template <class A, class B>
constexpr NPB_HD auto __npb_fmax(A a, B b) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        return a > b ? a : b;
    } else {
        return a != a ? a : (b != b ? b : (a > b ? a : b));
    }
}
template <class A, class B>
constexpr NPB_HD auto __npb_fmin(A a, B b) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        return a < b ? a : b;
    } else {
        return a != a ? a : (b != b ? b : (a < b ? a : b));
    }
}
/* ``np.sign``: numpy ``sign(nan) == nan`` and ``sign(0) == 0``. The
 * naive ``(x>0)-(x<0)`` gives 0 for NaN and evaluates ``x`` twice. */
inline NPB_HD double __npb_sign(double x) {
    return x != x ? x : (double)((x > 0) - (x < 0));
}
/* Python ``//`` floors toward -inf; C++ ``/`` truncates toward zero.
 * C++ has no built-in floor-division, so it is always this helper. The
 * INTEGRAL/floating split is decided by the operand TYPE here rather than
 * inferred from the source AST -- guessing it wrong emitted a no-op floor
 * over an already-truncated integer quotient. */
template <class A, class B>
constexpr NPB_HD auto int_floor(A a, B b) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        return a / b - ((a % b != 0) && ((a < 0) ^ (b < 0)));
    } else {
        return std::floor(static_cast<double>(a) / static_cast<double>(b));
    }
}
/* Ceil-division counterpart (toward +inf), exact for both signs -- unlike
 * the ``(a + b - 1) / b`` idiom, which holds only for a positive divisor
 * and overflows near the integer maximum. */
template <class A, class B>
constexpr NPB_HD auto int_ceil(A a, B b) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        return a / b + ((a % b != 0) && ((a < 0) == (b < 0)));
    } else {
        return std::ceil(static_cast<double>(a) / static_cast<double>(b));
    }
}
/* Python ``%`` returns the sign of the divisor; C/C++ the dividend.
 * Same type-dispatch as int_floor (floating operands need npy_remainder,
 * which the integer form cannot express on doubles). */
template <class A, class B>
constexpr NPB_HD auto python_mod(A a, B b) {
    if constexpr (std::is_integral_v<A> && std::is_integral_v<B>) {
        return (a % b + b) % b;
    } else {
        double m = std::fmod(static_cast<double>(a), static_cast<double>(b));
        if (m != 0.0 && ((b < 0.0) != (m < 0.0))) m += static_cast<double>(b);
        return m;
    }
}
/* Floating-point ``%``: numpy floored modulo (sign of the divisor),
 * which integer ``python_mod`` cannot express on doubles. Mirrors
 * numpy ``npy_remainder`` (fmod + sign-of-divisor fixup). */
inline NPB_HD double python_fmod(double a, double b) {
    double m = std::fmod(a, b);
    if (m != 0.0 && ((b < 0.0) != (m < 0.0))) m += b;
    return m;
}

extern "C" {

void quatrex_rgf_fp64(const double _Complex *__restrict__ a_diag, const double _Complex *__restrict__ a_lower, const double _Complex *__restrict__ a_upper, const double _Complex *__restrict__ sigma_greater_diag, const double _Complex *__restrict__ sigma_greater_upper, const double _Complex *__restrict__ sigma_lesser_diag, const double _Complex *__restrict__ sigma_lesser_upper, double _Complex *__restrict__ x_greater_diag, double _Complex *__restrict__ x_greater_lower, double _Complex *__restrict__ x_greater_upper, double _Complex *__restrict__ x_lesser_diag, double _Complex *__restrict__ x_lesser_lower, double _Complex *__restrict__ x_lesser_upper, double _Complex *__restrict__ x_retarded_diag, const int64_t BS, const int64_t NB, const int64_t NE) {
        int64_t __inv_p;
        double _Complex __inv_factor;
        int64_t j;
        double _Complex __inv_tmp;
        double _Complex *xr_d = (double _Complex *)malloc((size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        memset(xr_d, 0, (size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xl_d = (double _Complex *)malloc((size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        memset(xl_d, 0, (size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xg_d = (double _Complex *)malloc((size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        memset(xg_d, 0, (size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        double _Complex *m = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(m, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *dag = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *a_ji_dag = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(a_ji_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr_jj_dag = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_jj_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *t1 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(t1, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *t2 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(t2, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *t3 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(t3, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr_ii_a_ij = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_ii_a_ij, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr_jj_a_ji = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_jj_a_ji, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr_ii_a_ij_xr_jj = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_ii_a_ij_xr_jj, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr_ii_a_ij_xr_jj_a_ji = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_ii_a_ij_xr_jj_a_ji, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *a_ij_dag_xr_ii_dag = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(a_ij_dag_xr_ii_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *a_ji_dag_xr_jj_dag = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(a_ji_dag_xr_jj_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr_jj_dag_a_ij_dag_xr_ii_dag = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_jj_dag_a_ij_dag_xr_ii_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *temp_1x = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(temp_1x, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *temp_2x = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(temp_2x, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *cj = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(cj, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb1 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb2 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm3 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm4 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm5 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm6 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb7 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm8 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm9 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb10 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb11 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm12 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb13 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm14 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm15 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm16 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm17 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm18 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb19 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm20 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm21 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm22 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm23 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb24 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb25 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb26 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm27 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb28 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm29 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb30 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm31 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb32 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm33 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm34 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm35 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm36 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb37 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm38 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm39 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm40 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm41 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb42 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm43 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb44 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm45 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm46 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm47 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb48 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm49 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm50 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm51 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm52 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb53 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm54 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__cb55 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__mm56 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__inv_aw0 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *__inv_aw1 = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        double _Complex *xr = (double _Complex *)malloc((size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_d, 0, (size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        memset(xl_d, 0, (size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        memset(xg_d, 0, (size_t)((NB) * (BS) * (BS)) * sizeof(double _Complex));
        memset(m, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(a_ji_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_jj_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(t1, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(t2, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(t3, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_ii_a_ij, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_jj_a_ji, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_ii_a_ij_xr_jj, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_ii_a_ij_xr_jj_a_ji, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(a_ij_dag_xr_ii_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(a_ji_dag_xr_jj_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(xr_jj_dag_a_ij_dag_xr_ii_dag, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(temp_1x, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(temp_2x, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        memset(cj, 0, (size_t)((BS) * (BS)) * sizeof(double _Complex));
        for (int64_t e = 0; e < NE; ++e) {
          for (int64_t si0 = 0; si0 < BS; ++si0) {
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              m[(si0)*(BS) + (si1)] = a_diag[(((e)*(NB) + (0))*(BS) + (si0))*(BS) + (si1)];
            }
          }
          /* numpy: np.linalg.inv(m) */
          for (int64_t __inv_i = 0; __inv_i < BS; ++__inv_i) {
            for (int64_t __inv_j = 0; __inv_j < BS; ++__inv_j) {
              __inv_aw0[(__inv_i)*(BS) + (__inv_j)] = m[(__inv_i)*(BS) + (__inv_j)];
              __cb1[(__inv_i)*(BS) + (__inv_j)] = ((__inv_i == __inv_j) ? 1.0 : 0.0);
            }
          }
          for (int64_t __inv_k = 0; __inv_k < BS; ++__inv_k) {
            __inv_p = __inv_k;
            for (int64_t __inv_r = (__inv_k + 1); __inv_r < BS; ++__inv_r) {
              if ((cabs(__inv_aw0[(__inv_r)*(BS) + (__inv_k)]) > cabs(__inv_aw0[(__inv_p)*(BS) + (__inv_k)]))) {
                __inv_p = __inv_r;
              }
            }
            for (int64_t __inv_c = 0; __inv_c < BS; ++__inv_c) {
              __inv_tmp = __inv_aw0[(__inv_k)*(BS) + (__inv_c)];
              __inv_aw0[(__inv_k)*(BS) + (__inv_c)] = __inv_aw0[(__inv_p)*(BS) + (__inv_c)];
              __inv_aw0[(__inv_p)*(BS) + (__inv_c)] = __inv_tmp;
              __inv_tmp = __cb1[(__inv_k)*(BS) + (__inv_c)];
              __cb1[(__inv_k)*(BS) + (__inv_c)] = __cb1[(__inv_p)*(BS) + (__inv_c)];
              __cb1[(__inv_p)*(BS) + (__inv_c)] = __inv_tmp;
            }
            __inv_factor = __inv_aw0[(__inv_k)*(BS) + (__inv_k)];
            for (int64_t __inv_c = 0; __inv_c < BS; ++__inv_c) {
              __cb1[(__inv_k)*(BS) + (__inv_c)] = (__cb1[(__inv_k)*(BS) + (__inv_c)] / __inv_factor);
              __inv_aw0[(__inv_k)*(BS) + (__inv_c)] = (__inv_aw0[(__inv_k)*(BS) + (__inv_c)] / __inv_factor);
            }
            for (int64_t __inv_r = 0; __inv_r < BS; ++__inv_r) {
              if ((__inv_r != __inv_k)) {
                __inv_factor = __inv_aw0[(__inv_r)*(BS) + (__inv_k)];
                for (int64_t __inv_c = 0; __inv_c < BS; ++__inv_c) {
                  __cb1[(__inv_r)*(BS) + (__inv_c)] = (__cb1[(__inv_r)*(BS) + (__inv_c)] - (__inv_factor * __cb1[(__inv_k)*(BS) + (__inv_c)]));
                  __inv_aw0[(__inv_r)*(BS) + (__inv_c)] = (__inv_aw0[(__inv_r)*(BS) + (__inv_c)] - (__inv_factor * __inv_aw0[(__inv_k)*(BS) + (__inv_c)]));
                }
              }
            }
          }
          for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
            for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
              xr[(__w0)*(BS) + (__w1)] = __cb1[(__w0)*(BS) + (__w1)];
            }
          }
          for (int64_t si1 = 0; si1 < BS; ++si1) {
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              xr_d[((0)*(BS) + (si1))*(BS) + (si2)] = xr[(si1)*(BS) + (si2)];
            }
          }
          for (int64_t si0 = 0; si0 < BS; ++si0) {
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              cj[(si0)*(BS) + (si1)] = __npb_conj(xr[(si0)*(BS) + (si1)]);
            }
          }
          /* numpy: np.transpose(cj) */
          for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
            for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
              __cb2[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
            for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
              xr_jj_dag[(__w0)*(BS) + (__w1)] = __cb2[(__w0)*(BS) + (__w1)];
            }
          }
          for (int64_t __mmi3 = 0; __mmi3 < BS; ++__mmi3) {
            for (int64_t __mmj3 = 0; __mmj3 < BS; ++__mmj3) {
              __mm3[(__mmi3)*(BS) + (__mmj3)] = 0.0;
              for (int64_t __mml3 = 0; __mml3 < BS; ++__mml3) {
                __mm3[(__mmi3)*(BS) + (__mmj3)] += (xr[(__mmi3)*(BS) + (__mml3)] * sigma_lesser_diag[(((e)*(NB) + (0))*(BS) + (__mml3))*(BS) + (__mmj3)]);
              }
            }
          }
          for (int64_t __i = 0; __i < BS; ++__i) {
            for (int64_t __j = 0; __j < BS; ++__j) {
              __mm4[(__i)*(BS) + (__j)] = 0.0;
              for (int64_t __l = 0; __l < BS; ++__l) {
                __mm4[(__i)*(BS) + (__j)] += (__mm3[(__i)*(BS) + (__l)] * xr_jj_dag[(__l)*(BS) + (__j)]);
              }
            }
          }
          for (int64_t si1 = 0; si1 < BS; ++si1) {
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              xl_d[((0)*(BS) + (si1))*(BS) + (si2)] = __mm4[(si1)*(BS) + (si2)];
            }
          }
          for (int64_t __mmi5 = 0; __mmi5 < BS; ++__mmi5) {
            for (int64_t __mmj5 = 0; __mmj5 < BS; ++__mmj5) {
              __mm5[(__mmi5)*(BS) + (__mmj5)] = 0.0;
              for (int64_t __mml5 = 0; __mml5 < BS; ++__mml5) {
                __mm5[(__mmi5)*(BS) + (__mmj5)] += (xr[(__mmi5)*(BS) + (__mml5)] * sigma_greater_diag[(((e)*(NB) + (0))*(BS) + (__mml5))*(BS) + (__mmj5)]);
              }
            }
          }
          for (int64_t __i = 0; __i < BS; ++__i) {
            for (int64_t __j = 0; __j < BS; ++__j) {
              __mm6[(__i)*(BS) + (__j)] = 0.0;
              for (int64_t __l = 0; __l < BS; ++__l) {
                __mm6[(__i)*(BS) + (__j)] += (__mm5[(__i)*(BS) + (__l)] * xr_jj_dag[(__l)*(BS) + (__j)]);
              }
            }
          }
          for (int64_t si1 = 0; si1 < BS; ++si1) {
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              xg_d[((0)*(BS) + (si1))*(BS) + (si2)] = __mm6[(si1)*(BS) + (si2)];
            }
          }
          for (int64_t i = 0; i < (NB - 1); ++i) {
            j = (i + 1);
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(a_lower[(((e)*((NB - 1)) + (i))*(BS) + (si0))*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb7[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                a_ji_dag[(__w0)*(BS) + (__w1)] = __cb7[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi8 = 0; __mmi8 < BS; ++__mmi8) {
              for (int64_t __mmj8 = 0; __mmj8 < BS; ++__mmj8) {
                __mm8[(__mmi8)*(BS) + (__mmj8)] = 0.0;
                for (int64_t __mml8 = 0; __mml8 < BS; ++__mml8) {
                  __mm8[(__mmi8)*(BS) + (__mmj8)] += (a_lower[(((e)*((NB - 1)) + (i))*(BS) + (__mmi8))*(BS) + (__mml8)] * xr_d[((i)*(BS) + (__mml8))*(BS) + (__mmj8)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t1[(si0)*(BS) + (si1)] = __mm8[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t __mmi9 = 0; __mmi9 < BS; ++__mmi9) {
              for (int64_t __mmj9 = 0; __mmj9 < BS; ++__mmj9) {
                __mm9[(__mmi9)*(BS) + (__mmj9)] = 0.0;
                for (int64_t __mml9 = 0; __mml9 < BS; ++__mml9) {
                  __mm9[(__mmi9)*(BS) + (__mmj9)] += (t1[(__mmi9)*(BS) + (__mml9)] * a_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml9))*(BS) + (__mmj9)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                m[(si0)*(BS) + (si1)] = (a_diag[(((e)*(NB) + (j))*(BS) + (si0))*(BS) + (si1)] - __mm9[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.linalg.inv(m) */
            for (int64_t __inv_i = 0; __inv_i < BS; ++__inv_i) {
              for (int64_t __inv_j = 0; __inv_j < BS; ++__inv_j) {
                __inv_aw1[(__inv_i)*(BS) + (__inv_j)] = m[(__inv_i)*(BS) + (__inv_j)];
                __cb10[(__inv_i)*(BS) + (__inv_j)] = ((__inv_i == __inv_j) ? 1.0 : 0.0);
              }
            }
            for (int64_t __inv_k = 0; __inv_k < BS; ++__inv_k) {
              __inv_p = __inv_k;
              for (int64_t __inv_r = (__inv_k + 1); __inv_r < BS; ++__inv_r) {
                if ((cabs(__inv_aw1[(__inv_r)*(BS) + (__inv_k)]) > cabs(__inv_aw1[(__inv_p)*(BS) + (__inv_k)]))) {
                  __inv_p = __inv_r;
                }
              }
              for (int64_t __inv_c = 0; __inv_c < BS; ++__inv_c) {
                __inv_tmp = __inv_aw1[(__inv_k)*(BS) + (__inv_c)];
                __inv_aw1[(__inv_k)*(BS) + (__inv_c)] = __inv_aw1[(__inv_p)*(BS) + (__inv_c)];
                __inv_aw1[(__inv_p)*(BS) + (__inv_c)] = __inv_tmp;
                __inv_tmp = __cb10[(__inv_k)*(BS) + (__inv_c)];
                __cb10[(__inv_k)*(BS) + (__inv_c)] = __cb10[(__inv_p)*(BS) + (__inv_c)];
                __cb10[(__inv_p)*(BS) + (__inv_c)] = __inv_tmp;
              }
              __inv_factor = __inv_aw1[(__inv_k)*(BS) + (__inv_k)];
              for (int64_t __inv_c = 0; __inv_c < BS; ++__inv_c) {
                __cb10[(__inv_k)*(BS) + (__inv_c)] = (__cb10[(__inv_k)*(BS) + (__inv_c)] / __inv_factor);
                __inv_aw1[(__inv_k)*(BS) + (__inv_c)] = (__inv_aw1[(__inv_k)*(BS) + (__inv_c)] / __inv_factor);
              }
              for (int64_t __inv_r = 0; __inv_r < BS; ++__inv_r) {
                if ((__inv_r != __inv_k)) {
                  __inv_factor = __inv_aw1[(__inv_r)*(BS) + (__inv_k)];
                  for (int64_t __inv_c = 0; __inv_c < BS; ++__inv_c) {
                    __cb10[(__inv_r)*(BS) + (__inv_c)] = (__cb10[(__inv_r)*(BS) + (__inv_c)] - (__inv_factor * __cb10[(__inv_k)*(BS) + (__inv_c)]));
                    __inv_aw1[(__inv_r)*(BS) + (__inv_c)] = (__inv_aw1[(__inv_r)*(BS) + (__inv_c)] - (__inv_factor * __inv_aw1[(__inv_k)*(BS) + (__inv_c)]));
                  }
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                xr[(__w0)*(BS) + (__w1)] = __cb10[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              for (int64_t si2 = 0; si2 < BS; ++si2) {
                xr_d[((j)*(BS) + (si1))*(BS) + (si2)] = xr[(si1)*(BS) + (si2)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(xr[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb11[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                xr_jj_dag[(__w0)*(BS) + (__w1)] = __cb11[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi12 = 0; __mmi12 < BS; ++__mmi12) {
              for (int64_t __mmj12 = 0; __mmj12 < BS; ++__mmj12) {
                __mm12[(__mmi12)*(BS) + (__mmj12)] = 0.0;
                for (int64_t __mml12 = 0; __mml12 < BS; ++__mml12) {
                  __mm12[(__mmi12)*(BS) + (__mmj12)] += (t1[(__mmi12)*(BS) + (__mml12)] * sigma_lesser_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml12))*(BS) + (__mmj12)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t2[(si0)*(BS) + (si1)] = __mm12[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t2[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb13[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb13[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi14 = 0; __mmi14 < BS; ++__mmi14) {
              for (int64_t __mmj14 = 0; __mmj14 < BS; ++__mmj14) {
                __mm14[(__mmi14)*(BS) + (__mmj14)] = 0.0;
                for (int64_t __mml14 = 0; __mml14 < BS; ++__mml14) {
                  __mm14[(__mmi14)*(BS) + (__mmj14)] += (a_lower[(((e)*((NB - 1)) + (i))*(BS) + (__mmi14))*(BS) + (__mml14)] * xl_d[((i)*(BS) + (__mml14))*(BS) + (__mmj14)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm15[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm15[(__i)*(BS) + (__j)] += (__mm14[(__i)*(BS) + (__l)] * a_ji_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t3[(si0)*(BS) + (si1)] = (((sigma_lesser_diag[(((e)*(NB) + (j))*(BS) + (si0))*(BS) + (si1)] + __mm15[(si0)*(BS) + (si1)]) + dag[(si0)*(BS) + (si1)]) - t2[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm16[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm16[(__i)*(BS) + (__j)] += (xr[(__i)*(BS) + (__l)] * t3[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm17[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm17[(__i)*(BS) + (__j)] += (__mm16[(__i)*(BS) + (__l)] * xr_jj_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              for (int64_t si2 = 0; si2 < BS; ++si2) {
                xl_d[((j)*(BS) + (si1))*(BS) + (si2)] = __mm17[(si1)*(BS) + (si2)];
              }
            }
            for (int64_t __mmi18 = 0; __mmi18 < BS; ++__mmi18) {
              for (int64_t __mmj18 = 0; __mmj18 < BS; ++__mmj18) {
                __mm18[(__mmi18)*(BS) + (__mmj18)] = 0.0;
                for (int64_t __mml18 = 0; __mml18 < BS; ++__mml18) {
                  __mm18[(__mmi18)*(BS) + (__mmj18)] += (t1[(__mmi18)*(BS) + (__mml18)] * sigma_greater_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml18))*(BS) + (__mmj18)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t2[(si0)*(BS) + (si1)] = __mm18[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t2[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb19[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb19[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi20 = 0; __mmi20 < BS; ++__mmi20) {
              for (int64_t __mmj20 = 0; __mmj20 < BS; ++__mmj20) {
                __mm20[(__mmi20)*(BS) + (__mmj20)] = 0.0;
                for (int64_t __mml20 = 0; __mml20 < BS; ++__mml20) {
                  __mm20[(__mmi20)*(BS) + (__mmj20)] += (a_lower[(((e)*((NB - 1)) + (i))*(BS) + (__mmi20))*(BS) + (__mml20)] * xg_d[((i)*(BS) + (__mml20))*(BS) + (__mmj20)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm21[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm21[(__i)*(BS) + (__j)] += (__mm20[(__i)*(BS) + (__l)] * a_ji_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t3[(si0)*(BS) + (si1)] = (((sigma_greater_diag[(((e)*(NB) + (j))*(BS) + (si0))*(BS) + (si1)] + __mm21[(si0)*(BS) + (si1)]) + dag[(si0)*(BS) + (si1)]) - t2[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm22[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm22[(__i)*(BS) + (__j)] += (xr[(__i)*(BS) + (__l)] * t3[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm23[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm23[(__i)*(BS) + (__j)] += (__mm22[(__i)*(BS) + (__l)] * xr_jj_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              for (int64_t si2 = 0; si2 < BS; ++si2) {
                xg_d[((j)*(BS) + (si1))*(BS) + (si2)] = __mm23[(si1)*(BS) + (si2)];
              }
            }
          }
          for (int64_t si0 = 0; si0 < BS; ++si0) {
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              cj[(si0)*(BS) + (si1)] = __npb_conj(xl_d[(((NB - 1))*(BS) + (si0))*(BS) + (si1)]);
            }
          }
          /* numpy: np.transpose(cj) */
          for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
            for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
              __cb24[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
            for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
              dag[(__w0)*(BS) + (__w1)] = __cb24[(__w0)*(BS) + (__w1)];
            }
          }
          for (int64_t si2 = 0; si2 < BS; ++si2) {
            for (int64_t si3 = 0; si3 < BS; ++si3) {
              x_lesser_diag[(((e)*(NB) + ((NB - 1)))*(BS) + (si2))*(BS) + (si3)] = (0.5 * (xl_d[(((NB - 1))*(BS) + (si2))*(BS) + (si3)] - dag[(si2)*(BS) + (si3)]));
            }
          }
          for (int64_t si0 = 0; si0 < BS; ++si0) {
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              cj[(si0)*(BS) + (si1)] = __npb_conj(xg_d[(((NB - 1))*(BS) + (si0))*(BS) + (si1)]);
            }
          }
          /* numpy: np.transpose(cj) */
          for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
            for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
              __cb25[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
            for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
              dag[(__w0)*(BS) + (__w1)] = __cb25[(__w0)*(BS) + (__w1)];
            }
          }
          for (int64_t si2 = 0; si2 < BS; ++si2) {
            for (int64_t si3 = 0; si3 < BS; ++si3) {
              x_greater_diag[(((e)*(NB) + ((NB - 1)))*(BS) + (si2))*(BS) + (si3)] = (0.5 * (xg_d[(((NB - 1))*(BS) + (si2))*(BS) + (si3)] - dag[(si2)*(BS) + (si3)]));
            }
          }
          for (int64_t si2 = 0; si2 < BS; ++si2) {
            for (int64_t si3 = 0; si3 < BS; ++si3) {
              x_retarded_diag[(((e)*(NB) + ((NB - 1)))*(BS) + (si2))*(BS) + (si3)] = xr_d[(((NB - 1))*(BS) + (si2))*(BS) + (si3)];
            }
          }
          for (int64_t i = (NB - 2); i > -1; --i) {
            j = (i + 1);
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(xr_d[((j)*(BS) + (si0))*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb26[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                xr_jj_dag[(__w0)*(BS) + (__w1)] = __cb26[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi27 = 0; __mmi27 < BS; ++__mmi27) {
              for (int64_t __mmj27 = 0; __mmj27 < BS; ++__mmj27) {
                __mm27[(__mmi27)*(BS) + (__mmj27)] = 0.0;
                for (int64_t __mml27 = 0; __mml27 < BS; ++__mml27) {
                  __mm27[(__mmi27)*(BS) + (__mmj27)] += (xr_d[((i)*(BS) + (__mmi27))*(BS) + (__mml27)] * a_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml27))*(BS) + (__mmj27)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                xr_ii_a_ij[(si0)*(BS) + (si1)] = __mm27[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(xr_ii_a_ij[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb28[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                a_ij_dag_xr_ii_dag[(__w0)*(BS) + (__w1)] = __cb28[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi29 = 0; __mmi29 < BS; ++__mmi29) {
              for (int64_t __mmj29 = 0; __mmj29 < BS; ++__mmj29) {
                __mm29[(__mmi29)*(BS) + (__mmj29)] = 0.0;
                for (int64_t __mml29 = 0; __mml29 < BS; ++__mml29) {
                  __mm29[(__mmi29)*(BS) + (__mmj29)] += (xr_d[((j)*(BS) + (__mmi29))*(BS) + (__mml29)] * a_lower[(((e)*((NB - 1)) + (i))*(BS) + (__mml29))*(BS) + (__mmj29)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                xr_jj_a_ji[(si0)*(BS) + (si1)] = __mm29[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(xr_jj_a_ji[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb30[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                a_ji_dag_xr_jj_dag[(__w0)*(BS) + (__w1)] = __cb30[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __mmi31 = 0; __mmi31 < BS; ++__mmi31) {
              for (int64_t __mmj31 = 0; __mmj31 < BS; ++__mmj31) {
                __mm31[(__mmi31)*(BS) + (__mmj31)] = 0.0;
                for (int64_t __mml31 = 0; __mml31 < BS; ++__mml31) {
                  __mm31[(__mmi31)*(BS) + (__mmj31)] += (xr_ii_a_ij[(__mmi31)*(BS) + (__mml31)] * xr_d[((j)*(BS) + (__mml31))*(BS) + (__mmj31)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                xr_ii_a_ij_xr_jj[(si0)*(BS) + (si1)] = __mm31[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(xr_ii_a_ij_xr_jj[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb32[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                xr_jj_dag_a_ij_dag_xr_ii_dag[(__w0)*(BS) + (__w1)] = __cb32[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm33[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm33[(__i)*(BS) + (__j)] += (xr_ii_a_ij[(__i)*(BS) + (__l)] * xr_jj_a_ji[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                xr_ii_a_ij_xr_jj_a_ji[(si0)*(BS) + (si1)] = __mm33[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t __mmi34 = 0; __mmi34 < BS; ++__mmi34) {
              for (int64_t __mmj34 = 0; __mmj34 < BS; ++__mmj34) {
                __mm34[(__mmi34)*(BS) + (__mmj34)] = 0.0;
                for (int64_t __mml34 = 0; __mml34 < BS; ++__mml34) {
                  __mm34[(__mmi34)*(BS) + (__mmj34)] += (xr_ii_a_ij_xr_jj_a_ji[(__mmi34)*(BS) + (__mml34)] * xl_d[((i)*(BS) + (__mml34))*(BS) + (__mmj34)]);
                }
              }
            }
            for (int64_t __mmi35 = 0; __mmi35 < BS; ++__mmi35) {
              for (int64_t __mmj35 = 0; __mmj35 < BS; ++__mmj35) {
                __mm35[(__mmi35)*(BS) + (__mmj35)] = 0.0;
                for (int64_t __mml35 = 0; __mml35 < BS; ++__mml35) {
                  __mm35[(__mmi35)*(BS) + (__mmj35)] += (xr_d[((i)*(BS) + (__mmi35))*(BS) + (__mml35)] * sigma_lesser_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml35))*(BS) + (__mmj35)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm36[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm36[(__i)*(BS) + (__j)] += (__mm35[(__i)*(BS) + (__l)] * xr_jj_dag_a_ij_dag_xr_ii_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t1[(si0)*(BS) + (si1)] = (__mm34[(si0)*(BS) + (si1)] - __mm36[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t1[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb37[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb37[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                temp_1x[(si0)*(BS) + (si1)] = (t1[(si0)*(BS) + (si1)] - dag[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t __mmi38 = 0; __mmi38 < BS; ++__mmi38) {
              for (int64_t __mmj38 = 0; __mmj38 < BS; ++__mmj38) {
                __mm38[(__mmi38)*(BS) + (__mmj38)] = 0.0;
                for (int64_t __mml38 = 0; __mml38 < BS; ++__mml38) {
                  __mm38[(__mmi38)*(BS) + (__mmj38)] += (xr_ii_a_ij[(__mmi38)*(BS) + (__mml38)] * xl_d[((j)*(BS) + (__mml38))*(BS) + (__mmj38)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                temp_2x[(si0)*(BS) + (si1)] = __mm38[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t __mmi39 = 0; __mmi39 < BS; ++__mmi39) {
              for (int64_t __mmj39 = 0; __mmj39 < BS; ++__mmj39) {
                __mm39[(__mmi39)*(BS) + (__mmj39)] = 0.0;
                for (int64_t __mml39 = 0; __mml39 < BS; ++__mml39) {
                  __mm39[(__mmi39)*(BS) + (__mmj39)] += (xl_d[((i)*(BS) + (__mmi39))*(BS) + (__mml39)] * a_ji_dag_xr_jj_dag[(__mml39)*(BS) + (__mmj39)]);
                }
              }
            }
            for (int64_t __mmi40 = 0; __mmi40 < BS; ++__mmi40) {
              for (int64_t __mmj40 = 0; __mmj40 < BS; ++__mmj40) {
                __mm40[(__mmi40)*(BS) + (__mmj40)] = 0.0;
                for (int64_t __mml40 = 0; __mml40 < BS; ++__mml40) {
                  __mm40[(__mmi40)*(BS) + (__mmj40)] += (xr_d[((i)*(BS) + (__mmi40))*(BS) + (__mml40)] * sigma_lesser_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml40))*(BS) + (__mmj40)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm41[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm41[(__i)*(BS) + (__j)] += (__mm40[(__i)*(BS) + (__l)] * xr_jj_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t2[(si0)*(BS) + (si1)] = (((-temp_2x[(si0)*(BS) + (si1)]) - __mm39[(si0)*(BS) + (si1)]) + __mm41[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_lesser_upper[(((e)*((NB - 1)) + (i))*(BS) + (si2))*(BS) + (si3)] = t2[(si2)*(BS) + (si3)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t2[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb42[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb42[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_lesser_lower[(((e)*((NB - 1)) + (i))*(BS) + (si2))*(BS) + (si3)] = (-dag[(si2)*(BS) + (si3)]);
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm43[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm43[(__i)*(BS) + (__j)] += (temp_2x[(__i)*(BS) + (__l)] * a_ij_dag_xr_ii_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t3[(si0)*(BS) + (si1)] = ((xl_d[((i)*(BS) + (si0))*(BS) + (si1)] + __mm43[(si0)*(BS) + (si1)]) + temp_1x[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              for (int64_t si2 = 0; si2 < BS; ++si2) {
                xl_d[((i)*(BS) + (si1))*(BS) + (si2)] = t3[(si1)*(BS) + (si2)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t3[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb44[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb44[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_lesser_diag[(((e)*(NB) + (i))*(BS) + (si2))*(BS) + (si3)] = (0.5 * (t3[(si2)*(BS) + (si3)] - dag[(si2)*(BS) + (si3)]));
              }
            }
            for (int64_t __mmi45 = 0; __mmi45 < BS; ++__mmi45) {
              for (int64_t __mmj45 = 0; __mmj45 < BS; ++__mmj45) {
                __mm45[(__mmi45)*(BS) + (__mmj45)] = 0.0;
                for (int64_t __mml45 = 0; __mml45 < BS; ++__mml45) {
                  __mm45[(__mmi45)*(BS) + (__mmj45)] += (xr_ii_a_ij_xr_jj_a_ji[(__mmi45)*(BS) + (__mml45)] * xg_d[((i)*(BS) + (__mml45))*(BS) + (__mmj45)]);
                }
              }
            }
            for (int64_t __mmi46 = 0; __mmi46 < BS; ++__mmi46) {
              for (int64_t __mmj46 = 0; __mmj46 < BS; ++__mmj46) {
                __mm46[(__mmi46)*(BS) + (__mmj46)] = 0.0;
                for (int64_t __mml46 = 0; __mml46 < BS; ++__mml46) {
                  __mm46[(__mmi46)*(BS) + (__mmj46)] += (xr_d[((i)*(BS) + (__mmi46))*(BS) + (__mml46)] * sigma_greater_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml46))*(BS) + (__mmj46)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm47[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm47[(__i)*(BS) + (__j)] += (__mm46[(__i)*(BS) + (__l)] * xr_jj_dag_a_ij_dag_xr_ii_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t1[(si0)*(BS) + (si1)] = (__mm45[(si0)*(BS) + (si1)] - __mm47[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t1[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb48[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb48[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                temp_1x[(si0)*(BS) + (si1)] = (t1[(si0)*(BS) + (si1)] - dag[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t __mmi49 = 0; __mmi49 < BS; ++__mmi49) {
              for (int64_t __mmj49 = 0; __mmj49 < BS; ++__mmj49) {
                __mm49[(__mmi49)*(BS) + (__mmj49)] = 0.0;
                for (int64_t __mml49 = 0; __mml49 < BS; ++__mml49) {
                  __mm49[(__mmi49)*(BS) + (__mmj49)] += (xr_ii_a_ij[(__mmi49)*(BS) + (__mml49)] * xg_d[((j)*(BS) + (__mml49))*(BS) + (__mmj49)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                temp_2x[(si0)*(BS) + (si1)] = __mm49[(si0)*(BS) + (si1)];
              }
            }
            for (int64_t __mmi50 = 0; __mmi50 < BS; ++__mmi50) {
              for (int64_t __mmj50 = 0; __mmj50 < BS; ++__mmj50) {
                __mm50[(__mmi50)*(BS) + (__mmj50)] = 0.0;
                for (int64_t __mml50 = 0; __mml50 < BS; ++__mml50) {
                  __mm50[(__mmi50)*(BS) + (__mmj50)] += (xg_d[((i)*(BS) + (__mmi50))*(BS) + (__mml50)] * a_ji_dag_xr_jj_dag[(__mml50)*(BS) + (__mmj50)]);
                }
              }
            }
            for (int64_t __mmi51 = 0; __mmi51 < BS; ++__mmi51) {
              for (int64_t __mmj51 = 0; __mmj51 < BS; ++__mmj51) {
                __mm51[(__mmi51)*(BS) + (__mmj51)] = 0.0;
                for (int64_t __mml51 = 0; __mml51 < BS; ++__mml51) {
                  __mm51[(__mmi51)*(BS) + (__mmj51)] += (xr_d[((i)*(BS) + (__mmi51))*(BS) + (__mml51)] * sigma_greater_upper[(((e)*((NB - 1)) + (i))*(BS) + (__mml51))*(BS) + (__mmj51)]);
                }
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm52[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm52[(__i)*(BS) + (__j)] += (__mm51[(__i)*(BS) + (__l)] * xr_jj_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t2[(si0)*(BS) + (si1)] = (((-temp_2x[(si0)*(BS) + (si1)]) - __mm50[(si0)*(BS) + (si1)]) + __mm52[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_greater_upper[(((e)*((NB - 1)) + (i))*(BS) + (si2))*(BS) + (si3)] = t2[(si2)*(BS) + (si3)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t2[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb53[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb53[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_greater_lower[(((e)*((NB - 1)) + (i))*(BS) + (si2))*(BS) + (si3)] = (-dag[(si2)*(BS) + (si3)]);
              }
            }
            for (int64_t __i = 0; __i < BS; ++__i) {
              for (int64_t __j = 0; __j < BS; ++__j) {
                __mm54[(__i)*(BS) + (__j)] = 0.0;
                for (int64_t __l = 0; __l < BS; ++__l) {
                  __mm54[(__i)*(BS) + (__j)] += (temp_2x[(__i)*(BS) + (__l)] * a_ij_dag_xr_ii_dag[(__l)*(BS) + (__j)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t3[(si0)*(BS) + (si1)] = ((xg_d[((i)*(BS) + (si0))*(BS) + (si1)] + __mm54[(si0)*(BS) + (si1)]) + temp_1x[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              for (int64_t si2 = 0; si2 < BS; ++si2) {
                xg_d[((i)*(BS) + (si1))*(BS) + (si2)] = t3[(si1)*(BS) + (si2)];
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                cj[(si0)*(BS) + (si1)] = __npb_conj(t3[(si0)*(BS) + (si1)]);
              }
            }
            /* numpy: np.transpose(cj) */
            for (int64_t __t0 = 0; __t0 < BS; ++__t0) {
              for (int64_t __t1 = 0; __t1 < BS; ++__t1) {
                __cb55[(__t1)*(BS) + (__t0)] = cj[(__t0)*(BS) + (__t1)];
              }
            }
            for (int64_t __w0 = 0; __w0 < BS; ++__w0) {
              for (int64_t __w1 = 0; __w1 < BS; ++__w1) {
                dag[(__w0)*(BS) + (__w1)] = __cb55[(__w0)*(BS) + (__w1)];
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_greater_diag[(((e)*(NB) + (i))*(BS) + (si2))*(BS) + (si3)] = (0.5 * (t3[(si2)*(BS) + (si3)] - dag[(si2)*(BS) + (si3)]));
              }
            }
            for (int64_t __mmi56 = 0; __mmi56 < BS; ++__mmi56) {
              for (int64_t __mmj56 = 0; __mmj56 < BS; ++__mmj56) {
                __mm56[(__mmi56)*(BS) + (__mmj56)] = 0.0;
                for (int64_t __mml56 = 0; __mml56 < BS; ++__mml56) {
                  __mm56[(__mmi56)*(BS) + (__mmj56)] += (xr_ii_a_ij_xr_jj_a_ji[(__mmi56)*(BS) + (__mml56)] * xr_d[((i)*(BS) + (__mml56))*(BS) + (__mmj56)]);
                }
              }
            }
            for (int64_t si0 = 0; si0 < BS; ++si0) {
              for (int64_t si1 = 0; si1 < BS; ++si1) {
                t3[(si0)*(BS) + (si1)] = (xr_d[((i)*(BS) + (si0))*(BS) + (si1)] + __mm56[(si0)*(BS) + (si1)]);
              }
            }
            for (int64_t si1 = 0; si1 < BS; ++si1) {
              for (int64_t si2 = 0; si2 < BS; ++si2) {
                xr_d[((i)*(BS) + (si1))*(BS) + (si2)] = t3[(si1)*(BS) + (si2)];
              }
            }
            for (int64_t si2 = 0; si2 < BS; ++si2) {
              for (int64_t si3 = 0; si3 < BS; ++si3) {
                x_retarded_diag[(((e)*(NB) + (i))*(BS) + (si2))*(BS) + (si3)] = t3[(si2)*(BS) + (si3)];
              }
            }
          }
        }
        free(xr_d);
        free(xl_d);
        free(xg_d);
        free(m);
        free(dag);
        free(a_ji_dag);
        free(xr_jj_dag);
        free(t1);
        free(t2);
        free(t3);
        free(xr_ii_a_ij);
        free(xr_jj_a_ji);
        free(xr_ii_a_ij_xr_jj);
        free(xr_ii_a_ij_xr_jj_a_ji);
        free(a_ij_dag_xr_ii_dag);
        free(a_ji_dag_xr_jj_dag);
        free(xr_jj_dag_a_ij_dag_xr_ii_dag);
        free(temp_1x);
        free(temp_2x);
        free(cj);
        free(__cb1);
        free(__cb2);
        free(__mm3);
        free(__mm4);
        free(__mm5);
        free(__mm6);
        free(__cb7);
        free(__mm8);
        free(__mm9);
        free(__cb10);
        free(__cb11);
        free(__mm12);
        free(__cb13);
        free(__mm14);
        free(__mm15);
        free(__mm16);
        free(__mm17);
        free(__mm18);
        free(__cb19);
        free(__mm20);
        free(__mm21);
        free(__mm22);
        free(__mm23);
        free(__cb24);
        free(__cb25);
        free(__cb26);
        free(__mm27);
        free(__cb28);
        free(__mm29);
        free(__cb30);
        free(__mm31);
        free(__cb32);
        free(__mm33);
        free(__mm34);
        free(__mm35);
        free(__mm36);
        free(__cb37);
        free(__mm38);
        free(__mm39);
        free(__mm40);
        free(__mm41);
        free(__cb42);
        free(__mm43);
        free(__cb44);
        free(__mm45);
        free(__mm46);
        free(__mm47);
        free(__cb48);
        free(__mm49);
        free(__mm50);
        free(__mm51);
        free(__mm52);
        free(__cb53);
        free(__mm54);
        free(__cb55);
        free(__mm56);
        free(__inv_aw0);
        free(__inv_aw1);
        free(xr);
}
} // extern "C"
