#!/usr/bin/env python3
"""How far each floating-point cell's output is from the NumPy oracle (not a timing).

The matrix only records validated=True/False. This probe answers the follow-up question: where the
GCC builds use FMA (C, C++) or not (Fortran's protected parentheses), and Numba neither, do the
outputs actually differ from NumPy's, and by how much? It runs every float cell ONCE through the
harness's own path (``cli._run_cell``, preset M, validate on, repeat 1, no warm-up) and wraps the
harness comparator ``frameworks.utilities.compare_arrays`` to record, per output array, the number
of elements that differ from the oracle at all, the largest relative error and the largest
distance in units in the last place (ULP; complex values per real and imaginary part).

Run on a compute node with the sweep's environment (no uenv; these kernels link no BLAS):
    LLR40_BENCH=<bench 26a4f0cf> python precision_probe.py --out precision_probe.csv
"""
import argparse, csv, os, pathlib, sys

import numpy as np

BENCH = pathlib.Path(os.environ["LLR40_BENCH"]).resolve()
sys.path[:0] = [str(BENCH), str(BENCH / "hpcagent_bench/numpy_translators/src")]
os.chdir(BENCH)
os.environ.update({"PYTHONPATH": f"{BENCH}:{BENCH}/hpcagent_bench/numpy_translators/src",  # the emit subprocess
                   "OMP_NUM_THREADS": "1", "NUMBA_NUM_THREADS": "1", "HPCAGENT_BENCH_MEASUREMENT_WARMUP": "0",
                   "HPCAGENT_BENCH_DB_SHARD": os.environ.get("HPCAGENT_BENCH_DB_SHARD", "190")})

KERNELS = ["quatrex_rgf", "warpx_boris_push", "warpx_esirkepov_deposition", "warpx_field_gather"]
FRAMEWORKS = {"cc": "c", "cpp": "cpp", "fortran": "fortran", "numba": "numba"}


def ulps(a, b):
    """Largest ULP distance between two float64 arrays (finite, same-shape)."""
    ia = a.view(np.int64).astype(np.int64)
    ib = b.view(np.int64).astype(np.int64)
    # map the sign-magnitude bit pattern onto a monotone integer line
    ia = np.where(ia < 0, np.int64(-0x8000000000000000) - ia, ia)
    ib = np.where(ib < 0, np.int64(-0x8000000000000000) - ib, ib)
    return int(np.max(np.abs(ia - ib))) if ia.size else 0


def stats(ref, val):
    r = np.ascontiguousarray(np.asarray(ref)); v = np.ascontiguousarray(np.asarray(val))
    parts = [(r.real.copy(), v.real.copy()), (r.imag.copy(), v.imag.copy())] if np.iscomplexobj(r) else [(r, v)]
    n_diff = 0; max_rel = 0.0; max_ulp = 0
    differs = np.zeros(r.shape, dtype=bool)
    for a, b in parts:
        a = a.astype(np.float64); b = b.astype(np.float64)
        d = ~((a == b) | (np.isnan(a) & np.isnan(b)))
        differs |= d
        if d.any():
            fin = d & np.isfinite(a) & np.isfinite(b)
            if fin.any():
                max_ulp = max(max_ulp, ulps(a[fin], b[fin]))
    n_diff = int(np.count_nonzero(differs))
    if n_diff:
        e = r.astype(np.complex128 if np.iscomplexobj(r) else np.float64)
        g = v.astype(e.dtype)
        den = np.maximum(np.abs(e), np.finfo(np.float64).tiny)
        max_rel = float(np.max(np.where(differs, np.abs(e - g) / den, 0.0)))
    return r.size, n_diff, max_rel, max_ulp


RECORD = []


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--out", required=True); ap.add_argument("--kernels", default=",".join(KERNELS))
    a = ap.parse_args()
    from hpcagent_bench.frameworks import utilities as U
    from hpcagent_bench import cli
    from hpcagent_bench.precision import Precision
    orig = U.compare_arrays

    def wrapped(ref, val, *args, **kw):
        if np.asarray(ref).dtype.kind in "fc":
            RECORD.append(stats(ref, val))
        return orig(ref, val, *args, **kw)

    U.compare_arrays = wrapped
    rows = []
    for k in a.kernels.split(","):
        for fw, col in FRAMEWORKS.items():
            RECORD.clear()
            res = cli._run_cell(k, fw, Precision.FP64, "default", "M", 1, 1800.0, True)
            impls = res.get("impls") or {}
            ok = all(i.get("validated") for i in impls.values()) if impls else False
            for idx, (n, nd, mr, mu) in enumerate(RECORD):
                rows.append({"kernel": k, "column": col, "output_index": idx, "validated": ok, "n_elements": n,
                             "n_differ_from_numpy": nd, "frac_differ": f"{nd / n:.4f}" if n else "",
                             "max_rel_error": f"{mr:.3e}", "max_ulp": mu})
                print(rows[-1], flush=True)
            if not RECORD:
                rows.append({"kernel": k, "column": col, "output_index": "", "validated": ok, "n_elements": "",
                             "n_differ_from_numpy": "", "frac_differ": "", "max_rel_error": "",
                             "max_ulp": f"no comparison recorded: {res.get('status')} {str(res.get('reason', ''))[:200]}"})
                print(rows[-1], flush=True)
    with open(a.out, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)


if __name__ == "__main__":
    main()
