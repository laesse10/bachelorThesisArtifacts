import numpy as np
import numba as nb
from numba import njit, prange

@njit(parallel=True, fastmath=False)
def rk4_ensemble_nb(y0, y, NSYS, NSTEPS, a, b, ep, t_end):
    h = t_end / NSTEPS
    h2 = 0.5 * h
    h6 = h / 6.0
    for n in prange(NSYS):
        u = y0[n, 0]; v = y0[n, 1]; w = y0[n, 2]
        for _step in range(NSTEPS):
            k1_0 = a - (w + 1.0) * u + v * u * u
            k1_1 = w * u - v * u * u
            k1_2 = (b - w) / ep - w * u
            k2_0 = a - (w2 if False else 0)  # placeholder to be replaced
            u = u; v = v; w = w
    return y

# Write the real one
def _build():
    from numba import njit, prange
    import numpy as np
    src = '''
def _impl(y0, y, NSYS, NSTEPS, a, b, ep, t_end):
    h = t_end / NSTEPS
    h2 = 0.5 * h
    h6 = h / 6.0
    for n in prange(NSYS):
        u = y0[n, 0]; v = y0[n, 1]; w = y0[n, 2]
        for _step in range(NSTEPS):
            k1_0 = a - (w + 1.0) * u + v * u * u
            k1_1 = w * u - v * u * u
            k1_2 = (b - w) / ep - w * u
            u2 = u + 0.5 * h * k1_0; v2 = v + 0.5 * h * k1_1; w2 = w + 0.5 * h * k1_2
            k2_0 = a - (w2 + 1.0) * u2 + v2 * u2 * u2
            k2_1 = w2 * u2 - v2 * u2 * u2
            k2_2 = (b - w2) / ep - w2 * u2
            u3 = u + 0.5 * h * k2_0; v3 = v + 0.5 * h * k2_1; w3 = w + 0.5 * h * k2_2
            k3_0 = a - (w3 + 1.0) * u3 + v3 * u3 * u3
            k3_1 = w3 * u3 - v3 * u3 * u3
            k3_2 = (b - w3) / ep - w3 * u3
            u4 = u + h * k3_0; v4 = v + h * k3_1; w4 = w + h * k3_2
            k4_0 = a - (w4 + 1.0) * u4 + v4 * u4 * u4
            k4_1 = w4 * u4 - v4 * u4 * u4
            k4_2 = (b - w4) / ep - w4 * u4
            u = u + (h / 6.0) * (k1_0 + 2.0 * k2_0 + 2.0 * k3_0 + k4_0)
            v = v + (h / 6.0) * (k1_1 + 2.0 * k2_1 + 2.0 * k3_1 + k4_1)
            w = w + (h / 6.0) * (k1_2 + 2.0 * k2_2 + 2.0 * k3_2 + k4_2)
        y[n, 0] = u; y[n, 1] = v; y[n, 2] = w
'''
    ns = {}
    exec("from numba import njit, prange\n" + src, ns)
    impl = ns['_impl']
    return njit(parallel=True, cache=False)(impl)

rk4_ensemble_nb = _build()
