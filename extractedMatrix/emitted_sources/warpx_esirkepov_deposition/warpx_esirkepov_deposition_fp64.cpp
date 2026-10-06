// hpcagent_bench-autogen -- generated from warpx_esirkepov_deposition_numpy.py; edit the numpy reference and regenerate, or delete this line to keep local edits as a hand override.
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

void warpx_esirkepov_deposition_fp64(double *__restrict__ Jx, double *__restrict__ Jy, double *__restrict__ Jz, const double *__restrict__ dinv, const int32_t *__restrict__ ion_lev, const int32_t *__restrict__ lo, const int32_t *__restrict__ reduced_particle_shape_mask, const double *__restrict__ uxp, const double *__restrict__ uyp, const double *__restrict__ uzp, const double *__restrict__ wp, const double *__restrict__ xp, const double *__restrict__ xyzmin, const double *__restrict__ yp, const double *__restrict__ zp, const int64_t depos_order, const int64_t do_ionization, const double dt, const int64_t enable_reduced_shape, int64_t geom, const int64_t n_rz_azimuthal_modes, const int64_t ncells, const int64_t np_particles, const double q, const double relative_time) {
        int64_t o;
        int64_t n_modes;
        int64_t do_ion;
        bool reduce_enabled;
        bool rz_modes;
        double dinvx;
        double dinvy;
        double dinvz;
        double xmin;
        double ymin;
        double zmin;
        int64_t lox;
        int64_t loy;
        int64_t loz;
        double invvol;
        double invdtd_x;
        double invdtd_y;
        double invdtd_z;
        double half_dt_step;
        int64_t half;
        int64_t width;
        double wqi;
        int64_t i0;
        int64_t i1;
        int64_t j0_;
        int64_t j1_;
        int64_t k0;
        int64_t k1;
        int64_t ib;
        int64_t jb;
        int64_t kb;
        int64_t i0y;
        int64_t i1y;
        int64_t j0y;
        int64_t j1y;
        int64_t k0y;
        int64_t k1y;
        int64_t i0z;
        int64_t i1z;
        int64_t j0z;
        int64_t j1z;
        int64_t k0z;
        int64_t k1z;
        double xy_mid_re;
        double xy_mid_im;
        double xy_new_re;
        double xy_new_im;
        double xy_old_re;
        double xy_old_im;
        int64_t i0x;
        int64_t i1x;
        double nxt_mid_re;
        double nxt_mid_im;
        double nxt_new_re;
        double nxt_new_im;
        double nxt_old_re;
        double nxt_old_im;
        double *x_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(x_new, 0, (size_t)((np_particles)) * sizeof(double));
        double *x_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(x_old, 0, (size_t)((np_particles)) * sizeof(double));
        double *y_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(y_new, 0, (size_t)((np_particles)) * sizeof(double));
        double *y_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(y_old, 0, (size_t)((np_particles)) * sizeof(double));
        double *z_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(z_new, 0, (size_t)((np_particles)) * sizeof(double));
        double *z_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(z_old, 0, (size_t)((np_particles)) * sizeof(double));
        double *vx = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(vx, 0, (size_t)((np_particles)) * sizeof(double));
        double *vy = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(vy, 0, (size_t)((np_particles)) * sizeof(double));
        double *vz = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(vz, 0, (size_t)((np_particles)) * sizeof(double));
        double *xy_new0_re = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(xy_new0_re, 0, (size_t)((np_particles)) * sizeof(double));
        double *xy_mid0_re = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(xy_mid0_re, 0, (size_t)((np_particles)) * sizeof(double));
        double *xy_old0_re = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(xy_old0_re, 0, (size_t)((np_particles)) * sizeof(double));
        double *xy_new0_im = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(xy_new0_im, 0, (size_t)((np_particles)) * sizeof(double));
        double *xy_mid0_im = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(xy_mid0_im, 0, (size_t)((np_particles)) * sizeof(double));
        double *xy_old0_im = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(xy_old0_im, 0, (size_t)((np_particles)) * sizeof(double));
        double *reduce_shape_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(reduce_shape_old, 0, (size_t)((np_particles)) * sizeof(double));
        double *reduce_shape_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        memset(reduce_shape_new, 0, (size_t)((np_particles)) * sizeof(double));
        int64_t *i_new = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        memset(i_new, 0, (size_t)((np_particles)) * sizeof(int64_t));
        int64_t *dil = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        for (int64_t __zf = 0; __zf < ((np_particles)); ++__zf) dil[__zf] = 1;
        double *__cb1 = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *gaminv = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *wq = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *xp_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *yp_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *xp_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *yp_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *xp_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *yp_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *costheta_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *sintheta_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *rp_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *cosphi_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *sinphi_mid = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *i_old = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *j_new = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *j_old = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *k_new = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *k_old = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl11_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl11_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl12_i_shift = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl12_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl12_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl15_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl15_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl16_i_shift = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl16_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl16_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl19_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl19_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *__inl20_i_shift = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl20_idx = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl20_xint = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *diu = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *djl = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *dju = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *dkl = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *dku = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *rp_new = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        double *rp_old = (double *)malloc((size_t)((np_particles)) * sizeof(double));
        int64_t *fx_o = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *fz_o = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *fx_n = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *fz_n = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl11_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl12_i = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl15_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl16_i = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl19_j = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        int64_t *__inl20_i = (int64_t *)malloc((size_t)((np_particles)) * sizeof(int64_t));
        double *__inl11_sx = NULL;
        double *__inl12_sx = NULL;
        double *__inl13_sx = NULL;
        double *__inl14_sx = NULL;
        double *__inl15_sx = NULL;
        double *__inl16_sx = NULL;
        double *__inl17_sx = NULL;
        double *__inl18_sx = NULL;
        double *__inl19_sx = NULL;
        double *__inl20_sx = NULL;
        double *__inl21_sx = NULL;
        double *__inl22_sx = NULL;
        double *__cb25 = NULL;
        double *__cb26 = NULL;
        double *__cb30 = NULL;
        double *__cb31 = NULL;
        double *__cb35 = NULL;
        double *__cb36 = NULL;
        double *__cb43 = NULL;
        double *__cb45 = NULL;
        double *__cb47 = NULL;
        double *__cb49 = NULL;
        int64_t *__cb51 = NULL;
        double *__cb52 = NULL;
        double *__cb54 = NULL;
        double *__cb56 = NULL;
        double *cum_x = NULL;
        double *cum_y = NULL;
        double *cum_z = NULL;
        double *cum_x__v1 = NULL;
        double *sx_new = NULL;
        double *sx_old = NULL;
        double *ov_new = NULL;
        double *ov_old = NULL;
        double *sy_new = NULL;
        double *sy_old = NULL;
        double *sz_new = NULL;
        double *sz_old = NULL;
        double *gx = NULL;
        double *gy = NULL;
        double *gz = NULL;
        double *zavg_x = NULL;
        double *sdxi = NULL;
        double *djr = NULL;
        double *sdyj = NULL;
        double *a_re = NULL;
        double *b_re = NULL;
        int64_t *i_local = NULL;
        double *neg2coef = NULL;
        double *sum_re = NULL;
        double *sum_im = NULL;
        double *coef_m = NULL;
        double *xavg_z = NULL;
        double *sdzk = NULL;
        double *djz = NULL;
        double *zavg = NULL;
        double *xavg = NULL;
        o = ((int64_t)(depos_order));
        geom = ((int64_t)(geom));
        n_modes = ((int64_t)(n_rz_azimuthal_modes));
        do_ion = ((int64_t)(do_ionization));
        reduce_enabled = ((((int64_t)(enable_reduced_shape)) != 0) && (o > 1));
        rz_modes = ((geom == 2) && (n_modes > 1));
        dinvx = dinv[0];
        dinvy = dinv[1];
        dinvz = dinv[2];
        xmin = xyzmin[0];
        ymin = xyzmin[1];
        zmin = xyzmin[2];
        lox = ((int64_t)(((int64_t)(lo[0]))));
        loy = ((int64_t)(((int64_t)(lo[1]))));
        loz = ((int64_t)(((int64_t)(lo[2]))));
        invvol = ((dinvx * dinvy) * dinvz);
        invdtd_x = (((1.0 / dt) * dinvy) * dinvz);
        invdtd_y = (((1.0 / dt) * dinvx) * dinvz);
        invdtd_z = (((1.0 / dt) * dinvx) * dinvy);
        /* numpy: np.sqrt(1.0 + (uxp * uxp + uyp * uyp + uzp * uzp) * 1.1126500560536185e-17) */
        for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
          __cb1[__r0] = sqrt((1.0 + ((((uxp[__r0] * uxp[__r0]) + (uyp[__r0] * uyp[__r0])) + (uzp[__r0] * uzp[__r0])) * 1.1126500560536185e-17)));
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          gaminv[__w0] = (1.0 / __cb1[__w0]);
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          wq[__w0] = (q * wp[__w0]);
        }
        if ((do_ion != 0)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            wq[__w0] = (wq[__w0] * ((int64_t)(ion_lev[__w0])));
          }
        }
        half_dt_step = (relative_time + (0.5 * dt));
        memset(x_new, 0, (size_t)((np_particles)) * sizeof(double));
        memset(x_old, 0, (size_t)((np_particles)) * sizeof(double));
        memset(y_new, 0, (size_t)((np_particles)) * sizeof(double));
        memset(y_old, 0, (size_t)((np_particles)) * sizeof(double));
        memset(z_new, 0, (size_t)((np_particles)) * sizeof(double));
        memset(z_old, 0, (size_t)((np_particles)) * sizeof(double));
        memset(vx, 0, (size_t)((np_particles)) * sizeof(double));
        memset(vy, 0, (size_t)((np_particles)) * sizeof(double));
        memset(vz, 0, (size_t)((np_particles)) * sizeof(double));
        memset(xy_new0_re, 0, (size_t)((np_particles)) * sizeof(double));
        memset(xy_mid0_re, 0, (size_t)((np_particles)) * sizeof(double));
        memset(xy_old0_re, 0, (size_t)((np_particles)) * sizeof(double));
        memset(xy_new0_im, 0, (size_t)((np_particles)) * sizeof(double));
        memset(xy_mid0_im, 0, (size_t)((np_particles)) * sizeof(double));
        memset(xy_old0_im, 0, (size_t)((np_particles)) * sizeof(double));
        if (((geom == 2) || (geom == 4))) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            xp_new[__w0] = (xp[__w0] + ((half_dt_step * uxp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            yp_new[__w0] = (yp[__w0] + ((half_dt_step * uyp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            xp_mid[__w0] = (xp_new[__w0] - (((0.5 * dt) * uxp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            yp_mid[__w0] = (yp_new[__w0] - (((0.5 * dt) * uyp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            xp_old[__w0] = (xp_new[__w0] - ((dt * uxp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            yp_old[__w0] = (yp_new[__w0] - ((dt * uyp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rp_new[__w0] = hypot(xp_new[__w0], yp_new[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rp_mid[__w0] = hypot(xp_mid[__w0], yp_mid[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rp_old[__w0] = hypot(xp_old[__w0], yp_old[__w0]);
          }
          double *__cb2 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, rp_mid, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb2[__r0] = ((rp_mid[__r0] > 0.0) ? rp_mid[__r0] : 1.0);
          }
          double *__inl1_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl1_denom_safe[__w0] = __cb2[__w0];
          }
          double *__cb3 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, xp_mid / __inl1_denom_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb3[__r0] = ((rp_mid[__r0] > 0.0) ? (xp_mid[__r0] / __inl1_denom_safe[__r0]) : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            costheta_mid[__w0] = __cb3[__w0];
          }
          double *__cb4 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, rp_mid, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb4[__r0] = ((rp_mid[__r0] > 0.0) ? rp_mid[__r0] : 1.0);
          }
          double *__inl2_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl2_denom_safe[__w0] = __cb4[__w0];
          }
          double *__cb5 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, yp_mid / __inl2_denom_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb5[__r0] = ((rp_mid[__r0] > 0.0) ? (yp_mid[__r0] / __inl2_denom_safe[__r0]) : 0.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            sintheta_mid[__w0] = __cb5[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            x_new[__w0] = ((rp_new[__w0] - xmin) * dinvx);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            x_old[__w0] = ((rp_old[__w0] - xmin) * dinvx);
          }
          if ((geom == 2)) {
            double *__cb6 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_new > 0.0, rp_new, 1.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb6[__r0] = ((rp_new[__r0] > 0.0) ? rp_new[__r0] : 1.0);
            }
            double *__inl3_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl3_denom_safe[__w0] = __cb6[__w0];
            }
            double *__cb7 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_new > 0.0, xp_new / __inl3_denom_safe, 1.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb7[__r0] = ((rp_new[__r0] > 0.0) ? (xp_new[__r0] / __inl3_denom_safe[__r0]) : 1.0);
            }
            double *costheta_new = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              costheta_new[__w0] = __cb7[__w0];
            }
            double *__cb8 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_new > 0.0, rp_new, 1.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb8[__r0] = ((rp_new[__r0] > 0.0) ? rp_new[__r0] : 1.0);
            }
            double *__inl4_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl4_denom_safe[__w0] = __cb8[__w0];
            }
            double *__cb9 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_new > 0.0, yp_new / __inl4_denom_safe, 0.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb9[__r0] = ((rp_new[__r0] > 0.0) ? (yp_new[__r0] / __inl4_denom_safe[__r0]) : 0.0);
            }
            double *sintheta_new = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              sintheta_new[__w0] = __cb9[__w0];
            }
            double *__cb10 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_old > 0.0, rp_old, 1.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb10[__r0] = ((rp_old[__r0] > 0.0) ? rp_old[__r0] : 1.0);
            }
            double *__inl5_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl5_denom_safe[__w0] = __cb10[__w0];
            }
            double *__cb11 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_old > 0.0, xp_old / __inl5_denom_safe, 1.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb11[__r0] = ((rp_old[__r0] > 0.0) ? (xp_old[__r0] / __inl5_denom_safe[__r0]) : 1.0);
            }
            double *costheta_old = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              costheta_old[__w0] = __cb11[__w0];
            }
            double *__cb12 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_old > 0.0, rp_old, 1.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb12[__r0] = ((rp_old[__r0] > 0.0) ? rp_old[__r0] : 1.0);
            }
            double *__inl6_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl6_denom_safe[__w0] = __cb12[__w0];
            }
            double *__cb13 = (double *)malloc(((np_particles)) * sizeof(double));
            /* numpy: np.where(rp_old > 0.0, yp_old / __inl6_denom_safe, 0.0) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              __cb13[__r0] = ((rp_old[__r0] > 0.0) ? (yp_old[__r0] / __inl6_denom_safe[__r0]) : 0.0);
            }
            double *sintheta_old = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              sintheta_old[__w0] = __cb13[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              xy_new0_re[__w0] = costheta_new[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              xy_new0_im[__w0] = sintheta_new[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              xy_mid0_re[__w0] = costheta_mid[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              xy_mid0_im[__w0] = sintheta_mid[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              xy_old0_re[__w0] = costheta_old[__w0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              xy_old0_im[__w0] = sintheta_old[__w0];
            }
            free(__cb6);
            free(__inl3_denom_safe);
            free(__cb7);
            free(costheta_new);
            free(__cb8);
            free(__inl4_denom_safe);
            free(__cb9);
            free(sintheta_new);
            free(__cb10);
            free(__inl5_denom_safe);
            free(__cb11);
            free(costheta_old);
            free(__cb12);
            free(__inl6_denom_safe);
            free(__cb13);
            free(sintheta_old);
          }
          free(__cb2);
          free(__inl1_denom_safe);
          free(__cb3);
          free(__cb4);
          free(__inl2_denom_safe);
          free(__cb5);
        }
        else if ((geom == 5)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            xp_new[__w0] = (xp[__w0] + ((half_dt_step * uxp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            yp_new[__w0] = (yp[__w0] + ((half_dt_step * uyp[__w0]) * gaminv[__w0]));
          }
          double *zp_new = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            zp_new[__w0] = (zp[__w0] + ((half_dt_step * uzp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            xp_mid[__w0] = (xp_new[__w0] - (((0.5 * dt) * uxp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            yp_mid[__w0] = (yp_new[__w0] - (((0.5 * dt) * uyp[__w0]) * gaminv[__w0]));
          }
          double *zp_mid = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            zp_mid[__w0] = (zp_new[__w0] - (((0.5 * dt) * uzp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            xp_old[__w0] = (xp_new[__w0] - ((dt * uxp[__w0]) * gaminv[__w0]));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            yp_old[__w0] = (yp_new[__w0] - ((dt * uyp[__w0]) * gaminv[__w0]));
          }
          double *zp_old = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            zp_old[__w0] = (zp_new[__w0] - ((dt * uzp[__w0]) * gaminv[__w0]));
          }
          double *rpxy_mid = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rpxy_mid[__w0] = hypot(xp_mid[__w0], yp_mid[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rp_new[__w0] = sqrt((((xp_new[__w0] * xp_new[__w0]) + (yp_new[__w0] * yp_new[__w0])) + (zp_new[__w0] * zp_new[__w0])));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rp_old[__w0] = sqrt((((xp_old[__w0] * xp_old[__w0]) + (yp_old[__w0] * yp_old[__w0])) + (zp_old[__w0] * zp_old[__w0])));
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            rp_mid[__w0] = ((rp_new[__w0] + rp_old[__w0]) * 0.5);
          }
          double *__cb14 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rpxy_mid > 0.0, rpxy_mid, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb14[__r0] = ((rpxy_mid[__r0] > 0.0) ? rpxy_mid[__r0] : 1.0);
          }
          double *__inl7_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl7_denom_safe[__w0] = __cb14[__w0];
          }
          double *__cb15 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rpxy_mid > 0.0, xp_mid / __inl7_denom_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb15[__r0] = ((rpxy_mid[__r0] > 0.0) ? (xp_mid[__r0] / __inl7_denom_safe[__r0]) : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            costheta_mid[__w0] = __cb15[__w0];
          }
          double *__cb16 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rpxy_mid > 0.0, rpxy_mid, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb16[__r0] = ((rpxy_mid[__r0] > 0.0) ? rpxy_mid[__r0] : 1.0);
          }
          double *__inl8_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl8_denom_safe[__w0] = __cb16[__w0];
          }
          double *__cb17 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rpxy_mid > 0.0, yp_mid / __inl8_denom_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb17[__r0] = ((rpxy_mid[__r0] > 0.0) ? (yp_mid[__r0] / __inl8_denom_safe[__r0]) : 0.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            sintheta_mid[__w0] = __cb17[__w0];
          }
          double *__cb18 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, rp_mid, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb18[__r0] = ((rp_mid[__r0] > 0.0) ? rp_mid[__r0] : 1.0);
          }
          double *__inl9_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl9_denom_safe[__w0] = __cb18[__w0];
          }
          double *__cb19 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, rpxy_mid / __inl9_denom_safe, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb19[__r0] = ((rp_mid[__r0] > 0.0) ? (rpxy_mid[__r0] / __inl9_denom_safe[__r0]) : 1.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            cosphi_mid[__w0] = __cb19[__w0];
          }
          double *__cb20 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, rp_mid, 1.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb20[__r0] = ((rp_mid[__r0] > 0.0) ? rp_mid[__r0] : 1.0);
          }
          double *__inl10_denom_safe = (double *)malloc(((np_particles)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl10_denom_safe[__w0] = __cb20[__w0];
          }
          double *__cb21 = (double *)malloc(((np_particles)) * sizeof(double));
          /* numpy: np.where(rp_mid > 0.0, zp_mid / __inl10_denom_safe, 0.0) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb21[__r0] = ((rp_mid[__r0] > 0.0) ? (zp_mid[__r0] / __inl10_denom_safe[__r0]) : 0.0);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            sinphi_mid[__w0] = __cb21[__w0];
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            x_new[__w0] = ((rp_new[__w0] - xmin) * dinvx);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            x_old[__w0] = ((rp_old[__w0] - xmin) * dinvx);
          }
          free(zp_new);
          free(zp_mid);
          free(zp_old);
          free(rpxy_mid);
          free(__cb14);
          free(__inl7_denom_safe);
          free(__cb15);
          free(__cb16);
          free(__inl8_denom_safe);
          free(__cb17);
          free(__cb18);
          free(__inl9_denom_safe);
          free(__cb19);
          free(__cb20);
          free(__inl10_denom_safe);
          free(__cb21);
        }
        else if ((geom != 0)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            x_new[__w0] = (((xp[__w0] - xmin) + ((half_dt_step * uxp[__w0]) * gaminv[__w0])) * dinvx);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            x_old[__w0] = (x_new[__w0] - (((dt * dinvx) * uxp[__w0]) * gaminv[__w0]));
          }
        }
        if ((geom == 3)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            y_new[__w0] = (((yp[__w0] - ymin) + ((half_dt_step * uyp[__w0]) * gaminv[__w0])) * dinvy);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            y_old[__w0] = (y_new[__w0] - (((dt * dinvy) * uyp[__w0]) * gaminv[__w0]));
          }
        }
        if (((geom != 4) && (geom != 5))) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            z_new[__w0] = (((zp[__w0] - zmin) + ((half_dt_step * uzp[__w0]) * gaminv[__w0])) * dinvz);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            z_old[__w0] = (z_new[__w0] - (((dt * dinvz) * uzp[__w0]) * gaminv[__w0]));
          }
        }
        memset(reduce_shape_old, 0, (size_t)((np_particles)) * sizeof(double));
        memset(reduce_shape_new, 0, (size_t)((np_particles)) * sizeof(double));
        if (reduce_enabled) {
          if ((geom == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fx_o[__w0] = ((int64_t)(floor(x_old[__w0])));
            }
            int64_t *fy_o = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fy_o[__w0] = ((int64_t)(floor(y_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fz_o[__w0] = ((int64_t)(floor(z_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fx_n[__w0] = ((int64_t)(floor(x_new[__w0])));
            }
            int64_t *fy_n = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fy_n[__w0] = ((int64_t)(floor(y_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fz_n[__w0] = ((int64_t)(floor(z_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_old[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fx_o[__w0]))*(((ncells + (2 * depos_order)) + 6)) + ((loy + fy_o[__w0])))*(((ncells + (2 * depos_order)) + 6)) + ((loz + fz_o[__w0]))]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_new[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fx_n[__w0]))*(((ncells + (2 * depos_order)) + 6)) + ((loy + fy_n[__w0])))*(((ncells + (2 * depos_order)) + 6)) + ((loz + fz_n[__w0]))]));
            }
            free(fy_o);
            free(fy_n);
          }
          else if (((geom == 1) || (geom == 2))) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fx_o[__w0] = ((int64_t)(floor(x_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fz_o[__w0] = ((int64_t)(floor(z_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fx_n[__w0] = ((int64_t)(floor(x_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fz_n[__w0] = ((int64_t)(floor(z_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_old[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fx_o[__w0]))*(((ncells + (2 * depos_order)) + 6)) + ((loy + fz_o[__w0])))*(((ncells + (2 * depos_order)) + 6)) + (0)]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_new[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fx_n[__w0]))*(((ncells + (2 * depos_order)) + 6)) + ((loy + fz_n[__w0])))*(((ncells + (2 * depos_order)) + 6)) + (0)]));
            }
          }
          else if (((geom == 4) || (geom == 5))) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fx_o[__w0] = ((int64_t)(floor(x_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fx_n[__w0] = ((int64_t)(floor(x_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_old[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fx_o[__w0]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0)]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_new[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fx_n[__w0]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0)]));
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fz_o[__w0] = ((int64_t)(floor(z_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              fz_n[__w0] = ((int64_t)(floor(z_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_old[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fz_o[__w0]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0)]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              reduce_shape_new[__w0] = ((int64_t)(reduced_particle_shape_mask[(((lox + fz_n[__w0]))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0)]));
            }
          }
        }
        if ((geom == 2)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vy[__w0] = ((((-uxp[__w0]) * sintheta_mid[__w0]) + (uyp[__w0] * costheta_mid[__w0])) * gaminv[__w0]);
          }
        }
        else if ((geom == 1)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vy[__w0] = (uyp[__w0] * gaminv[__w0]);
          }
        }
        else if ((geom == 0)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vx[__w0] = (uxp[__w0] * gaminv[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vy[__w0] = (uyp[__w0] * gaminv[__w0]);
          }
        }
        else if ((geom == 4)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vy[__w0] = ((((-uxp[__w0]) * sintheta_mid[__w0]) + (uyp[__w0] * costheta_mid[__w0])) * gaminv[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vz[__w0] = (uzp[__w0] * gaminv[__w0]);
          }
        }
        else if ((geom == 5)) {
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vy[__w0] = ((((-uxp[__w0]) * sintheta_mid[__w0]) + (uyp[__w0] * costheta_mid[__w0])) * gaminv[__w0]);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            vz[__w0] = ((((((-uxp[__w0]) * costheta_mid[__w0]) * sinphi_mid[__w0]) - ((uyp[__w0] * sintheta_mid[__w0]) * sinphi_mid[__w0])) + (uzp[__w0] * cosphi_mid[__w0])) * gaminv[__w0]);
          }
        }
        half = int_floor(o, 2);
        width = (o + 3);
        memset(i_new, 0, (size_t)((np_particles)) * sizeof(int64_t));
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          i_old[__w0] = i_new[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          j_new[__w0] = i_new[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          j_old[__w0] = i_new[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          k_new[__w0] = i_new[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          k_old[__w0] = i_new[__w0];
        }
        if ((geom != 0)) {
          free(__inl11_sx);
          __inl11_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          memset(__inl11_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
          if ((o == 0)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_j[__w0] = ((int64_t)(trunc((x_new[__w0] + 0.5))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (1)] = 1.0;
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_idx[__w0] = __inl11_j[__w0];
            }
          }
          else if ((o == 1)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_j[__w0] = ((int64_t)(trunc(x_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_xint[__w0] = (x_new[__w0] - __inl11_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (1)] = (1.0 - __inl11_xint[si0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (2)] = __inl11_xint[si0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_idx[__w0] = __inl11_j[__w0];
            }
          }
          else if ((o == 2)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_j[__w0] = ((int64_t)(trunc((x_new[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_xint[__w0] = (x_new[__w0] - __inl11_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (1)] = ((0.5 * (0.5 - __inl11_xint[si0])) * (0.5 - __inl11_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (2)] = (0.75 - (__inl11_xint[si0] * __inl11_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (3)] = ((0.5 * (0.5 + __inl11_xint[si0])) * (0.5 + __inl11_xint[si0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_idx[__w0] = (__inl11_j[__w0] - 1);
            }
          }
          else if ((o == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_j[__w0] = ((int64_t)(trunc(x_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_xint[__w0] = (x_new[__w0] - __inl11_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (1)] = (((0.16666666666666666 * (1.0 - __inl11_xint[si0])) * (1.0 - __inl11_xint[si0])) * (1.0 - __inl11_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (2)] = ((2.0 / 3.0) - ((__inl11_xint[si0] * __inl11_xint[si0]) * (1.0 - (__inl11_xint[si0] / 2.0))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (3)] = ((2.0 / 3.0) - (((1.0 - __inl11_xint[si0]) * (1.0 - __inl11_xint[si0])) * (1.0 - (0.5 * (1.0 - __inl11_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (4)] = (((0.16666666666666666 * __inl11_xint[si0]) * __inl11_xint[si0]) * __inl11_xint[si0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_idx[__w0] = (__inl11_j[__w0] - 1);
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_j[__w0] = ((int64_t)(trunc((x_new[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_xint[__w0] = (x_new[__w0] - __inl11_j[__w0]);
            }
            double *__inl11_sm = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_sm[__w0] = (0.5 - __inl11_xint[__w0]);
            }
            double *__inl11_sp = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_sp[__w0] = (0.5 + __inl11_xint[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (1)] = (((((1.0 / 24.0) * __inl11_sm[si0]) * __inl11_sm[si0]) * __inl11_sm[si0]) * __inl11_sm[si0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (2)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl11_xint[si0])) + (((4.0 * __inl11_xint[si0]) * __inl11_xint[si0]) * ((1.5 + __inl11_xint[si0]) - (__inl11_xint[si0] * __inl11_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (3)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl11_xint[si0]) * __inl11_xint[si0]) * ((__inl11_xint[si0] * __inl11_xint[si0]) - 2.5))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (4)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl11_xint[si0])) + (((4.0 * __inl11_xint[si0]) * __inl11_xint[si0]) * ((1.5 - __inl11_xint[si0]) - (__inl11_xint[si0] * __inl11_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl11_sx[(si0)*(width) + (5)] = (((((1.0 / 24.0) * __inl11_sp[si0]) * __inl11_sp[si0]) * __inl11_sp[si0]) * __inl11_sp[si0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl11_idx[__w0] = (__inl11_j[__w0] - 2);
            }
            free(__inl11_sm);
            free(__inl11_sp);
          }
          free(sx_new);
          sx_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            for (int64_t __w1 = 0; __w1 < width; ++__w1) {
              sx_new[(__w0)*(width) + (__w1)] = __inl11_sx[(__w0)*(width) + (__w1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            i_new[__w0] = __inl11_idx[__w0];
          }
          free(__inl12_sx);
          __inl12_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          memset(__inl12_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
          int64_t *__cb22 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.arange(np_particles) */
          for (int64_t __i = 0; __i < np_particles; ++__i) {
            __cb22[__i] = __i;
          }
          int64_t *__inl12_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl12_rows[__w0] = __cb22[__w0];
          }
          if ((o == 0)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i[__w0] = ((int64_t)(floor((x_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i_shift[__w0] = (__inl12_i[__w0] - i_new[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((1 + __inl12_i_shift[__sc0]))] = 1.0;
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_idx[__w0] = __inl12_i[__w0];
            }
          }
          else if ((o == 1)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i[__w0] = ((int64_t)(floor(x_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i_shift[__w0] = (__inl12_i[__w0] - i_new[__w0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_xint[__w0] = (x_old[__w0] - __inl12_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((1 + __inl12_i_shift[__sc0]))] = (1.0 - __inl12_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((2 + __inl12_i_shift[__sc0]))] = __inl12_xint[__sc0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_idx[__w0] = __inl12_i[__w0];
            }
          }
          else if ((o == 2)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i[__w0] = ((int64_t)(trunc((x_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i_shift[__w0] = (__inl12_i[__w0] - (i_new[__w0] + 1));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_xint[__w0] = (x_old[__w0] - __inl12_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((1 + __inl12_i_shift[__sc0]))] = ((0.5 * (0.5 - __inl12_xint[__sc0])) * (0.5 - __inl12_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((2 + __inl12_i_shift[__sc0]))] = (0.75 - (__inl12_xint[__sc0] * __inl12_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((3 + __inl12_i_shift[__sc0]))] = ((0.5 * (0.5 + __inl12_xint[__sc0])) * (0.5 + __inl12_xint[__sc0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_idx[__w0] = (__inl12_i[__w0] - 1);
            }
          }
          else if ((o == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i[__w0] = ((int64_t)(trunc(x_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i_shift[__w0] = (__inl12_i[__w0] - (i_new[__w0] + 1));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_xint[__w0] = (x_old[__w0] - __inl12_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((1 + __inl12_i_shift[__sc0]))] = (((0.16666666666666666 * (1.0 - __inl12_xint[__sc0])) * (1.0 - __inl12_xint[__sc0])) * (1.0 - __inl12_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((2 + __inl12_i_shift[__sc0]))] = ((2.0 / 3.0) - ((__inl12_xint[__sc0] * __inl12_xint[__sc0]) * (1.0 - (__inl12_xint[__sc0] / 2.0))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((3 + __inl12_i_shift[__sc0]))] = ((2.0 / 3.0) - (((1.0 - __inl12_xint[__sc0]) * (1.0 - __inl12_xint[__sc0])) * (1.0 - (0.5 * (1.0 - __inl12_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((4 + __inl12_i_shift[__sc0]))] = (((0.16666666666666666 * __inl12_xint[__sc0]) * __inl12_xint[__sc0]) * __inl12_xint[__sc0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_idx[__w0] = (__inl12_i[__w0] - 1);
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i[__w0] = ((int64_t)(trunc((x_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_i_shift[__w0] = (__inl12_i[__w0] - (i_new[__w0] + 2));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_xint[__w0] = (x_old[__w0] - __inl12_i[__w0]);
            }
            double *__inl12_sm = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_sm[__w0] = (0.5 - __inl12_xint[__w0]);
            }
            double *__inl12_sp = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_sp[__w0] = (0.5 + __inl12_xint[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((1 + __inl12_i_shift[__sc0]))] = (((((1.0 / 24.0) * __inl12_sm[__sc0]) * __inl12_sm[__sc0]) * __inl12_sm[__sc0]) * __inl12_sm[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((2 + __inl12_i_shift[__sc0]))] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl12_xint[__sc0])) + (((4.0 * __inl12_xint[__sc0]) * __inl12_xint[__sc0]) * ((1.5 + __inl12_xint[__sc0]) - (__inl12_xint[__sc0] * __inl12_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((3 + __inl12_i_shift[__sc0]))] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl12_xint[__sc0]) * __inl12_xint[__sc0]) * ((__inl12_xint[__sc0] * __inl12_xint[__sc0]) - 2.5))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((4 + __inl12_i_shift[__sc0]))] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl12_xint[__sc0])) + (((4.0 * __inl12_xint[__sc0]) * __inl12_xint[__sc0]) * ((1.5 - __inl12_xint[__sc0]) - (__inl12_xint[__sc0] * __inl12_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl12_sx[(__inl12_rows[__sc0])*(width) + ((5 + __inl12_i_shift[__sc0]))] = (((((1.0 / 24.0) * __inl12_sp[__sc0]) * __inl12_sp[__sc0]) * __inl12_sp[__sc0]) * __inl12_sp[__sc0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl12_idx[__w0] = (__inl12_i[__w0] - 2);
            }
            free(__inl12_sm);
            free(__inl12_sp);
          }
          free(sx_old);
          sx_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            for (int64_t __w1 = 0; __w1 < width; ++__w1) {
              sx_old[(__w0)*(width) + (__w1)] = __inl12_sx[(__w0)*(width) + (__w1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            i_old[__w0] = __inl12_idx[__w0];
          }
          if (reduce_enabled) {
            free(__inl13_sx);
            __inl13_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            memset(__inl13_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
            int64_t *__cb23 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            /* numpy: np.arange(np_particles) */
            for (int64_t __i = 0; __i < np_particles; ++__i) {
              __cb23[__i] = __i;
            }
            int64_t *__inl13_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl13_rows[__w0] = __cb23[__w0];
            }
            int64_t *__inl13_i = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl13_i[__w0] = ((int64_t)(floor(x_new[__w0])));
            }
            int64_t *__inl13_i_shift = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl13_i_shift[__w0] = (__inl13_i[__w0] - (i_new[__w0] + half));
            }
            double *__inl13_xint = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl13_xint[__w0] = (x_new[__w0] - __inl13_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl13_sx[(__inl13_rows[__sc0])*(width) + (((half + 1) + __inl13_i_shift[__sc0]))] = (1.0 - __inl13_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl13_sx[(__inl13_rows[__sc0])*(width) + (((half + 2) + __inl13_i_shift[__sc0]))] = __inl13_xint[__sc0];
            }
            int64_t *__inl13_idx = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl13_idx[__w0] = __inl13_i[__w0];
            }
            free(ov_new);
            ov_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                ov_new[(__w0)*(width) + (__w1)] = __inl13_sx[(__w0)*(width) + (__w1)];
              }
            }
            free(__inl14_sx);
            __inl14_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            memset(__inl14_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
            int64_t *__cb24 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            /* numpy: np.arange(np_particles) */
            for (int64_t __i = 0; __i < np_particles; ++__i) {
              __cb24[__i] = __i;
            }
            int64_t *__inl14_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl14_rows[__w0] = __cb24[__w0];
            }
            int64_t *__inl14_i = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl14_i[__w0] = ((int64_t)(floor(x_old[__w0])));
            }
            int64_t *__inl14_i_shift = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl14_i_shift[__w0] = (__inl14_i[__w0] - (i_new[__w0] + half));
            }
            double *__inl14_xint = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl14_xint[__w0] = (x_old[__w0] - __inl14_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl14_sx[(__inl14_rows[__sc0])*(width) + (((half + 1) + __inl14_i_shift[__sc0]))] = (1.0 - __inl14_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl14_sx[(__inl14_rows[__sc0])*(width) + (((half + 2) + __inl14_i_shift[__sc0]))] = __inl14_xint[__sc0];
            }
            int64_t *__inl14_idx = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl14_idx[__w0] = __inl14_i[__w0];
            }
            free(ov_old);
            ov_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                ov_old[(__w0)*(width) + (__w1)] = __inl14_sx[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb25);
            __cb25 = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            /* numpy: np.where((reduce_shape_new != 0)[:, None], ov_new, sx_new) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              for (int64_t __r1 = 0; __r1 < width; ++__r1) {
                __cb25[(__r0)*(width) + (__r1)] = ((reduce_shape_new[__r0] != 0) ? ov_new[(__r0)*(width) + (__r1)] : sx_new[(__r0)*(width) + (__r1)]);
              }
            }
            free(sx_new);
            sx_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                sx_new[(__w0)*(width) + (__w1)] = __cb25[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb26);
            __cb26 = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            /* numpy: np.where((reduce_shape_old != 0)[:, None], ov_old, sx_old) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              for (int64_t __r1 = 0; __r1 < width; ++__r1) {
                __cb26[(__r0)*(width) + (__r1)] = ((reduce_shape_old[__r0] != 0) ? ov_old[(__r0)*(width) + (__r1)] : sx_old[(__r0)*(width) + (__r1)]);
              }
            }
            free(sx_old);
            sx_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                sx_old[(__w0)*(width) + (__w1)] = __cb26[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb23);
            free(__inl13_rows);
            free(__inl13_i);
            free(__inl13_i_shift);
            free(__inl13_xint);
            free(__inl13_idx);
            free(__cb24);
            free(__inl14_rows);
            free(__inl14_i);
            free(__inl14_i_shift);
            free(__inl14_xint);
            free(__inl14_idx);
          }
          free(__cb22);
          free(__inl12_rows);
        }
        if ((geom == 3)) {
          free(__inl15_sx);
          __inl15_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          memset(__inl15_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
          if ((o == 0)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_j[__w0] = ((int64_t)(trunc((y_new[__w0] + 0.5))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (1)] = 1.0;
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_idx[__w0] = __inl15_j[__w0];
            }
          }
          else if ((o == 1)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_j[__w0] = ((int64_t)(trunc(y_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_xint[__w0] = (y_new[__w0] - __inl15_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (1)] = (1.0 - __inl15_xint[si0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (2)] = __inl15_xint[si0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_idx[__w0] = __inl15_j[__w0];
            }
          }
          else if ((o == 2)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_j[__w0] = ((int64_t)(trunc((y_new[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_xint[__w0] = (y_new[__w0] - __inl15_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (1)] = ((0.5 * (0.5 - __inl15_xint[si0])) * (0.5 - __inl15_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (2)] = (0.75 - (__inl15_xint[si0] * __inl15_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (3)] = ((0.5 * (0.5 + __inl15_xint[si0])) * (0.5 + __inl15_xint[si0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_idx[__w0] = (__inl15_j[__w0] - 1);
            }
          }
          else if ((o == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_j[__w0] = ((int64_t)(trunc(y_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_xint[__w0] = (y_new[__w0] - __inl15_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (1)] = (((0.16666666666666666 * (1.0 - __inl15_xint[si0])) * (1.0 - __inl15_xint[si0])) * (1.0 - __inl15_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (2)] = ((2.0 / 3.0) - ((__inl15_xint[si0] * __inl15_xint[si0]) * (1.0 - (__inl15_xint[si0] / 2.0))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (3)] = ((2.0 / 3.0) - (((1.0 - __inl15_xint[si0]) * (1.0 - __inl15_xint[si0])) * (1.0 - (0.5 * (1.0 - __inl15_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (4)] = (((0.16666666666666666 * __inl15_xint[si0]) * __inl15_xint[si0]) * __inl15_xint[si0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_idx[__w0] = (__inl15_j[__w0] - 1);
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_j[__w0] = ((int64_t)(trunc((y_new[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_xint[__w0] = (y_new[__w0] - __inl15_j[__w0]);
            }
            double *__inl15_sm = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_sm[__w0] = (0.5 - __inl15_xint[__w0]);
            }
            double *__inl15_sp = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_sp[__w0] = (0.5 + __inl15_xint[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (1)] = (((((1.0 / 24.0) * __inl15_sm[si0]) * __inl15_sm[si0]) * __inl15_sm[si0]) * __inl15_sm[si0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (2)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl15_xint[si0])) + (((4.0 * __inl15_xint[si0]) * __inl15_xint[si0]) * ((1.5 + __inl15_xint[si0]) - (__inl15_xint[si0] * __inl15_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (3)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl15_xint[si0]) * __inl15_xint[si0]) * ((__inl15_xint[si0] * __inl15_xint[si0]) - 2.5))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (4)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl15_xint[si0])) + (((4.0 * __inl15_xint[si0]) * __inl15_xint[si0]) * ((1.5 - __inl15_xint[si0]) - (__inl15_xint[si0] * __inl15_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl15_sx[(si0)*(width) + (5)] = (((((1.0 / 24.0) * __inl15_sp[si0]) * __inl15_sp[si0]) * __inl15_sp[si0]) * __inl15_sp[si0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl15_idx[__w0] = (__inl15_j[__w0] - 2);
            }
            free(__inl15_sm);
            free(__inl15_sp);
          }
          free(sy_new);
          sy_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            for (int64_t __w1 = 0; __w1 < width; ++__w1) {
              sy_new[(__w0)*(width) + (__w1)] = __inl15_sx[(__w0)*(width) + (__w1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            j_new[__w0] = __inl15_idx[__w0];
          }
          free(__inl16_sx);
          __inl16_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          memset(__inl16_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
          int64_t *__cb27 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.arange(np_particles) */
          for (int64_t __i = 0; __i < np_particles; ++__i) {
            __cb27[__i] = __i;
          }
          int64_t *__inl16_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl16_rows[__w0] = __cb27[__w0];
          }
          if ((o == 0)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i[__w0] = ((int64_t)(floor((y_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i_shift[__w0] = (__inl16_i[__w0] - j_new[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((1 + __inl16_i_shift[__sc0]))] = 1.0;
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_idx[__w0] = __inl16_i[__w0];
            }
          }
          else if ((o == 1)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i[__w0] = ((int64_t)(floor(y_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i_shift[__w0] = (__inl16_i[__w0] - j_new[__w0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_xint[__w0] = (y_old[__w0] - __inl16_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((1 + __inl16_i_shift[__sc0]))] = (1.0 - __inl16_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((2 + __inl16_i_shift[__sc0]))] = __inl16_xint[__sc0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_idx[__w0] = __inl16_i[__w0];
            }
          }
          else if ((o == 2)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i[__w0] = ((int64_t)(trunc((y_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i_shift[__w0] = (__inl16_i[__w0] - (j_new[__w0] + 1));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_xint[__w0] = (y_old[__w0] - __inl16_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((1 + __inl16_i_shift[__sc0]))] = ((0.5 * (0.5 - __inl16_xint[__sc0])) * (0.5 - __inl16_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((2 + __inl16_i_shift[__sc0]))] = (0.75 - (__inl16_xint[__sc0] * __inl16_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((3 + __inl16_i_shift[__sc0]))] = ((0.5 * (0.5 + __inl16_xint[__sc0])) * (0.5 + __inl16_xint[__sc0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_idx[__w0] = (__inl16_i[__w0] - 1);
            }
          }
          else if ((o == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i[__w0] = ((int64_t)(trunc(y_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i_shift[__w0] = (__inl16_i[__w0] - (j_new[__w0] + 1));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_xint[__w0] = (y_old[__w0] - __inl16_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((1 + __inl16_i_shift[__sc0]))] = (((0.16666666666666666 * (1.0 - __inl16_xint[__sc0])) * (1.0 - __inl16_xint[__sc0])) * (1.0 - __inl16_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((2 + __inl16_i_shift[__sc0]))] = ((2.0 / 3.0) - ((__inl16_xint[__sc0] * __inl16_xint[__sc0]) * (1.0 - (__inl16_xint[__sc0] / 2.0))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((3 + __inl16_i_shift[__sc0]))] = ((2.0 / 3.0) - (((1.0 - __inl16_xint[__sc0]) * (1.0 - __inl16_xint[__sc0])) * (1.0 - (0.5 * (1.0 - __inl16_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((4 + __inl16_i_shift[__sc0]))] = (((0.16666666666666666 * __inl16_xint[__sc0]) * __inl16_xint[__sc0]) * __inl16_xint[__sc0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_idx[__w0] = (__inl16_i[__w0] - 1);
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i[__w0] = ((int64_t)(trunc((y_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_i_shift[__w0] = (__inl16_i[__w0] - (j_new[__w0] + 2));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_xint[__w0] = (y_old[__w0] - __inl16_i[__w0]);
            }
            double *__inl16_sm = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_sm[__w0] = (0.5 - __inl16_xint[__w0]);
            }
            double *__inl16_sp = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_sp[__w0] = (0.5 + __inl16_xint[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((1 + __inl16_i_shift[__sc0]))] = (((((1.0 / 24.0) * __inl16_sm[__sc0]) * __inl16_sm[__sc0]) * __inl16_sm[__sc0]) * __inl16_sm[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((2 + __inl16_i_shift[__sc0]))] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl16_xint[__sc0])) + (((4.0 * __inl16_xint[__sc0]) * __inl16_xint[__sc0]) * ((1.5 + __inl16_xint[__sc0]) - (__inl16_xint[__sc0] * __inl16_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((3 + __inl16_i_shift[__sc0]))] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl16_xint[__sc0]) * __inl16_xint[__sc0]) * ((__inl16_xint[__sc0] * __inl16_xint[__sc0]) - 2.5))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((4 + __inl16_i_shift[__sc0]))] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl16_xint[__sc0])) + (((4.0 * __inl16_xint[__sc0]) * __inl16_xint[__sc0]) * ((1.5 - __inl16_xint[__sc0]) - (__inl16_xint[__sc0] * __inl16_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl16_sx[(__inl16_rows[__sc0])*(width) + ((5 + __inl16_i_shift[__sc0]))] = (((((1.0 / 24.0) * __inl16_sp[__sc0]) * __inl16_sp[__sc0]) * __inl16_sp[__sc0]) * __inl16_sp[__sc0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl16_idx[__w0] = (__inl16_i[__w0] - 2);
            }
            free(__inl16_sm);
            free(__inl16_sp);
          }
          free(sy_old);
          sy_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            for (int64_t __w1 = 0; __w1 < width; ++__w1) {
              sy_old[(__w0)*(width) + (__w1)] = __inl16_sx[(__w0)*(width) + (__w1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            j_old[__w0] = __inl16_idx[__w0];
          }
          if (reduce_enabled) {
            free(__inl17_sx);
            __inl17_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            memset(__inl17_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
            int64_t *__cb28 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            /* numpy: np.arange(np_particles) */
            for (int64_t __i = 0; __i < np_particles; ++__i) {
              __cb28[__i] = __i;
            }
            int64_t *__inl17_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl17_rows[__w0] = __cb28[__w0];
            }
            int64_t *__inl17_i = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl17_i[__w0] = ((int64_t)(floor(y_new[__w0])));
            }
            int64_t *__inl17_i_shift = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl17_i_shift[__w0] = (__inl17_i[__w0] - (j_new[__w0] + half));
            }
            double *__inl17_xint = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl17_xint[__w0] = (y_new[__w0] - __inl17_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl17_sx[(__inl17_rows[__sc0])*(width) + (((half + 1) + __inl17_i_shift[__sc0]))] = (1.0 - __inl17_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl17_sx[(__inl17_rows[__sc0])*(width) + (((half + 2) + __inl17_i_shift[__sc0]))] = __inl17_xint[__sc0];
            }
            int64_t *__inl17_idx = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl17_idx[__w0] = __inl17_i[__w0];
            }
            free(ov_new);
            ov_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                ov_new[(__w0)*(width) + (__w1)] = __inl17_sx[(__w0)*(width) + (__w1)];
              }
            }
            free(__inl18_sx);
            __inl18_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            memset(__inl18_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
            int64_t *__cb29 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            /* numpy: np.arange(np_particles) */
            for (int64_t __i = 0; __i < np_particles; ++__i) {
              __cb29[__i] = __i;
            }
            int64_t *__inl18_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl18_rows[__w0] = __cb29[__w0];
            }
            int64_t *__inl18_i = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl18_i[__w0] = ((int64_t)(floor(y_old[__w0])));
            }
            int64_t *__inl18_i_shift = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl18_i_shift[__w0] = (__inl18_i[__w0] - (j_new[__w0] + half));
            }
            double *__inl18_xint = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl18_xint[__w0] = (y_old[__w0] - __inl18_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl18_sx[(__inl18_rows[__sc0])*(width) + (((half + 1) + __inl18_i_shift[__sc0]))] = (1.0 - __inl18_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl18_sx[(__inl18_rows[__sc0])*(width) + (((half + 2) + __inl18_i_shift[__sc0]))] = __inl18_xint[__sc0];
            }
            int64_t *__inl18_idx = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl18_idx[__w0] = __inl18_i[__w0];
            }
            free(ov_old);
            ov_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                ov_old[(__w0)*(width) + (__w1)] = __inl18_sx[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb30);
            __cb30 = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            /* numpy: np.where((reduce_shape_new != 0)[:, None], ov_new, sy_new) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              for (int64_t __r1 = 0; __r1 < width; ++__r1) {
                __cb30[(__r0)*(width) + (__r1)] = ((reduce_shape_new[__r0] != 0) ? ov_new[(__r0)*(width) + (__r1)] : sy_new[(__r0)*(width) + (__r1)]);
              }
            }
            free(sy_new);
            sy_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                sy_new[(__w0)*(width) + (__w1)] = __cb30[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb31);
            __cb31 = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            /* numpy: np.where((reduce_shape_old != 0)[:, None], ov_old, sy_old) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              for (int64_t __r1 = 0; __r1 < width; ++__r1) {
                __cb31[(__r0)*(width) + (__r1)] = ((reduce_shape_old[__r0] != 0) ? ov_old[(__r0)*(width) + (__r1)] : sy_old[(__r0)*(width) + (__r1)]);
              }
            }
            free(sy_old);
            sy_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                sy_old[(__w0)*(width) + (__w1)] = __cb31[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb28);
            free(__inl17_rows);
            free(__inl17_i);
            free(__inl17_i_shift);
            free(__inl17_xint);
            free(__inl17_idx);
            free(__cb29);
            free(__inl18_rows);
            free(__inl18_i);
            free(__inl18_i_shift);
            free(__inl18_xint);
            free(__inl18_idx);
          }
          free(__cb27);
          free(__inl16_rows);
        }
        if (((geom != 4) && (geom != 5))) {
          free(__inl19_sx);
          __inl19_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          memset(__inl19_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
          if ((o == 0)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_j[__w0] = ((int64_t)(trunc((z_new[__w0] + 0.5))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (1)] = 1.0;
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_idx[__w0] = __inl19_j[__w0];
            }
          }
          else if ((o == 1)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_j[__w0] = ((int64_t)(trunc(z_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_xint[__w0] = (z_new[__w0] - __inl19_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (1)] = (1.0 - __inl19_xint[si0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (2)] = __inl19_xint[si0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_idx[__w0] = __inl19_j[__w0];
            }
          }
          else if ((o == 2)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_j[__w0] = ((int64_t)(trunc((z_new[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_xint[__w0] = (z_new[__w0] - __inl19_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (1)] = ((0.5 * (0.5 - __inl19_xint[si0])) * (0.5 - __inl19_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (2)] = (0.75 - (__inl19_xint[si0] * __inl19_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (3)] = ((0.5 * (0.5 + __inl19_xint[si0])) * (0.5 + __inl19_xint[si0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_idx[__w0] = (__inl19_j[__w0] - 1);
            }
          }
          else if ((o == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_j[__w0] = ((int64_t)(trunc(z_new[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_xint[__w0] = (z_new[__w0] - __inl19_j[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (1)] = (((0.16666666666666666 * (1.0 - __inl19_xint[si0])) * (1.0 - __inl19_xint[si0])) * (1.0 - __inl19_xint[si0]));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (2)] = ((2.0 / 3.0) - ((__inl19_xint[si0] * __inl19_xint[si0]) * (1.0 - (__inl19_xint[si0] / 2.0))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (3)] = ((2.0 / 3.0) - (((1.0 - __inl19_xint[si0]) * (1.0 - __inl19_xint[si0])) * (1.0 - (0.5 * (1.0 - __inl19_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (4)] = (((0.16666666666666666 * __inl19_xint[si0]) * __inl19_xint[si0]) * __inl19_xint[si0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_idx[__w0] = (__inl19_j[__w0] - 1);
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_j[__w0] = ((int64_t)(trunc((z_new[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_xint[__w0] = (z_new[__w0] - __inl19_j[__w0]);
            }
            double *__inl19_sm = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_sm[__w0] = (0.5 - __inl19_xint[__w0]);
            }
            double *__inl19_sp = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_sp[__w0] = (0.5 + __inl19_xint[__w0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (1)] = (((((1.0 / 24.0) * __inl19_sm[si0]) * __inl19_sm[si0]) * __inl19_sm[si0]) * __inl19_sm[si0]);
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (2)] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl19_xint[si0])) + (((4.0 * __inl19_xint[si0]) * __inl19_xint[si0]) * ((1.5 + __inl19_xint[si0]) - (__inl19_xint[si0] * __inl19_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (3)] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl19_xint[si0]) * __inl19_xint[si0]) * ((__inl19_xint[si0] * __inl19_xint[si0]) - 2.5))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (4)] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl19_xint[si0])) + (((4.0 * __inl19_xint[si0]) * __inl19_xint[si0]) * ((1.5 - __inl19_xint[si0]) - (__inl19_xint[si0] * __inl19_xint[si0])))));
            }
            for (int64_t si0 = 0; si0 < np_particles; ++si0) {
              __inl19_sx[(si0)*(width) + (5)] = (((((1.0 / 24.0) * __inl19_sp[si0]) * __inl19_sp[si0]) * __inl19_sp[si0]) * __inl19_sp[si0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl19_idx[__w0] = (__inl19_j[__w0] - 2);
            }
            free(__inl19_sm);
            free(__inl19_sp);
          }
          free(sz_new);
          sz_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            for (int64_t __w1 = 0; __w1 < width; ++__w1) {
              sz_new[(__w0)*(width) + (__w1)] = __inl19_sx[(__w0)*(width) + (__w1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            k_new[__w0] = __inl19_idx[__w0];
          }
          free(__inl20_sx);
          __inl20_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          memset(__inl20_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
          int64_t *__cb32 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.arange(np_particles) */
          for (int64_t __i = 0; __i < np_particles; ++__i) {
            __cb32[__i] = __i;
          }
          int64_t *__inl20_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            __inl20_rows[__w0] = __cb32[__w0];
          }
          if ((o == 0)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i[__w0] = ((int64_t)(floor((z_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i_shift[__w0] = (__inl20_i[__w0] - k_new[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((1 + __inl20_i_shift[__sc0]))] = 1.0;
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_idx[__w0] = __inl20_i[__w0];
            }
          }
          else if ((o == 1)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i[__w0] = ((int64_t)(floor(z_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i_shift[__w0] = (__inl20_i[__w0] - k_new[__w0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_xint[__w0] = (z_old[__w0] - __inl20_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((1 + __inl20_i_shift[__sc0]))] = (1.0 - __inl20_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((2 + __inl20_i_shift[__sc0]))] = __inl20_xint[__sc0];
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_idx[__w0] = __inl20_i[__w0];
            }
          }
          else if ((o == 2)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i[__w0] = ((int64_t)(trunc((z_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i_shift[__w0] = (__inl20_i[__w0] - (k_new[__w0] + 1));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_xint[__w0] = (z_old[__w0] - __inl20_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((1 + __inl20_i_shift[__sc0]))] = ((0.5 * (0.5 - __inl20_xint[__sc0])) * (0.5 - __inl20_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((2 + __inl20_i_shift[__sc0]))] = (0.75 - (__inl20_xint[__sc0] * __inl20_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((3 + __inl20_i_shift[__sc0]))] = ((0.5 * (0.5 + __inl20_xint[__sc0])) * (0.5 + __inl20_xint[__sc0]));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_idx[__w0] = (__inl20_i[__w0] - 1);
            }
          }
          else if ((o == 3)) {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i[__w0] = ((int64_t)(trunc(z_old[__w0])));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i_shift[__w0] = (__inl20_i[__w0] - (k_new[__w0] + 1));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_xint[__w0] = (z_old[__w0] - __inl20_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((1 + __inl20_i_shift[__sc0]))] = (((0.16666666666666666 * (1.0 - __inl20_xint[__sc0])) * (1.0 - __inl20_xint[__sc0])) * (1.0 - __inl20_xint[__sc0]));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((2 + __inl20_i_shift[__sc0]))] = ((2.0 / 3.0) - ((__inl20_xint[__sc0] * __inl20_xint[__sc0]) * (1.0 - (__inl20_xint[__sc0] / 2.0))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((3 + __inl20_i_shift[__sc0]))] = ((2.0 / 3.0) - (((1.0 - __inl20_xint[__sc0]) * (1.0 - __inl20_xint[__sc0])) * (1.0 - (0.5 * (1.0 - __inl20_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((4 + __inl20_i_shift[__sc0]))] = (((0.16666666666666666 * __inl20_xint[__sc0]) * __inl20_xint[__sc0]) * __inl20_xint[__sc0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_idx[__w0] = (__inl20_i[__w0] - 1);
            }
          }
          else {
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i[__w0] = ((int64_t)(trunc((z_old[__w0] + 0.5))));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_i_shift[__w0] = (__inl20_i[__w0] - (k_new[__w0] + 2));
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_xint[__w0] = (z_old[__w0] - __inl20_i[__w0]);
            }
            double *__inl20_sm = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_sm[__w0] = (0.5 - __inl20_xint[__w0]);
            }
            double *__inl20_sp = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_sp[__w0] = (0.5 + __inl20_xint[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((1 + __inl20_i_shift[__sc0]))] = (((((1.0 / 24.0) * __inl20_sm[__sc0]) * __inl20_sm[__sc0]) * __inl20_sm[__sc0]) * __inl20_sm[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((2 + __inl20_i_shift[__sc0]))] = ((1.0 / 24.0) * ((4.75 - (11.0 * __inl20_xint[__sc0])) + (((4.0 * __inl20_xint[__sc0]) * __inl20_xint[__sc0]) * ((1.5 + __inl20_xint[__sc0]) - (__inl20_xint[__sc0] * __inl20_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((3 + __inl20_i_shift[__sc0]))] = ((1.0 / 24.0) * (14.375 + (((6.0 * __inl20_xint[__sc0]) * __inl20_xint[__sc0]) * ((__inl20_xint[__sc0] * __inl20_xint[__sc0]) - 2.5))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((4 + __inl20_i_shift[__sc0]))] = ((1.0 / 24.0) * ((4.75 + (11.0 * __inl20_xint[__sc0])) + (((4.0 * __inl20_xint[__sc0]) * __inl20_xint[__sc0]) * ((1.5 - __inl20_xint[__sc0]) - (__inl20_xint[__sc0] * __inl20_xint[__sc0])))));
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl20_sx[(__inl20_rows[__sc0])*(width) + ((5 + __inl20_i_shift[__sc0]))] = (((((1.0 / 24.0) * __inl20_sp[__sc0]) * __inl20_sp[__sc0]) * __inl20_sp[__sc0]) * __inl20_sp[__sc0]);
            }
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl20_idx[__w0] = (__inl20_i[__w0] - 2);
            }
            free(__inl20_sm);
            free(__inl20_sp);
          }
          free(sz_old);
          sz_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            for (int64_t __w1 = 0; __w1 < width; ++__w1) {
              sz_old[(__w0)*(width) + (__w1)] = __inl20_sx[(__w0)*(width) + (__w1)];
            }
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            k_old[__w0] = __inl20_idx[__w0];
          }
          if (reduce_enabled) {
            free(__inl21_sx);
            __inl21_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            memset(__inl21_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
            int64_t *__cb33 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            /* numpy: np.arange(np_particles) */
            for (int64_t __i = 0; __i < np_particles; ++__i) {
              __cb33[__i] = __i;
            }
            int64_t *__inl21_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl21_rows[__w0] = __cb33[__w0];
            }
            int64_t *__inl21_i = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl21_i[__w0] = ((int64_t)(floor(z_new[__w0])));
            }
            int64_t *__inl21_i_shift = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl21_i_shift[__w0] = (__inl21_i[__w0] - (k_new[__w0] + half));
            }
            double *__inl21_xint = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl21_xint[__w0] = (z_new[__w0] - __inl21_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl21_sx[(__inl21_rows[__sc0])*(width) + (((half + 1) + __inl21_i_shift[__sc0]))] = (1.0 - __inl21_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl21_sx[(__inl21_rows[__sc0])*(width) + (((half + 2) + __inl21_i_shift[__sc0]))] = __inl21_xint[__sc0];
            }
            int64_t *__inl21_idx = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl21_idx[__w0] = __inl21_i[__w0];
            }
            free(ov_new);
            ov_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                ov_new[(__w0)*(width) + (__w1)] = __inl21_sx[(__w0)*(width) + (__w1)];
              }
            }
            free(__inl22_sx);
            __inl22_sx = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            memset(__inl22_sx, 0, (size_t)((np_particles) * (width)) * sizeof(double));
            int64_t *__cb34 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            /* numpy: np.arange(np_particles) */
            for (int64_t __i = 0; __i < np_particles; ++__i) {
              __cb34[__i] = __i;
            }
            int64_t *__inl22_rows = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl22_rows[__w0] = __cb34[__w0];
            }
            int64_t *__inl22_i = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl22_i[__w0] = ((int64_t)(floor(z_old[__w0])));
            }
            int64_t *__inl22_i_shift = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl22_i_shift[__w0] = (__inl22_i[__w0] - (k_new[__w0] + half));
            }
            double *__inl22_xint = (double *)malloc(((np_particles)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl22_xint[__w0] = (z_old[__w0] - __inl22_i[__w0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl22_sx[(__inl22_rows[__sc0])*(width) + (((half + 1) + __inl22_i_shift[__sc0]))] = (1.0 - __inl22_xint[__sc0]);
            }
            for (int64_t __sc0 = 0; __sc0 < np_particles; ++__sc0) {
              __inl22_sx[(__inl22_rows[__sc0])*(width) + (((half + 2) + __inl22_i_shift[__sc0]))] = __inl22_xint[__sc0];
            }
            int64_t *__inl22_idx = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              __inl22_idx[__w0] = __inl22_i[__w0];
            }
            free(ov_old);
            ov_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                ov_old[(__w0)*(width) + (__w1)] = __inl22_sx[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb35);
            __cb35 = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            /* numpy: np.where((reduce_shape_new != 0)[:, None], ov_new, sz_new) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              for (int64_t __r1 = 0; __r1 < width; ++__r1) {
                __cb35[(__r0)*(width) + (__r1)] = ((reduce_shape_new[__r0] != 0) ? ov_new[(__r0)*(width) + (__r1)] : sz_new[(__r0)*(width) + (__r1)]);
              }
            }
            free(sz_new);
            sz_new = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                sz_new[(__w0)*(width) + (__w1)] = __cb35[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb36);
            __cb36 = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            /* numpy: np.where((reduce_shape_old != 0)[:, None], ov_old, sz_old) */
            for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
              for (int64_t __r1 = 0; __r1 < width; ++__r1) {
                __cb36[(__r0)*(width) + (__r1)] = ((reduce_shape_old[__r0] != 0) ? ov_old[(__r0)*(width) + (__r1)] : sz_old[(__r0)*(width) + (__r1)]);
              }
            }
            free(sz_old);
            sz_old = (double *)malloc((size_t)((np_particles) * (width)) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
              for (int64_t __w1 = 0; __w1 < width; ++__w1) {
                sz_old[(__w0)*(width) + (__w1)] = __cb36[(__w0)*(width) + (__w1)];
              }
            }
            free(__cb33);
            free(__inl21_rows);
            free(__inl21_i);
            free(__inl21_i_shift);
            free(__inl21_xint);
            free(__inl21_idx);
            free(__cb34);
            free(__inl22_rows);
            free(__inl22_i);
            free(__inl22_i_shift);
            free(__inl22_xint);
            free(__inl22_idx);
          }
          free(__cb32);
          free(__inl20_rows);
        }
        for (int64_t __zf = 0; __zf < ((np_particles)); ++__zf) dil[__zf] = 1;
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          diu[__w0] = dil[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          djl[__w0] = dil[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          dju[__w0] = dil[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          dkl[__w0] = dil[__w0];
        }
        for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
          dku[__w0] = dil[__w0];
        }
        if ((geom != 0)) {
          int64_t *__cb37 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(i_old < i_new, 0, 1) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb37[__r0] = ((i_old[__r0] < i_new[__r0]) ? 0 : 1);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            dil[__w0] = __cb37[__w0];
          }
          int64_t *__cb38 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(i_old > i_new, 0, 1) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb38[__r0] = ((i_old[__r0] > i_new[__r0]) ? 0 : 1);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            diu[__w0] = __cb38[__w0];
          }
          free(__cb37);
          free(__cb38);
        }
        if ((geom == 3)) {
          int64_t *__cb39 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(j_old < j_new, 0, 1) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb39[__r0] = ((j_old[__r0] < j_new[__r0]) ? 0 : 1);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            djl[__w0] = __cb39[__w0];
          }
          int64_t *__cb40 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(j_old > j_new, 0, 1) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb40[__r0] = ((j_old[__r0] > j_new[__r0]) ? 0 : 1);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            dju[__w0] = __cb40[__w0];
          }
          free(__cb39);
          free(__cb40);
        }
        if (((geom != 4) && (geom != 5))) {
          int64_t *__cb41 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(k_old < k_new, 0, 1) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb41[__r0] = ((k_old[__r0] < k_new[__r0]) ? 0 : 1);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            dkl[__w0] = __cb41[__w0];
          }
          int64_t *__cb42 = (int64_t *)malloc(((np_particles)) * sizeof(int64_t));
          /* numpy: np.where(k_old > k_new, 0, 1) */
          for (int64_t __r0 = 0; __r0 < np_particles; ++__r0) {
            __cb42[__r0] = ((k_old[__r0] > k_new[__r0]) ? 0 : 1);
          }
          for (int64_t __w0 = 0; __w0 < np_particles; ++__w0) {
            dku[__w0] = __cb42[__w0];
          }
          free(__cb41);
          free(__cb42);
        }
        for (int64_t ip = 0; ip < np_particles; ++ip) {
          wqi = wq[ip];
          if ((geom == 3)) {
            i0 = ((int64_t)(dil[ip]));
            i1 = ((o + 2) - ((int64_t)(diu[ip])));
            j0_ = ((int64_t)(djl[ip]));
            j1_ = ((o + 3) - ((int64_t)(dju[ip])));
            k0 = ((int64_t)(dkl[ip]));
            k1 = ((o + 3) - ((int64_t)(dku[ip])));
            ib = (((int64_t)(i_new[ip])) - 1);
            jb = (((int64_t)(j_new[ip])) - 1);
            kb = (((int64_t)(k_new[ip])) - 1);
            free(gx);
            gx = (double *)malloc((size_t)(((j1_ - j0_)) * ((k1 - k0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (j1_ - j0_); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (k1 - k0); ++__w1) {
                gx[(__w0)*((k1 - k0)) + (__w1)] = ((0.3333333333333333 * ((sy_new[(ip)*(width) + ((__w0 + (j0_ - 0)))] * sz_new[(ip)*(width) + ((__w1 + (k0 - 0)))]) + (sy_old[(ip)*(width) + ((__w0 + (j0_ - 0)))] * sz_old[(ip)*(width) + ((__w1 + (k0 - 0)))]))) + (0.16666666666666666 * ((sy_new[(ip)*(width) + ((__w0 + (j0_ - 0)))] * sz_old[(ip)*(width) + ((__w1 + (k0 - 0)))]) + (sy_old[(ip)*(width) + ((__w0 + (j0_ - 0)))] * sz_new[(ip)*(width) + ((__w1 + (k0 - 0)))]))));
              }
            }
            free(__cb43);
            __cb43 = (double *)malloc((size_t)(((i1 - i0))) * sizeof(double));
            free(__cb43);
            __cb43 = (double *)malloc((size_t)(((i1 - i0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1 - i0); ++__w0) {
              __cb43[__w0] = ((wqi * invdtd_x) * (sx_old[(ip)*(width) + ((__w0 + (i0 - 0)))] - sx_new[(ip)*(width) + ((__w0 + (i0 - 0)))]));
            }
            free(cum_x);
            cum_x = (double *)malloc((size_t)(((i1 - i0))) * sizeof(double));
            /* numpy: np.cumsum(wqi * invdtd_x * (sx_old[ip, i0:i1] - sx_new[ip, i0:i1])) */
            cum_x[0] = __cb43[0];
            for (int64_t __cs0 = 1; __cs0 < (i1 - i0); ++__cs0) {
              cum_x[__cs0] = (cum_x[(__cs0 - 1)] + __cb43[__cs0]);
            }
            for (int64_t si0 = ((lox + ib) + i0); si0 < ((lox + ib) + i1); ++si0) {
              for (int64_t si1 = ((loy + jb) + j0_); si1 < ((loy + jb) + j1_); ++si1) {
                for (int64_t si2 = ((loz + kb) + k0); si2 < ((loz + kb) + k1); ++si2) {
                  Jx[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (si2))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (cum_x[(si0 + (0 - ((lox + ib) + i0)))] * gx[((si1 + (0 - ((loy + jb) + j0_))))*((k1 - k0)) + ((si2 + (0 - ((loz + kb) + k0))))]);
                }
              }
            }
            i0y = ((int64_t)(dil[ip]));
            i1y = ((o + 3) - ((int64_t)(diu[ip])));
            j0y = ((int64_t)(djl[ip]));
            j1y = ((o + 2) - ((int64_t)(dju[ip])));
            k0y = ((int64_t)(dkl[ip]));
            k1y = ((o + 3) - ((int64_t)(dku[ip])));
            free(gy);
            gy = (double *)malloc((size_t)(((i1y - i0y)) * ((k1y - k0y))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (k1y - k0y); ++__w1) {
                gy[(__w0)*((k1y - k0y)) + (__w1)] = ((0.3333333333333333 * ((sx_new[(ip)*(width) + ((__w0 + (i0y - 0)))] * sz_new[(ip)*(width) + ((__w1 + (k0y - 0)))]) + (sx_old[(ip)*(width) + ((__w0 + (i0y - 0)))] * sz_old[(ip)*(width) + ((__w1 + (k0y - 0)))]))) + (0.16666666666666666 * ((sx_new[(ip)*(width) + ((__w0 + (i0y - 0)))] * sz_old[(ip)*(width) + ((__w1 + (k0y - 0)))]) + (sx_old[(ip)*(width) + ((__w0 + (i0y - 0)))] * sz_new[(ip)*(width) + ((__w1 + (k0y - 0)))]))));
              }
            }
            free(__cb45);
            __cb45 = (double *)malloc((size_t)(((j1y - j0y))) * sizeof(double));
            free(__cb45);
            __cb45 = (double *)malloc((size_t)(((j1y - j0y))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (j1y - j0y); ++__w0) {
              __cb45[__w0] = ((wqi * invdtd_y) * (sy_old[(ip)*(width) + ((__w0 + (j0y - 0)))] - sy_new[(ip)*(width) + ((__w0 + (j0y - 0)))]));
            }
            free(cum_y);
            cum_y = (double *)malloc((size_t)(((j1y - j0y))) * sizeof(double));
            /* numpy: np.cumsum(wqi * invdtd_y * (sy_old[ip, j0y:j1y] - sy_new[ip, j0y:j1y])) */
            cum_y[0] = __cb45[0];
            for (int64_t __cs0 = 1; __cs0 < (j1y - j0y); ++__cs0) {
              cum_y[__cs0] = (cum_y[(__cs0 - 1)] + __cb45[__cs0]);
            }
            for (int64_t si0 = ((lox + ib) + i0y); si0 < ((lox + ib) + i1y); ++si0) {
              for (int64_t si1 = ((loy + jb) + j0y); si1 < ((loy + jb) + j1y); ++si1) {
                for (int64_t si2 = ((loz + kb) + k0y); si2 < ((loz + kb) + k1y); ++si2) {
                  Jy[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (si2))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (gy[((si0 + (0 - ((lox + ib) + i0y))))*((k1y - k0y)) + ((si2 + (0 - ((loz + kb) + k0y))))] * cum_y[(si1 + (0 - ((loy + jb) + j0y)))]);
                }
              }
            }
            i0z = ((int64_t)(dil[ip]));
            i1z = ((o + 3) - ((int64_t)(diu[ip])));
            j0z = ((int64_t)(djl[ip]));
            j1z = ((o + 3) - ((int64_t)(dju[ip])));
            k0z = ((int64_t)(dkl[ip]));
            k1z = ((o + 2) - ((int64_t)(dku[ip])));
            free(gz);
            gz = (double *)malloc((size_t)(((i1z - i0z)) * ((j1z - j0z))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1z - i0z); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (j1z - j0z); ++__w1) {
                gz[(__w0)*((j1z - j0z)) + (__w1)] = ((0.3333333333333333 * ((sx_new[(ip)*(width) + ((__w0 + (i0z - 0)))] * sy_new[(ip)*(width) + ((__w1 + (j0z - 0)))]) + (sx_old[(ip)*(width) + ((__w0 + (i0z - 0)))] * sy_old[(ip)*(width) + ((__w1 + (j0z - 0)))]))) + (0.16666666666666666 * ((sx_new[(ip)*(width) + ((__w0 + (i0z - 0)))] * sy_old[(ip)*(width) + ((__w1 + (j0z - 0)))]) + (sx_old[(ip)*(width) + ((__w0 + (i0z - 0)))] * sy_new[(ip)*(width) + ((__w1 + (j0z - 0)))]))));
              }
            }
            free(__cb47);
            __cb47 = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            free(__cb47);
            __cb47 = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (k1z - k0z); ++__w0) {
              __cb47[__w0] = ((wqi * invdtd_z) * (sz_old[(ip)*(width) + ((__w0 + (k0z - 0)))] - sz_new[(ip)*(width) + ((__w0 + (k0z - 0)))]));
            }
            free(cum_z);
            cum_z = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            /* numpy: np.cumsum(wqi * invdtd_z * (sz_old[ip, k0z:k1z] - sz_new[ip, k0z:k1z])) */
            cum_z[0] = __cb47[0];
            for (int64_t __cs0 = 1; __cs0 < (k1z - k0z); ++__cs0) {
              cum_z[__cs0] = (cum_z[(__cs0 - 1)] + __cb47[__cs0]);
            }
            for (int64_t si0 = ((lox + ib) + i0z); si0 < ((lox + ib) + i1z); ++si0) {
              for (int64_t si1 = ((loy + jb) + j0z); si1 < ((loy + jb) + j1z); ++si1) {
                for (int64_t si2 = ((loz + kb) + k0z); si2 < ((loz + kb) + k1z); ++si2) {
                  Jz[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (si2))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (gz[((si0 + (0 - ((lox + ib) + i0z))))*((j1z - j0z)) + ((si1 + (0 - ((loy + jb) + j0z))))] * cum_z[(si2 + (0 - ((loz + kb) + k0z)))]);
                }
              }
            }
          }
          else if (((geom == 1) || (geom == 2))) {
            i0 = ((int64_t)(dil[ip]));
            i1 = ((o + 2) - ((int64_t)(diu[ip])));
            k0 = ((int64_t)(dkl[ip]));
            k1 = ((o + 3) - ((int64_t)(dku[ip])));
            ib = (((int64_t)(i_new[ip])) - 1);
            kb = (((int64_t)(k_new[ip])) - 1);
            free(__cb49);
            __cb49 = (double *)malloc((size_t)(((i1 - i0))) * sizeof(double));
            free(__cb49);
            __cb49 = (double *)malloc((size_t)(((i1 - i0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1 - i0); ++__w0) {
              __cb49[__w0] = ((wqi * invdtd_x) * (sx_old[(ip)*(width) + ((__w0 + (i0 - 0)))] - sx_new[(ip)*(width) + ((__w0 + (i0 - 0)))]));
            }
            /* numpy: np.cumsum(wqi * invdtd_x * (sx_old[ip, i0:i1] - sx_new[ip, i0:i1])) */
            cum_x[0] = __cb49[0];
            for (int64_t __cs0 = 1; __cs0 < (i1 - i0); ++__cs0) {
              cum_x[__cs0] = (cum_x[(__cs0 - 1)] + __cb49[__cs0]);
            }
            free(zavg_x);
            zavg_x = (double *)malloc((size_t)(((k1 - k0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (k1 - k0); ++__w0) {
              zavg_x[__w0] = (0.5 * (sz_new[(ip)*(width) + ((__w0 + (k0 - 0)))] + sz_old[(ip)*(width) + ((__w0 + (k0 - 0)))]));
            }
            free(sdxi);
            sdxi = (double *)malloc((size_t)(((i1 - i0)) * ((k1 - k0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1 - i0); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (k1 - k0); ++__w1) {
                sdxi[(__w0)*((k1 - k0)) + (__w1)] = (cum_x[__w0] * zavg_x[__w1]);
              }
            }
            for (int64_t si0 = ((lox + ib) + i0); si0 < ((lox + ib) + i1); ++si0) {
              for (int64_t si1 = ((loy + kb) + k0); si1 < ((loy + kb) + k1); ++si1) {
                Jx[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += sdxi[((si0 - ((lox + ib) + i0)))*((k1 - k0)) + ((si1 - ((loy + kb) + k0)))];
              }
            }
            if (rz_modes) {
              free(djr);
              djr = (double *)malloc((size_t)(((i1 - i0)) * ((k1 - k0))) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < (i1 - i0); ++__w0) {
                for (int64_t __w1 = 0; __w1 < (k1 - k0); ++__w1) {
                  djr[(__w0)*((k1 - k0)) + (__w1)] = (2.0 * sdxi[(__w0)*((k1 - k0)) + (__w1)]);
                }
              }
              xy_mid_re = xy_mid0_re[ip];
              xy_mid_im = xy_mid0_im[ip];
              for (int64_t imode = 1; imode < n_modes; ++imode) {
                for (int64_t si0 = ((lox + ib) + i0); si0 < ((lox + ib) + i1); ++si0) {
                  for (int64_t si1 = ((loy + kb) + k0); si1 < ((loy + kb) + k1); ++si1) {
                    Jx[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * imode) - 1))] += (djr[((si0 - ((lox + ib) + i0)))*((k1 - k0)) + ((si1 - ((loy + kb) + k0)))] * xy_mid_re);
                  }
                }
                for (int64_t si0 = ((lox + ib) + i0); si0 < ((lox + ib) + i1); ++si0) {
                  for (int64_t si1 = ((loy + kb) + k0); si1 < ((loy + kb) + k1); ++si1) {
                    Jx[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * imode))] += (djr[((si0 - ((lox + ib) + i0)))*((k1 - k0)) + ((si1 - ((loy + kb) + k0)))] * xy_mid_im);
                  }
                }
                nxt_mid_re = ((xy_mid_re * xy_mid0_re[ip]) - (xy_mid_im * xy_mid0_im[ip]));
                nxt_mid_im = ((xy_mid_re * xy_mid0_im[ip]) + (xy_mid_im * xy_mid0_re[ip]));
                xy_mid_re = nxt_mid_re;
                xy_mid_im = nxt_mid_im;
              }
            }
            i0y = ((int64_t)(dil[ip]));
            i1y = ((o + 3) - ((int64_t)(diu[ip])));
            k0y = ((int64_t)(dkl[ip]));
            k1y = ((o + 3) - ((int64_t)(dku[ip])));
            free(sdyj);
            sdyj = (double *)malloc((size_t)(((i1y - i0y)) * ((k1y - k0y))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (k1y - k0y); ++__w1) {
                sdyj[(__w0)*((k1y - k0y)) + (__w1)] = (((wqi * vy[ip]) * invvol) * ((0.3333333333333333 * ((sx_new[(ip)*(width) + ((i0y + __w0))] * sz_new[(ip)*(width) + ((k0y + __w1))]) + (sx_old[(ip)*(width) + ((i0y + __w0))] * sz_old[(ip)*(width) + ((k0y + __w1))]))) + (0.16666666666666666 * ((sx_new[(ip)*(width) + ((i0y + __w0))] * sz_old[(ip)*(width) + ((k0y + __w1))]) + (sx_old[(ip)*(width) + ((i0y + __w0))] * sz_new[(ip)*(width) + ((k0y + __w1))])))));
              }
            }
            for (int64_t si0 = ((lox + ib) + i0y); si0 < ((lox + ib) + i1y); ++si0) {
              for (int64_t si1 = ((loy + kb) + k0y); si1 < ((loy + kb) + k1y); ++si1) {
                Jy[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += sdyj[((si0 - ((lox + ib) + i0y)))*((k1y - k0y)) + ((si1 - ((loy + kb) + k0y)))];
              }
            }
            if (rz_modes) {
              free(a_re);
              a_re = (double *)malloc((size_t)(((i1y - i0y)) * ((k1y - k0y))) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                for (int64_t __w1 = 0; __w1 < (k1y - k0y); ++__w1) {
                  a_re[(__w0)*((k1y - k0y)) + (__w1)] = (sx_new[(ip)*(width) + ((i0y + __w0))] * sz_new[(ip)*(width) + ((k0y + __w1))]);
                }
              }
              free(b_re);
              b_re = (double *)malloc((size_t)(((i1y - i0y)) * ((k1y - k0y))) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                for (int64_t __w1 = 0; __w1 < (k1y - k0y); ++__w1) {
                  b_re[(__w0)*((k1y - k0y)) + (__w1)] = (sx_old[(ip)*(width) + ((i0y + __w0))] * sz_old[(ip)*(width) + ((k0y + __w1))]);
                }
              }
              free(__cb51);
              __cb51 = (int64_t *)malloc((size_t)(((i1y - i0y))) * sizeof(int64_t));
              /* numpy: np.arange(i0y, i1y) */
              for (int64_t __i = 0; __i < (i1y - i0y); ++__i) {
                __cb51[__i] = (i0y + __i);
              }
              free(i_local);
              i_local = (int64_t *)malloc((size_t)(((i1y - i0y))) * sizeof(int64_t));
              for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                i_local[__w0] = (ib + __cb51[__w0]);
              }
              free(neg2coef);
              neg2coef = (double *)malloc((size_t)(((i1y - i0y))) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                neg2coef[__w0] = ((((-2.0) * (i_local[__w0] + (xmin * dinvx))) * wqi) * invdtd_x);
              }
              xy_new_re = xy_new0_re[ip];
              xy_new_im = xy_new0_im[ip];
              xy_mid_re = xy_mid0_re[ip];
              xy_mid_im = xy_mid0_im[ip];
              xy_old_re = xy_old0_re[ip];
              xy_old_im = xy_old0_im[ip];
              for (int64_t imode = 1; imode < n_modes; ++imode) {
                free(sum_re);
                sum_re = (double *)malloc((size_t)(((i1y - i0y)) * ((k1y - k0y))) * sizeof(double));
                for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                  for (int64_t __w1 = 0; __w1 < (k1y - k0y); ++__w1) {
                    sum_re[(__w0)*((k1y - k0y)) + (__w1)] = ((a_re[(__w0)*((k1y - k0y)) + (__w1)] * (xy_new_re - xy_mid_re)) + (b_re[(__w0)*((k1y - k0y)) + (__w1)] * (xy_mid_re - xy_old_re)));
                  }
                }
                free(sum_im);
                sum_im = (double *)malloc((size_t)(((i1y - i0y)) * ((k1y - k0y))) * sizeof(double));
                for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                  for (int64_t __w1 = 0; __w1 < (k1y - k0y); ++__w1) {
                    sum_im[(__w0)*((k1y - k0y)) + (__w1)] = ((a_re[(__w0)*((k1y - k0y)) + (__w1)] * (xy_new_im - xy_mid_im)) + (b_re[(__w0)*((k1y - k0y)) + (__w1)] * (xy_mid_im - xy_old_im)));
                  }
                }
                free(coef_m);
                coef_m = (double *)malloc((size_t)(((i1y - i0y))) * sizeof(double));
                for (int64_t __w0 = 0; __w0 < (i1y - i0y); ++__w0) {
                  coef_m[__w0] = (neg2coef[__w0] / ((double)(imode)));
                }
                for (int64_t si0 = ((lox + ib) + i0y); si0 < ((lox + ib) + i1y); ++si0) {
                  for (int64_t si1 = ((loy + kb) + k0y); si1 < ((loy + kb) + k1y); ++si1) {
                    Jy[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * imode) - 1))] += (coef_m[(si0 + (0 - ((lox + ib) + i0y)))] * (-sum_im[((si0 - ((lox + ib) + i0y)))*((k1y - k0y)) + ((si1 - ((loy + kb) + k0y)))]));
                  }
                }
                for (int64_t si0 = ((lox + ib) + i0y); si0 < ((lox + ib) + i1y); ++si0) {
                  for (int64_t si1 = ((loy + kb) + k0y); si1 < ((loy + kb) + k1y); ++si1) {
                    Jy[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * imode))] += (coef_m[(si0 + (0 - ((lox + ib) + i0y)))] * sum_re[((si0 - ((lox + ib) + i0y)))*((k1y - k0y)) + ((si1 - ((loy + kb) + k0y)))]);
                  }
                }
                nxt_new_re = ((xy_new_re * xy_new0_re[ip]) - (xy_new_im * xy_new0_im[ip]));
                nxt_new_im = ((xy_new_re * xy_new0_im[ip]) + (xy_new_im * xy_new0_re[ip]));
                nxt_mid_re = ((xy_mid_re * xy_mid0_re[ip]) - (xy_mid_im * xy_mid0_im[ip]));
                nxt_mid_im = ((xy_mid_re * xy_mid0_im[ip]) + (xy_mid_im * xy_mid0_re[ip]));
                nxt_old_re = ((xy_old_re * xy_old0_re[ip]) - (xy_old_im * xy_old0_im[ip]));
                nxt_old_im = ((xy_old_re * xy_old0_im[ip]) + (xy_old_im * xy_old0_re[ip]));
                xy_new_re = nxt_new_re;
                xy_new_im = nxt_new_im;
                xy_mid_re = nxt_mid_re;
                xy_mid_im = nxt_mid_im;
                xy_old_re = nxt_old_re;
                xy_old_im = nxt_old_im;
              }
            }
            i0z = ((int64_t)(dil[ip]));
            i1z = ((o + 3) - ((int64_t)(diu[ip])));
            k0z = ((int64_t)(dkl[ip]));
            k1z = ((o + 2) - ((int64_t)(dku[ip])));
            free(__cb52);
            __cb52 = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            free(__cb52);
            __cb52 = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (k1z - k0z); ++__w0) {
              __cb52[__w0] = ((wqi * invdtd_z) * (sz_old[(ip)*(width) + ((__w0 + (k0z - 0)))] - sz_new[(ip)*(width) + ((__w0 + (k0z - 0)))]));
            }
            /* numpy: np.cumsum(wqi * invdtd_z * (sz_old[ip, k0z:k1z] - sz_new[ip, k0z:k1z])) */
            cum_z[0] = __cb52[0];
            for (int64_t __cs0 = 1; __cs0 < (k1z - k0z); ++__cs0) {
              cum_z[__cs0] = (cum_z[(__cs0 - 1)] + __cb52[__cs0]);
            }
            free(xavg_z);
            xavg_z = (double *)malloc((size_t)(((i1z - i0z))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1z - i0z); ++__w0) {
              xavg_z[__w0] = (0.5 * (sx_new[(ip)*(width) + ((__w0 + (i0z - 0)))] + sx_old[(ip)*(width) + ((__w0 + (i0z - 0)))]));
            }
            free(sdzk);
            sdzk = (double *)malloc((size_t)(((i1z - i0z)) * ((k1z - k0z))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1z - i0z); ++__w0) {
              for (int64_t __w1 = 0; __w1 < (k1z - k0z); ++__w1) {
                sdzk[(__w0)*((k1z - k0z)) + (__w1)] = (xavg_z[__w0] * cum_z[__w1]);
              }
            }
            for (int64_t si0 = ((lox + ib) + i0z); si0 < ((lox + ib) + i1z); ++si0) {
              for (int64_t si1 = ((loy + kb) + k0z); si1 < ((loy + kb) + k1z); ++si1) {
                Jz[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += sdzk[((si0 - ((lox + ib) + i0z)))*((k1z - k0z)) + ((si1 - ((loy + kb) + k0z)))];
              }
            }
            if (rz_modes) {
              free(djz);
              djz = (double *)malloc((size_t)(((i1z - i0z)) * ((k1z - k0z))) * sizeof(double));
              for (int64_t __w0 = 0; __w0 < (i1z - i0z); ++__w0) {
                for (int64_t __w1 = 0; __w1 < (k1z - k0z); ++__w1) {
                  djz[(__w0)*((k1z - k0z)) + (__w1)] = (2.0 * sdzk[(__w0)*((k1z - k0z)) + (__w1)]);
                }
              }
              xy_mid_re = xy_mid0_re[ip];
              xy_mid_im = xy_mid0_im[ip];
              for (int64_t imode = 1; imode < n_modes; ++imode) {
                for (int64_t si0 = ((lox + ib) + i0z); si0 < ((lox + ib) + i1z); ++si0) {
                  for (int64_t si1 = ((loy + kb) + k0z); si1 < ((loy + kb) + k1z); ++si1) {
                    Jz[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (((2 * imode) - 1))] += (djz[((si0 - ((lox + ib) + i0z)))*((k1z - k0z)) + ((si1 - ((loy + kb) + k0z)))] * xy_mid_re);
                  }
                }
                for (int64_t si0 = ((lox + ib) + i0z); si0 < ((lox + ib) + i1z); ++si0) {
                  for (int64_t si1 = ((loy + kb) + k0z); si1 < ((loy + kb) + k1z); ++si1) {
                    Jz[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (si1))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + ((2 * imode))] += (djz[((si0 - ((lox + ib) + i0z)))*((k1z - k0z)) + ((si1 - ((loy + kb) + k0z)))] * xy_mid_im);
                  }
                }
                nxt_mid_re = ((xy_mid_re * xy_mid0_re[ip]) - (xy_mid_im * xy_mid0_im[ip]));
                nxt_mid_im = ((xy_mid_re * xy_mid0_im[ip]) + (xy_mid_im * xy_mid0_re[ip]));
                xy_mid_re = nxt_mid_re;
                xy_mid_im = nxt_mid_im;
              }
            }
          }
          else if ((geom == 0)) {
            k0 = ((int64_t)(dkl[ip]));
            k1 = ((o + 3) - ((int64_t)(dku[ip])));
            kb = (((int64_t)(k_new[ip])) - 1);
            free(zavg);
            zavg = (double *)malloc((size_t)(((k1 - k0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (k1 - k0); ++__w0) {
              zavg[__w0] = (0.5 * (sz_old[(ip)*(width) + ((__w0 + (k0 - 0)))] + sz_new[(ip)*(width) + ((__w0 + (k0 - 0)))]));
            }
            for (int64_t si0 = ((lox + kb) + k0); si0 < ((lox + kb) + k1); ++si0) {
              Jx[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (((wqi * vx[ip]) * invvol) * zavg[(si0 - ((lox + kb) + k0))]);
            }
            for (int64_t si0 = ((lox + kb) + k0); si0 < ((lox + kb) + k1); ++si0) {
              Jy[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (((wqi * vy[ip]) * invvol) * zavg[(si0 - ((lox + kb) + k0))]);
            }
            k0z = ((int64_t)(dkl[ip]));
            k1z = ((o + 2) - ((int64_t)(dku[ip])));
            free(__cb54);
            __cb54 = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            free(__cb54);
            __cb54 = (double *)malloc((size_t)(((k1z - k0z))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (k1z - k0z); ++__w0) {
              __cb54[__w0] = ((wqi * invdtd_z) * (sz_old[(ip)*(width) + ((__w0 + (k0z - 0)))] - sz_new[(ip)*(width) + ((__w0 + (k0z - 0)))]));
            }
            /* numpy: np.cumsum(wqi * invdtd_z * (sz_old[ip, k0z:k1z] - sz_new[ip, k0z:k1z])) */
            cum_z[0] = __cb54[0];
            for (int64_t __cs0 = 1; __cs0 < (k1z - k0z); ++__cs0) {
              cum_z[__cs0] = (cum_z[(__cs0 - 1)] + __cb54[__cs0]);
            }
            for (int64_t si0 = ((lox + kb) + k0z); si0 < ((lox + kb) + k1z); ++si0) {
              Jz[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += cum_z[(si0 - ((lox + kb) + k0z))];
            }
          }
          else {
            i0x = ((int64_t)(dil[ip]));
            i1x = ((o + 2) - ((int64_t)(diu[ip])));
            ib = (((int64_t)(i_new[ip])) - 1);
            free(__cb56);
            __cb56 = (double *)malloc((size_t)(((i1x - i0x))) * sizeof(double));
            free(__cb56);
            __cb56 = (double *)malloc((size_t)(((i1x - i0x))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1x - i0x); ++__w0) {
              __cb56[__w0] = ((wqi * invdtd_x) * (sx_old[(ip)*(width) + ((__w0 + (i0x - 0)))] - sx_new[(ip)*(width) + ((__w0 + (i0x - 0)))]));
            }
            free(cum_x__v1);
            cum_x__v1 = (double *)malloc((size_t)(((i1x - i0x))) * sizeof(double));
            /* numpy: np.cumsum(wqi * invdtd_x * (sx_old[ip, i0x:i1x] - sx_new[ip, i0x:i1x])) */
            cum_x__v1[0] = __cb56[0];
            for (int64_t __cs0 = 1; __cs0 < (i1x - i0x); ++__cs0) {
              cum_x__v1[__cs0] = (cum_x__v1[(__cs0 - 1)] + __cb56[__cs0]);
            }
            for (int64_t si0 = ((lox + ib) + i0x); si0 < ((lox + ib) + i1x); ++si0) {
              Jx[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += cum_x__v1[(si0 - ((lox + ib) + i0x))];
            }
            i0 = ((int64_t)(dil[ip]));
            i1 = ((o + 3) - ((int64_t)(diu[ip])));
            free(xavg);
            xavg = (double *)malloc((size_t)(((i1 - i0))) * sizeof(double));
            for (int64_t __w0 = 0; __w0 < (i1 - i0); ++__w0) {
              xavg[__w0] = (0.5 * (sx_old[(ip)*(width) + ((__w0 + (i0 - 0)))] + sx_new[(ip)*(width) + ((__w0 + (i0 - 0)))]));
            }
            for (int64_t si0 = ((lox + ib) + i0); si0 < ((lox + ib) + i1); ++si0) {
              Jy[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (((wqi * vy[ip]) * invvol) * xavg[(si0 - ((lox + ib) + i0))]);
            }
            for (int64_t si0 = ((lox + ib) + i0); si0 < ((lox + ib) + i1); ++si0) {
              Jz[(((si0)*(((ncells + (2 * depos_order)) + 6)) + (0))*(((ncells + (2 * depos_order)) + 6)) + (0))*(((2 * n_rz_azimuthal_modes) - 1)) + (0)] += (((wqi * vz[ip]) * invvol) * xavg[(si0 - ((lox + ib) + i0))]);
            }
          }
        }
        free(x_new);
        free(x_old);
        free(y_new);
        free(y_old);
        free(z_new);
        free(z_old);
        free(vx);
        free(vy);
        free(vz);
        free(xy_new0_re);
        free(xy_mid0_re);
        free(xy_old0_re);
        free(xy_new0_im);
        free(xy_mid0_im);
        free(xy_old0_im);
        free(reduce_shape_old);
        free(reduce_shape_new);
        free(i_new);
        free(dil);
        free(__cb1);
        free(gaminv);
        free(wq);
        free(xp_new);
        free(yp_new);
        free(xp_mid);
        free(yp_mid);
        free(xp_old);
        free(yp_old);
        free(costheta_mid);
        free(sintheta_mid);
        free(rp_mid);
        free(cosphi_mid);
        free(sinphi_mid);
        free(i_old);
        free(j_new);
        free(j_old);
        free(k_new);
        free(k_old);
        free(__inl11_idx);
        free(__inl11_xint);
        free(__inl12_i_shift);
        free(__inl12_idx);
        free(__inl12_xint);
        free(__inl15_idx);
        free(__inl15_xint);
        free(__inl16_i_shift);
        free(__inl16_idx);
        free(__inl16_xint);
        free(__inl19_idx);
        free(__inl19_xint);
        free(__inl20_i_shift);
        free(__inl20_idx);
        free(__inl20_xint);
        free(diu);
        free(djl);
        free(dju);
        free(dkl);
        free(dku);
        free(rp_new);
        free(rp_old);
        free(fx_o);
        free(fz_o);
        free(fx_n);
        free(fz_n);
        free(__inl11_j);
        free(__inl12_i);
        free(__inl15_j);
        free(__inl16_i);
        free(__inl19_j);
        free(__inl20_i);
        free(__inl11_sx);
        free(__inl12_sx);
        free(__inl13_sx);
        free(__inl14_sx);
        free(__inl15_sx);
        free(__inl16_sx);
        free(__inl17_sx);
        free(__inl18_sx);
        free(__inl19_sx);
        free(__inl20_sx);
        free(__inl21_sx);
        free(__inl22_sx);
        free(__cb25);
        free(__cb26);
        free(__cb30);
        free(__cb31);
        free(__cb35);
        free(__cb36);
        free(__cb43);
        free(__cb45);
        free(__cb47);
        free(__cb49);
        free(__cb51);
        free(__cb52);
        free(__cb54);
        free(__cb56);
        free(cum_x);
        free(cum_y);
        free(cum_z);
        free(cum_x__v1);
        free(sx_new);
        free(sx_old);
        free(ov_new);
        free(ov_old);
        free(sy_new);
        free(sy_old);
        free(sz_new);
        free(sz_old);
        free(gx);
        free(gy);
        free(gz);
        free(zavg_x);
        free(sdxi);
        free(djr);
        free(sdyj);
        free(a_re);
        free(b_re);
        free(i_local);
        free(neg2coef);
        free(sum_re);
        free(sum_im);
        free(coef_m);
        free(xavg_z);
        free(sdzk);
        free(djz);
        free(zavg);
        free(xavg);
}
} // extern "C"
