import importlib.util
import numpy as np
import ctypes

# Problem size
NX, NY, NZ = 48, 48, 40
theta = 0.03
n0 = NX * NY * NZ

# Helper to map 3D indices to linear

def idx(i, j, k):
    return i + NX * j + NX * NY * k

# Build 27-point stencil CSR (uniform weights, diagonal = neighbor count)
rows = []
cols = []
vals = []
for k in range(NZ):
    for j in range(NY):
        for i in range(NX):
            r = idx(i, j, k)
            neighbor_cnt = 0
            for dz in (-1, 0, 1):
                nk = k + dz
                if nk < 0 or nk >= NZ:
                    continue
                for dy in (-1, 0, 1):
                    nj = j + dy
                    if nj < 0 or nj >= NY:
                        continue
                    for dx in (-1, 0, 1):
                        ni = i + dx
                        if ni < 0 or ni >= NX:
                            continue
                        c = idx(ni, nj, nk)
                        if c == r:
                            continue
                        rows.append(r)
                        cols.append(c)
                        vals.append(1.0)
                        neighbor_cnt += 1
            # diagonal entry
            rows.append(r)
            cols.append(r)
            vals.append(float(neighbor_cnt))

# Build CSR arrays
A_indptr = np.zeros(n0 + 1, dtype=np.int64)
for r in rows:
    A_indptr[r + 1] += 1
for i in range(n0):
    A_indptr[i + 1] += A_indptr[i]
nnz = len(vals)
A_indices = np.empty(nnz, dtype=np.int64)
A_data = np.empty(nnz, dtype=np.float64)
counter = np.zeros(n0, dtype=np.int64)
for r, c, v in zip(rows, cols, vals):
    pos = A_indptr[r] + counter[r]
    A_indices[pos] = c
    A_data[pos] = v
    counter[r] += 1
# Apply deterministic jitter to simulate perturbation
rng = np.random.default_rng(12345)
# Jitter up to +/-10%
jitter = 0.1 * (rng.random(nnz) - 0.5)
A_data = A_data * (1.0 + jitter)

# Load reference NumPy implementation
spec_numpy = importlib.util.spec_from_file_location('amg_setup_numpy', '/shared/agent-4/amg_setup_numpy.py')
numpy_mod = importlib.util.module_from_spec(spec_numpy)
spec_numpy.loader.exec_module(numpy_mod)

LMAX = numpy_mod.LMAX
agg0 = np.zeros(n0, dtype=np.int64)
level_n = np.zeros(LMAX, dtype=np.int64)
level_nnz = np.zeros(LMAX, dtype=np.int64)
nlevels = np.zeros(1, dtype=np.int64)

# Compute workspace size (same as in C implementation)
nnz0 = A_data.size
npad = n0
zpad = (3 * (nnz0 + n0)) // 2

def add(cnt):
    return cnt * 8
need = (
    add(npad + 1) + add(zpad) * 3 +      # cur_* arrays
    add(npad + 1) + add(zpad) * 3 +      # nxt_* arrays
    add(npad + 1) + add(zpad) +          # s_*
    add(npad + 1) + add(zpad) * 2 +      # p_*
    add(npad + 1) + add(zpad) * 2 +      # pt_*
    add(npad + 1) + add(zpad) * 2 +      # ap_*
    add(npad) * 6 +                      # diag, agg, cnt, tscale, vv, ww
    add(npad) * 3 +                      # acc, mark, touched
    add(npad + 1) + add(1) * 2           # fill, na_out, rho_out
)

workspace = np.zeros(need, dtype=np.uint8)
workspace_ptr = workspace.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8))

# Load the compiled C kernel
lib = ctypes.CDLL('/shared/agent-4/libamg_setup.so')
lib.amg_setup_csr_fp64.argtypes = [ctypes.POINTER(ctypes.c_double),
                                   ctypes.POINTER(ctypes.c_int64),
                                   ctypes.POINTER(ctypes.c_int64),
                                   ctypes.POINTER(ctypes.c_int64),
                                   ctypes.POINTER(ctypes.c_int64),
                                   ctypes.POINTER(ctypes.c_int64),
                                   ctypes.POINTER(ctypes.c_int64),
                                   ctypes.c_int64, ctypes.c_int64, ctypes.c_int64,
                                   ctypes.c_double,
                                   ctypes.POINTER(ctypes.c_uint8), ctypes.c_int64]

# Run reference implementation using NumPy version
agg0_ref = np.zeros_like(agg0)
level_n_ref = np.zeros_like(level_n)
level_nnz_ref = np.zeros_like(level_nnz)
nlevels_ref = np.zeros_like(nlevels)
numpy_mod.amg_setup(A_data, A_indices.astype(np.int64), A_indptr.astype(np.int64),
                    level_n_ref, level_nnz_ref, nlevels_ref, agg0_ref,
                    NX, NY, NZ, theta)
print('Ref Depth', int(nlevels_ref[0]))
print('Ref level_n', level_n_ref[:int(nlevels_ref[0])])
print('Ref level_nnz', level_nnz_ref[:int(nlevels_ref[0])])

# Run the C implementation
lib.amg_setup_csr_fp64(A_data.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
                       A_indices.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)),
                       A_indptr.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)),
                       agg0.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)),
                       level_n.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)),
                       level_nnz.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)),
                       nlevels.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)),
                       ctypes.c_int64(NX), ctypes.c_int64(NY), ctypes.c_int64(NZ),
                       ctypes.c_double(theta),
                       workspace_ptr, ctypes.c_int64(need))
print('C Depth', int(nlevels[0]))
print('C level_n', level_n[:int(nlevels[0])])
print('C level_nnz', level_nnz[:int(nlevels[0])])
