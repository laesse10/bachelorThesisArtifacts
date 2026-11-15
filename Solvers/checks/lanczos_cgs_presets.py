"""lanczos_reorth: the reference's two modified Gram-Schmidt passes against two classical passes
(each pass batched into two matrix-vector products), on the benchmark's own operator and inputs at
every preset (S, M, L, XL of the manifest).

    python3 lanczos_cgs_presets.py [S M L XL]

Prints the budget ratio r = max |a - e| / (atol + rtol |e|) of Q, alpha, and beta, in the fp64 band
(rtol = 1e-9, atol = 1e-11) without the raise of atol for reassociated sums, as in Appendix C.
The reference is vectorized per column (numpy dot), so it does not reproduce the reference's own
summation order; the ratio measures how far the change of pass moves the answer.
"""

import pathlib
import sys
import time

import numpy as np
import scipy.sparse as sp

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent / "kernels" / "lanczos_reorth"))
from lanczos_reorth import initialize  # noqa: E402

PRESETS = {"S": (16, 50), "M": (64, 100), "L": (96, 123), "XL": (145, 152)}
RTOL, ATOL = 1e-9, 1e-11


def budget(got, ref):
    return float(np.max(np.abs(got - ref) / (ATOL + RTOL * np.abs(ref))))


def lanczos(A, b, m, mode):
    N = A.shape[0]
    Q = np.zeros((N, m), order="F")
    alpha, beta = np.zeros(m), np.zeros(m)
    Q[:, 0] = b / np.sqrt(b @ b)
    q_prev, beta_prev = np.zeros(N), 0.0
    for j in range(m):
        w = A @ Q[:, j]
        if j > 0:
            w -= beta_prev * q_prev
        a = float(Q[:, j] @ w)
        alpha[j] = a
        w -= a * Q[:, j]
        for _ in range(2):
            if mode == "mgs":
                for p in range(j + 1):
                    w -= float(Q[:, p] @ w) * Q[:, p]
            else:
                w -= Q[:, : j + 1] @ (Q[:, : j + 1].T @ w)
        bj = float(np.sqrt(w @ w))
        beta[j] = bj
        q_prev = Q[:, j].copy()
        if j + 1 < m:
            Q[:, j + 1] = w / bj
        beta_prev = bj
    return Q, alpha, beta


def main(names):
    for name in names:
        edge, m = PRESETS[name]
        indptr, indices, data, b, *_ = initialize(edge, edge, edge, m)
        N = edge**3
        A = sp.csr_matrix((data, indices, indptr), shape=(N, N))
        t = time.time()
        Qr, ar, br = lanczos(A, b, m, "mgs")
        Qc, ac, bc = lanczos(A, b, m, "cgs")
        orth = float(np.max(np.abs(Qc.T @ Qc - np.eye(m))))
        print(
            f"{name:>2} {edge}^3, m={m:>3}:  Q {budget(Qc, Qr):.1e}   alpha {budget(ac, ar):.1e}   "
            f"beta {budget(bc, br):.1e}   max|Q^T Q - I| (CGS) {orth:.1e}   ({time.time() - t:.0f} s)",
            flush=True,
        )
        del Qr, Qc


if __name__ == "__main__":
    main(sys.argv[1:] or list(PRESETS))
