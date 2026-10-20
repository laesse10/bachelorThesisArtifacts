// hpcagent_bench-autogen -- generated from warpx_field_gather_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
#define _USE_MATH_DEFINES
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#ifndef NPB_HD
#if defined(__HIPCC__) || defined(__CUDACC__)
#define NPB_HD __host__ __device__
#else
#define NPB_HD
#endif
#endif
/* ``z.conjugate()`` -- named helper so the C and C++ preludes
 * offer the same spelling. C has the standard one: ``conj``
 * from <complex.h>. The C++ prelude, which has no <complex.h>,
 * writes its own. */
static inline NPB_HD double _Complex __npb_conj(double _Complex z) {
    return conj(z);
}
/* M_PI / M_E etc. are POSIX/GNU extensions -- ensure they
 * are defined even on strict-C builds (glibc 2.27+ /
 * BSDs / MSVC). */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif
/* ``<complex.h>`` defines ``I`` as the imaginary unit;
 * undef it so user variable names like ``I`` (mandelbrot
 * boolean mask) don''t collide. Complex literals continue
 * to use the portable ``_Complex_I`` form. */
#ifdef I
#undef I
#endif
/* ``max``/``min`` PROPAGATE NaN (a NaN in EITHER operand yields NaN):
 * these serve the elementwise ``np.maximum``/``np.minimum`` broadcast
 * and the ``np.maximum.at`` / ``np.minimum.at`` scatter folds, which
 * follow numpy (propagate), not Python's builtin max (which drops a NaN
 * second operand). ``(a)+(b)`` is NaN whenever either operand is; for
 * finite operands the ternary picks the larger/smaller -- identical to
 * a plain comparison, so the 3-way builtin max (needleman_wunsch, always
 * finite) is unchanged. For integer operands the NaN test is dead. */
#ifndef min
#define min(a, b) ((((a) != (a)) || ((b) != (b))) ? ((a) + (b)) : (((b) < (a)) ? (b) : (a)))
#endif
#ifndef max
#define max(a, b) ((((a) != (a)) || ((b) != (b))) ? ((a) + (b)) : (((b) > (a)) ? (b) : (a)))
#endif
/* Elementwise ``np.maximum``/``np.minimum`` lower to ``fmax``/``fmin``;
 * libm ``fmax``/``fmin`` SUPPRESS NaN (return the non-NaN operand) but
 * numpy PROPAGATES it. These single-evaluation helpers return NaN when
 * either operand is NaN, else the larger/smaller.
 * Integer operands take the INTEGER form, dispatched on the promoted operand
 * type exactly as int_floor is: routing them through the double helper rounds
 * every value above 2**53 to the nearest representable double, so
 * min(2**53 + 1, 2**53 + 2) returned 2**53 -- a value neither operand had. */
static inline NPB_HD double __npb_fmax_f(double a, double b) {
    return (a != a) ? a : (b != b) ? b : (a > b ? a : b);
}
static inline NPB_HD double __npb_fmin_f(double a, double b) {
    return (a != a) ? a : (b != b) ? b : (a < b ? a : b);
}
static inline NPB_HD int64_t __npb_fmax_i(int64_t a, int64_t b) { return a > b ? a : b; }
static inline NPB_HD int64_t __npb_fmin_i(int64_t a, int64_t b) { return a < b ? a : b; }
static inline NPB_HD uint64_t __npb_fmax_u(uint64_t a, uint64_t b) { return a > b ? a : b; }
static inline NPB_HD uint64_t __npb_fmin_u(uint64_t a, uint64_t b) { return a < b ? a : b; }
/* ``np.sign``: numpy ``sign(nan) == nan`` and ``sign(0) == 0``. The
 * naive ``(x>0)-(x<0)`` gives 0 for NaN and evaluates ``x`` twice. */
static inline NPB_HD double __npb_sign(double x) {
    return x != x ? x : (double)((x > 0) - (x < 0));
}
/* Python ``//`` floors toward -inf; C ``/`` truncates toward zero. Integer and
 * floating operands need different corrections, so the division helpers dispatch
 * on the PROMOTED OPERAND TYPE -- the emitter never has to infer the dtype from
 * the source AST (guessing it wrong silently truncated instead of flooring).
 * _Generic's controlling expression is unevaluated and each argument is spelled
 * once, so operands with side effects are evaluated exactly once. */
static inline NPB_HD int64_t __npb_floordiv_i(int64_t a, int64_t b) {
    return a / b - ((a % b != 0) && ((a < 0) ^ (b < 0)));
}
static inline NPB_HD double __npb_floordiv_f(double a, double b) { return floor(a / b); }
/* Unsigned operands need their own form: floor == truncate for them, and routing
 * them through the SIGNED helper reinterprets any value above INT64_MAX as
 * negative ((2**63 + 5) // 2 came back negative). */
static inline NPB_HD uint64_t __npb_floordiv_u(uint64_t a, uint64_t b) { return a / b; }
static inline NPB_HD uint64_t __npb_ceildiv_u(uint64_t a, uint64_t b) { return a / b + (a % b != 0); }
static inline NPB_HD uint64_t __npb_mod_u(uint64_t a, uint64_t b) { return a % b; }
/* _Float16 is NOT promoted by GCC in arithmetic, so `_Float16 + _Float16` has type
 * _Float16 and fell to `default:` -- the INTEGER helper. 0.5 // 0.25 became
 * int_floor(0, 0) and died with SIGFPE. Spelled as a macro because the association
 * only exists where the type does. */
#if defined(__FLT16_MANT_DIG__)
#define __NPB_F16_ASSOC(fn) _Float16: fn,
#else
#define __NPB_F16_ASSOC(fn)
#endif
#define __NPB_UNSIGNED_ASSOC(fn) \
    unsigned int: fn, unsigned long: fn, unsigned long long: fn,
/* min/max dispatch (declared above): integer operands stay exact, floating ones
 * propagate NaN. Spelled here because the type associations are. */
#define __npb_fmin(a, b) _Generic((a) + (b), \
    __NPB_F16_ASSOC(__npb_fmin_f) \
    __NPB_UNSIGNED_ASSOC(__npb_fmin_u) \
    float: __npb_fmin_f, double: __npb_fmin_f, long double: __npb_fmin_f, \
    default: __npb_fmin_i)((a), (b))
#define __npb_fmax(a, b) _Generic((a) + (b), \
    __NPB_F16_ASSOC(__npb_fmax_f) \
    __NPB_UNSIGNED_ASSOC(__npb_fmax_u) \
    float: __npb_fmax_f, double: __npb_fmax_f, long double: __npb_fmax_f, \
    default: __npb_fmax_i)((a), (b))
#ifndef int_floor
#define int_floor(a, b) _Generic((a) + (b), \
    __NPB_F16_ASSOC(__npb_floordiv_f) \
    __NPB_UNSIGNED_ASSOC(__npb_floordiv_u) \
    float: __npb_floordiv_f, double: __npb_floordiv_f, long double: __npb_floordiv_f, \
    default: __npb_floordiv_i)((a), (b))
#endif
/* Ceil-division counterpart (toward +inf), exact for both signs -- unlike the
 * ``(a + b - 1) / b`` idiom, which is correct only for a positive divisor and
 * overflows near the integer maximum. */
static inline NPB_HD int64_t __npb_ceildiv_i(int64_t a, int64_t b) {
    return a / b + ((a % b != 0) && ((a < 0) == (b < 0)));
}
static inline NPB_HD double __npb_ceildiv_f(double a, double b) { return ceil(a / b); }
#ifndef int_ceil
#define int_ceil(a, b) _Generic((a) + (b), \
    __NPB_F16_ASSOC(__npb_ceildiv_f) \
    __NPB_UNSIGNED_ASSOC(__npb_ceildiv_u) \
    float: __npb_ceildiv_f, double: __npb_ceildiv_f, long double: __npb_ceildiv_f, \
    default: __npb_ceildiv_i)((a), (b))
#endif
/* pet's named quasi-affine builtins (POLYCC-008); guarded because polycc prepends
 * its own #define floord/ceild, which would expand these declarators (POLYCC-004). */
#ifndef floord
static inline NPB_HD int64_t floord(int64_t a, int64_t b) {
    return __npb_floordiv_i(a, b);
}
#endif
#ifndef ceild
static inline NPB_HD int64_t ceild(int64_t a, int64_t b) {
    return __npb_ceildiv_i(a, b);
}
#endif
/* Python ``%`` returns sign of divisor; C returns sign of dividend. Same
 * type-dispatch as int_floor: integer operands use the exact integer form,
 * floating operands numpy's npy_remainder (see python_fmod). */
static inline NPB_HD int64_t __npb_mod_i(int64_t a, int64_t b) { return (a % b + b) % b; }
/* Floating-point ``%``: numpy's floored modulo takes the sign of the
 * divisor, which integer ``python_mod`` cannot express on doubles.
 * Mirrors numpy ``npy_remainder`` (fmod + sign-of-divisor fixup). */
static inline NPB_HD double python_fmod(double a, double b) {
    double m = fmod(a, b);
    if (m != 0.0 && ((b < 0.0) != (m < 0.0))) m += b;
    return m;
}
#ifndef python_mod
#define python_mod(a, b) _Generic((a) + (b), \
    __NPB_F16_ASSOC(python_fmod) \
    __NPB_UNSIGNED_ASSOC(__npb_mod_u) \
    float: python_fmod, double: python_fmod, long double: python_fmod, \
    default: __npb_mod_i)((a), (b))
