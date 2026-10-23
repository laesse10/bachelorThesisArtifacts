// hpcagent_bench-autogen -- generated from nfa_frontier_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
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

void nfa_frontier_fp64(int64_t *restrict activation_counts, const int64_t *restrict col_idx, const int64_t *restrict comp_ptr, const uint8_t *restrict is_report, int64_t *restrict report_counts, const int64_t *restrict row_ptr, const int64_t *restrict start_idx, const int64_t *restrict start_ptr, const uint8_t *restrict start_sod, const int64_t *restrict stream, const uint8_t *restrict symbol_cols, const int64_t C, const int64_t NE, const int64_t NS, const int64_t NSTART, const int64_t T) {
        int64_t n_front;
        int64_t reports;
        int64_t s;
        int64_t eod;
        int64_t n_match;
        int64_t child;
        int64_t *enabled = (int64_t *)malloc((size_t)((NS)) * sizeof(int64_t));
        memset(enabled, 0, (size_t)((NS)) * sizeof(int64_t));
        int64_t *frontier = (int64_t *)malloc((size_t)((NS)) * sizeof(int64_t));
        memset(frontier, 0, (size_t)((NS)) * sizeof(int64_t));
        int64_t *matched = (int64_t *)malloc((size_t)((NS)) * sizeof(int64_t));
        memset(matched, 0, (size_t)((NS)) * sizeof(int64_t));
        memset(enabled, 0, (size_t)((NS)) * sizeof(int64_t));
        memset(frontier, 0, (size_t)((NS)) * sizeof(int64_t));
        memset(matched, 0, (size_t)((NS)) * sizeof(int64_t));
        for (int64_t c = 0; c < C; ++c) {
          n_front = 0;
          for (int64_t k = start_ptr[c]; k < start_ptr[(c + 1)]; ++k) {
            s = start_idx[k];
            if ((enabled[s] == 0)) {
              enabled[s] = 1;
              frontier[(comp_ptr[c] + n_front)] = s;
              n_front += 1;
            }
          }
          reports = ((int64_t)(0));
          for (int64_t t = 0; t < T; ++t) {
            eod = 0;
            if ((t == (T - 1))) {
              eod = 1;
            }
            else if ((stream[t] == 10)) {
              eod = 1;
            }
            n_match = 0;
            for (int64_t k = 0; k < n_front; ++k) {
              s = frontier[(comp_ptr[c] + k)];
              if ((((int64_t)(symbol_cols[(s)*(256) + (stream[t])])) != 0)) {
                matched[(comp_ptr[c] + n_match)] = s;
                n_match += 1;
                activation_counts[s] += 1;
                if ((((int64_t)(is_report[s])) != 0)) {
                  reports += 1;
                }
              }
              enabled[s] = 0;
            }
            n_front = 0;
            for (int64_t k = 0; k < n_match; ++k) {
              s = matched[(comp_ptr[c] + k)];
              for (int64_t e = row_ptr[s]; e < row_ptr[(s + 1)]; ++e) {
                child = col_idx[e];
                if ((enabled[child] == 0)) {
                  enabled[child] = 1;
                  frontier[(comp_ptr[c] + n_front)] = child;
                  n_front += 1;
                }
              }
            }
            for (int64_t k = start_ptr[c]; k < start_ptr[(c + 1)]; ++k) {
              if (((((int64_t)(start_sod[k])) == 0) || (eod == 1))) {
                s = start_idx[k];
                if ((enabled[s] == 0)) {
                  enabled[s] = 1;
                  frontier[(comp_ptr[c] + n_front)] = s;
                  n_front += 1;
                }
              }
            }
          }
          report_counts[c] = reports;
        }
        free(enabled);
        free(frontier);
        free(matched);
}
