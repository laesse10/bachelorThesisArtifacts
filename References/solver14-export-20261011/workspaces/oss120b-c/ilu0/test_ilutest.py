import numpy as np, ctypes, importlib.util

# Load reference implementation
spec = importlib.util.spec_from_file_location('ilu0_numpy', '/shared/agent-5/ilu0_numpy.py')
ilu0_mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ilu0_mod)
ilu0_ref = ilu0_mod.ilu0

# Small test matrix
N = 8
rng = np.random.default_rng(42)
# Generate a dense symmetric positive definite matrix
A_dense = rng.random((N, N))
A_dense = A_dense + A_dense.T + N * np.eye(N)
# Random sparsity pattern, keep diagonal
mask = rng.random(A_dense.shape) < 0.6
mask[np.arange(N), np.arange(N)] = True
A_sparse = np.where(mask, A_dense, 0.0)
# Build CSR arrays
A_indptr = np.empty(N + 1, dtype=np.int64)
indices = []
values = []
idx = 0
for i in range(N):
    A_indptr[i] = idx
    for j in range(N):
        if A_sparse[i, j] != 0:
            indices.append(j)
            values.append(A_sparse[i, j])
            idx += 1
A_indptr[N] = idx
A_indices = np.array(indices, dtype=np.int64)
A_data = np.array(values, dtype=np.float64)

# Run reference implementation
A_data_ref = A_data.copy()
ilu0_ref(A_data_ref, A_indices, A_indptr, N)

# Load compiled C library
lib = ctypes.CDLL('/shared/agent-5/libilu0.so')
lib.ilu0_fp64.argtypes = [ctypes.POINTER(ctypes.c_double),
                         ctypes.POINTER(ctypes.c_longlong),
                         ctypes.POINTER(ctypes.c_longlong),
                         ctypes.c_longlong,
                         ctypes.c_longlong,
                         ctypes.POINTER(ctypes.c_uint8),
                         ctypes.c_longlong]
# Allocate workspace (diag + row_val + in_row)
bytes_needed = N * (ctypes.sizeof(ctypes.c_double) * 2 + ctypes.sizeof(ctypes.c_uint8))
workspace = ctypes.create_string_buffer(bytes_needed)
workspace_ptr = ctypes.cast(workspace, ctypes.POINTER(ctypes.c_uint8))

# Run C implementation
A_data_c = A_data.copy()
lib.ilu0_fp64(A_data_c.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
              A_indices.ctypes.data_as(ctypes.POINTER(ctypes.c_longlong)),
              A_indptr.ctypes.data_as(ctypes.POINTER(ctypes.c_longlong)),
              ctypes.c_longlong(N),
              ctypes.c_longlong(A_data_c.size),
              workspace_ptr,
              ctypes.c_longlong(bytes_needed))

# Compare results
if np.allclose(A_data_ref, A_data_c, rtol=1e-12, atol=1e-14):
    print('PASS: C implementation matches reference')
else:
    diff = np.abs(A_data_ref - A_data_c)
    print('FAIL: max diff =', diff.max())
    # Print mismatched indices
    idxs = np.where(diff > 1e-12)[0]
    for i in idxs[:10]:
        print(i, A_data_ref[i], A_data_c[i])