#endif
/* Integer power for VLA shape bounds like ``R ** K``. */
static inline NPB_HD int64_t __npb_int_pow(int64_t base, int64_t exp) {
    int64_t result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

constexpr int64_t galerkin_interpolation = 1;
constexpr int64_t geom = 3;
constexpr int64_t n_rz_azimuthal_modes = 1;

void warpx_field_gather_fp64(double *restrict Bxp, double *restrict Byp, double *restrict Bzp, double *restrict Exp, double *restrict Eyp, double *restrict Ezp, const double *restrict bx_arr, const int32_t *restrict bx_type, const double *restrict by_arr, const int32_t *restrict by_type, const double *restrict bz_arr, const int32_t *restrict bz_type, const double *restrict dinv, const double *restrict ex_arr, const int32_t *restrict ex_type, const double *restrict ey_arr, const int32_t *restrict ey_type, const double *restrict ez_arr, const int32_t *restrict ez_type, const int32_t *restrict lo, const double *restrict xp, const double *restrict xyzmin, const double *restrict yp, const double *restrict zp, const int64_t depos_order, const int64_t ncells, const int64_t np_particles) {
        int64_t o;
        int64_t gal;
        int64_t g;
        int64_t nmodes;
        int64_t __inl1_o;
        int64_t __inl1_og;
        int32_t __inl1_lox;
        int32_t __inl1_loy;
        int32_t __inl1_loz;
        int64_t __inl1_zdir;
        int64_t __inl1_n_sx_ex;
        int64_t __inl1_n_sx_by;
        int64_t __inl1_n_sx_bz;
        int64_t __inl1_n_sx_ey;
        int64_t __inl1_n_sx_ez;
        int64_t __inl1_n_sx_bx;
        int64_t __inl1_n_sy_ey;
        int64_t __inl1_n_sy_bx;
        int64_t __inl1_n_sy_bz;
        int64_t __inl1_n_sy_ex;
        int64_t __inl1_n_sy_ez;
        int64_t __inl1_n_sy_by;
        int64_t __inl1_n_sz_ez;
        int64_t __inl1_n_sz_bx;
        int64_t __inl1_n_sz_by;
        int64_t __inl1_n_sz_ex;
        int64_t __inl1_n_sz_ey;
        int64_t __inl1_n_sz_bz;
        int64_t *__inl1_j_node = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_j_node, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_cell = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_j_cell, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_node_v = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_j_node_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_cell_v = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_j_cell_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl2_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl2_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl3_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl3_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl4_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl4_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl5_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl5_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_node = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_k_node, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_cell = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_k_cell, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_node_v = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_k_node_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_cell_v = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_k_cell_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl6_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl6_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl7_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl7_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl8_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl8_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl9_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl9_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_node = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_l_node, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_cell = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_l_cell, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_node_v = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_l_node_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_cell_v = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl1_l_cell_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl10_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl10_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl11_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl11_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl12_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl12_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl13_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(__inl13_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
        double *__cb111 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb115 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb119 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb123 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb127 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb131 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb135 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb139 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb143 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb147 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb151 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__cb155 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_rp = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_x = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl2_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl3_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl4_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl5_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl1_j_ex = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_ey = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_ez = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_bx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_by = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_j_bz = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl6_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl7_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl8_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl9_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl1_k_ex = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_ey = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_ez = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_bx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_by = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_k_bz = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl10_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl11_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl12_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl13_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl1_l_ex = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_ey = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_ez = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_bx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_by = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl1_l_bz = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl1_Ethetap = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_Erp = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_Brp = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_Bthetap = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_rp_safe = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_costheta = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_sintheta = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_xy_re = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_xy_im = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall15 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall16 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_dEy = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall17 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall18 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_dEx = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall19 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall20 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_dBz = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall21 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall22 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_dEz = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall23 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall24 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_dBx = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall25 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__hcall26 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_dBy = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_tmp_re = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *__inl1_tmp_im = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl2_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl3_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl4_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl5_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl6_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl7_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl8_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl9_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl10_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl11_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl12_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl13_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl1_sx_node = NULL;
        double *__inl1_sx_cell = NULL;
        double *__inl1_sx_node_g = NULL;
        double *__inl1_sx_cell_g = NULL;
        double *__inl1_sy_node = NULL;
        double *__inl1_sy_cell = NULL;
        double *__inl1_sy_node_v = NULL;
        double *__inl1_sy_cell_v = NULL;
        double *__inl1_sz_node = NULL;
        double *__inl1_sz_cell = NULL;
        double *__inl1_sz_node_v = NULL;
        double *__inl1_sz_cell_v = NULL;
        int64_t *__inl20_ia_b = NULL;
        int64_t *__inl20_ib_b = NULL;
        int64_t *__inl21_ia_b = NULL;
        int64_t *__inl21_ib_b = NULL;
        int64_t *__inl22_ia_b = NULL;
        int64_t *__inl22_ib_b = NULL;
        int64_t *__inl23_ia_b = NULL;
        int64_t *__inl23_ib_b = NULL;
        int64_t *__inl24_ia_b = NULL;
        int64_t *__inl24_ib_b = NULL;
        int64_t *__inl25_ia_b = NULL;
        int64_t *__inl25_ib_b = NULL;
        int64_t *__inl26_ia_b = NULL;
        int64_t *__inl26_ib_b = NULL;
        int64_t *__inl27_ia_b = NULL;
        int64_t *__inl27_ib_b = NULL;
        int64_t *__inl28_ia_b = NULL;
        int64_t *__inl28_ib_b = NULL;
        int64_t *__inl29_ia_b = NULL;
        int64_t *__inl29_ib_b = NULL;
        int64_t *__inl30_ia_b = NULL;
        int64_t *__inl30_ib_b = NULL;
        int64_t *__inl31_ia_b = NULL;
        int64_t *__inl31_ib_b = NULL;
        int64_t *__inl32_ia_b = NULL;
        int64_t *__inl32_ib_b = NULL;
        int64_t *__inl33_ia_b = NULL;
        int64_t *__inl33_ib_b = NULL;
        int64_t *__inl34_ia_b = NULL;
        int64_t *__inl34_ib_b = NULL;
        int64_t *__inl35_ia_b = NULL;
        int64_t *__inl35_ib_b = NULL;
        int64_t *__inl36_ia_b = NULL;
        int64_t *__inl36_ib_b = NULL;
        int64_t *__inl37_ia_b = NULL;
        int64_t *__inl37_ib_b = NULL;
        int64_t *__inl38_ia_b = NULL;
        int64_t *__inl38_ib_b = NULL;
        int64_t *__inl39_ia_b = NULL;
        int64_t *__inl39_ib_b = NULL;
        int64_t *__inl40_ia_b = NULL;
        int64_t *__inl40_ib_b = NULL;
        int64_t *__inl41_ia_b = NULL;
        int64_t *__inl41_ib_b = NULL;
        int64_t *__inl42_ia_b = NULL;
        int64_t *__inl42_ib_b = NULL;
        int64_t *__inl43_ia_b = NULL;
        int64_t *__inl43_ib_b = NULL;
        int64_t *__inl56_ix_b = NULL;
        int64_t *__inl56_iy_b = NULL;
        int64_t *__inl56_iz_b = NULL;
        int64_t *__inl57_ix_b = NULL;
        int64_t *__inl57_iy_b = NULL;
        int64_t *__inl57_iz_b = NULL;
        int64_t *__inl58_ix_b = NULL;
        int64_t *__inl58_iy_b = NULL;
        int64_t *__inl58_iz_b = NULL;
        int64_t *__inl59_ix_b = NULL;
        int64_t *__inl59_iy_b = NULL;
        int64_t *__inl59_iz_b = NULL;
        int64_t *__inl60_ix_b = NULL;
        int64_t *__inl60_iy_b = NULL;
        int64_t *__inl60_iz_b = NULL;
        int64_t *__inl61_ix_b = NULL;
        int64_t *__inl61_iy_b = NULL;
        int64_t *__inl61_iz_b = NULL;
        double *__cb3 = NULL;
        double *__cb4 = NULL;
        double *__cb5 = NULL;
        double *__cb6 = NULL;
        double *__cb7 = NULL;
        double *__cb8 = NULL;
        double *__cb15 = NULL;
        double *__cb16 = NULL;
        double *__cb17 = NULL;
        double *__cb18 = NULL;
        double *__cb19 = NULL;
        double *__cb20 = NULL;
        double *__cb27 = NULL;
        double *__cb28 = NULL;
        double *__cb29 = NULL;
        double *__cb30 = NULL;
        double *__cb31 = NULL;
        double *__cb32 = NULL;
        int64_t *__cb39 = NULL;
        double *__cb40 = NULL;
        int64_t *__cb42 = NULL;
        double *__cb43 = NULL;
        int64_t *__cb45 = NULL;
        double *__cb46 = NULL;
        int64_t *__cb48 = NULL;
        double *__cb49 = NULL;
        int64_t *__cb51 = NULL;
        double *__cb52 = NULL;
        int64_t *__cb54 = NULL;
        double *__cb55 = NULL;
        int64_t *__cb57 = NULL;
        int64_t *__cb58 = NULL;
        double *__cb59 = NULL;
        int64_t *__cb61 = NULL;
        int64_t *__cb62 = NULL;
        double *__cb63 = NULL;
        int64_t *__cb65 = NULL;
        int64_t *__cb66 = NULL;
        double *__cb67 = NULL;
        int64_t *__cb69 = NULL;
        int64_t *__cb70 = NULL;
        double *__cb71 = NULL;
        int64_t *__cb73 = NULL;
        int64_t *__cb74 = NULL;
        double *__cb75 = NULL;
        int64_t *__cb77 = NULL;
        int64_t *__cb78 = NULL;
        double *__cb79 = NULL;
        int64_t *__cb81 = NULL;
        int64_t *__cb82 = NULL;
        double *__cb83 = NULL;
        int64_t *__cb85 = NULL;
        int64_t *__cb86 = NULL;
        double *__cb87 = NULL;
        int64_t *__cb89 = NULL;
        int64_t *__cb90 = NULL;
        double *__cb91 = NULL;
        int64_t *__cb93 = NULL;
        int64_t *__cb94 = NULL;
        double *__cb95 = NULL;
        int64_t *__cb97 = NULL;
        int64_t *__cb98 = NULL;
        double *__cb99 = NULL;
        int64_t *__cb101 = NULL;
        int64_t *__cb102 = NULL;
        double *__cb103 = NULL;
        int64_t *__cb108 = NULL;
        int64_t *__cb109 = NULL;
        double *__cb110 = NULL;
        int64_t *__cb112 = NULL;
        int64_t *__cb113 = NULL;
        double *__cb114 = NULL;
        int64_t *__cb116 = NULL;
        int64_t *__cb117 = NULL;
        double *__cb118 = NULL;
        int64_t *__cb120 = NULL;
        int64_t *__cb121 = NULL;
        double *__cb122 = NULL;
        int64_t *__cb124 = NULL;
        int64_t *__cb125 = NULL;
        double *__cb126 = NULL;
        int64_t *__cb128 = NULL;
        int64_t *__cb129 = NULL;
        double *__cb130 = NULL;
        int64_t *__cb132 = NULL;
        int64_t *__cb133 = NULL;
        double *__cb134 = NULL;
        int64_t *__cb136 = NULL;
        int64_t *__cb137 = NULL;
        double *__cb138 = NULL;
        int64_t *__cb140 = NULL;
        int64_t *__cb141 = NULL;
        double *__cb142 = NULL;
        int64_t *__cb144 = NULL;
        int64_t *__cb145 = NULL;
        double *__cb146 = NULL;
        int64_t *__cb148 = NULL;
        int64_t *__cb149 = NULL;
        double *__cb150 = NULL;
        int64_t *__cb152 = NULL;
        int64_t *__cb153 = NULL;
        double *__cb154 = NULL;
        int64_t *__cb156 = NULL;
        double *__cb157 = NULL;
        int64_t *__cb159 = NULL;
        double *__cb160 = NULL;
        int64_t *__cb162 = NULL;
        double *__cb163 = NULL;
        int64_t *__cb165 = NULL;
        double *__cb166 = NULL;
        int64_t *__cb168 = NULL;
        double *__cb169 = NULL;
        int64_t *__cb171 = NULL;
        double *__cb172 = NULL;
        int64_t *__cb177 = NULL;
        double *__cb178 = NULL;
        int64_t *__cb180 = NULL;
        double *__cb181 = NULL;
        int64_t *__cb183 = NULL;
        double *__cb184 = NULL;
        int64_t *__cb186 = NULL;
        double *__cb187 = NULL;
        int64_t *__cb189 = NULL;
        double *__cb190 = NULL;
        int64_t *__cb192 = NULL;
        double *__cb193 = NULL;
        int64_t *__cb202 = NULL;
        int64_t *__cb203 = NULL;
        int64_t *__cb204 = NULL;
        double *__cb205 = NULL;
        int64_t *__cb207 = NULL;
        int64_t *__cb208 = NULL;
        int64_t *__cb209 = NULL;
        double *__cb210 = NULL;
        int64_t *__cb212 = NULL;
        int64_t *__cb213 = NULL;
        int64_t *__cb214 = NULL;
        double *__cb215 = NULL;
        int64_t *__cb217 = NULL;
        int64_t *__cb218 = NULL;
        int64_t *__cb219 = NULL;
        double *__cb220 = NULL;
        int64_t *__cb222 = NULL;
        int64_t *__cb223 = NULL;
        int64_t *__cb224 = NULL;
        double *__cb225 = NULL;
        int64_t *__cb227 = NULL;
        int64_t *__cb228 = NULL;
        int64_t *__cb229 = NULL;
        double *__cb230 = NULL;
        double *__inl1_sx_ex = NULL;
        double *__inl1_sx_ey = NULL;
        double *__inl1_sx_ez = NULL;
        double *__inl1_sx_bx = NULL;
        double *__inl1_sx_by = NULL;
        double *__inl1_sx_bz = NULL;
        double *__inl1_sy_ex = NULL;
        double *__inl1_sy_ey = NULL;
        double *__inl1_sy_ez = NULL;
        double *__inl1_sy_bx = NULL;
        double *__inl1_sy_by = NULL;
        double *__inl1_sy_bz = NULL;
        double *__inl1_sz_ex = NULL;
        double *__inl1_sz_ey = NULL;
        double *__inl1_sz_ez = NULL;
        double *__inl1_sz_bx = NULL;
        double *__inl1_sz_by = NULL;
        double *__inl1_sz_bz = NULL;
        int64_t *__inl14_taps = NULL;
        int64_t *__inl14_rows = NULL;
        int64_t *__inl15_taps = NULL;
        int64_t *__inl15_rows = NULL;
        int64_t *__inl16_taps = NULL;
        int64_t *__inl16_rows = NULL;
        int64_t *__inl17_taps = NULL;
        int64_t *__inl17_rows = NULL;
        int64_t *__inl18_taps = NULL;
        int64_t *__inl18_rows = NULL;
        int64_t *__inl19_taps = NULL;
        int64_t *__inl19_rows = NULL;
        int64_t *__inl20_ta = NULL;
        int64_t *__inl20_tb = NULL;
        double *__inl20_weight = NULL;
        int64_t *__inl21_ta = NULL;
        int64_t *__inl21_tb = NULL;
        double *__inl21_weight = NULL;
        int64_t *__inl22_ta = NULL;
        int64_t *__inl22_tb = NULL;
        double *__inl22_weight = NULL;
        int64_t *__inl23_ta = NULL;
        int64_t *__inl23_tb = NULL;
        double *__inl23_weight = NULL;
        int64_t *__inl24_ta = NULL;
        int64_t *__inl24_tb = NULL;
        double *__inl24_weight = NULL;
        int64_t *__inl25_ta = NULL;
        int64_t *__inl25_tb = NULL;
        double *__inl25_weight = NULL;
        int64_t *__inl26_ta = NULL;
        int64_t *__inl26_tb = NULL;
        double *__inl26_weight = NULL;
        int64_t *__inl27_ta = NULL;
        int64_t *__inl27_tb = NULL;
        double *__inl27_weight = NULL;
        int64_t *__inl28_ta = NULL;
        int64_t *__inl28_tb = NULL;
        double *__inl28_weight = NULL;
        int64_t *__inl29_ta = NULL;
        int64_t *__inl29_tb = NULL;
        double *__inl29_weight = NULL;
        int64_t *__inl30_ta = NULL;
        int64_t *__inl30_tb = NULL;
        double *__inl30_weight = NULL;
        int64_t *__inl31_ta = NULL;
        int64_t *__inl31_tb = NULL;
        double *__inl31_weight = NULL;
        int64_t *__inl32_ta = NULL;
        int64_t *__inl32_tb = NULL;
        double *__inl32_weight = NULL;
        int64_t *__inl33_ta = NULL;
        int64_t *__inl33_tb = NULL;
        double *__inl33_weight = NULL;
        int64_t *__inl34_ta = NULL;
        int64_t *__inl34_tb = NULL;
        double *__inl34_weight = NULL;
        int64_t *__inl35_ta = NULL;
        int64_t *__inl35_tb = NULL;
        double *__inl35_weight = NULL;
        int64_t *__inl36_ta = NULL;
        int64_t *__inl36_tb = NULL;
        double *__inl36_weight = NULL;
        int64_t *__inl37_ta = NULL;
        int64_t *__inl37_tb = NULL;
        double *__inl37_weight = NULL;
        int64_t *__inl38_ta = NULL;
        int64_t *__inl38_tb = NULL;
        double *__inl38_weight = NULL;
        int64_t *__inl39_ta = NULL;
        int64_t *__inl39_tb = NULL;
        double *__inl39_weight = NULL;
        int64_t *__inl40_ta = NULL;
        int64_t *__inl40_tb = NULL;
        double *__inl40_weight = NULL;
        int64_t *__inl41_ta = NULL;
        int64_t *__inl41_tb = NULL;
        double *__inl41_weight = NULL;
        int64_t *__inl42_ta = NULL;
        int64_t *__inl42_tb = NULL;
        double *__inl42_weight = NULL;
        int64_t *__inl43_ta = NULL;
        int64_t *__inl43_tb = NULL;
        double *__inl43_weight = NULL;
        int64_t *__inl44_taps = NULL;
        int64_t *__inl44_rows = NULL;
        int64_t *__inl45_taps = NULL;
        int64_t *__inl45_rows = NULL;
        int64_t *__inl46_taps = NULL;
        int64_t *__inl46_rows = NULL;
        int64_t *__inl47_taps = NULL;
        int64_t *__inl47_rows = NULL;
        int64_t *__inl48_taps = NULL;
        int64_t *__inl48_rows = NULL;
        int64_t *__inl49_taps = NULL;
        int64_t *__inl49_rows = NULL;
        int64_t *__inl50_taps = NULL;
        int64_t *__inl50_rows = NULL;
        int64_t *__inl51_taps = NULL;
        int64_t *__inl51_rows = NULL;
        int64_t *__inl52_taps = NULL;
        int64_t *__inl52_rows = NULL;
        int64_t *__inl53_taps = NULL;
        int64_t *__inl53_rows = NULL;
        int64_t *__inl54_taps = NULL;
        int64_t *__inl54_rows = NULL;
        int64_t *__inl55_taps = NULL;
        int64_t *__inl55_rows = NULL;
        int64_t *__inl56_tx = NULL;
        int64_t *__inl56_ty = NULL;
        int64_t *__inl56_tz = NULL;
        double *__inl56_weight = NULL;
        int64_t *__inl57_tx = NULL;
        int64_t *__inl57_ty = NULL;
        int64_t *__inl57_tz = NULL;
        double *__inl57_weight = NULL;
        int64_t *__inl58_tx = NULL;
        int64_t *__inl58_ty = NULL;
        int64_t *__inl58_tz = NULL;
        double *__inl58_weight = NULL;
        int64_t *__inl59_tx = NULL;
        int64_t *__inl59_ty = NULL;
        int64_t *__inl59_tz = NULL;
        double *__inl59_weight = NULL;
        int64_t *__inl60_tx = NULL;
        int64_t *__inl60_ty = NULL;
        int64_t *__inl60_tz = NULL;
        double *__inl60_weight = NULL;
        int64_t *__inl61_tx = NULL;
        int64_t *__inl61_ty = NULL;
        int64_t *__inl61_tz = NULL;
        double *__inl61_weight = NULL;
        double *__inl14_gathered = NULL;
        double *__inl15_gathered = NULL;
        double *__inl16_gathered = NULL;
        double *__inl17_gathered = NULL;
        double *__inl18_gathered = NULL;
        double *__inl19_gathered = NULL;
        double *__inl20_ia = NULL;
        double *__inl20_ib = NULL;
        double *__inl20_gathered = NULL;
        double *__inl21_ia = NULL;
        double *__inl21_ib = NULL;
        double *__inl21_gathered = NULL;
        double *__inl22_ia = NULL;
        double *__inl22_ib = NULL;
        double *__inl22_gathered = NULL;
        double *__inl23_ia = NULL;
        double *__inl23_ib = NULL;
        double *__inl23_gathered = NULL;
        double *__inl24_ia = NULL;
        double *__inl24_ib = NULL;
        double *__inl24_gathered = NULL;
        double *__inl25_ia = NULL;
        double *__inl25_ib = NULL;
        double *__inl25_gathered = NULL;
        double *__inl26_ia = NULL;
        double *__inl26_ib = NULL;
        double *__inl26_gathered = NULL;
        double *__inl27_ia = NULL;
        double *__inl27_ib = NULL;
        double *__inl27_gathered = NULL;
        double *__inl28_ia = NULL;
        double *__inl28_ib = NULL;
        double *__inl28_gathered = NULL;
        double *__inl29_ia = NULL;
        double *__inl29_ib = NULL;
        double *__inl29_gathered = NULL;
        double *__inl30_ia = NULL;
        double *__inl30_ib = NULL;
        double *__inl30_gathered = NULL;
        double *__inl31_ia = NULL;
        double *__inl31_ib = NULL;
        double *__inl31_gathered = NULL;
        double *__inl32_ia = NULL;
        double *__inl32_ib = NULL;
        double *__inl32_gathered = NULL;
        double *__inl33_ia = NULL;
        double *__inl33_ib = NULL;
        double *__inl33_gathered = NULL;
        double *__inl34_ia = NULL;
        double *__inl34_ib = NULL;
        double *__inl34_gathered = NULL;
        double *__inl35_ia = NULL;
        double *__inl35_ib = NULL;
        double *__inl35_gathered = NULL;
        double *__inl36_ia = NULL;
        double *__inl36_ib = NULL;
        double *__inl36_gathered = NULL;
        double *__inl37_ia = NULL;
        double *__inl37_ib = NULL;
        double *__inl37_gathered = NULL;
        double *__inl38_ia = NULL;
        double *__inl38_ib = NULL;
        double *__inl38_gathered = NULL;
        double *__inl39_ia = NULL;
        double *__inl39_ib = NULL;
        double *__inl39_gathered = NULL;
        double *__inl40_ia = NULL;
        double *__inl40_ib = NULL;
        double *__inl40_gathered = NULL;
        double *__inl41_ia = NULL;
        double *__inl41_ib = NULL;
        double *__inl41_gathered = NULL;
        double *__inl42_ia = NULL;
        double *__inl42_ib = NULL;
        double *__inl42_gathered = NULL;
        double *__inl43_ia = NULL;
        double *__inl43_ib = NULL;
        double *__inl43_gathered = NULL;
        double *__inl44_gathered = NULL;
        double *__inl45_gathered = NULL;
        double *__inl46_gathered = NULL;
        double *__inl47_gathered = NULL;
        double *__inl48_gathered = NULL;
        double *__inl49_gathered = NULL;
        double *__inl50_gathered = NULL;
        double *__inl51_gathered = NULL;
        double *__inl52_gathered = NULL;
        double *__inl53_gathered = NULL;
        double *__inl54_gathered = NULL;
        double *__inl55_gathered = NULL;
        double *__inl56_ix = NULL;
        double *__inl56_iy = NULL;
        double *__inl56_iz = NULL;
        double *__inl56_gathered = NULL;
        double *__inl57_ix = NULL;
        double *__inl57_iy = NULL;
        double *__inl57_iz = NULL;
        double *__inl57_gathered = NULL;
        double *__inl58_ix = NULL;
        double *__inl58_iy = NULL;
        double *__inl58_iz = NULL;
        double *__inl58_gathered = NULL;
        double *__inl59_ix = NULL;
        double *__inl59_iy = NULL;
        double *__inl59_iz = NULL;
        double *__inl59_gathered = NULL;
        double *__inl60_ix = NULL;
        double *__inl60_iy = NULL;
        double *__inl60_iz = NULL;
        double *__inl60_gathered = NULL;
        double *__inl61_ix = NULL;
        double *__inl61_iy = NULL;
        double *__inl61_iz = NULL;
        double *__inl61_gathered = NULL;
        o = ((int64_t)(depos_order));
        gal = ((int64_t)(galerkin_interpolation));
        g = ((int64_t)(geom));
        nmodes = ((int64_t)(n_rz_azimuthal_modes));
        __inl1_o = o;
        __inl1_og = (o - gal);
        if (((g == 1) || (g == 2))) {
          __inl1_zdir = 1;
        }
        else if ((g == 3)) {
          __inl1_zdir = 2;
        }
        else {
          __inl1_zdir = 0;
        }
        if ((g != 0)) {
          if (((g == 2) || (g == 4))) {
            double *__cb1 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.sqrt(xp * xp + yp * yp) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb1[__r0] = sqrt(((xp[__r0] * xp[__r0]) + (yp[__r0] * yp[__r0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_rp[__w0] = __cb1[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_x[__w0] = ((__inl1_rp[__w0] - xyzmin[0]) * dinv[0]);
            }
            free(__cb1);
          }
          else if ((g == 5)) {
            double *__cb2 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.sqrt(xp * xp + yp * yp + zp * zp) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb2[__r0] = sqrt((((xp[__r0] * xp[__r0]) + (yp[__r0] * yp[__r0])) + (zp[__r0] * zp[__r0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_rp[__w0] = __cb2[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_x[__w0] = ((__inl1_rp[__w0] - xyzmin[0]) * dinv[0]);
            }
            free(__cb2);
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_x[__w0] = ((xp[__w0] - xyzmin[0]) * dinv[0]);
            }
          }
          free(__inl1_sx_node);
          __inl1_sx_node = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sx_node, 0, (size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sx_cell);
          __inl1_sx_cell = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sx_cell, 0, (size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sx_node_g);
          __inl1_sx_node_g = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sx_node_g, 0, (size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sx_cell_g);
          __inl1_sx_cell_g = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sx_cell_g, 0, (size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_j_node, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_j_cell, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_j_node_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_j_cell_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
          if (((((int64_t)(ey_type[0])) == 1) || (((int64_t)(ez_type[0])) == 1) || (((int64_t)(bx_type[0])) == 1))) {
            memset(__inl2_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_o == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_j[__w0] = ((int64_t)((__inl1_x[__w0] + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_idx[__w0] = __inl2_j[__w0];
              }
            }
            if ((__inl1_o == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_j[__w0] = ((int64_t)(__inl1_x[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_xint[__w0] = (__inl1_x[__w0] - __inl2_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(0)*(np_particles) + (si1)] = (1.0 - __inl2_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(1)*(np_particles) + (si1)] = __inl2_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_idx[__w0] = __inl2_j[__w0];
              }
            }
            if ((__inl1_o == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_j[__w0] = ((int64_t)((__inl1_x[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_xint[__w0] = (__inl1_x[__w0] - __inl2_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl2_xint[si1])) * (0.5 - __inl2_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(1)*(np_particles) + (si1)] = (0.75 - (__inl2_xint[si1] * __inl2_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl2_xint[si1])) * (0.5 + __inl2_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_idx[__w0] = (__inl2_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_j[__w0] = ((int64_t)(__inl1_x[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_xint[__w0] = (__inl1_x[__w0] - __inl2_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl2_xint[si1])) * (1.0 - __inl2_xint[si1])) * (1.0 - __inl2_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl2_xint[si1] * __inl2_xint[si1]) * (1.0 - (__inl2_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl2_xint[si1]) * (1.0 - __inl2_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl2_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl2_xint[si1]) * __inl2_xint[si1]) * __inl2_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_idx[__w0] = (__inl2_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_j[__w0] = ((int64_t)((__inl1_x[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_xint[__w0] = (__inl1_x[__w0] - __inl2_j[__w0]);
              }
              double *__inl2_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_sm[__w0] = (0.5 - __inl2_xint[__w0]);
              }
              double *__inl2_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_sp[__w0] = (0.5 + __inl2_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl2_sm[si1]) * __inl2_sm[si1]) * __inl2_sm[si1]) * __inl2_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl2_xint[si1])) + (((4.0 * __inl2_xint[si1]) * __inl2_xint[si1]) * ((1.5 + __inl2_xint[si1]) - (__inl2_xint[si1] * __inl2_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl2_xint[si1]) * __inl2_xint[si1]) * ((__inl2_xint[si1] * __inl2_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl2_xint[si1])) + (((4.0 * __inl2_xint[si1]) * __inl2_xint[si1]) * ((1.5 - __inl2_xint[si1]) - (__inl2_xint[si1] * __inl2_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl2_sp[si1]) * __inl2_sp[si1]) * __inl2_sp[si1]) * __inl2_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl2_idx[__w0] = (__inl2_j[__w0] - 2);
              }
              free(__inl2_sm);
              free(__inl2_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_j_node[__w0] = __inl2_idx[__w0];
            }
          }
          if (((((int64_t)(ey_type[0])) == 0) || (((int64_t)(ez_type[0])) == 0) || (((int64_t)(bx_type[0])) == 0))) {
            memset(__inl3_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_o == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_j[__w0] = ((int64_t)(((__inl1_x[__w0] - 0.5) + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_idx[__w0] = __inl3_j[__w0];
              }
            }
            if ((__inl1_o == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_j[__w0] = ((int64_t)((__inl1_x[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl3_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(0)*(np_particles) + (si1)] = (1.0 - __inl3_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(1)*(np_particles) + (si1)] = __inl3_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_idx[__w0] = __inl3_j[__w0];
              }
            }
            if ((__inl1_o == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_j[__w0] = ((int64_t)(((__inl1_x[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl3_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl3_xint[si1])) * (0.5 - __inl3_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(1)*(np_particles) + (si1)] = (0.75 - (__inl3_xint[si1] * __inl3_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl3_xint[si1])) * (0.5 + __inl3_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_idx[__w0] = (__inl3_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_j[__w0] = ((int64_t)((__inl1_x[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl3_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl3_xint[si1])) * (1.0 - __inl3_xint[si1])) * (1.0 - __inl3_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl3_xint[si1] * __inl3_xint[si1]) * (1.0 - (__inl3_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl3_xint[si1]) * (1.0 - __inl3_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl3_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl3_xint[si1]) * __inl3_xint[si1]) * __inl3_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_idx[__w0] = (__inl3_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_j[__w0] = ((int64_t)(((__inl1_x[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl3_j[__w0]);
              }
              double *__inl3_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_sm[__w0] = (0.5 - __inl3_xint[__w0]);
              }
              double *__inl3_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_sp[__w0] = (0.5 + __inl3_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl3_sm[si1]) * __inl3_sm[si1]) * __inl3_sm[si1]) * __inl3_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl3_xint[si1])) + (((4.0 * __inl3_xint[si1]) * __inl3_xint[si1]) * ((1.5 + __inl3_xint[si1]) - (__inl3_xint[si1] * __inl3_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl3_xint[si1]) * __inl3_xint[si1]) * ((__inl3_xint[si1] * __inl3_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl3_xint[si1])) + (((4.0 * __inl3_xint[si1]) * __inl3_xint[si1]) * ((1.5 - __inl3_xint[si1]) - (__inl3_xint[si1] * __inl3_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl3_sp[si1]) * __inl3_sp[si1]) * __inl3_sp[si1]) * __inl3_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl3_idx[__w0] = (__inl3_j[__w0] - 2);
              }
              free(__inl3_sm);
              free(__inl3_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_j_cell[__w0] = __inl3_idx[__w0];
            }
          }
          if (((((int64_t)(ex_type[0])) == 1) || (((int64_t)(by_type[0])) == 1) || (((int64_t)(bz_type[0])) == 1))) {
            memset(__inl4_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_og == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_j[__w0] = ((int64_t)((__inl1_x[__w0] + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_idx[__w0] = __inl4_j[__w0];
              }
            }
            if ((__inl1_og == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_j[__w0] = ((int64_t)(__inl1_x[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_xint[__w0] = (__inl1_x[__w0] - __inl4_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(0)*(np_particles) + (si1)] = (1.0 - __inl4_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(1)*(np_particles) + (si1)] = __inl4_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_idx[__w0] = __inl4_j[__w0];
              }
            }
            if ((__inl1_og == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_j[__w0] = ((int64_t)((__inl1_x[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_xint[__w0] = (__inl1_x[__w0] - __inl4_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl4_xint[si1])) * (0.5 - __inl4_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(1)*(np_particles) + (si1)] = (0.75 - (__inl4_xint[si1] * __inl4_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl4_xint[si1])) * (0.5 + __inl4_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_idx[__w0] = (__inl4_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_j[__w0] = ((int64_t)(__inl1_x[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_xint[__w0] = (__inl1_x[__w0] - __inl4_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl4_xint[si1])) * (1.0 - __inl4_xint[si1])) * (1.0 - __inl4_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl4_xint[si1] * __inl4_xint[si1]) * (1.0 - (__inl4_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl4_xint[si1]) * (1.0 - __inl4_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl4_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl4_xint[si1]) * __inl4_xint[si1]) * __inl4_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_idx[__w0] = (__inl4_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_j[__w0] = ((int64_t)((__inl1_x[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_xint[__w0] = (__inl1_x[__w0] - __inl4_j[__w0]);
              }
              double *__inl4_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_sm[__w0] = (0.5 - __inl4_xint[__w0]);
              }
              double *__inl4_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_sp[__w0] = (0.5 + __inl4_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl4_sm[si1]) * __inl4_sm[si1]) * __inl4_sm[si1]) * __inl4_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl4_xint[si1])) + (((4.0 * __inl4_xint[si1]) * __inl4_xint[si1]) * ((1.5 + __inl4_xint[si1]) - (__inl4_xint[si1] * __inl4_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl4_xint[si1]) * __inl4_xint[si1]) * ((__inl4_xint[si1] * __inl4_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl4_xint[si1])) + (((4.0 * __inl4_xint[si1]) * __inl4_xint[si1]) * ((1.5 - __inl4_xint[si1]) - (__inl4_xint[si1] * __inl4_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_node_g[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl4_sp[si1]) * __inl4_sp[si1]) * __inl4_sp[si1]) * __inl4_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl4_idx[__w0] = (__inl4_j[__w0] - 2);
              }
              free(__inl4_sm);
              free(__inl4_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_j_node_v[__w0] = __inl4_idx[__w0];
            }
          }
          if (((((int64_t)(ex_type[0])) == 0) || (((int64_t)(by_type[0])) == 0) || (((int64_t)(bz_type[0])) == 0))) {
            memset(__inl5_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_og == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_j[__w0] = ((int64_t)(((__inl1_x[__w0] - 0.5) + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_idx[__w0] = __inl5_j[__w0];
              }
            }
            if ((__inl1_og == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_j[__w0] = ((int64_t)((__inl1_x[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl5_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(0)*(np_particles) + (si1)] = (1.0 - __inl5_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(1)*(np_particles) + (si1)] = __inl5_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_idx[__w0] = __inl5_j[__w0];
              }
            }
            if ((__inl1_og == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_j[__w0] = ((int64_t)(((__inl1_x[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl5_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl5_xint[si1])) * (0.5 - __inl5_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(1)*(np_particles) + (si1)] = (0.75 - (__inl5_xint[si1] * __inl5_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl5_xint[si1])) * (0.5 + __inl5_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_idx[__w0] = (__inl5_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_j[__w0] = ((int64_t)((__inl1_x[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl5_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl5_xint[si1])) * (1.0 - __inl5_xint[si1])) * (1.0 - __inl5_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl5_xint[si1] * __inl5_xint[si1]) * (1.0 - (__inl5_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl5_xint[si1]) * (1.0 - __inl5_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl5_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl5_xint[si1]) * __inl5_xint[si1]) * __inl5_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_idx[__w0] = (__inl5_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_j[__w0] = ((int64_t)(((__inl1_x[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_xint[__w0] = ((__inl1_x[__w0] - 0.5) - __inl5_j[__w0]);
              }
              double *__inl5_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_sm[__w0] = (0.5 - __inl5_xint[__w0]);
              }
              double *__inl5_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_sp[__w0] = (0.5 + __inl5_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl5_sm[si1]) * __inl5_sm[si1]) * __inl5_sm[si1]) * __inl5_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl5_xint[si1])) + (((4.0 * __inl5_xint[si1]) * __inl5_xint[si1]) * ((1.5 + __inl5_xint[si1]) - (__inl5_xint[si1] * __inl5_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl5_xint[si1]) * __inl5_xint[si1]) * ((__inl5_xint[si1] * __inl5_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl5_xint[si1])) + (((4.0 * __inl5_xint[si1]) * __inl5_xint[si1]) * ((1.5 - __inl5_xint[si1]) - (__inl5_xint[si1] * __inl5_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sx_cell_g[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl5_sp[si1]) * __inl5_sp[si1]) * __inl5_sp[si1]) * __inl5_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl5_idx[__w0] = (__inl5_j[__w0] - 2);
              }
              free(__inl5_sm);
              free(__inl5_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_j_cell_v[__w0] = __inl5_idx[__w0];
            }
          }
          free(__cb3);
          __cb3 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ex_type[0] == 1, __inl1_sx_node_g, __inl1_sx_cell_g) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb3[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ex_type[0])) == 1) ? __inl1_sx_node_g[(__r0)*(np_particles) + (__r1)] : __inl1_sx_cell_g[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sx_ex);
          __inl1_sx_ex = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sx_ex[(__w0)*(np_particles) + (__w1)] = __cb3[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb4);
          __cb4 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ey_type[0] == 1, __inl1_sx_node, __inl1_sx_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb4[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ey_type[0])) == 1) ? __inl1_sx_node[(__r0)*(np_particles) + (__r1)] : __inl1_sx_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sx_ey);
          __inl1_sx_ey = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sx_ey[(__w0)*(np_particles) + (__w1)] = __cb4[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb5);
          __cb5 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ez_type[0] == 1, __inl1_sx_node, __inl1_sx_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb5[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ez_type[0])) == 1) ? __inl1_sx_node[(__r0)*(np_particles) + (__r1)] : __inl1_sx_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sx_ez);
          __inl1_sx_ez = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sx_ez[(__w0)*(np_particles) + (__w1)] = __cb5[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb6);
          __cb6 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(bx_type[0] == 1, __inl1_sx_node, __inl1_sx_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb6[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(bx_type[0])) == 1) ? __inl1_sx_node[(__r0)*(np_particles) + (__r1)] : __inl1_sx_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sx_bx);
          __inl1_sx_bx = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sx_bx[(__w0)*(np_particles) + (__w1)] = __cb6[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb7);
          __cb7 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(by_type[0] == 1, __inl1_sx_node_g, __inl1_sx_cell_g) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb7[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(by_type[0])) == 1) ? __inl1_sx_node_g[(__r0)*(np_particles) + (__r1)] : __inl1_sx_cell_g[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sx_by);
          __inl1_sx_by = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sx_by[(__w0)*(np_particles) + (__w1)] = __cb7[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb8);
          __cb8 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(bz_type[0] == 1, __inl1_sx_node_g, __inl1_sx_cell_g) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb8[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(bz_type[0])) == 1) ? __inl1_sx_node_g[(__r0)*(np_particles) + (__r1)] : __inl1_sx_cell_g[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sx_bz);
          __inl1_sx_bz = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sx_bz[(__w0)*(np_particles) + (__w1)] = __cb8[(__w0)*(np_particles) + (__w1)];
            }
          }
          __inl1_n_sx_ex = (__inl1_og + 1);
          __inl1_n_sx_by = (__inl1_og + 1);
          __inl1_n_sx_bz = (__inl1_og + 1);
          __inl1_n_sx_ey = (__inl1_o + 1);
          __inl1_n_sx_ez = (__inl1_o + 1);
          __inl1_n_sx_bx = (__inl1_o + 1);
          int64_t *__cb9 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ex_type[0] == 1, __inl1_j_node_v, __inl1_j_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb9[__r0] = ((((int64_t)(ex_type[0])) == 1) ? __inl1_j_node_v[__r0] : __inl1_j_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_j_ex[__w0] = __cb9[__w0];
          }
          int64_t *__cb10 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ey_type[0] == 1, __inl1_j_node, __inl1_j_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb10[__r0] = ((((int64_t)(ey_type[0])) == 1) ? __inl1_j_node[__r0] : __inl1_j_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_j_ey[__w0] = __cb10[__w0];
          }
          int64_t *__cb11 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ez_type[0] == 1, __inl1_j_node, __inl1_j_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb11[__r0] = ((((int64_t)(ez_type[0])) == 1) ? __inl1_j_node[__r0] : __inl1_j_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_j_ez[__w0] = __cb11[__w0];
          }
          int64_t *__cb12 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(bx_type[0] == 1, __inl1_j_node, __inl1_j_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb12[__r0] = ((((int64_t)(bx_type[0])) == 1) ? __inl1_j_node[__r0] : __inl1_j_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_j_bx[__w0] = __cb12[__w0];
          }
          int64_t *__cb13 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(by_type[0] == 1, __inl1_j_node_v, __inl1_j_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb13[__r0] = ((((int64_t)(by_type[0])) == 1) ? __inl1_j_node_v[__r0] : __inl1_j_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_j_by[__w0] = __cb13[__w0];
          }
          int64_t *__cb14 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(bz_type[0] == 1, __inl1_j_node_v, __inl1_j_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb14[__r0] = ((((int64_t)(bz_type[0])) == 1) ? __inl1_j_node_v[__r0] : __inl1_j_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_j_bz[__w0] = __cb14[__w0];
          }
          free(__cb9);
          free(__cb10);
          free(__cb11);
          free(__cb12);
          free(__cb13);
          free(__cb14);
        }
        if ((g == 3)) {
          double *__inl1_y = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_y[__w0] = ((yp[__w0] - xyzmin[1]) * dinv[1]);
          }
          free(__inl1_sy_node);
          __inl1_sy_node = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sy_node, 0, (size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sy_cell);
          __inl1_sy_cell = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sy_cell, 0, (size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sy_node_v);
          __inl1_sy_node_v = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sy_node_v, 0, (size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sy_cell_v);
          __inl1_sy_cell_v = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sy_cell_v, 0, (size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_k_node, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_k_cell, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_k_node_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_k_cell_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
          if (((((int64_t)(ex_type[1])) == 1) || (((int64_t)(ez_type[1])) == 1) || (((int64_t)(by_type[1])) == 1))) {
            memset(__inl6_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_o == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_j[__w0] = ((int64_t)((__inl1_y[__w0] + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_idx[__w0] = __inl6_j[__w0];
              }
            }
            if ((__inl1_o == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_j[__w0] = ((int64_t)(__inl1_y[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_xint[__w0] = (__inl1_y[__w0] - __inl6_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(0)*(np_particles) + (si1)] = (1.0 - __inl6_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(1)*(np_particles) + (si1)] = __inl6_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_idx[__w0] = __inl6_j[__w0];
              }
            }
            if ((__inl1_o == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_j[__w0] = ((int64_t)((__inl1_y[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_xint[__w0] = (__inl1_y[__w0] - __inl6_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl6_xint[si1])) * (0.5 - __inl6_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(1)*(np_particles) + (si1)] = (0.75 - (__inl6_xint[si1] * __inl6_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl6_xint[si1])) * (0.5 + __inl6_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_idx[__w0] = (__inl6_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_j[__w0] = ((int64_t)(__inl1_y[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_xint[__w0] = (__inl1_y[__w0] - __inl6_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl6_xint[si1])) * (1.0 - __inl6_xint[si1])) * (1.0 - __inl6_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl6_xint[si1] * __inl6_xint[si1]) * (1.0 - (__inl6_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl6_xint[si1]) * (1.0 - __inl6_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl6_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl6_xint[si1]) * __inl6_xint[si1]) * __inl6_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_idx[__w0] = (__inl6_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_j[__w0] = ((int64_t)((__inl1_y[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_xint[__w0] = (__inl1_y[__w0] - __inl6_j[__w0]);
              }
              double *__inl6_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_sm[__w0] = (0.5 - __inl6_xint[__w0]);
              }
              double *__inl6_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_sp[__w0] = (0.5 + __inl6_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl6_sm[si1]) * __inl6_sm[si1]) * __inl6_sm[si1]) * __inl6_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl6_xint[si1])) + (((4.0 * __inl6_xint[si1]) * __inl6_xint[si1]) * ((1.5 + __inl6_xint[si1]) - (__inl6_xint[si1] * __inl6_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl6_xint[si1]) * __inl6_xint[si1]) * ((__inl6_xint[si1] * __inl6_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl6_xint[si1])) + (((4.0 * __inl6_xint[si1]) * __inl6_xint[si1]) * ((1.5 - __inl6_xint[si1]) - (__inl6_xint[si1] * __inl6_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl6_sp[si1]) * __inl6_sp[si1]) * __inl6_sp[si1]) * __inl6_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl6_idx[__w0] = (__inl6_j[__w0] - 2);
              }
              free(__inl6_sm);
              free(__inl6_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_k_node[__w0] = __inl6_idx[__w0];
            }
          }
          if (((((int64_t)(ex_type[1])) == 0) || (((int64_t)(ez_type[1])) == 0) || (((int64_t)(by_type[1])) == 0))) {
            memset(__inl7_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_o == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_j[__w0] = ((int64_t)(((__inl1_y[__w0] - 0.5) + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_idx[__w0] = __inl7_j[__w0];
              }
            }
            if ((__inl1_o == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_j[__w0] = ((int64_t)((__inl1_y[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl7_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(0)*(np_particles) + (si1)] = (1.0 - __inl7_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(1)*(np_particles) + (si1)] = __inl7_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_idx[__w0] = __inl7_j[__w0];
              }
            }
            if ((__inl1_o == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_j[__w0] = ((int64_t)(((__inl1_y[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl7_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl7_xint[si1])) * (0.5 - __inl7_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(1)*(np_particles) + (si1)] = (0.75 - (__inl7_xint[si1] * __inl7_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl7_xint[si1])) * (0.5 + __inl7_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_idx[__w0] = (__inl7_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_j[__w0] = ((int64_t)((__inl1_y[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl7_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl7_xint[si1])) * (1.0 - __inl7_xint[si1])) * (1.0 - __inl7_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl7_xint[si1] * __inl7_xint[si1]) * (1.0 - (__inl7_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl7_xint[si1]) * (1.0 - __inl7_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl7_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl7_xint[si1]) * __inl7_xint[si1]) * __inl7_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_idx[__w0] = (__inl7_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_j[__w0] = ((int64_t)(((__inl1_y[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl7_j[__w0]);
              }
              double *__inl7_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_sm[__w0] = (0.5 - __inl7_xint[__w0]);
              }
              double *__inl7_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_sp[__w0] = (0.5 + __inl7_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl7_sm[si1]) * __inl7_sm[si1]) * __inl7_sm[si1]) * __inl7_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl7_xint[si1])) + (((4.0 * __inl7_xint[si1]) * __inl7_xint[si1]) * ((1.5 + __inl7_xint[si1]) - (__inl7_xint[si1] * __inl7_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl7_xint[si1]) * __inl7_xint[si1]) * ((__inl7_xint[si1] * __inl7_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl7_xint[si1])) + (((4.0 * __inl7_xint[si1]) * __inl7_xint[si1]) * ((1.5 - __inl7_xint[si1]) - (__inl7_xint[si1] * __inl7_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl7_sp[si1]) * __inl7_sp[si1]) * __inl7_sp[si1]) * __inl7_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl7_idx[__w0] = (__inl7_j[__w0] - 2);
              }
              free(__inl7_sm);
              free(__inl7_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_k_cell[__w0] = __inl7_idx[__w0];
            }
          }
          if (((((int64_t)(ey_type[1])) == 1) || (((int64_t)(bx_type[1])) == 1) || (((int64_t)(bz_type[1])) == 1))) {
            memset(__inl8_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_og == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_j[__w0] = ((int64_t)((__inl1_y[__w0] + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_idx[__w0] = __inl8_j[__w0];
              }
            }
            if ((__inl1_og == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_j[__w0] = ((int64_t)(__inl1_y[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_xint[__w0] = (__inl1_y[__w0] - __inl8_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(0)*(np_particles) + (si1)] = (1.0 - __inl8_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(1)*(np_particles) + (si1)] = __inl8_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_idx[__w0] = __inl8_j[__w0];
              }
            }
            if ((__inl1_og == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_j[__w0] = ((int64_t)((__inl1_y[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_xint[__w0] = (__inl1_y[__w0] - __inl8_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl8_xint[si1])) * (0.5 - __inl8_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(1)*(np_particles) + (si1)] = (0.75 - (__inl8_xint[si1] * __inl8_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl8_xint[si1])) * (0.5 + __inl8_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_idx[__w0] = (__inl8_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_j[__w0] = ((int64_t)(__inl1_y[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_xint[__w0] = (__inl1_y[__w0] - __inl8_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl8_xint[si1])) * (1.0 - __inl8_xint[si1])) * (1.0 - __inl8_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl8_xint[si1] * __inl8_xint[si1]) * (1.0 - (__inl8_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl8_xint[si1]) * (1.0 - __inl8_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl8_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl8_xint[si1]) * __inl8_xint[si1]) * __inl8_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_idx[__w0] = (__inl8_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_j[__w0] = ((int64_t)((__inl1_y[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_xint[__w0] = (__inl1_y[__w0] - __inl8_j[__w0]);
              }
              double *__inl8_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_sm[__w0] = (0.5 - __inl8_xint[__w0]);
              }
              double *__inl8_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_sp[__w0] = (0.5 + __inl8_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl8_sm[si1]) * __inl8_sm[si1]) * __inl8_sm[si1]) * __inl8_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl8_xint[si1])) + (((4.0 * __inl8_xint[si1]) * __inl8_xint[si1]) * ((1.5 + __inl8_xint[si1]) - (__inl8_xint[si1] * __inl8_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl8_xint[si1]) * __inl8_xint[si1]) * ((__inl8_xint[si1] * __inl8_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl8_xint[si1])) + (((4.0 * __inl8_xint[si1]) * __inl8_xint[si1]) * ((1.5 - __inl8_xint[si1]) - (__inl8_xint[si1] * __inl8_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_node_v[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl8_sp[si1]) * __inl8_sp[si1]) * __inl8_sp[si1]) * __inl8_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl8_idx[__w0] = (__inl8_j[__w0] - 2);
              }
              free(__inl8_sm);
              free(__inl8_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_k_node_v[__w0] = __inl8_idx[__w0];
            }
          }
          if (((((int64_t)(ey_type[1])) == 0) || (((int64_t)(bx_type[1])) == 0) || (((int64_t)(bz_type[1])) == 0))) {
            memset(__inl9_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_og == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_j[__w0] = ((int64_t)(((__inl1_y[__w0] - 0.5) + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_idx[__w0] = __inl9_j[__w0];
              }
            }
            if ((__inl1_og == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_j[__w0] = ((int64_t)((__inl1_y[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl9_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(0)*(np_particles) + (si1)] = (1.0 - __inl9_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(1)*(np_particles) + (si1)] = __inl9_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_idx[__w0] = __inl9_j[__w0];
              }
            }
            if ((__inl1_og == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_j[__w0] = ((int64_t)(((__inl1_y[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl9_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl9_xint[si1])) * (0.5 - __inl9_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(1)*(np_particles) + (si1)] = (0.75 - (__inl9_xint[si1] * __inl9_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl9_xint[si1])) * (0.5 + __inl9_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_idx[__w0] = (__inl9_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_j[__w0] = ((int64_t)((__inl1_y[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl9_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl9_xint[si1])) * (1.0 - __inl9_xint[si1])) * (1.0 - __inl9_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl9_xint[si1] * __inl9_xint[si1]) * (1.0 - (__inl9_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl9_xint[si1]) * (1.0 - __inl9_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl9_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl9_xint[si1]) * __inl9_xint[si1]) * __inl9_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_idx[__w0] = (__inl9_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_j[__w0] = ((int64_t)(((__inl1_y[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_xint[__w0] = ((__inl1_y[__w0] - 0.5) - __inl9_j[__w0]);
              }
              double *__inl9_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_sm[__w0] = (0.5 - __inl9_xint[__w0]);
              }
              double *__inl9_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_sp[__w0] = (0.5 + __inl9_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl9_sm[si1]) * __inl9_sm[si1]) * __inl9_sm[si1]) * __inl9_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl9_xint[si1])) + (((4.0 * __inl9_xint[si1]) * __inl9_xint[si1]) * ((1.5 + __inl9_xint[si1]) - (__inl9_xint[si1] * __inl9_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl9_xint[si1]) * __inl9_xint[si1]) * ((__inl9_xint[si1] * __inl9_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl9_xint[si1])) + (((4.0 * __inl9_xint[si1]) * __inl9_xint[si1]) * ((1.5 - __inl9_xint[si1]) - (__inl9_xint[si1] * __inl9_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sy_cell_v[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl9_sp[si1]) * __inl9_sp[si1]) * __inl9_sp[si1]) * __inl9_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl9_idx[__w0] = (__inl9_j[__w0] - 2);
              }
              free(__inl9_sm);
              free(__inl9_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_k_cell_v[__w0] = __inl9_idx[__w0];
            }
          }
          free(__cb15);
          __cb15 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ex_type[1] == 1, __inl1_sy_node, __inl1_sy_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb15[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ex_type[1])) == 1) ? __inl1_sy_node[(__r0)*(np_particles) + (__r1)] : __inl1_sy_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sy_ex);
          __inl1_sy_ex = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sy_ex[(__w0)*(np_particles) + (__w1)] = __cb15[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb16);
          __cb16 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ey_type[1] == 1, __inl1_sy_node_v, __inl1_sy_cell_v) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb16[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ey_type[1])) == 1) ? __inl1_sy_node_v[(__r0)*(np_particles) + (__r1)] : __inl1_sy_cell_v[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sy_ey);
          __inl1_sy_ey = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sy_ey[(__w0)*(np_particles) + (__w1)] = __cb16[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb17);
          __cb17 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ez_type[1] == 1, __inl1_sy_node, __inl1_sy_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb17[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ez_type[1])) == 1) ? __inl1_sy_node[(__r0)*(np_particles) + (__r1)] : __inl1_sy_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sy_ez);
          __inl1_sy_ez = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sy_ez[(__w0)*(np_particles) + (__w1)] = __cb17[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb18);
          __cb18 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(bx_type[1] == 1, __inl1_sy_node_v, __inl1_sy_cell_v) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb18[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(bx_type[1])) == 1) ? __inl1_sy_node_v[(__r0)*(np_particles) + (__r1)] : __inl1_sy_cell_v[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sy_bx);
          __inl1_sy_bx = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sy_bx[(__w0)*(np_particles) + (__w1)] = __cb18[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb19);
          __cb19 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(by_type[1] == 1, __inl1_sy_node, __inl1_sy_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb19[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(by_type[1])) == 1) ? __inl1_sy_node[(__r0)*(np_particles) + (__r1)] : __inl1_sy_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sy_by);
          __inl1_sy_by = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sy_by[(__w0)*(np_particles) + (__w1)] = __cb19[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb20);
          __cb20 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(bz_type[1] == 1, __inl1_sy_node_v, __inl1_sy_cell_v) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb20[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(bz_type[1])) == 1) ? __inl1_sy_node_v[(__r0)*(np_particles) + (__r1)] : __inl1_sy_cell_v[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sy_bz);
          __inl1_sy_bz = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sy_bz[(__w0)*(np_particles) + (__w1)] = __cb20[(__w0)*(np_particles) + (__w1)];
            }
          }
          __inl1_n_sy_ey = (__inl1_og + 1);
          __inl1_n_sy_bx = (__inl1_og + 1);
          __inl1_n_sy_bz = (__inl1_og + 1);
          __inl1_n_sy_ex = (__inl1_o + 1);
          __inl1_n_sy_ez = (__inl1_o + 1);
          __inl1_n_sy_by = (__inl1_o + 1);
          int64_t *__cb21 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ex_type[1] == 1, __inl1_k_node, __inl1_k_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb21[__r0] = ((((int64_t)(ex_type[1])) == 1) ? __inl1_k_node[__r0] : __inl1_k_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_k_ex[__w0] = __cb21[__w0];
          }
          int64_t *__cb22 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ey_type[1] == 1, __inl1_k_node_v, __inl1_k_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb22[__r0] = ((((int64_t)(ey_type[1])) == 1) ? __inl1_k_node_v[__r0] : __inl1_k_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_k_ey[__w0] = __cb22[__w0];
          }
          int64_t *__cb23 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ez_type[1] == 1, __inl1_k_node, __inl1_k_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb23[__r0] = ((((int64_t)(ez_type[1])) == 1) ? __inl1_k_node[__r0] : __inl1_k_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_k_ez[__w0] = __cb23[__w0];
          }
          int64_t *__cb24 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(bx_type[1] == 1, __inl1_k_node_v, __inl1_k_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb24[__r0] = ((((int64_t)(bx_type[1])) == 1) ? __inl1_k_node_v[__r0] : __inl1_k_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_k_bx[__w0] = __cb24[__w0];
          }
          int64_t *__cb25 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(by_type[1] == 1, __inl1_k_node, __inl1_k_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb25[__r0] = ((((int64_t)(by_type[1])) == 1) ? __inl1_k_node[__r0] : __inl1_k_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_k_by[__w0] = __cb25[__w0];
          }
          int64_t *__cb26 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(bz_type[1] == 1, __inl1_k_node_v, __inl1_k_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb26[__r0] = ((((int64_t)(bz_type[1])) == 1) ? __inl1_k_node_v[__r0] : __inl1_k_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_k_bz[__w0] = __cb26[__w0];
          }
          free(__inl1_y);
          free(__cb21);
          free(__cb22);
          free(__cb23);
          free(__cb24);
          free(__cb25);
          free(__cb26);
        }
        if (((g != 4) && (g != 5))) {
          double *__inl1_z = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_z[__w0] = ((zp[__w0] - xyzmin[2]) * dinv[2]);
          }
          free(__inl1_sz_node);
          __inl1_sz_node = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sz_node, 0, (size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sz_cell);
          __inl1_sz_cell = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sz_cell, 0, (size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sz_node_v);
          __inl1_sz_node_v = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sz_node_v, 0, (size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          free(__inl1_sz_cell_v);
          __inl1_sz_cell_v = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_sz_cell_v, 0, (size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          memset(__inl1_l_node, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_l_cell, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_l_node_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
          memset(__inl1_l_cell_v, 0, (size_t)((np_particles)) * sizeof(int64_t));
          if (((((int64_t)(ex_type[__inl1_zdir])) == 1) || (((int64_t)(ey_type[__inl1_zdir])) == 1) || (((int64_t)(bz_type[__inl1_zdir])) == 1))) {
            memset(__inl10_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_o == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_j[__w0] = ((int64_t)((__inl1_z[__w0] + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_idx[__w0] = __inl10_j[__w0];
              }
            }
            if ((__inl1_o == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_j[__w0] = ((int64_t)(__inl1_z[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_xint[__w0] = (__inl1_z[__w0] - __inl10_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(0)*(np_particles) + (si1)] = (1.0 - __inl10_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(1)*(np_particles) + (si1)] = __inl10_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_idx[__w0] = __inl10_j[__w0];
              }
            }
            if ((__inl1_o == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_j[__w0] = ((int64_t)((__inl1_z[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_xint[__w0] = (__inl1_z[__w0] - __inl10_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl10_xint[si1])) * (0.5 - __inl10_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(1)*(np_particles) + (si1)] = (0.75 - (__inl10_xint[si1] * __inl10_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl10_xint[si1])) * (0.5 + __inl10_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_idx[__w0] = (__inl10_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_j[__w0] = ((int64_t)(__inl1_z[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_xint[__w0] = (__inl1_z[__w0] - __inl10_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl10_xint[si1])) * (1.0 - __inl10_xint[si1])) * (1.0 - __inl10_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl10_xint[si1] * __inl10_xint[si1]) * (1.0 - (__inl10_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl10_xint[si1]) * (1.0 - __inl10_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl10_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl10_xint[si1]) * __inl10_xint[si1]) * __inl10_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_idx[__w0] = (__inl10_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_j[__w0] = ((int64_t)((__inl1_z[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_xint[__w0] = (__inl1_z[__w0] - __inl10_j[__w0]);
              }
              double *__inl10_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_sm[__w0] = (0.5 - __inl10_xint[__w0]);
              }
              double *__inl10_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_sp[__w0] = (0.5 + __inl10_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl10_sm[si1]) * __inl10_sm[si1]) * __inl10_sm[si1]) * __inl10_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl10_xint[si1])) + (((4.0 * __inl10_xint[si1]) * __inl10_xint[si1]) * ((1.5 + __inl10_xint[si1]) - (__inl10_xint[si1] * __inl10_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl10_xint[si1]) * __inl10_xint[si1]) * ((__inl10_xint[si1] * __inl10_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl10_xint[si1])) + (((4.0 * __inl10_xint[si1]) * __inl10_xint[si1]) * ((1.5 - __inl10_xint[si1]) - (__inl10_xint[si1] * __inl10_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl10_sp[si1]) * __inl10_sp[si1]) * __inl10_sp[si1]) * __inl10_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl10_idx[__w0] = (__inl10_j[__w0] - 2);
              }
              free(__inl10_sm);
              free(__inl10_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_l_node[__w0] = __inl10_idx[__w0];
            }
          }
          if (((((int64_t)(ex_type[__inl1_zdir])) == 0) || (((int64_t)(ey_type[__inl1_zdir])) == 0) || (((int64_t)(bz_type[__inl1_zdir])) == 0))) {
            memset(__inl11_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_o == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_j[__w0] = ((int64_t)(((__inl1_z[__w0] - 0.5) + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_idx[__w0] = __inl11_j[__w0];
              }
            }
            if ((__inl1_o == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_j[__w0] = ((int64_t)((__inl1_z[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl11_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(0)*(np_particles) + (si1)] = (1.0 - __inl11_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(1)*(np_particles) + (si1)] = __inl11_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_idx[__w0] = __inl11_j[__w0];
              }
            }
            if ((__inl1_o == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_j[__w0] = ((int64_t)(((__inl1_z[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl11_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl11_xint[si1])) * (0.5 - __inl11_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(1)*(np_particles) + (si1)] = (0.75 - (__inl11_xint[si1] * __inl11_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl11_xint[si1])) * (0.5 + __inl11_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_idx[__w0] = (__inl11_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_j[__w0] = ((int64_t)((__inl1_z[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl11_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl11_xint[si1])) * (1.0 - __inl11_xint[si1])) * (1.0 - __inl11_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl11_xint[si1] * __inl11_xint[si1]) * (1.0 - (__inl11_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl11_xint[si1]) * (1.0 - __inl11_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl11_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl11_xint[si1]) * __inl11_xint[si1]) * __inl11_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_idx[__w0] = (__inl11_j[__w0] - 1);
              }
            }
            if ((__inl1_o == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_j[__w0] = ((int64_t)(((__inl1_z[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl11_j[__w0]);
              }
              double *__inl11_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_sm[__w0] = (0.5 - __inl11_xint[__w0]);
              }
              double *__inl11_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_sp[__w0] = (0.5 + __inl11_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl11_sm[si1]) * __inl11_sm[si1]) * __inl11_sm[si1]) * __inl11_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl11_xint[si1])) + (((4.0 * __inl11_xint[si1]) * __inl11_xint[si1]) * ((1.5 + __inl11_xint[si1]) - (__inl11_xint[si1] * __inl11_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl11_xint[si1]) * __inl11_xint[si1]) * ((__inl11_xint[si1] * __inl11_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl11_xint[si1])) + (((4.0 * __inl11_xint[si1]) * __inl11_xint[si1]) * ((1.5 - __inl11_xint[si1]) - (__inl11_xint[si1] * __inl11_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl11_sp[si1]) * __inl11_sp[si1]) * __inl11_sp[si1]) * __inl11_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl11_idx[__w0] = (__inl11_j[__w0] - 2);
              }
              free(__inl11_sm);
              free(__inl11_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_l_cell[__w0] = __inl11_idx[__w0];
            }
          }
          if (((((int64_t)(ez_type[__inl1_zdir])) == 1) || (((int64_t)(bx_type[__inl1_zdir])) == 1) || (((int64_t)(by_type[__inl1_zdir])) == 1))) {
            memset(__inl12_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_og == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_j[__w0] = ((int64_t)((__inl1_z[__w0] + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_idx[__w0] = __inl12_j[__w0];
              }
            }
            if ((__inl1_og == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_j[__w0] = ((int64_t)(__inl1_z[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_xint[__w0] = (__inl1_z[__w0] - __inl12_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(0)*(np_particles) + (si1)] = (1.0 - __inl12_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(1)*(np_particles) + (si1)] = __inl12_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_idx[__w0] = __inl12_j[__w0];
              }
            }
            if ((__inl1_og == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_j[__w0] = ((int64_t)((__inl1_z[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_xint[__w0] = (__inl1_z[__w0] - __inl12_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl12_xint[si1])) * (0.5 - __inl12_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(1)*(np_particles) + (si1)] = (0.75 - (__inl12_xint[si1] * __inl12_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl12_xint[si1])) * (0.5 + __inl12_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_idx[__w0] = (__inl12_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_j[__w0] = ((int64_t)(__inl1_z[__w0]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_xint[__w0] = (__inl1_z[__w0] - __inl12_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl12_xint[si1])) * (1.0 - __inl12_xint[si1])) * (1.0 - __inl12_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl12_xint[si1] * __inl12_xint[si1]) * (1.0 - (__inl12_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl12_xint[si1]) * (1.0 - __inl12_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl12_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl12_xint[si1]) * __inl12_xint[si1]) * __inl12_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_idx[__w0] = (__inl12_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_j[__w0] = ((int64_t)((__inl1_z[__w0] + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_xint[__w0] = (__inl1_z[__w0] - __inl12_j[__w0]);
              }
              double *__inl12_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_sm[__w0] = (0.5 - __inl12_xint[__w0]);
              }
              double *__inl12_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_sp[__w0] = (0.5 + __inl12_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl12_sm[si1]) * __inl12_sm[si1]) * __inl12_sm[si1]) * __inl12_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl12_xint[si1])) + (((4.0 * __inl12_xint[si1]) * __inl12_xint[si1]) * ((1.5 + __inl12_xint[si1]) - (__inl12_xint[si1] * __inl12_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl12_xint[si1]) * __inl12_xint[si1]) * ((__inl12_xint[si1] * __inl12_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl12_xint[si1])) + (((4.0 * __inl12_xint[si1]) * __inl12_xint[si1]) * ((1.5 - __inl12_xint[si1]) - (__inl12_xint[si1] * __inl12_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_node_v[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl12_sp[si1]) * __inl12_sp[si1]) * __inl12_sp[si1]) * __inl12_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl12_idx[__w0] = (__inl12_j[__w0] - 2);
              }
              free(__inl12_sm);
              free(__inl12_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_l_node_v[__w0] = __inl12_idx[__w0];
            }
          }
          if (((((int64_t)(ez_type[__inl1_zdir])) == 0) || (((int64_t)(bx_type[__inl1_zdir])) == 0) || (((int64_t)(by_type[__inl1_zdir])) == 0))) {
            memset(__inl13_idx, 0, (size_t)((np_particles)) * sizeof(int64_t));
            if ((__inl1_og == 0)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_j[__w0] = ((int64_t)(((__inl1_z[__w0] - 0.5) + 0.5)));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(0)*(np_particles) + (si1)] = 1.0;
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_idx[__w0] = __inl13_j[__w0];
              }
            }
            if ((__inl1_og == 1)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_j[__w0] = ((int64_t)((__inl1_z[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl13_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(0)*(np_particles) + (si1)] = (1.0 - __inl13_xint[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(1)*(np_particles) + (si1)] = __inl13_xint[si1];
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_idx[__w0] = __inl13_j[__w0];
              }
            }
            if ((__inl1_og == 2)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_j[__w0] = ((int64_t)(((__inl1_z[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl13_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(0)*(np_particles) + (si1)] = ((0.5 * (0.5 - __inl13_xint[si1])) * (0.5 - __inl13_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(1)*(np_particles) + (si1)] = (0.75 - (__inl13_xint[si1] * __inl13_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(2)*(np_particles) + (si1)] = ((0.5 * (0.5 + __inl13_xint[si1])) * (0.5 + __inl13_xint[si1]));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_idx[__w0] = (__inl13_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 3)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_j[__w0] = ((int64_t)((__inl1_z[__w0] - 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl13_j[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(0)*(np_particles) + (si1)] = ((((1.0 / 6.0) * (1.0 - __inl13_xint[si1])) * (1.0 - __inl13_xint[si1])) * (1.0 - __inl13_xint[si1]));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(1)*(np_particles) + (si1)] = ((2.0 / 3.0) - ((__inl13_xint[si1] * __inl13_xint[si1]) * (1.0 - (__inl13_xint[si1] / 2.0))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(2)*(np_particles) + (si1)] = ((2.0 / 3.0) - (((1.0 - __inl13_xint[si1]) * (1.0 - __inl13_xint[si1])) * (1.0 - (0.5 * (1.0 - __inl13_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(3)*(np_particles) + (si1)] = ((((1.0 / 6.0) * __inl13_xint[si1]) * __inl13_xint[si1]) * __inl13_xint[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_idx[__w0] = (__inl13_j[__w0] - 1);
              }
            }
            if ((__inl1_og == 4)) {
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_j[__w0] = ((int64_t)(((__inl1_z[__w0] - 0.5) + 0.5)));
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_xint[__w0] = ((__inl1_z[__w0] - 0.5) - __inl13_j[__w0]);
              }
              double *__inl13_sm = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_sm[__w0] = (0.5 - __inl13_xint[__w0]);
              }
              double *__inl13_sp = (double *)malloc(((np_particles)) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_sp[__w0] = (0.5 + __inl13_xint[__w0]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(0)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl13_sm[si1]) * __inl13_sm[si1]) * __inl13_sm[si1]) * __inl13_sm[si1]);
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(1)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl13_xint[si1])) + (((4.0 * __inl13_xint[si1]) * __inl13_xint[si1]) * ((1.5 + __inl13_xint[si1]) - (__inl13_xint[si1] * __inl13_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(2)*(np_particles) + (si1)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl13_xint[si1]) * __inl13_xint[si1]) * ((__inl13_xint[si1] * __inl13_xint[si1]) - 2.5))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(3)*(np_particles) + (si1)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl13_xint[si1])) + (((4.0 * __inl13_xint[si1]) * __inl13_xint[si1]) * ((1.5 - __inl13_xint[si1]) - (__inl13_xint[si1] * __inl13_xint[si1])))));
              }
              for (int64_t si1 = 0; si1 < np_particles; ++si1) {
                __inl1_sz_cell_v[(4)*(np_particles) + (si1)] = (((((1.0 / 24.0) * __inl13_sp[si1]) * __inl13_sp[si1]) * __inl13_sp[si1]) * __inl13_sp[si1]);
              }
              for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
                __inl13_idx[__w0] = (__inl13_j[__w0] - 2);
              }
              free(__inl13_sm);
              free(__inl13_sp);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_l_cell_v[__w0] = __inl13_idx[__w0];
            }
          }
          free(__cb27);
          __cb27 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ex_type[__inl1_zdir] == 1, __inl1_sz_node, __inl1_sz_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb27[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ex_type[__inl1_zdir])) == 1) ? __inl1_sz_node[(__r0)*(np_particles) + (__r1)] : __inl1_sz_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sz_ex);
          __inl1_sz_ex = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sz_ex[(__w0)*(np_particles) + (__w1)] = __cb27[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb28);
          __cb28 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ey_type[__inl1_zdir] == 1, __inl1_sz_node, __inl1_sz_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb28[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ey_type[__inl1_zdir])) == 1) ? __inl1_sz_node[(__r0)*(np_particles) + (__r1)] : __inl1_sz_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sz_ey);
          __inl1_sz_ey = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sz_ey[(__w0)*(np_particles) + (__w1)] = __cb28[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb29);
          __cb29 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(ez_type[__inl1_zdir] == 1, __inl1_sz_node_v, __inl1_sz_cell_v) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb29[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(ez_type[__inl1_zdir])) == 1) ? __inl1_sz_node_v[(__r0)*(np_particles) + (__r1)] : __inl1_sz_cell_v[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sz_ez);
          __inl1_sz_ez = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sz_ez[(__w0)*(np_particles) + (__w1)] = __cb29[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb30);
          __cb30 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(bx_type[__inl1_zdir] == 1, __inl1_sz_node_v, __inl1_sz_cell_v) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb30[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(bx_type[__inl1_zdir])) == 1) ? __inl1_sz_node_v[(__r0)*(np_particles) + (__r1)] : __inl1_sz_cell_v[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sz_bx);
          __inl1_sz_bx = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sz_bx[(__w0)*(np_particles) + (__w1)] = __cb30[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb31);
          __cb31 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(by_type[__inl1_zdir] == 1, __inl1_sz_node_v, __inl1_sz_cell_v) */
          for (int64_t __r0 = 0; __r0 < ((o - gal) + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb31[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(by_type[__inl1_zdir])) == 1) ? __inl1_sz_node_v[(__r0)*(np_particles) + (__r1)] : __inl1_sz_cell_v[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sz_by);
          __inl1_sz_by = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sz_by[(__w0)*(np_particles) + (__w1)] = __cb31[(__w0)*(np_particles) + (__w1)];
            }
          }
          free(__cb32);
          __cb32 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          /* numpy: np.where(bz_type[__inl1_zdir] == 1, __inl1_sz_node, __inl1_sz_cell) */
          for (int64_t __r0 = 0; __r0 < (o + 1); ++__r0) {
            for (int64_t __r1 = 0; __r1 < np_particles; ++__r1) {
              __cb32[(__r0)*(np_particles) + (__r1)] = ((((int64_t)(bz_type[__inl1_zdir])) == 1) ? __inl1_sz_node[(__r0)*(np_particles) + (__r1)] : __inl1_sz_cell[(__r0)*(np_particles) + (__r1)]);
            }
          }
          free(__inl1_sz_bz);
          __inl1_sz_bz = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl1_sz_bz[(__w0)*(np_particles) + (__w1)] = __cb32[(__w0)*(np_particles) + (__w1)];
            }
          }
          __inl1_n_sz_ez = (__inl1_og + 1);
          __inl1_n_sz_bx = (__inl1_og + 1);
          __inl1_n_sz_by = (__inl1_og + 1);
          __inl1_n_sz_ex = (__inl1_o + 1);
          __inl1_n_sz_ey = (__inl1_o + 1);
          __inl1_n_sz_bz = (__inl1_o + 1);
          int64_t *__cb33 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ex_type[__inl1_zdir] == 1, __inl1_l_node, __inl1_l_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb33[__r0] = ((((int64_t)(ex_type[__inl1_zdir])) == 1) ? __inl1_l_node[__r0] : __inl1_l_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_l_ex[__w0] = __cb33[__w0];
          }
          int64_t *__cb34 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ey_type[__inl1_zdir] == 1, __inl1_l_node, __inl1_l_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb34[__r0] = ((((int64_t)(ey_type[__inl1_zdir])) == 1) ? __inl1_l_node[__r0] : __inl1_l_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_l_ey[__w0] = __cb34[__w0];
          }
          int64_t *__cb35 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(ez_type[__inl1_zdir] == 1, __inl1_l_node_v, __inl1_l_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb35[__r0] = ((((int64_t)(ez_type[__inl1_zdir])) == 1) ? __inl1_l_node_v[__r0] : __inl1_l_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_l_ez[__w0] = __cb35[__w0];
          }
          int64_t *__cb36 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(bx_type[__inl1_zdir] == 1, __inl1_l_node_v, __inl1_l_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb36[__r0] = ((((int64_t)(bx_type[__inl1_zdir])) == 1) ? __inl1_l_node_v[__r0] : __inl1_l_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_l_bx[__w0] = __cb36[__w0];
          }
          int64_t *__cb37 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(by_type[__inl1_zdir] == 1, __inl1_l_node_v, __inl1_l_cell_v) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb37[__r0] = ((((int64_t)(by_type[__inl1_zdir])) == 1) ? __inl1_l_node_v[__r0] : __inl1_l_cell_v[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_l_by[__w0] = __cb37[__w0];
          }
          int64_t *__cb38 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(bz_type[__inl1_zdir] == 1, __inl1_l_node, __inl1_l_cell) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb38[__r0] = ((((int64_t)(bz_type[__inl1_zdir])) == 1) ? __inl1_l_node[__r0] : __inl1_l_cell[__r0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_l_bz[__w0] = __cb38[__w0];
          }
          free(__inl1_z);
          free(__cb33);
          free(__cb34);
          free(__cb35);
          free(__cb36);
          free(__cb37);
          free(__cb38);
        }
        __inl1_lox = ((int64_t)(lo[0]));
        __inl1_loy = ((int64_t)(lo[1]));
        __inl1_loz = ((int64_t)(lo[2]));
        if ((g == 0)) {
          free(__cb39);
          __cb39 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ey; ++__i) {
            __cb39[__i] = __i;
          }
          free(__inl14_taps);
          __inl14_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
            __inl14_taps[__w0] = __cb39[__w0];
          }
          free(__inl14_rows);
          __inl14_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl14_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_l_ey[__w1]) + __inl14_taps[__w0]);
            }
          }
          free(__inl14_gathered);
          __inl14_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl14_gathered[(__w0)*(np_particles) + (__w1)] = ey_arr[(((__inl14_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb40);
          __cb40 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb40[(__w0)*(np_particles) + (__w1)] = (__inl1_sz_ey[(__w0)*(np_particles) + (__w1)] * __inl14_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb41 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb40, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb41[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb41[__ax0] = (__cb41[__ax0] + __cb40[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall1 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall1[__w0] = __cb41[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Eyp[__w0] += __hcall1[__w0];
          }
          free(__cb42);
          __cb42 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ex; ++__i) {
            __cb42[__i] = __i;
          }
          free(__inl15_taps);
          __inl15_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
            __inl15_taps[__w0] = __cb42[__w0];
          }
          free(__inl15_rows);
          __inl15_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl15_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_l_ex[__w1]) + __inl15_taps[__w0]);
            }
          }
          free(__inl15_gathered);
          __inl15_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl15_gathered[(__w0)*(np_particles) + (__w1)] = ex_arr[(((__inl15_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb43);
          __cb43 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb43[(__w0)*(np_particles) + (__w1)] = (__inl1_sz_ex[(__w0)*(np_particles) + (__w1)] * __inl15_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb44 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb43, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb44[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb44[__ax0] = (__cb44[__ax0] + __cb43[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall2 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall2[__w0] = __cb44[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Exp[__w0] += __hcall2[__w0];
          }
          free(__cb45);
          __cb45 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bz; ++__i) {
            __cb45[__i] = __i;
          }
          free(__inl16_taps);
          __inl16_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
            __inl16_taps[__w0] = __cb45[__w0];
          }
          free(__inl16_rows);
          __inl16_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl16_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_l_bz[__w1]) + __inl16_taps[__w0]);
            }
          }
          free(__inl16_gathered);
          __inl16_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl16_gathered[(__w0)*(np_particles) + (__w1)] = bz_arr[(((__inl16_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb46);
          __cb46 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb46[(__w0)*(np_particles) + (__w1)] = (__inl1_sz_bz[(__w0)*(np_particles) + (__w1)] * __inl16_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb47 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb46, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb47[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb47[__ax0] = (__cb47[__ax0] + __cb46[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall3 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall3[__w0] = __cb47[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bzp[__w0] += __hcall3[__w0];
          }
          free(__cb48);
          __cb48 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ez; ++__i) {
            __cb48[__i] = __i;
          }
          free(__inl17_taps);
          __inl17_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
            __inl17_taps[__w0] = __cb48[__w0];
          }
          free(__inl17_rows);
          __inl17_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl17_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_l_ez[__w1]) + __inl17_taps[__w0]);
            }
          }
          free(__inl17_gathered);
          __inl17_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl17_gathered[(__w0)*(np_particles) + (__w1)] = ez_arr[(((__inl17_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb49);
          __cb49 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb49[(__w0)*(np_particles) + (__w1)] = (__inl1_sz_ez[(__w0)*(np_particles) + (__w1)] * __inl17_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb50 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb49, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb50[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb50[__ax0] = (__cb50[__ax0] + __cb49[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall4 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall4[__w0] = __cb50[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Ezp[__w0] += __hcall4[__w0];
          }
          free(__cb51);
          __cb51 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bx; ++__i) {
            __cb51[__i] = __i;
          }
          free(__inl18_taps);
          __inl18_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
            __inl18_taps[__w0] = __cb51[__w0];
          }
          free(__inl18_rows);
          __inl18_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl18_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_l_bx[__w1]) + __inl18_taps[__w0]);
            }
          }
          free(__inl18_gathered);
          __inl18_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl18_gathered[(__w0)*(np_particles) + (__w1)] = bx_arr[(((__inl18_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb52);
          __cb52 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb52[(__w0)*(np_particles) + (__w1)] = (__inl1_sz_bx[(__w0)*(np_particles) + (__w1)] * __inl18_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb53 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb52, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb53[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb53[__ax0] = (__cb53[__ax0] + __cb52[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall5 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall5[__w0] = __cb53[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bxp[__w0] += __hcall5[__w0];
          }
          free(__cb54);
          __cb54 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_by) */
          for (int64_t __i = 0; __i < __inl1_n_sz_by; ++__i) {
            __cb54[__i] = __i;
          }
          free(__inl19_taps);
          __inl19_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
            __inl19_taps[__w0] = __cb54[__w0];
          }
          free(__inl19_rows);
          __inl19_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl19_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_l_by[__w1]) + __inl19_taps[__w0]);
            }
          }
          free(__inl19_gathered);
          __inl19_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl19_gathered[(__w0)*(np_particles) + (__w1)] = by_arr[(((__inl19_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb55);
          __cb55 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb55[(__w0)*(np_particles) + (__w1)] = (__inl1_sz_by[(__w0)*(np_particles) + (__w1)] * __inl19_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb56 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb55, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb56[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb56[__ax0] = (__cb56[__ax0] + __cb55[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall6 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall6[__w0] = __cb56[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Byp[__w0] += __hcall6[__w0];
          }
          free(__cb41);
          free(__hcall1);
          free(__cb44);
          free(__hcall2);
          free(__cb47);
          free(__hcall3);
          free(__cb50);
          free(__hcall4);
          free(__cb53);
          free(__hcall5);
          free(__cb56);
          free(__hcall6);
        }
        else if ((g == 1)) {
          free(__cb57);
          __cb57 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
            __cb57[__i] = __i;
          }
          free(__inl20_ta);
          __inl20_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            __inl20_ta[__w0] = __cb57[__w0];
          }
          free(__cb58);
          __cb58 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ey; ++__i) {
            __cb58[__i] = __i;
          }
          free(__inl20_tb);
          __inl20_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
            __inl20_tb[__w0] = __cb58[__w0];
          }
          free(__inl20_ia);
          __inl20_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl20_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ey[si2]) + __inl20_ta[si0]);
              }
            }
          }
          free(__inl20_ib);
          __inl20_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl20_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ey[si2]) + __inl20_tb[si1]);
              }
            }
          }
          free(__inl20_ia_b);
          __inl20_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl20_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl20_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl20_ib_b);
          __inl20_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl20_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl20_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl20_gathered);
          __inl20_gathered = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl20_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ey_arr[(((__inl20_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl20_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl20_weight);
          __inl20_weight = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl20_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ey[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ey[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb59);
          __cb59 = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb59[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl20_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl20_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb60 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb59, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb60[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                __cb60[__ax0] = (__cb60[__ax0] + __cb59[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall7 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall7[__w0] = __cb60[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Eyp[__w0] += __hcall7[__w0];
          }
          free(__cb61);
          __cb61 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
            __cb61[__i] = __i;
          }
          free(__inl21_ta);
          __inl21_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            __inl21_ta[__w0] = __cb61[__w0];
          }
          free(__cb62);
          __cb62 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ex; ++__i) {
            __cb62[__i] = __i;
          }
          free(__inl21_tb);
          __inl21_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
            __inl21_tb[__w0] = __cb62[__w0];
          }
          free(__inl21_ia);
          __inl21_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl21_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ex[si2]) + __inl21_ta[si0]);
              }
            }
          }
          free(__inl21_ib);
          __inl21_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl21_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ex[si2]) + __inl21_tb[si1]);
              }
            }
          }
          free(__inl21_ia_b);
          __inl21_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl21_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl21_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl21_ib_b);
          __inl21_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl21_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl21_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl21_gathered);
          __inl21_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl21_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ex_arr[(((__inl21_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl21_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl21_weight);
          __inl21_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl21_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ex[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ex[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb63);
          __cb63 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb63[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl21_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl21_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb64 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb63, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb64[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                __cb64[__ax0] = (__cb64[__ax0] + __cb63[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall8 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall8[__w0] = __cb64[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Exp[__w0] += __hcall8[__w0];
          }
          free(__cb65);
          __cb65 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
            __cb65[__i] = __i;
          }
          free(__inl22_ta);
          __inl22_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            __inl22_ta[__w0] = __cb65[__w0];
          }
          free(__cb66);
          __cb66 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bz; ++__i) {
            __cb66[__i] = __i;
          }
          free(__inl22_tb);
          __inl22_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
            __inl22_tb[__w0] = __cb66[__w0];
          }
          free(__inl22_ia);
          __inl22_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl22_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bz[si2]) + __inl22_ta[si0]);
              }
            }
          }
          free(__inl22_ib);
          __inl22_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl22_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bz[si2]) + __inl22_tb[si1]);
              }
            }
          }
          free(__inl22_ia_b);
          __inl22_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl22_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl22_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl22_ib_b);
          __inl22_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl22_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl22_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl22_gathered);
          __inl22_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl22_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = bz_arr[(((__inl22_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl22_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl22_weight);
          __inl22_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl22_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bz[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bz[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb67);
          __cb67 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb67[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl22_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl22_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb68 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb67, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb68[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                __cb68[__ax0] = (__cb68[__ax0] + __cb67[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall9 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall9[__w0] = __cb68[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bzp[__w0] += __hcall9[__w0];
          }
          free(__cb69);
          __cb69 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
            __cb69[__i] = __i;
          }
          free(__inl23_ta);
          __inl23_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            __inl23_ta[__w0] = __cb69[__w0];
          }
          free(__cb70);
          __cb70 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ez; ++__i) {
            __cb70[__i] = __i;
          }
          free(__inl23_tb);
          __inl23_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
            __inl23_tb[__w0] = __cb70[__w0];
          }
          free(__inl23_ia);
          __inl23_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl23_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ez[si2]) + __inl23_ta[si0]);
              }
            }
          }
          free(__inl23_ib);
          __inl23_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl23_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ez[si2]) + __inl23_tb[si1]);
              }
            }
          }
          free(__inl23_ia_b);
          __inl23_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl23_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl23_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl23_ib_b);
          __inl23_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl23_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl23_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl23_gathered);
          __inl23_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl23_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = ez_arr[(((__inl23_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl23_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl23_weight);
          __inl23_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl23_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ez[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ez[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb71);
          __cb71 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb71[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl23_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl23_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb72 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb71, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb72[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                __cb72[__ax0] = (__cb72[__ax0] + __cb71[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall10 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall10[__w0] = __cb72[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Ezp[__w0] += __hcall10[__w0];
          }
          free(__cb73);
          __cb73 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
            __cb73[__i] = __i;
          }
          free(__inl24_ta);
          __inl24_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            __inl24_ta[__w0] = __cb73[__w0];
          }
          free(__cb74);
          __cb74 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bx; ++__i) {
            __cb74[__i] = __i;
          }
          free(__inl24_tb);
          __inl24_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
            __inl24_tb[__w0] = __cb74[__w0];
          }
          free(__inl24_ia);
          __inl24_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl24_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bx[si2]) + __inl24_ta[si0]);
              }
            }
          }
          free(__inl24_ib);
          __inl24_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl24_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bx[si2]) + __inl24_tb[si1]);
              }
            }
          }
          free(__inl24_ia_b);
          __inl24_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl24_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl24_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl24_ib_b);
          __inl24_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl24_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl24_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl24_gathered);
          __inl24_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl24_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = bx_arr[(((__inl24_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl24_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl24_weight);
          __inl24_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl24_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bx[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bx[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb75);
          __cb75 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb75[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl24_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl24_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb76 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb75, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb76[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                __cb76[__ax0] = (__cb76[__ax0] + __cb75[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall11 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall11[__w0] = __cb76[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bxp[__w0] += __hcall11[__w0];
          }
          free(__cb77);
          __cb77 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_by) */
          for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
            __cb77[__i] = __i;
          }
          free(__inl25_ta);
          __inl25_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            __inl25_ta[__w0] = __cb77[__w0];
          }
          free(__cb78);
          __cb78 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_by) */
          for (int64_t __i = 0; __i < __inl1_n_sz_by; ++__i) {
            __cb78[__i] = __i;
          }
          free(__inl25_tb);
          __inl25_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
            __inl25_tb[__w0] = __cb78[__w0];
          }
          free(__inl25_ia);
          __inl25_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl25_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_by[si2]) + __inl25_ta[si0]);
              }
            }
          }
          free(__inl25_ib);
          __inl25_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl25_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_by[si2]) + __inl25_tb[si1]);
              }
            }
          }
          free(__inl25_ia_b);
          __inl25_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl25_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl25_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl25_ib_b);
          __inl25_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl25_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl25_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl25_gathered);
          __inl25_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl25_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = by_arr[(((__inl25_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl25_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl25_weight);
          __inl25_weight = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl25_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_by[(__w0)*(np_particles) + (__w2)] * __inl1_sz_by[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb79);
          __cb79 = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb79[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl25_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl25_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb80 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb79, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb80[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                __cb80[__ax0] = (__cb80[__ax0] + __cb79[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall12 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall12[__w0] = __cb80[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Byp[__w0] += __hcall12[__w0];
          }
          free(__cb60);
          free(__hcall7);
          free(__cb64);
          free(__hcall8);
          free(__cb68);
          free(__hcall9);
          free(__cb72);
          free(__hcall10);
          free(__cb76);
          free(__hcall11);
          free(__cb80);
          free(__hcall12);
        }
        else if ((g == 2)) {
          free(__cb81);
          __cb81 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
            __cb81[__i] = __i;
          }
          free(__inl26_ta);
          __inl26_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            __inl26_ta[__w0] = __cb81[__w0];
          }
          free(__cb82);
          __cb82 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ey; ++__i) {
            __cb82[__i] = __i;
          }
          free(__inl26_tb);
          __inl26_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
            __inl26_tb[__w0] = __cb82[__w0];
          }
          free(__inl26_ia);
          __inl26_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl26_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ey[si2]) + __inl26_ta[si0]);
              }
            }
          }
          free(__inl26_ib);
          __inl26_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl26_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ey[si2]) + __inl26_tb[si1]);
              }
            }
          }
          free(__inl26_ia_b);
          __inl26_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl26_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl26_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl26_ib_b);
          __inl26_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl26_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl26_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl26_gathered);
          __inl26_gathered = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl26_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ey_arr[(((__inl26_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl26_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl26_weight);
          __inl26_weight = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl26_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ey[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ey[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb83);
          __cb83 = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb83[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl26_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl26_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb84 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb83, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb84[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                __cb84[__ax0] = (__cb84[__ax0] + __cb83[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Ethetap[__w0] = __cb84[__w0];
          }
          free(__cb85);
          __cb85 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
            __cb85[__i] = __i;
          }
          free(__inl27_ta);
          __inl27_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            __inl27_ta[__w0] = __cb85[__w0];
          }
          free(__cb86);
          __cb86 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ex; ++__i) {
            __cb86[__i] = __i;
          }
          free(__inl27_tb);
          __inl27_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
            __inl27_tb[__w0] = __cb86[__w0];
          }
          free(__inl27_ia);
          __inl27_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl27_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ex[si2]) + __inl27_ta[si0]);
              }
            }
          }
          free(__inl27_ib);
          __inl27_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl27_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ex[si2]) + __inl27_tb[si1]);
              }
            }
          }
          free(__inl27_ia_b);
          __inl27_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl27_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl27_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl27_ib_b);
          __inl27_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl27_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl27_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl27_gathered);
          __inl27_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl27_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ex_arr[(((__inl27_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl27_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl27_weight);
          __inl27_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl27_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ex[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ex[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb87);
          __cb87 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb87[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl27_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl27_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb88 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb87, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb88[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                __cb88[__ax0] = (__cb88[__ax0] + __cb87[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Erp[__w0] = __cb88[__w0];
          }
          free(__cb89);
          __cb89 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
            __cb89[__i] = __i;
          }
          free(__inl28_ta);
          __inl28_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            __inl28_ta[__w0] = __cb89[__w0];
          }
          free(__cb90);
          __cb90 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bz; ++__i) {
            __cb90[__i] = __i;
          }
          free(__inl28_tb);
          __inl28_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
            __inl28_tb[__w0] = __cb90[__w0];
          }
          free(__inl28_ia);
          __inl28_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl28_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bz[si2]) + __inl28_ta[si0]);
              }
            }
          }
          free(__inl28_ib);
          __inl28_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl28_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bz[si2]) + __inl28_tb[si1]);
              }
            }
          }
          free(__inl28_ia_b);
          __inl28_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl28_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl28_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl28_ib_b);
          __inl28_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl28_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl28_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl28_gathered);
          __inl28_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl28_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = bz_arr[(((__inl28_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl28_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl28_weight);
          __inl28_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl28_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bz[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bz[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb91);
          __cb91 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb91[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl28_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl28_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb92 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb91, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb92[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                __cb92[__ax0] = (__cb92[__ax0] + __cb91[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall13 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall13[__w0] = __cb92[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bzp[__w0] += __hcall13[__w0];
          }
          free(__cb93);
          __cb93 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
            __cb93[__i] = __i;
          }
          free(__inl29_ta);
          __inl29_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            __inl29_ta[__w0] = __cb93[__w0];
          }
          free(__cb94);
          __cb94 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ez; ++__i) {
            __cb94[__i] = __i;
          }
          free(__inl29_tb);
          __inl29_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
            __inl29_tb[__w0] = __cb94[__w0];
          }
          free(__inl29_ia);
          __inl29_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl29_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ez[si2]) + __inl29_ta[si0]);
              }
            }
          }
          free(__inl29_ib);
          __inl29_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl29_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ez[si2]) + __inl29_tb[si1]);
              }
            }
          }
          free(__inl29_ia_b);
          __inl29_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl29_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl29_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl29_ib_b);
          __inl29_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl29_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl29_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl29_gathered);
          __inl29_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl29_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = ez_arr[(((__inl29_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl29_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl29_weight);
          __inl29_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl29_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ez[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ez[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb95);
          __cb95 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb95[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl29_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl29_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb96 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb95, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb96[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                __cb96[__ax0] = (__cb96[__ax0] + __cb95[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          double *__hcall14 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall14[__w0] = __cb96[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Ezp[__w0] += __hcall14[__w0];
          }
          free(__cb97);
          __cb97 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
            __cb97[__i] = __i;
          }
          free(__inl30_ta);
          __inl30_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            __inl30_ta[__w0] = __cb97[__w0];
          }
          free(__cb98);
          __cb98 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bx; ++__i) {
            __cb98[__i] = __i;
          }
          free(__inl30_tb);
          __inl30_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
            __inl30_tb[__w0] = __cb98[__w0];
          }
          free(__inl30_ia);
          __inl30_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl30_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bx[si2]) + __inl30_ta[si0]);
              }
            }
          }
          free(__inl30_ib);
          __inl30_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl30_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bx[si2]) + __inl30_tb[si1]);
              }
            }
          }
          free(__inl30_ia_b);
          __inl30_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl30_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl30_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl30_ib_b);
          __inl30_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl30_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl30_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl30_gathered);
          __inl30_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl30_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = bx_arr[(((__inl30_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl30_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl30_weight);
          __inl30_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl30_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bx[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bx[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb99);
          __cb99 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb99[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl30_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl30_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb100 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb99, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb100[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                __cb100[__ax0] = (__cb100[__ax0] + __cb99[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Brp[__w0] = __cb100[__w0];
          }
          free(__cb101);
          __cb101 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_by) */
          for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
            __cb101[__i] = __i;
          }
          free(__inl31_ta);
          __inl31_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            __inl31_ta[__w0] = __cb101[__w0];
          }
          free(__cb102);
          __cb102 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_by) */
          for (int64_t __i = 0; __i < __inl1_n_sz_by; ++__i) {
            __cb102[__i] = __i;
          }
          free(__inl31_tb);
          __inl31_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
            __inl31_tb[__w0] = __cb102[__w0];
          }
          free(__inl31_ia);
          __inl31_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl31_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_by[si2]) + __inl31_ta[si0]);
              }
            }
          }
          free(__inl31_ib);
          __inl31_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl31_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_by[si2]) + __inl31_tb[si1]);
              }
            }
          }
          free(__inl31_ia_b);
          __inl31_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl31_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl31_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl31_ib_b);
          __inl31_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                __inl31_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl31_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
              }
            }
          }
          free(__inl31_gathered);
          __inl31_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl31_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = by_arr[(((__inl31_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl31_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
              }
            }
          }
          free(__inl31_weight);
          __inl31_weight = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __inl31_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_by[(__w0)*(np_particles) + (__w2)] * __inl1_sz_by[(__w1)*(np_particles) + (__w2)]);
              }
            }
          }
          free(__cb103);
          __cb103 = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                __cb103[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl31_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl31_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
              }
            }
          }
          double *__cb104 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb103, axis=(0, 1)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb104[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                __cb104[__ax0] = (__cb104[__ax0] + __cb103[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
              }
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Bthetap[__w0] = __cb104[__w0];
          }
          double *__cb105 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, __inl1_rp, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb105[__r0] = ((__inl1_rp[__r0] > 0.0) ? __inl1_rp[__r0] : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_rp_safe[__w0] = __cb105[__w0];
          }
          double *__cb106 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, xp / __inl1_rp_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb106[__r0] = ((__inl1_rp[__r0] > 0.0) ? (xp[__r0] / __inl1_rp_safe[__r0]) : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_costheta[__w0] = __cb106[__w0];
          }
          double *__cb107 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, yp / __inl1_rp_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb107[__r0] = ((__inl1_rp[__r0] > 0.0) ? (yp[__r0] / __inl1_rp_safe[__r0]) : 0.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_sintheta[__w0] = __cb107[__w0];
          }
          double *__inl1_xy0_re = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_xy0_re[__w0] = __inl1_costheta[__w0];
          }
          double *__inl1_xy0_im = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_xy0_im[__w0] = (-__inl1_sintheta[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_xy_re[__w0] = __inl1_xy0_re[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_xy_im[__w0] = __inl1_xy0_im[__w0];
          }
          for (int64_t __inl1_imode = 1; __inl1_imode < nmodes; ++__inl1_imode) {
            free(__cb108);
            __cb108 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_ey) */
            for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
              __cb108[__i] = __i;
            }
            free(__inl32_ta);
            __inl32_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
              __inl32_ta[__w0] = __cb108[__w0];
            }
            free(__cb109);
            __cb109 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_ey) */
            for (int64_t __i = 0; __i < __inl1_n_sz_ey; ++__i) {
              __cb109[__i] = __i;
            }
            free(__inl32_tb);
            __inl32_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
              __inl32_tb[__w0] = __cb109[__w0];
            }
            free(__inl32_ia);
            __inl32_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl32_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ey[si2]) + __inl32_ta[si0]);
                }
              }
            }
            free(__inl32_ib);
            __inl32_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl32_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ey[si2]) + __inl32_tb[si1]);
                }
              }
            }
            free(__inl32_ia_b);
            __inl32_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl32_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl32_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl32_ib_b);
            __inl32_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl32_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl32_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl32_gathered);
            __inl32_gathered = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl32_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ey_arr[(((__inl32_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl32_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * __inl1_imode) - 1))];
                }
              }
            }
            free(__inl32_weight);
            __inl32_weight = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl32_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ey[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ey[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb110);
            __cb110 = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb110[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl32_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl32_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb110, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb111[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                  __cb111[__ax0] = (__cb111[__ax0] + __cb110[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall15[__w0] = __cb111[__w0];
            }
            free(__cb112);
            __cb112 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_ey) */
            for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
              __cb112[__i] = __i;
            }
            free(__inl33_ta);
            __inl33_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
              __inl33_ta[__w0] = __cb112[__w0];
            }
            free(__cb113);
            __cb113 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_ey) */
            for (int64_t __i = 0; __i < __inl1_n_sz_ey; ++__i) {
              __cb113[__i] = __i;
            }
            free(__inl33_tb);
            __inl33_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
              __inl33_tb[__w0] = __cb113[__w0];
            }
            free(__inl33_ia);
            __inl33_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl33_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ey[si2]) + __inl33_ta[si0]);
                }
              }
            }
            free(__inl33_ib);
            __inl33_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl33_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ey[si2]) + __inl33_tb[si1]);
                }
              }
            }
            free(__inl33_ia_b);
            __inl33_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl33_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl33_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl33_ib_b);
            __inl33_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl33_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl33_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl33_gathered);
            __inl33_gathered = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl33_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ey_arr[(((__inl33_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl33_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * __inl1_imode))];
                }
              }
            }
            free(__inl33_weight);
            __inl33_weight = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl33_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ey[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ey[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb114);
            __cb114 = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb114[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl33_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl33_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb114, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb115[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                  __cb115[__ax0] = (__cb115[__ax0] + __cb114[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall16[__w0] = __cb115[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_dEy[__w0] = ((__inl1_xy_re[__w0] * __hcall15[__w0]) - (__inl1_xy_im[__w0] * __hcall16[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_Ethetap[__w0] += __inl1_dEy[__w0];
            }
            free(__cb116);
            __cb116 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_ex) */
            for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
              __cb116[__i] = __i;
            }
            free(__inl34_ta);
            __inl34_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
              __inl34_ta[__w0] = __cb116[__w0];
            }
            free(__cb117);
            __cb117 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_ex) */
            for (int64_t __i = 0; __i < __inl1_n_sz_ex; ++__i) {
              __cb117[__i] = __i;
            }
            free(__inl34_tb);
            __inl34_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
              __inl34_tb[__w0] = __cb117[__w0];
            }
            free(__inl34_ia);
            __inl34_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl34_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ex[si2]) + __inl34_ta[si0]);
                }
              }
            }
            free(__inl34_ib);
            __inl34_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl34_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ex[si2]) + __inl34_tb[si1]);
                }
              }
            }
            free(__inl34_ia_b);
            __inl34_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl34_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl34_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl34_ib_b);
            __inl34_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl34_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl34_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl34_gathered);
            __inl34_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl34_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ex_arr[(((__inl34_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl34_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * __inl1_imode) - 1))];
                }
              }
            }
            free(__inl34_weight);
            __inl34_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl34_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ex[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ex[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb118);
            __cb118 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb118[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl34_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl34_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb118, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb119[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                  __cb119[__ax0] = (__cb119[__ax0] + __cb118[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall17[__w0] = __cb119[__w0];
            }
            free(__cb120);
            __cb120 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_ex) */
            for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
              __cb120[__i] = __i;
            }
            free(__inl35_ta);
            __inl35_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
              __inl35_ta[__w0] = __cb120[__w0];
            }
            free(__cb121);
            __cb121 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_ex) */
            for (int64_t __i = 0; __i < __inl1_n_sz_ex; ++__i) {
              __cb121[__i] = __i;
            }
            free(__inl35_tb);
            __inl35_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
              __inl35_tb[__w0] = __cb121[__w0];
            }
            free(__inl35_ia);
            __inl35_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl35_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ex[si2]) + __inl35_ta[si0]);
                }
              }
            }
            free(__inl35_ib);
            __inl35_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl35_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ex[si2]) + __inl35_tb[si1]);
                }
              }
            }
            free(__inl35_ia_b);
            __inl35_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl35_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl35_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl35_ib_b);
            __inl35_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl35_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl35_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl35_gathered);
            __inl35_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl35_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = ex_arr[(((__inl35_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl35_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * __inl1_imode))];
                }
              }
            }
            free(__inl35_weight);
            __inl35_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl35_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ex[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ex[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb122);
            __cb122 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb122[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl35_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl35_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb122, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb123[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                  __cb123[__ax0] = (__cb123[__ax0] + __cb122[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall18[__w0] = __cb123[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_dEx[__w0] = ((__inl1_xy_re[__w0] * __hcall17[__w0]) - (__inl1_xy_im[__w0] * __hcall18[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_Erp[__w0] += __inl1_dEx[__w0];
            }
            free(__cb124);
            __cb124 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_bz) */
            for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
              __cb124[__i] = __i;
            }
            free(__inl36_ta);
            __inl36_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
              __inl36_ta[__w0] = __cb124[__w0];
            }
            free(__cb125);
            __cb125 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_bz) */
            for (int64_t __i = 0; __i < __inl1_n_sz_bz; ++__i) {
              __cb125[__i] = __i;
            }
            free(__inl36_tb);
            __inl36_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
              __inl36_tb[__w0] = __cb125[__w0];
            }
            free(__inl36_ia);
            __inl36_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl36_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bz[si2]) + __inl36_ta[si0]);
                }
              }
            }
            free(__inl36_ib);
            __inl36_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl36_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bz[si2]) + __inl36_tb[si1]);
                }
              }
            }
            free(__inl36_ia_b);
            __inl36_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl36_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl36_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl36_ib_b);
            __inl36_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl36_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl36_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl36_gathered);
            __inl36_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl36_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = bz_arr[(((__inl36_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl36_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * __inl1_imode) - 1))];
                }
              }
            }
            free(__inl36_weight);
            __inl36_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl36_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bz[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bz[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb126);
            __cb126 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb126[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl36_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl36_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb126, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb127[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                  __cb127[__ax0] = (__cb127[__ax0] + __cb126[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall19[__w0] = __cb127[__w0];
            }
            free(__cb128);
            __cb128 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_bz) */
            for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
              __cb128[__i] = __i;
            }
            free(__inl37_ta);
            __inl37_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
              __inl37_ta[__w0] = __cb128[__w0];
            }
            free(__cb129);
            __cb129 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_bz) */
            for (int64_t __i = 0; __i < __inl1_n_sz_bz; ++__i) {
              __cb129[__i] = __i;
            }
            free(__inl37_tb);
            __inl37_tb = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
              __inl37_tb[__w0] = __cb129[__w0];
            }
            free(__inl37_ia);
            __inl37_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl37_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bz[si2]) + __inl37_ta[si0]);
                }
              }
            }
            free(__inl37_ib);
            __inl37_ib = (double *)malloc((size_t)((1) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl37_ib[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bz[si2]) + __inl37_tb[si1]);
                }
              }
            }
            free(__inl37_ia_b);
            __inl37_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl37_ia_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl37_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl37_ib_b);
            __inl37_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl37_ib_b[((si0)*((o + 1)) + (si1))*(np_particles) + (si2)] = __inl37_ib[((0)*((o + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl37_gathered);
            __inl37_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl37_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = bz_arr[(((__inl37_ia_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl37_ib_b[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * __inl1_imode))];
                }
              }
            }
            free(__inl37_weight);
            __inl37_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl37_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bz[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bz[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb130);
            __cb130 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb130[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl37_weight[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)] * __inl37_gathered[((__w0)*((o + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb130, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb131[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                  __cb131[__ax0] = (__cb131[__ax0] + __cb130[((__rd0)*((o + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall20[__w0] = __cb131[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_dBz[__w0] = ((__inl1_xy_re[__w0] * __hcall19[__w0]) - (__inl1_xy_im[__w0] * __hcall20[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              Bzp[__w0] += __inl1_dBz[__w0];
            }
            free(__cb132);
            __cb132 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_ez) */
            for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
              __cb132[__i] = __i;
            }
            free(__inl38_ta);
            __inl38_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
              __inl38_ta[__w0] = __cb132[__w0];
            }
            free(__cb133);
            __cb133 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_ez) */
            for (int64_t __i = 0; __i < __inl1_n_sz_ez; ++__i) {
              __cb133[__i] = __i;
            }
            free(__inl38_tb);
            __inl38_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
              __inl38_tb[__w0] = __cb133[__w0];
            }
            free(__inl38_ia);
            __inl38_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl38_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ez[si2]) + __inl38_ta[si0]);
                }
              }
            }
            free(__inl38_ib);
            __inl38_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl38_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ez[si2]) + __inl38_tb[si1]);
                }
              }
            }
            free(__inl38_ia_b);
            __inl38_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl38_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl38_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl38_ib_b);
            __inl38_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl38_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl38_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl38_gathered);
            __inl38_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl38_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = ez_arr[(((__inl38_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl38_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * __inl1_imode) - 1))];
                }
              }
            }
            free(__inl38_weight);
            __inl38_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl38_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ez[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ez[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb134);
            __cb134 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb134[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl38_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl38_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb134, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb135[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                  __cb135[__ax0] = (__cb135[__ax0] + __cb134[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall21[__w0] = __cb135[__w0];
            }
            free(__cb136);
            __cb136 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_ez) */
            for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
              __cb136[__i] = __i;
            }
            free(__inl39_ta);
            __inl39_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
              __inl39_ta[__w0] = __cb136[__w0];
            }
            free(__cb137);
            __cb137 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_ez) */
            for (int64_t __i = 0; __i < __inl1_n_sz_ez; ++__i) {
              __cb137[__i] = __i;
            }
            free(__inl39_tb);
            __inl39_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
              __inl39_tb[__w0] = __cb137[__w0];
            }
            free(__inl39_ia);
            __inl39_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl39_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_ez[si2]) + __inl39_ta[si0]);
                }
              }
            }
            free(__inl39_ib);
            __inl39_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl39_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_ez[si2]) + __inl39_tb[si1]);
                }
              }
            }
            free(__inl39_ia_b);
            __inl39_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl39_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl39_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl39_ib_b);
            __inl39_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl39_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl39_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl39_gathered);
            __inl39_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl39_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = ez_arr[(((__inl39_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl39_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * __inl1_imode))];
                }
              }
            }
            free(__inl39_weight);
            __inl39_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl39_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_ez[(__w0)*(np_particles) + (__w2)] * __inl1_sz_ez[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb138);
            __cb138 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb138[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl39_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl39_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb138, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb139[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                  __cb139[__ax0] = (__cb139[__ax0] + __cb138[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall22[__w0] = __cb139[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_dEz[__w0] = ((__inl1_xy_re[__w0] * __hcall21[__w0]) - (__inl1_xy_im[__w0] * __hcall22[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              Ezp[__w0] += __inl1_dEz[__w0];
            }
            free(__cb140);
            __cb140 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_bx) */
            for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
              __cb140[__i] = __i;
            }
            free(__inl40_ta);
            __inl40_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
              __inl40_ta[__w0] = __cb140[__w0];
            }
            free(__cb141);
            __cb141 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_bx) */
            for (int64_t __i = 0; __i < __inl1_n_sz_bx; ++__i) {
              __cb141[__i] = __i;
            }
            free(__inl40_tb);
            __inl40_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
              __inl40_tb[__w0] = __cb141[__w0];
            }
            free(__inl40_ia);
            __inl40_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl40_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bx[si2]) + __inl40_ta[si0]);
                }
              }
            }
            free(__inl40_ib);
            __inl40_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl40_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bx[si2]) + __inl40_tb[si1]);
                }
              }
            }
            free(__inl40_ia_b);
            __inl40_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl40_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl40_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl40_ib_b);
            __inl40_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl40_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl40_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl40_gathered);
            __inl40_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl40_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = bx_arr[(((__inl40_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl40_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * __inl1_imode) - 1))];
                }
              }
            }
            free(__inl40_weight);
            __inl40_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl40_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bx[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bx[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb142);
            __cb142 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb142[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl40_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl40_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb142, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb143[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                  __cb143[__ax0] = (__cb143[__ax0] + __cb142[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall23[__w0] = __cb143[__w0];
            }
            free(__cb144);
            __cb144 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_bx) */
            for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
              __cb144[__i] = __i;
            }
            free(__inl41_ta);
            __inl41_ta = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
              __inl41_ta[__w0] = __cb144[__w0];
            }
            free(__cb145);
            __cb145 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_bx) */
            for (int64_t __i = 0; __i < __inl1_n_sz_bx; ++__i) {
              __cb145[__i] = __i;
            }
            free(__inl41_tb);
            __inl41_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
              __inl41_tb[__w0] = __cb145[__w0];
            }
            free(__inl41_ia);
            __inl41_ia = (double *)malloc((size_t)(((o + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl41_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_bx[si2]) + __inl41_ta[si0]);
                }
              }
            }
            free(__inl41_ib);
            __inl41_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl41_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_bx[si2]) + __inl41_tb[si1]);
                }
              }
            }
            free(__inl41_ia_b);
            __inl41_ia_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl41_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl41_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl41_ib_b);
            __inl41_ib_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl41_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl41_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl41_gathered);
            __inl41_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl41_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = bx_arr[(((__inl41_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl41_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * __inl1_imode))];
                }
              }
            }
            free(__inl41_weight);
            __inl41_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl41_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_bx[(__w0)*(np_particles) + (__w2)] * __inl1_sz_bx[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb146);
            __cb146 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb146[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl41_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl41_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb146, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb147[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                  __cb147[__ax0] = (__cb147[__ax0] + __cb146[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall24[__w0] = __cb147[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_dBx[__w0] = ((__inl1_xy_re[__w0] * __hcall23[__w0]) - (__inl1_xy_im[__w0] * __hcall24[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_Brp[__w0] += __inl1_dBx[__w0];
            }
            free(__cb148);
            __cb148 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_by) */
            for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
              __cb148[__i] = __i;
            }
            free(__inl42_ta);
            __inl42_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
              __inl42_ta[__w0] = __cb148[__w0];
            }
            free(__cb149);
            __cb149 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_by) */
            for (int64_t __i = 0; __i < __inl1_n_sz_by; ++__i) {
              __cb149[__i] = __i;
            }
            free(__inl42_tb);
            __inl42_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
              __inl42_tb[__w0] = __cb149[__w0];
            }
            free(__inl42_ia);
            __inl42_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl42_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_by[si2]) + __inl42_ta[si0]);
                }
              }
            }
            free(__inl42_ib);
            __inl42_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl42_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_by[si2]) + __inl42_tb[si1]);
                }
              }
            }
            free(__inl42_ia_b);
            __inl42_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl42_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl42_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl42_ib_b);
            __inl42_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl42_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl42_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl42_gathered);
            __inl42_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl42_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = by_arr[(((__inl42_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl42_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * __inl1_imode) - 1))];
                }
              }
            }
            free(__inl42_weight);
            __inl42_weight = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl42_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_by[(__w0)*(np_particles) + (__w2)] * __inl1_sz_by[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb150);
            __cb150 = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb150[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl42_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl42_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb150, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb151[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                  __cb151[__ax0] = (__cb151[__ax0] + __cb150[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall25[__w0] = __cb151[__w0];
            }
            free(__cb152);
            __cb152 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sx_by) */
            for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
              __cb152[__i] = __i;
            }
            free(__inl43_ta);
            __inl43_ta = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
              __inl43_ta[__w0] = __cb152[__w0];
            }
            free(__cb153);
            __cb153 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            /* numpy: np.arange(__inl1_n_sz_by) */
            for (int64_t __i = 0; __i < __inl1_n_sz_by; ++__i) {
              __cb153[__i] = __i;
            }
            free(__inl43_tb);
            __inl43_tb = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
              __inl43_tb[__w0] = __cb153[__w0];
            }
            free(__inl43_ia);
            __inl43_ia = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < 1; ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl43_ia[((si0)*(1) + (si1))*(np_particles) + (si2)] = ((__inl1_lox + __inl1_j_by[si2]) + __inl43_ta[si0]);
                }
              }
            }
            free(__inl43_ib);
            __inl43_ib = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t si0 = 0; si0 < 1; ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl43_ib[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = ((__inl1_loy + __inl1_l_by[si2]) + __inl43_tb[si1]);
                }
              }
            }
            free(__inl43_ia_b);
            __inl43_ia_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl43_ia_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl43_ia[((si0)*(1) + (0))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl43_ib_b);
            __inl43_ib_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
            for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
              for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
                for (int64_t si2 = 0; si2 < np_particles; ++si2) {
                  __inl43_ib_b[((si0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)] = __inl43_ib[((0)*(((o - gal) + 1)) + (si1))*(np_particles) + (si2)];
                }
              }
            }
            free(__inl43_gathered);
            __inl43_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl43_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = by_arr[(((__inl43_ia_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)])*(((ncells + (2 * depos_order)) + 6)) + (__inl43_ib_b[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * __inl1_imode))];
                }
              }
            }
            free(__inl43_weight);
            __inl43_weight = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __inl43_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl1_sx_by[(__w0)*(np_particles) + (__w2)] * __inl1_sz_by[(__w1)*(np_particles) + (__w2)]);
                }
              }
            }
            free(__cb154);
            __cb154 = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
              for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
                for (int64_t __w2 = 0; __w2 < np_particles; ++__w2) {
                  __cb154[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] = (__inl43_weight[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)] * __inl43_gathered[((__w0)*(((o - gal) + 1)) + (__w1))*(np_particles) + (__w2)]);
                }
              }
            }
            /* numpy: np.sum(__cb154, axis=(0, 1)) */
            for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
              __cb155[__ax0] = 0.0;
              for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
                for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                  __cb155[__ax0] = (__cb155[__ax0] + __cb154[((__rd0)*(((o - gal) + 1)) + (__rd1))*(np_particles) + (__ax0)]);
                }
              }
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __hcall26[__w0] = __cb155[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_dBy[__w0] = ((__inl1_xy_re[__w0] * __hcall25[__w0]) - (__inl1_xy_im[__w0] * __hcall26[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_Bthetap[__w0] += __inl1_dBy[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_tmp_re[__w0] = ((__inl1_xy_re[__w0] * __inl1_xy0_re[__w0]) - (__inl1_xy_im[__w0] * __inl1_xy0_im[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_tmp_im[__w0] = ((__inl1_xy_re[__w0] * __inl1_xy0_im[__w0]) + (__inl1_xy_im[__w0] * __inl1_xy0_re[__w0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_xy_re[__w0] = __inl1_tmp_re[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl1_xy_im[__w0] = __inl1_tmp_im[__w0];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Exp[__w0] += ((__inl1_costheta[__w0] * __inl1_Erp[__w0]) - (__inl1_sintheta[__w0] * __inl1_Ethetap[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Eyp[__w0] += ((__inl1_costheta[__w0] * __inl1_Ethetap[__w0]) + (__inl1_sintheta[__w0] * __inl1_Erp[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bxp[__w0] += ((__inl1_costheta[__w0] * __inl1_Brp[__w0]) - (__inl1_sintheta[__w0] * __inl1_Bthetap[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Byp[__w0] += ((__inl1_costheta[__w0] * __inl1_Bthetap[__w0]) + (__inl1_sintheta[__w0] * __inl1_Brp[__w0]));
          }
          free(__cb84);
          free(__cb88);
          free(__cb92);
          free(__hcall13);
          free(__cb96);
          free(__hcall14);
          free(__cb100);
          free(__cb104);
          free(__cb105);
          free(__cb106);
          free(__cb107);
          free(__inl1_xy0_re);
          free(__inl1_xy0_im);
        }
        else if ((g == 4)) {
          free(__cb156);
          __cb156 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
            __cb156[__i] = __i;
          }
          free(__inl44_taps);
          __inl44_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            __inl44_taps[__w0] = __cb156[__w0];
          }
          free(__inl44_rows);
          __inl44_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl44_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_ey[__w1]) + __inl44_taps[__w0]);
            }
          }
          free(__inl44_gathered);
          __inl44_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl44_gathered[(__w0)*(np_particles) + (__w1)] = ey_arr[(((__inl44_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb157);
          __cb157 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb157[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_ey[(__w0)*(np_particles) + (__w1)] * __inl44_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb158 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb157, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb158[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb158[__ax0] = (__cb158[__ax0] + __cb157[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Ethetap[__w0] = __cb158[__w0];
          }
          free(__cb159);
          __cb159 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
            __cb159[__i] = __i;
          }
          free(__inl45_taps);
          __inl45_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            __inl45_taps[__w0] = __cb159[__w0];
          }
          free(__inl45_rows);
          __inl45_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl45_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_ex[__w1]) + __inl45_taps[__w0]);
            }
          }
          free(__inl45_gathered);
          __inl45_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl45_gathered[(__w0)*(np_particles) + (__w1)] = ex_arr[(((__inl45_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb160);
          __cb160 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb160[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_ex[(__w0)*(np_particles) + (__w1)] * __inl45_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb161 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb160, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb161[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb161[__ax0] = (__cb161[__ax0] + __cb160[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Erp[__w0] = __cb161[__w0];
          }
          free(__cb162);
          __cb162 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
            __cb162[__i] = __i;
          }
          free(__inl46_taps);
          __inl46_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            __inl46_taps[__w0] = __cb162[__w0];
          }
          free(__inl46_rows);
          __inl46_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl46_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_bz[__w1]) + __inl46_taps[__w0]);
            }
          }
          free(__inl46_gathered);
          __inl46_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl46_gathered[(__w0)*(np_particles) + (__w1)] = bz_arr[(((__inl46_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb163);
          __cb163 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb163[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_bz[(__w0)*(np_particles) + (__w1)] * __inl46_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb164 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb163, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb164[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb164[__ax0] = (__cb164[__ax0] + __cb163[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall27 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall27[__w0] = __cb164[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bzp[__w0] += __hcall27[__w0];
          }
          free(__cb165);
          __cb165 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
            __cb165[__i] = __i;
          }
          free(__inl47_taps);
          __inl47_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            __inl47_taps[__w0] = __cb165[__w0];
          }
          free(__inl47_rows);
          __inl47_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl47_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_ez[__w1]) + __inl47_taps[__w0]);
            }
          }
          free(__inl47_gathered);
          __inl47_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl47_gathered[(__w0)*(np_particles) + (__w1)] = ez_arr[(((__inl47_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb166);
          __cb166 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb166[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_ez[(__w0)*(np_particles) + (__w1)] * __inl47_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb167 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb166, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb167[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb167[__ax0] = (__cb167[__ax0] + __cb166[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__hcall28 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall28[__w0] = __cb167[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Ezp[__w0] += __hcall28[__w0];
          }
          free(__cb168);
          __cb168 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
            __cb168[__i] = __i;
          }
          free(__inl48_taps);
          __inl48_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            __inl48_taps[__w0] = __cb168[__w0];
          }
          free(__inl48_rows);
          __inl48_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl48_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_bx[__w1]) + __inl48_taps[__w0]);
            }
          }
          free(__inl48_gathered);
          __inl48_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl48_gathered[(__w0)*(np_particles) + (__w1)] = bx_arr[(((__inl48_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb169);
          __cb169 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb169[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_bx[(__w0)*(np_particles) + (__w1)] * __inl48_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb170 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb169, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb170[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb170[__ax0] = (__cb170[__ax0] + __cb169[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Brp[__w0] = __cb170[__w0];
          }
          free(__cb171);
          __cb171 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_by) */
          for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
            __cb171[__i] = __i;
          }
          free(__inl49_taps);
          __inl49_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            __inl49_taps[__w0] = __cb171[__w0];
          }
          free(__inl49_rows);
          __inl49_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl49_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_by[__w1]) + __inl49_taps[__w0]);
            }
          }
          free(__inl49_gathered);
          __inl49_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl49_gathered[(__w0)*(np_particles) + (__w1)] = by_arr[(((__inl49_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb172);
          __cb172 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb172[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_by[(__w0)*(np_particles) + (__w1)] * __inl49_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb173 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb172, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb173[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb173[__ax0] = (__cb173[__ax0] + __cb172[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Bthetap[__w0] = __cb173[__w0];
          }
          double *__cb174 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, __inl1_rp, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb174[__r0] = ((__inl1_rp[__r0] > 0.0) ? __inl1_rp[__r0] : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_rp_safe[__w0] = __cb174[__w0];
          }
          double *__cb175 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, xp / __inl1_rp_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb175[__r0] = ((__inl1_rp[__r0] > 0.0) ? (xp[__r0] / __inl1_rp_safe[__r0]) : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_costheta[__w0] = __cb175[__w0];
          }
          double *__cb176 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, yp / __inl1_rp_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb176[__r0] = ((__inl1_rp[__r0] > 0.0) ? (yp[__r0] / __inl1_rp_safe[__r0]) : 0.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_sintheta[__w0] = __cb176[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Exp[__w0] += ((__inl1_costheta[__w0] * __inl1_Erp[__w0]) - (__inl1_sintheta[__w0] * __inl1_Ethetap[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Eyp[__w0] += ((__inl1_costheta[__w0] * __inl1_Ethetap[__w0]) + (__inl1_sintheta[__w0] * __inl1_Erp[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bxp[__w0] += ((__inl1_costheta[__w0] * __inl1_Brp[__w0]) - (__inl1_sintheta[__w0] * __inl1_Bthetap[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Byp[__w0] += ((__inl1_costheta[__w0] * __inl1_Bthetap[__w0]) + (__inl1_sintheta[__w0] * __inl1_Brp[__w0]));
          }
          free(__cb158);
          free(__cb161);
          free(__cb164);
          free(__hcall27);
          free(__cb167);
          free(__hcall28);
          free(__cb170);
          free(__cb173);
          free(__cb174);
          free(__cb175);
          free(__cb176);
        }
        else if ((g == 5)) {
          free(__cb177);
          __cb177 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
            __cb177[__i] = __i;
          }
          free(__inl50_taps);
          __inl50_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            __inl50_taps[__w0] = __cb177[__w0];
          }
          free(__inl50_rows);
          __inl50_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl50_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_ey[__w1]) + __inl50_taps[__w0]);
            }
          }
          free(__inl50_gathered);
          __inl50_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl50_gathered[(__w0)*(np_particles) + (__w1)] = ey_arr[(((__inl50_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb178);
          __cb178 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb178[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_ey[(__w0)*(np_particles) + (__w1)] * __inl50_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb179 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb178, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb179[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb179[__ax0] = (__cb179[__ax0] + __cb178[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Ethetap[__w0] = __cb179[__w0];
          }
          free(__cb180);
          __cb180 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
            __cb180[__i] = __i;
          }
          free(__inl51_taps);
          __inl51_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            __inl51_taps[__w0] = __cb180[__w0];
          }
          free(__inl51_rows);
          __inl51_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl51_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_ex[__w1]) + __inl51_taps[__w0]);
            }
          }
          free(__inl51_gathered);
          __inl51_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl51_gathered[(__w0)*(np_particles) + (__w1)] = ex_arr[(((__inl51_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb181);
          __cb181 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb181[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_ex[(__w0)*(np_particles) + (__w1)] * __inl51_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb182 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb181, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb182[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb182[__ax0] = (__cb182[__ax0] + __cb181[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Erp[__w0] = __cb182[__w0];
          }
          free(__cb183);
          __cb183 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
            __cb183[__i] = __i;
          }
          free(__inl52_taps);
          __inl52_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            __inl52_taps[__w0] = __cb183[__w0];
          }
          free(__inl52_rows);
          __inl52_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl52_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_bz[__w1]) + __inl52_taps[__w0]);
            }
          }
          free(__inl52_gathered);
          __inl52_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl52_gathered[(__w0)*(np_particles) + (__w1)] = bz_arr[(((__inl52_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb184);
          __cb184 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb184[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_bz[(__w0)*(np_particles) + (__w1)] * __inl52_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb185 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb184, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb185[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb185[__ax0] = (__cb185[__ax0] + __cb184[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__inl1_Bphip = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Bphip[__w0] = __cb185[__w0];
          }
          free(__cb186);
          __cb186 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
            __cb186[__i] = __i;
          }
          free(__inl53_taps);
          __inl53_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            __inl53_taps[__w0] = __cb186[__w0];
          }
          free(__inl53_rows);
          __inl53_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl53_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_ez[__w1]) + __inl53_taps[__w0]);
            }
          }
          free(__inl53_gathered);
          __inl53_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl53_gathered[(__w0)*(np_particles) + (__w1)] = ez_arr[(((__inl53_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb187);
          __cb187 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb187[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_ez[(__w0)*(np_particles) + (__w1)] * __inl53_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb188 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb187, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb188[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb188[__ax0] = (__cb188[__ax0] + __cb187[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          double *__inl1_Ephip = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Ephip[__w0] = __cb188[__w0];
          }
          free(__cb189);
          __cb189 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
            __cb189[__i] = __i;
          }
          free(__inl54_taps);
          __inl54_taps = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            __inl54_taps[__w0] = __cb189[__w0];
          }
          free(__inl54_rows);
          __inl54_rows = (int64_t *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl54_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_bx[__w1]) + __inl54_taps[__w0]);
            }
          }
          free(__inl54_gathered);
          __inl54_gathered = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl54_gathered[(__w0)*(np_particles) + (__w1)] = bx_arr[(((__inl54_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb190);
          __cb190 = (double *)malloc((size_t)(((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb190[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_bx[(__w0)*(np_particles) + (__w1)] * __inl54_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb191 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb190, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb191[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              __cb191[__ax0] = (__cb191[__ax0] + __cb190[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Brp[__w0] = __cb191[__w0];
          }
          free(__cb192);
          __cb192 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_by) */
          for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
            __cb192[__i] = __i;
          }
          free(__inl55_taps);
          __inl55_taps = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            __inl55_taps[__w0] = __cb192[__w0];
          }
          free(__inl55_rows);
          __inl55_rows = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl55_rows[(__w0)*(np_particles) + (__w1)] = ((__inl1_lox + __inl1_j_by[__w1]) + __inl55_taps[__w0]);
            }
          }
          free(__inl55_gathered);
          __inl55_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __inl55_gathered[(__w0)*(np_particles) + (__w1)] = by_arr[(((__inl55_rows[(__w0)*(np_particles) + (__w1)])*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
            }
          }
          free(__cb193);
          __cb193 = (double *)malloc((size_t)((((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < np_particles; ++__w1) {
              __cb193[(__w0)*(np_particles) + (__w1)] = (__inl1_sx_by[(__w0)*(np_particles) + (__w1)] * __inl55_gathered[(__w0)*(np_particles) + (__w1)]);
            }
          }
          double *__cb194 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb193, axis=0) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb194[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              __cb194[__ax0] = (__cb194[__ax0] + __cb193[(__rd0)*(np_particles) + (__ax0)]);
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_Bthetap[__w0] = __cb194[__w0];
          }
          double *__cb195 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sqrt(xp * xp + yp * yp) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb195[__r0] = sqrt(((xp[__r0] * xp[__r0]) + (yp[__r0] * yp[__r0])));
          }
          double *__inl1_rpxy = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_rpxy[__w0] = __cb195[__w0];
          }
          double *__cb196 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rpxy > 0.0, __inl1_rpxy, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb196[__r0] = ((__inl1_rpxy[__r0] > 0.0) ? __inl1_rpxy[__r0] : 1.0);
          }
          double *__inl1_rpxy_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_rpxy_safe[__w0] = __cb196[__w0];
          }
          double *__cb197 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rpxy > 0.0, xp / __inl1_rpxy_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb197[__r0] = ((__inl1_rpxy[__r0] > 0.0) ? (xp[__r0] / __inl1_rpxy_safe[__r0]) : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_costheta[__w0] = __cb197[__w0];
          }
          double *__cb198 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rpxy > 0.0, yp / __inl1_rpxy_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb198[__r0] = ((__inl1_rpxy[__r0] > 0.0) ? (yp[__r0] / __inl1_rpxy_safe[__r0]) : 0.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_sintheta[__w0] = __cb198[__w0];
          }
          double *__cb199 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, __inl1_rp, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb199[__r0] = ((__inl1_rp[__r0] > 0.0) ? __inl1_rp[__r0] : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_rp_safe[__w0] = __cb199[__w0];
          }
          double *__cb200 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, __inl1_rpxy / __inl1_rp_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb200[__r0] = ((__inl1_rp[__r0] > 0.0) ? (__inl1_rpxy[__r0] / __inl1_rp_safe[__r0]) : 1.0);
          }
          double *__inl1_cosphi = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_cosphi[__w0] = __cb200[__w0];
          }
          double *__cb201 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(__inl1_rp > 0.0, zp / __inl1_rp_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb201[__r0] = ((__inl1_rp[__r0] > 0.0) ? (zp[__r0] / __inl1_rp_safe[__r0]) : 0.0);
          }
          double *__inl1_sinphi = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_sinphi[__w0] = __cb201[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Exp[__w0] += ((((__inl1_costheta[__w0] * __inl1_cosphi[__w0]) * __inl1_Erp[__w0]) - (__inl1_sintheta[__w0] * __inl1_Ethetap[__w0])) - ((__inl1_costheta[__w0] * __inl1_sinphi[__w0]) * __inl1_Ephip[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Eyp[__w0] += ((((__inl1_sintheta[__w0] * __inl1_cosphi[__w0]) * __inl1_Erp[__w0]) + (__inl1_costheta[__w0] * __inl1_Ethetap[__w0])) - ((__inl1_sintheta[__w0] * __inl1_sinphi[__w0]) * __inl1_Ephip[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Ezp[__w0] += ((__inl1_sinphi[__w0] * __inl1_Erp[__w0]) + (__inl1_cosphi[__w0] * __inl1_Ephip[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bxp[__w0] += ((((__inl1_costheta[__w0] * __inl1_cosphi[__w0]) * __inl1_Brp[__w0]) - (__inl1_sintheta[__w0] * __inl1_Bthetap[__w0])) - ((__inl1_costheta[__w0] * __inl1_sinphi[__w0]) * __inl1_Bphip[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Byp[__w0] += ((((__inl1_sintheta[__w0] * __inl1_cosphi[__w0]) * __inl1_Brp[__w0]) + (__inl1_costheta[__w0] * __inl1_Bthetap[__w0])) - ((__inl1_sintheta[__w0] * __inl1_sinphi[__w0]) * __inl1_Bphip[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bzp[__w0] += ((__inl1_sinphi[__w0] * __inl1_Brp[__w0]) + (__inl1_cosphi[__w0] * __inl1_Bphip[__w0]));
          }
          free(__cb179);
          free(__cb182);
          free(__cb185);
          free(__inl1_Bphip);
          free(__cb188);
          free(__inl1_Ephip);
          free(__cb191);
          free(__cb194);
          free(__cb195);
          free(__inl1_rpxy);
          free(__cb196);
          free(__inl1_rpxy_safe);
          free(__cb197);
          free(__cb198);
          free(__cb199);
          free(__cb200);
          free(__inl1_cosphi);
          free(__cb201);
          free(__inl1_sinphi);
        }
        else {
          free(__cb202);
          __cb202 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ex; ++__i) {
            __cb202[__i] = __i;
          }
          free(__inl56_tx);
          __inl56_tx = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ex; ++__w0) {
            __inl56_tx[__w0] = __cb202[__w0];
          }
          free(__cb203);
          __cb203 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sy_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sy_ex; ++__i) {
            __cb203[__i] = __i;
          }
          free(__inl56_ty);
          __inl56_ty = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sy_ex; ++__w0) {
            __inl56_ty[__w0] = __cb203[__w0];
          }
          free(__cb204);
          __cb204 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ex) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ex; ++__i) {
            __cb204[__i] = __i;
          }
          free(__inl56_tz);
          __inl56_tz = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ex; ++__w0) {
            __inl56_tz[__w0] = __cb204[__w0];
          }
          free(__inl56_ix);
          __inl56_ix = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl56_ix[(((si0)*(1) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_lox + __inl1_j_ex[si3]) + __inl56_tx[si0]);
                }
              }
            }
          }
          free(__inl56_iy);
          __inl56_iy = (double *)malloc((size_t)((1) * ((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl56_iy[(((si0)*((o + 1)) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_loy + __inl1_k_ex[si3]) + __inl56_ty[si1]);
                }
              }
            }
          }
          free(__inl56_iz);
          __inl56_iz = (double *)malloc((size_t)((1) * (1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl56_iz[(((si0)*(1) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = ((__inl1_loz + __inl1_l_ex[si3]) + __inl56_tz[si2]);
                }
              }
            }
          }
          free(__inl56_ix_b);
          __inl56_ix_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl56_ix_b[(((si0)*((o + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl56_ix[(((si0)*(1) + (0))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl56_iy_b);
          __inl56_iy_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl56_iy_b[(((si0)*((o + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl56_iy[(((0)*((o + 1)) + (si1))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl56_iz_b);
          __inl56_iz_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl56_iz_b[(((si0)*((o + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl56_iz[(((0)*(1) + (0))*((o + 1)) + (si2))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl56_gathered);
          __inl56_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl56_gathered[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = ex_arr[(((__inl56_ix_b[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)])*(((ncells + (2 * depos_order)) + 6)) + (__inl56_iy_b[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]))*(((ncells + (2 * depos_order)) + 6)) + (__inl56_iz_b[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
                }
              }
            }
          }
          free(__inl56_weight);
          __inl56_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl56_weight[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = ((__inl1_sx_ex[(__w0)*(np_particles) + (__w3)] * __inl1_sy_ex[(__w1)*(np_particles) + (__w3)]) * __inl1_sz_ex[(__w2)*(np_particles) + (__w3)]);
                }
              }
            }
          }
          free(__cb205);
          __cb205 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __cb205[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = (__inl56_weight[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] * __inl56_gathered[(((__w0)*((o + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]);
                }
              }
            }
          }
          double *__cb206 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb205, axis=(0, 1, 2)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb206[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                for (int64_t __rd2 = 0; __rd2 < (o + 1); ++__rd2) {
                  __cb206[__ax0] = (__cb206[__ax0] + __cb205[(((__rd0)*((o + 1)) + (__rd1))*((o + 1)) + (__rd2))*(np_particles) + (__ax0)]);
                }
              }
            }
          }
          double *__hcall29 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall29[__w0] = __cb206[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Exp[__w0] += __hcall29[__w0];
          }
          free(__cb207);
          __cb207 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ey; ++__i) {
            __cb207[__i] = __i;
          }
          free(__inl57_tx);
          __inl57_tx = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ey; ++__w0) {
            __inl57_tx[__w0] = __cb207[__w0];
          }
          free(__cb208);
          __cb208 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sy_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sy_ey; ++__i) {
            __cb208[__i] = __i;
          }
          free(__inl57_ty);
          __inl57_ty = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sy_ey; ++__w0) {
            __inl57_ty[__w0] = __cb208[__w0];
          }
          free(__cb209);
          __cb209 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ey) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ey; ++__i) {
            __cb209[__i] = __i;
          }
          free(__inl57_tz);
          __inl57_tz = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ey; ++__w0) {
            __inl57_tz[__w0] = __cb209[__w0];
          }
          free(__inl57_ix);
          __inl57_ix = (double *)malloc((size_t)(((o + 1)) * (1) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl57_ix[(((si0)*(1) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_lox + __inl1_j_ey[si3]) + __inl57_tx[si0]);
                }
              }
            }
          }
          free(__inl57_iy);
          __inl57_iy = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl57_iy[(((si0)*(((o - gal) + 1)) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_loy + __inl1_k_ey[si3]) + __inl57_ty[si1]);
                }
              }
            }
          }
          free(__inl57_iz);
          __inl57_iz = (double *)malloc((size_t)((1) * (1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl57_iz[(((si0)*(1) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = ((__inl1_loz + __inl1_l_ey[si3]) + __inl57_tz[si2]);
                }
              }
            }
          }
          free(__inl57_ix_b);
          __inl57_ix_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl57_ix_b[(((si0)*(((o - gal) + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl57_ix[(((si0)*(1) + (0))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl57_iy_b);
          __inl57_iy_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl57_iy_b[(((si0)*(((o - gal) + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl57_iy[(((0)*(((o - gal) + 1)) + (si1))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl57_iz_b);
          __inl57_iz_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl57_iz_b[(((si0)*(((o - gal) + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl57_iz[(((0)*(1) + (0))*((o + 1)) + (si2))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl57_gathered);
          __inl57_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl57_gathered[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = ey_arr[(((__inl57_ix_b[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)])*(((ncells + (2 * depos_order)) + 6)) + (__inl57_iy_b[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]))*(((ncells + (2 * depos_order)) + 6)) + (__inl57_iz_b[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
                }
              }
            }
          }
          free(__inl57_weight);
          __inl57_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl57_weight[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = ((__inl1_sx_ey[(__w0)*(np_particles) + (__w3)] * __inl1_sy_ey[(__w1)*(np_particles) + (__w3)]) * __inl1_sz_ey[(__w2)*(np_particles) + (__w3)]);
                }
              }
            }
          }
          free(__cb210);
          __cb210 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __cb210[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = (__inl57_weight[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] * __inl57_gathered[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]);
                }
              }
            }
          }
          double *__cb211 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb210, axis=(0, 1, 2)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb211[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                for (int64_t __rd2 = 0; __rd2 < (o + 1); ++__rd2) {
                  __cb211[__ax0] = (__cb211[__ax0] + __cb210[(((__rd0)*(((o - gal) + 1)) + (__rd1))*((o + 1)) + (__rd2))*(np_particles) + (__ax0)]);
                }
              }
            }
          }
          double *__hcall30 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall30[__w0] = __cb211[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Eyp[__w0] += __hcall30[__w0];
          }
          free(__cb212);
          __cb212 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sx_ez; ++__i) {
            __cb212[__i] = __i;
          }
          free(__inl58_tx);
          __inl58_tx = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_ez; ++__w0) {
            __inl58_tx[__w0] = __cb212[__w0];
          }
          free(__cb213);
          __cb213 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sy_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sy_ez; ++__i) {
            __cb213[__i] = __i;
          }
          free(__inl58_ty);
          __inl58_ty = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sy_ez; ++__w0) {
            __inl58_ty[__w0] = __cb213[__w0];
          }
          free(__cb214);
          __cb214 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_ez) */
          for (int64_t __i = 0; __i < __inl1_n_sz_ez; ++__i) {
            __cb214[__i] = __i;
          }
          free(__inl58_tz);
          __inl58_tz = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_ez; ++__w0) {
            __inl58_tz[__w0] = __cb214[__w0];
          }
          free(__inl58_ix);
          __inl58_ix = (double *)malloc((size_t)(((o + 1)) * (1) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl58_ix[(((si0)*(1) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_lox + __inl1_j_ez[si3]) + __inl58_tx[si0]);
                }
              }
            }
          }
          free(__inl58_iy);
          __inl58_iy = (double *)malloc((size_t)((1) * ((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl58_iy[(((si0)*((o + 1)) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_loy + __inl1_k_ez[si3]) + __inl58_ty[si1]);
                }
              }
            }
          }
          free(__inl58_iz);
          __inl58_iz = (double *)malloc((size_t)((1) * (1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl58_iz[(((si0)*(1) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = ((__inl1_loz + __inl1_l_ez[si3]) + __inl58_tz[si2]);
                }
              }
            }
          }
          free(__inl58_ix_b);
          __inl58_ix_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl58_ix_b[(((si0)*((o + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl58_ix[(((si0)*(1) + (0))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl58_iy_b);
          __inl58_iy_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl58_iy_b[(((si0)*((o + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl58_iy[(((0)*((o + 1)) + (si1))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl58_iz_b);
          __inl58_iz_b = (int64_t *)malloc((size_t)(((o + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl58_iz_b[(((si0)*((o + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl58_iz[(((0)*(1) + (0))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl58_gathered);
          __inl58_gathered = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl58_gathered[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = ez_arr[(((__inl58_ix_b[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)])*(((ncells + (2 * depos_order)) + 6)) + (__inl58_iy_b[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]))*(((ncells + (2 * depos_order)) + 6)) + (__inl58_iz_b[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
                }
              }
            }
          }
          free(__inl58_weight);
          __inl58_weight = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl58_weight[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = ((__inl1_sx_ez[(__w0)*(np_particles) + (__w3)] * __inl1_sy_ez[(__w1)*(np_particles) + (__w3)]) * __inl1_sz_ez[(__w2)*(np_particles) + (__w3)]);
                }
              }
            }
          }
          free(__cb215);
          __cb215 = (double *)malloc((size_t)(((o + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __cb215[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = (__inl58_weight[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] * __inl58_gathered[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]);
                }
              }
            }
          }
          double *__cb216 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb215, axis=(0, 1, 2)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb216[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                for (int64_t __rd2 = 0; __rd2 < ((o - gal) + 1); ++__rd2) {
                  __cb216[__ax0] = (__cb216[__ax0] + __cb215[(((__rd0)*((o + 1)) + (__rd1))*(((o - gal) + 1)) + (__rd2))*(np_particles) + (__ax0)]);
                }
              }
            }
          }
          double *__hcall31 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall31[__w0] = __cb216[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Ezp[__w0] += __hcall31[__w0];
          }
          free(__cb217);
          __cb217 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bz; ++__i) {
            __cb217[__i] = __i;
          }
          free(__inl59_tx);
          __inl59_tx = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bz; ++__w0) {
            __inl59_tx[__w0] = __cb217[__w0];
          }
          free(__cb218);
          __cb218 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sy_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sy_bz; ++__i) {
            __cb218[__i] = __i;
          }
          free(__inl59_ty);
          __inl59_ty = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sy_bz; ++__w0) {
            __inl59_ty[__w0] = __cb218[__w0];
          }
          free(__cb219);
          __cb219 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bz) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bz; ++__i) {
            __cb219[__i] = __i;
          }
          free(__inl59_tz);
          __inl59_tz = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bz; ++__w0) {
            __inl59_tz[__w0] = __cb219[__w0];
          }
          free(__inl59_ix);
          __inl59_ix = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl59_ix[(((si0)*(1) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_lox + __inl1_j_bz[si3]) + __inl59_tx[si0]);
                }
              }
            }
          }
          free(__inl59_iy);
          __inl59_iy = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl59_iy[(((si0)*(((o - gal) + 1)) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_loy + __inl1_k_bz[si3]) + __inl59_ty[si1]);
                }
              }
            }
          }
          free(__inl59_iz);
          __inl59_iz = (double *)malloc((size_t)((1) * (1) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl59_iz[(((si0)*(1) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = ((__inl1_loz + __inl1_l_bz[si3]) + __inl59_tz[si2]);
                }
              }
            }
          }
          free(__inl59_ix_b);
          __inl59_ix_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl59_ix_b[(((si0)*(((o - gal) + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl59_ix[(((si0)*(1) + (0))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl59_iy_b);
          __inl59_iy_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl59_iy_b[(((si0)*(((o - gal) + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl59_iy[(((0)*(((o - gal) + 1)) + (si1))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl59_iz_b);
          __inl59_iz_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < (o + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl59_iz_b[(((si0)*(((o - gal) + 1)) + (si1))*((o + 1)) + (si2))*(np_particles) + (si3)] = __inl59_iz[(((0)*(1) + (0))*((o + 1)) + (si2))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl59_gathered);
          __inl59_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl59_gathered[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = bz_arr[(((__inl59_ix_b[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)])*(((ncells + (2 * depos_order)) + 6)) + (__inl59_iy_b[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]))*(((ncells + (2 * depos_order)) + 6)) + (__inl59_iz_b[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
                }
              }
            }
          }
          free(__inl59_weight);
          __inl59_weight = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl59_weight[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = ((__inl1_sx_bz[(__w0)*(np_particles) + (__w3)] * __inl1_sy_bz[(__w1)*(np_particles) + (__w3)]) * __inl1_sz_bz[(__w2)*(np_particles) + (__w3)]);
                }
              }
            }
          }
          free(__cb220);
          __cb220 = (double *)malloc((size_t)((((o - gal) + 1)) * (((o - gal) + 1)) * ((o + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < (o + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __cb220[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] = (__inl59_weight[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)] * __inl59_gathered[(((__w0)*(((o - gal) + 1)) + (__w1))*((o + 1)) + (__w2))*(np_particles) + (__w3)]);
                }
              }
            }
          }
          double *__cb221 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb220, axis=(0, 1, 2)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb221[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                for (int64_t __rd2 = 0; __rd2 < (o + 1); ++__rd2) {
                  __cb221[__ax0] = (__cb221[__ax0] + __cb220[(((__rd0)*(((o - gal) + 1)) + (__rd1))*((o + 1)) + (__rd2))*(np_particles) + (__ax0)]);
                }
              }
            }
          }
          double *__hcall32 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall32[__w0] = __cb221[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bzp[__w0] += __hcall32[__w0];
          }
          free(__cb222);
          __cb222 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_by) */
          for (int64_t __i = 0; __i < __inl1_n_sx_by; ++__i) {
            __cb222[__i] = __i;
          }
          free(__inl60_tx);
          __inl60_tx = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_by; ++__w0) {
            __inl60_tx[__w0] = __cb222[__w0];
          }
          free(__cb223);
          __cb223 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sy_by) */
          for (int64_t __i = 0; __i < __inl1_n_sy_by; ++__i) {
            __cb223[__i] = __i;
          }
          free(__inl60_ty);
          __inl60_ty = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sy_by; ++__w0) {
            __inl60_ty[__w0] = __cb223[__w0];
          }
          free(__cb224);
          __cb224 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_by) */
          for (int64_t __i = 0; __i < __inl1_n_sz_by; ++__i) {
            __cb224[__i] = __i;
          }
          free(__inl60_tz);
          __inl60_tz = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_by; ++__w0) {
            __inl60_tz[__w0] = __cb224[__w0];
          }
          free(__inl60_ix);
          __inl60_ix = (double *)malloc((size_t)((((o - gal) + 1)) * (1) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl60_ix[(((si0)*(1) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_lox + __inl1_j_by[si3]) + __inl60_tx[si0]);
                }
              }
            }
          }
          free(__inl60_iy);
          __inl60_iy = (double *)malloc((size_t)((1) * ((o + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl60_iy[(((si0)*((o + 1)) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_loy + __inl1_k_by[si3]) + __inl60_ty[si1]);
                }
              }
            }
          }
          free(__inl60_iz);
          __inl60_iz = (double *)malloc((size_t)((1) * (1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl60_iz[(((si0)*(1) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = ((__inl1_loz + __inl1_l_by[si3]) + __inl60_tz[si2]);
                }
              }
            }
          }
          free(__inl60_ix_b);
          __inl60_ix_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl60_ix_b[(((si0)*((o + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl60_ix[(((si0)*(1) + (0))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl60_iy_b);
          __inl60_iy_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl60_iy_b[(((si0)*((o + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl60_iy[(((0)*((o + 1)) + (si1))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl60_iz_b);
          __inl60_iz_b = (int64_t *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < ((o - gal) + 1); ++si0) {
            for (int64_t si1 = 0; si1 < (o + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl60_iz_b[(((si0)*((o + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl60_iz[(((0)*(1) + (0))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl60_gathered);
          __inl60_gathered = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl60_gathered[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = by_arr[(((__inl60_ix_b[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)])*(((ncells + (2 * depos_order)) + 6)) + (__inl60_iy_b[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]))*(((ncells + (2 * depos_order)) + 6)) + (__inl60_iz_b[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
                }
              }
            }
          }
          free(__inl60_weight);
          __inl60_weight = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl60_weight[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = ((__inl1_sx_by[(__w0)*(np_particles) + (__w3)] * __inl1_sy_by[(__w1)*(np_particles) + (__w3)]) * __inl1_sz_by[(__w2)*(np_particles) + (__w3)]);
                }
              }
            }
          }
          free(__cb225);
          __cb225 = (double *)malloc((size_t)((((o - gal) + 1)) * ((o + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < ((o - gal) + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < (o + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __cb225[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = (__inl60_weight[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] * __inl60_gathered[(((__w0)*((o + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]);
                }
              }
            }
          }
          double *__cb226 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb225, axis=(0, 1, 2)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb226[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < ((o - gal) + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < (o + 1); ++__rd1) {
                for (int64_t __rd2 = 0; __rd2 < ((o - gal) + 1); ++__rd2) {
                  __cb226[__ax0] = (__cb226[__ax0] + __cb225[(((__rd0)*((o + 1)) + (__rd1))*(((o - gal) + 1)) + (__rd2))*(np_particles) + (__ax0)]);
                }
              }
            }
          }
          double *__hcall33 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall33[__w0] = __cb226[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Byp[__w0] += __hcall33[__w0];
          }
          free(__cb227);
          __cb227 = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sx_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sx_bx; ++__i) {
            __cb227[__i] = __i;
          }
          free(__inl61_tx);
          __inl61_tx = (int64_t *)malloc((size_t)(((o + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sx_bx; ++__w0) {
            __inl61_tx[__w0] = __cb227[__w0];
          }
          free(__cb228);
          __cb228 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sy_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sy_bx; ++__i) {
            __cb228[__i] = __i;
          }
          free(__inl61_ty);
          __inl61_ty = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sy_bx; ++__w0) {
            __inl61_ty[__w0] = __cb228[__w0];
          }
          free(__cb229);
          __cb229 = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          /* numpy: np.arange(__inl1_n_sz_bx) */
          for (int64_t __i = 0; __i < __inl1_n_sz_bx; ++__i) {
            __cb229[__i] = __i;
          }
          free(__inl61_tz);
          __inl61_tz = (int64_t *)malloc((size_t)((((o - gal) + 1))) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < __inl1_n_sz_bx; ++__w0) {
            __inl61_tz[__w0] = __cb229[__w0];
          }
          free(__inl61_ix);
          __inl61_ix = (double *)malloc((size_t)(((o + 1)) * (1) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl61_ix[(((si0)*(1) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_lox + __inl1_j_bx[si3]) + __inl61_tx[si0]);
                }
              }
            }
          }
          free(__inl61_iy);
          __inl61_iy = (double *)malloc((size_t)((1) * (((o - gal) + 1)) * (1) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < 1; ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl61_iy[(((si0)*(((o - gal) + 1)) + (si1))*(1) + (si2))*(np_particles) + (si3)] = ((__inl1_loy + __inl1_k_bx[si3]) + __inl61_ty[si1]);
                }
              }
            }
          }
          free(__inl61_iz);
          __inl61_iz = (double *)malloc((size_t)((1) * (1) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t si0 = 0; si0 < 1; ++si0) {
            for (int64_t si1 = 0; si1 < 1; ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl61_iz[(((si0)*(1) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = ((__inl1_loz + __inl1_l_bx[si3]) + __inl61_tz[si2]);
                }
              }
            }
          }
          free(__inl61_ix_b);
          __inl61_ix_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl61_ix_b[(((si0)*(((o - gal) + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl61_ix[(((si0)*(1) + (0))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl61_iy_b);
          __inl61_iy_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl61_iy_b[(((si0)*(((o - gal) + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl61_iy[(((0)*(((o - gal) + 1)) + (si1))*(1) + (0))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl61_iz_b);
          __inl61_iz_b = (int64_t *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(int64_t));
          for (int64_t si0 = 0; si0 < (o + 1); ++si0) {
            for (int64_t si1 = 0; si1 < ((o - gal) + 1); ++si1) {
              for (int64_t si2 = 0; si2 < ((o - gal) + 1); ++si2) {
                for (int64_t si3 = 0; si3 < np_particles; ++si3) {
                  __inl61_iz_b[(((si0)*(((o - gal) + 1)) + (si1))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)] = __inl61_iz[(((0)*(1) + (0))*(((o - gal) + 1)) + (si2))*(np_particles) + (si3)];
                }
              }
            }
          }
          free(__inl61_gathered);
          __inl61_gathered = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl61_gathered[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = bx_arr[(((__inl61_ix_b[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)])*(((ncells + (2 * depos_order)) + 6)) + (__inl61_iy_b[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]))*(((ncells + (2 * depos_order)) + 6)) + (__inl61_iz_b[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)];
                }
              }
            }
          }
          free(__inl61_weight);
          __inl61_weight = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __inl61_weight[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = ((__inl1_sx_bx[(__w0)*(np_particles) + (__w3)] * __inl1_sy_bx[(__w1)*(np_particles) + (__w3)]) * __inl1_sz_bx[(__w2)*(np_particles) + (__w3)]);
                }
              }
            }
          }
          free(__cb230);
          __cb230 = (double *)malloc((size_t)(((o + 1)) * (((o - gal) + 1)) * (((o - gal) + 1)) * (np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < (o + 1); ++__w0) {
            for (int64_t __w1 = 0; __w1 < ((o - gal) + 1); ++__w1) {
              for (int64_t __w2 = 0; __w2 < ((o - gal) + 1); ++__w2) {
                for (int64_t __w3 = 0; __w3 < np_particles; ++__w3) {
                  __cb230[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] = (__inl61_weight[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)] * __inl61_gathered[(((__w0)*(((o - gal) + 1)) + (__w1))*(((o - gal) + 1)) + (__w2))*(np_particles) + (__w3)]);
                }
              }
            }
          }
          double *__cb231 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.sum(__cb230, axis=(0, 1, 2)) */
          for (int64_t __ax0 = 0; __ax0 < np_particles; ++__ax0) {
            __cb231[__ax0] = 0.0;
            for (int64_t __rd0 = 0; __rd0 < (o + 1); ++__rd0) {
              for (int64_t __rd1 = 0; __rd1 < ((o - gal) + 1); ++__rd1) {
                for (int64_t __rd2 = 0; __rd2 < ((o - gal) + 1); ++__rd2) {
                  __cb231[__ax0] = (__cb231[__ax0] + __cb230[(((__rd0)*(((o - gal) + 1)) + (__rd1))*(((o - gal) + 1)) + (__rd2))*(np_particles) + (__ax0)]);
                }
              }
            }
          }
          double *__hcall34 = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __hcall34[__w0] = __cb231[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            Bxp[__w0] += __hcall34[__w0];
          }
          free(__cb206);
          free(__hcall29);
          free(__cb211);
          free(__hcall30);
          free(__cb216);
          free(__hcall31);
          free(__cb221);
          free(__hcall32);
          free(__cb226);
          free(__hcall33);
          free(__cb231);
          free(__hcall34);
        }
        free(__inl1_j_node);
        free(__inl1_j_cell);
        free(__inl1_j_node_v);
        free(__inl1_j_cell_v);
        free(__inl2_idx);
        free(__inl3_idx);
        free(__inl4_idx);
        free(__inl5_idx);
        free(__inl1_k_node);
        free(__inl1_k_cell);
        free(__inl1_k_node_v);
        free(__inl1_k_cell_v);
        free(__inl6_idx);
        free(__inl7_idx);
        free(__inl8_idx);
        free(__inl9_idx);
        free(__inl1_l_node);
        free(__inl1_l_cell);
        free(__inl1_l_node_v);
        free(__inl1_l_cell_v);
        free(__inl10_idx);
        free(__inl11_idx);
        free(__inl12_idx);
        free(__inl13_idx);
        free(__cb111);
        free(__cb115);
        free(__cb119);
        free(__cb123);
        free(__cb127);
        free(__cb131);
        free(__cb135);
        free(__cb139);
        free(__cb143);
        free(__cb147);
        free(__cb151);
        free(__cb155);
        free(__inl1_rp);
        free(__inl1_x);
        free(__inl2_xint);
        free(__inl3_xint);
        free(__inl4_xint);
        free(__inl5_xint);
        free(__inl1_j_ex);
        free(__inl1_j_ey);
        free(__inl1_j_ez);
        free(__inl1_j_bx);
        free(__inl1_j_by);
        free(__inl1_j_bz);
        free(__inl6_xint);
        free(__inl7_xint);
        free(__inl8_xint);
        free(__inl9_xint);
        free(__inl1_k_ex);
        free(__inl1_k_ey);
        free(__inl1_k_ez);
        free(__inl1_k_bx);
        free(__inl1_k_by);
        free(__inl1_k_bz);
        free(__inl10_xint);
        free(__inl11_xint);
        free(__inl12_xint);
        free(__inl13_xint);
        free(__inl1_l_ex);
        free(__inl1_l_ey);
        free(__inl1_l_ez);
        free(__inl1_l_bx);
        free(__inl1_l_by);
        free(__inl1_l_bz);
        free(__inl1_Ethetap);
        free(__inl1_Erp);
        free(__inl1_Brp);
        free(__inl1_Bthetap);
        free(__inl1_rp_safe);
        free(__inl1_costheta);
        free(__inl1_sintheta);
        free(__inl1_xy_re);
        free(__inl1_xy_im);
        free(__hcall15);
        free(__hcall16);
        free(__inl1_dEy);
        free(__hcall17);
        free(__hcall18);
        free(__inl1_dEx);
        free(__hcall19);
        free(__hcall20);
        free(__inl1_dBz);
        free(__hcall21);
        free(__hcall22);
        free(__inl1_dEz);
        free(__hcall23);
        free(__hcall24);
        free(__inl1_dBx);
        free(__hcall25);
        free(__hcall26);
        free(__inl1_dBy);
        free(__inl1_tmp_re);
        free(__inl1_tmp_im);
        free(__inl2_j);
        free(__inl3_j);
        free(__inl4_j);
        free(__inl5_j);
        free(__inl6_j);
        free(__inl7_j);
        free(__inl8_j);
        free(__inl9_j);
        free(__inl10_j);
        free(__inl11_j);
        free(__inl12_j);
        free(__inl13_j);
        free(__inl1_sx_node);
        free(__inl1_sx_cell);
        free(__inl1_sx_node_g);
        free(__inl1_sx_cell_g);
        free(__inl1_sy_node);
        free(__inl1_sy_cell);
        free(__inl1_sy_node_v);
        free(__inl1_sy_cell_v);
        free(__inl1_sz_node);
        free(__inl1_sz_cell);
        free(__inl1_sz_node_v);
        free(__inl1_sz_cell_v);
        free(__inl20_ia_b);
        free(__inl20_ib_b);
        free(__inl21_ia_b);
        free(__inl21_ib_b);
        free(__inl22_ia_b);
        free(__inl22_ib_b);
        free(__inl23_ia_b);
        free(__inl23_ib_b);
        free(__inl24_ia_b);
        free(__inl24_ib_b);
        free(__inl25_ia_b);
        free(__inl25_ib_b);
        free(__inl26_ia_b);
        free(__inl26_ib_b);
        free(__inl27_ia_b);
        free(__inl27_ib_b);
        free(__inl28_ia_b);
        free(__inl28_ib_b);
        free(__inl29_ia_b);
        free(__inl29_ib_b);
        free(__inl30_ia_b);
        free(__inl30_ib_b);
        free(__inl31_ia_b);
        free(__inl31_ib_b);
        free(__inl32_ia_b);
        free(__inl32_ib_b);
        free(__inl33_ia_b);
        free(__inl33_ib_b);
        free(__inl34_ia_b);
        free(__inl34_ib_b);
        free(__inl35_ia_b);
        free(__inl35_ib_b);
        free(__inl36_ia_b);
        free(__inl36_ib_b);
        free(__inl37_ia_b);
        free(__inl37_ib_b);
        free(__inl38_ia_b);
        free(__inl38_ib_b);
        free(__inl39_ia_b);
        free(__inl39_ib_b);
        free(__inl40_ia_b);
        free(__inl40_ib_b);
        free(__inl41_ia_b);
        free(__inl41_ib_b);
        free(__inl42_ia_b);
        free(__inl42_ib_b);
        free(__inl43_ia_b);
        free(__inl43_ib_b);
        free(__inl56_ix_b);
        free(__inl56_iy_b);
        free(__inl56_iz_b);
        free(__inl57_ix_b);
        free(__inl57_iy_b);
        free(__inl57_iz_b);
        free(__inl58_ix_b);
        free(__inl58_iy_b);
        free(__inl58_iz_b);
        free(__inl59_ix_b);
        free(__inl59_iy_b);
        free(__inl59_iz_b);
        free(__inl60_ix_b);
        free(__inl60_iy_b);
        free(__inl60_iz_b);
        free(__inl61_ix_b);
        free(__inl61_iy_b);
        free(__inl61_iz_b);
        free(__cb3);
        free(__cb4);
        free(__cb5);
        free(__cb6);
        free(__cb7);
        free(__cb8);
        free(__cb15);
        free(__cb16);
        free(__cb17);
        free(__cb18);
        free(__cb19);
        free(__cb20);
        free(__cb27);
        free(__cb28);
        free(__cb29);
        free(__cb30);
        free(__cb31);
        free(__cb32);
        free(__cb39);
        free(__cb40);
        free(__cb42);
        free(__cb43);
        free(__cb45);
        free(__cb46);
        free(__cb48);
        free(__cb49);
        free(__cb51);
        free(__cb52);
        free(__cb54);
        free(__cb55);
        free(__cb57);
        free(__cb58);
        free(__cb59);
        free(__cb61);
        free(__cb62);
        free(__cb63);
        free(__cb65);
        free(__cb66);
        free(__cb67);
        free(__cb69);
        free(__cb70);
        free(__cb71);
        free(__cb73);
        free(__cb74);
        free(__cb75);
        free(__cb77);
        free(__cb78);
        free(__cb79);
        free(__cb81);
        free(__cb82);
        free(__cb83);
        free(__cb85);
        free(__cb86);
        free(__cb87);
        free(__cb89);
        free(__cb90);
        free(__cb91);
        free(__cb93);
        free(__cb94);
        free(__cb95);
        free(__cb97);
        free(__cb98);
        free(__cb99);
        free(__cb101);
        free(__cb102);
        free(__cb103);
        free(__cb108);
        free(__cb109);
        free(__cb110);
        free(__cb112);
        free(__cb113);
        free(__cb114);
        free(__cb116);
        free(__cb117);
        free(__cb118);
        free(__cb120);
        free(__cb121);
        free(__cb122);
        free(__cb124);
        free(__cb125);
        free(__cb126);
        free(__cb128);
        free(__cb129);
        free(__cb130);
        free(__cb132);
        free(__cb133);
        free(__cb134);
        free(__cb136);
        free(__cb137);
        free(__cb138);
        free(__cb140);
        free(__cb141);
        free(__cb142);
        free(__cb144);
        free(__cb145);
        free(__cb146);
        free(__cb148);
        free(__cb149);
        free(__cb150);
        free(__cb152);
        free(__cb153);
        free(__cb154);
        free(__cb156);
        free(__cb157);
        free(__cb159);
        free(__cb160);
        free(__cb162);
        free(__cb163);
        free(__cb165);
        free(__cb166);
        free(__cb168);
        free(__cb169);
        free(__cb171);
        free(__cb172);
        free(__cb177);
        free(__cb178);
        free(__cb180);
        free(__cb181);
        free(__cb183);
        free(__cb184);
        free(__cb186);
        free(__cb187);
        free(__cb189);
        free(__cb190);
        free(__cb192);
        free(__cb193);
        free(__cb202);
        free(__cb203);
        free(__cb204);
        free(__cb205);
        free(__cb207);
        free(__cb208);
        free(__cb209);
        free(__cb210);
        free(__cb212);
        free(__cb213);
        free(__cb214);
        free(__cb215);
        free(__cb217);
        free(__cb218);
        free(__cb219);
        free(__cb220);
        free(__cb222);
        free(__cb223);
        free(__cb224);
        free(__cb225);
        free(__cb227);
        free(__cb228);
        free(__cb229);
        free(__cb230);
        free(__inl1_sx_ex);
        free(__inl1_sx_ey);
        free(__inl1_sx_ez);
        free(__inl1_sx_bx);
        free(__inl1_sx_by);
        free(__inl1_sx_bz);
        free(__inl1_sy_ex);
        free(__inl1_sy_ey);
        free(__inl1_sy_ez);
        free(__inl1_sy_bx);
        free(__inl1_sy_by);
        free(__inl1_sy_bz);
        free(__inl1_sz_ex);
        free(__inl1_sz_ey);
        free(__inl1_sz_ez);
        free(__inl1_sz_bx);
        free(__inl1_sz_by);
        free(__inl1_sz_bz);
        free(__inl14_taps);
        free(__inl14_rows);
        free(__inl15_taps);
        free(__inl15_rows);
        free(__inl16_taps);
        free(__inl16_rows);
        free(__inl17_taps);
        free(__inl17_rows);
        free(__inl18_taps);
        free(__inl18_rows);
        free(__inl19_taps);
        free(__inl19_rows);
        free(__inl20_ta);
        free(__inl20_tb);
        free(__inl20_weight);
        free(__inl21_ta);
        free(__inl21_tb);
        free(__inl21_weight);
        free(__inl22_ta);
        free(__inl22_tb);
        free(__inl22_weight);
        free(__inl23_ta);
        free(__inl23_tb);
        free(__inl23_weight);
        free(__inl24_ta);
        free(__inl24_tb);
        free(__inl24_weight);
        free(__inl25_ta);
        free(__inl25_tb);
        free(__inl25_weight);
        free(__inl26_ta);
        free(__inl26_tb);
        free(__inl26_weight);
        free(__inl27_ta);
        free(__inl27_tb);
        free(__inl27_weight);
        free(__inl28_ta);
        free(__inl28_tb);
        free(__inl28_weight);
        free(__inl29_ta);
        free(__inl29_tb);
        free(__inl29_weight);
        free(__inl30_ta);
        free(__inl30_tb);
        free(__inl30_weight);
        free(__inl31_ta);
        free(__inl31_tb);
        free(__inl31_weight);
        free(__inl32_ta);
        free(__inl32_tb);
        free(__inl32_weight);
        free(__inl33_ta);
        free(__inl33_tb);
        free(__inl33_weight);
        free(__inl34_ta);
        free(__inl34_tb);
        free(__inl34_weight);
        free(__inl35_ta);
        free(__inl35_tb);
        free(__inl35_weight);
        free(__inl36_ta);
        free(__inl36_tb);
        free(__inl36_weight);
        free(__inl37_ta);
        free(__inl37_tb);
        free(__inl37_weight);
        free(__inl38_ta);
        free(__inl38_tb);
        free(__inl38_weight);
        free(__inl39_ta);
        free(__inl39_tb);
        free(__inl39_weight);
        free(__inl40_ta);
        free(__inl40_tb);
        free(__inl40_weight);
        free(__inl41_ta);
        free(__inl41_tb);
        free(__inl41_weight);
        free(__inl42_ta);
        free(__inl42_tb);
        free(__inl42_weight);
        free(__inl43_ta);
        free(__inl43_tb);
        free(__inl43_weight);
        free(__inl44_taps);
        free(__inl44_rows);
        free(__inl45_taps);
        free(__inl45_rows);
        free(__inl46_taps);
        free(__inl46_rows);
        free(__inl47_taps);
        free(__inl47_rows);
        free(__inl48_taps);
        free(__inl48_rows);
        free(__inl49_taps);
        free(__inl49_rows);
        free(__inl50_taps);
        free(__inl50_rows);
        free(__inl51_taps);
        free(__inl51_rows);
        free(__inl52_taps);
        free(__inl52_rows);
        free(__inl53_taps);
        free(__inl53_rows);
        free(__inl54_taps);
        free(__inl54_rows);
        free(__inl55_taps);
        free(__inl55_rows);
        free(__inl56_tx);
        free(__inl56_ty);
        free(__inl56_tz);
        free(__inl56_weight);
        free(__inl57_tx);
        free(__inl57_ty);
        free(__inl57_tz);
        free(__inl57_weight);
        free(__inl58_tx);
        free(__inl58_ty);
        free(__inl58_tz);
        free(__inl58_weight);
        free(__inl59_tx);
        free(__inl59_ty);
        free(__inl59_tz);
        free(__inl59_weight);
        free(__inl60_tx);
        free(__inl60_ty);
        free(__inl60_tz);
        free(__inl60_weight);
        free(__inl61_tx);
        free(__inl61_ty);
        free(__inl61_tz);
        free(__inl61_weight);
        free(__inl14_gathered);
        free(__inl15_gathered);
        free(__inl16_gathered);
        free(__inl17_gathered);
        free(__inl18_gathered);
        free(__inl19_gathered);
        free(__inl20_ia);
        free(__inl20_ib);
        free(__inl20_gathered);
        free(__inl21_ia);
        free(__inl21_ib);
        free(__inl21_gathered);
        free(__inl22_ia);
        free(__inl22_ib);
        free(__inl22_gathered);
        free(__inl23_ia);
        free(__inl23_ib);
        free(__inl23_gathered);
        free(__inl24_ia);
        free(__inl24_ib);
        free(__inl24_gathered);
        free(__inl25_ia);
        free(__inl25_ib);
        free(__inl25_gathered);
        free(__inl26_ia);
        free(__inl26_ib);
        free(__inl26_gathered);
        free(__inl27_ia);
        free(__inl27_ib);
        free(__inl27_gathered);
        free(__inl28_ia);
        free(__inl28_ib);
        free(__inl28_gathered);
        free(__inl29_ia);
        free(__inl29_ib);
        free(__inl29_gathered);
        free(__inl30_ia);
        free(__inl30_ib);
        free(__inl30_gathered);
        free(__inl31_ia);
        free(__inl31_ib);
        free(__inl31_gathered);
        free(__inl32_ia);
        free(__inl32_ib);
        free(__inl32_gathered);
        free(__inl33_ia);
        free(__inl33_ib);
        free(__inl33_gathered);
        free(__inl34_ia);
        free(__inl34_ib);
        free(__inl34_gathered);
        free(__inl35_ia);
        free(__inl35_ib);
        free(__inl35_gathered);
        free(__inl36_ia);
        free(__inl36_ib);
        free(__inl36_gathered);
        free(__inl37_ia);
        free(__inl37_ib);
        free(__inl37_gathered);
        free(__inl38_ia);
        free(__inl38_ib);
        free(__inl38_gathered);
        free(__inl39_ia);
        free(__inl39_ib);
        free(__inl39_gathered);
        free(__inl40_ia);
        free(__inl40_ib);
        free(__inl40_gathered);
        free(__inl41_ia);
        free(__inl41_ib);
        free(__inl41_gathered);
        free(__inl42_ia);
        free(__inl42_ib);
        free(__inl42_gathered);
        free(__inl43_ia);
        free(__inl43_ib);
        free(__inl43_gathered);
        free(__inl44_gathered);
        free(__inl45_gathered);
        free(__inl46_gathered);
        free(__inl47_gathered);
        free(__inl48_gathered);
        free(__inl49_gathered);
        free(__inl50_gathered);
        free(__inl51_gathered);
        free(__inl52_gathered);
        free(__inl53_gathered);
        free(__inl54_gathered);
        free(__inl55_gathered);
        free(__inl56_ix);
        free(__inl56_iy);
        free(__inl56_iz);
        free(__inl56_gathered);
        free(__inl57_ix);
        free(__inl57_iy);
        free(__inl57_iz);
        free(__inl57_gathered);
        free(__inl58_ix);
        free(__inl58_iy);
        free(__inl58_iz);
        free(__inl58_gathered);
        free(__inl59_ix);
        free(__inl59_iy);
        free(__inl59_iz);
        free(__inl59_gathered);
        free(__inl60_ix);
        free(__inl60_iy);
        free(__inl60_iz);
        free(__inl60_gathered);
        free(__inl61_ix);
        free(__inl61_iy);
        free(__inl61_iz);
        free(__inl61_gathered);
}
