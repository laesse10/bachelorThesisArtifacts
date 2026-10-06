#!/usr/bin/env python3
"""Experiment 2b, bit-for-bit: compare each build's saved outputs with the saved NumPy-oracle outputs.

usage: fortran_parens_outputs.py --root DUMP_ROOT --kernel K --out CSV

DUMP_ROOT holds what sweep_followup.py --dump-outputs saved (outdump/sitecustomize.py):
    DUMP_ROOT/oracle/<k>/out<i>.npy                     the oracle's outputs (njit-compiled NumPy reference)
    DUMP_ROOT/default/<k>/<repr>/out<i>.npy, meta.json  c, cpp, fortran from the unpatched worktree
    DUMP_ROOT/noparens/<k>/fortran/out<i>.npy           fortran with -fno-protect-parens
One row per (output, build): bitwise identical?, differing elements, max |abs| and relative
difference, max ULP distance, NaN/Inf pattern mismatches, and the sha256 of both arrays.
Arrays are memory-mapped and compared in chunks, so multi-GB outputs fit in memory.
"""
import argparse, csv, json, os, pathlib, sys
import numpy as np

BUILDS = [("c", "default/{k}/c"), ("cpp", "default/{k}/cpp"),
          ("fortran_default", "default/{k}/fortran"), ("fortran_noparens", "noparens/{k}/fortran")]
CHUNK = 1 << 24
SIGN = np.uint64(0x8000000000000000)


def ordered(bits):
    """float64 bit patterns -> uint64 keys whose order is the float order (ULP distance = |key difference|)."""
    neg = (bits & SIGN) != 0
    return np.where(neg, ~bits, bits | SIGN)


def compare(o, b):
    n = o.size
    o1, b1 = o.reshape(-1), b.reshape(-1)
    res = dict(n=n, n_diff_bits=0, max_abs=0.0, max_rel=0.0, max_ulp=0, n_rel_undefined=0, n_nonfinite_mismatch=0)
    for s in range(0, n, CHUNK):
        oc, bc = np.asarray(o1[s:s + CHUNK]), np.asarray(b1[s:s + CHUNK])
        if oc.dtype.kind != "f":
            res["n_diff_bits"] += int(np.count_nonzero(oc != bc))
            continue
        ob, bb = oc.view(np.uint64), bc.view(np.uint64)
        diff = ob != bb
        nd = int(np.count_nonzero(diff))
        res["n_diff_bits"] += nd
        fin = np.isfinite(oc) & np.isfinite(bc)
        res["n_nonfinite_mismatch"] += int(np.count_nonzero((np.isnan(oc) != np.isnan(bc)) |
                                                            (np.isinf(oc) != np.isinf(bc)) |
                                                            (np.isinf(oc) & np.isinf(bc) & (np.sign(oc) != np.sign(bc)))))
        if nd == 0:
            continue
        m = diff & fin
        if np.any(m):
            d = np.abs(bc[m] - oc[m])
            res["max_abs"] = max(res["max_abs"], float(d.max()))
            ref = np.abs(oc[m])
            ok = ref > 0
            res["n_rel_undefined"] += int(np.count_nonzero(~ok))
            if np.any(ok):
                res["max_rel"] = max(res["max_rel"], float((d[ok] / ref[ok]).max()))
            ko, kb = ordered(ob[m]), ordered(bb[m])
            u = np.where(ko > kb, ko - kb, kb - ko)
            res["max_ulp"] = max(res["max_ulp"], int(u.max()))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--bench", default=os.environ.get("LLR40_BENCH", ""))
    a = ap.parse_args()
    root, k = pathlib.Path(a.root), a.kernel
    import yaml
    man = yaml.safe_load(open(pathlib.Path(a.bench) / "hpcagent_bench/benchmarks/loop_level_reasoning" / k / f"{k}.yaml"))
    names = man["output_args"]
    rows = []
    for build, sub in BUILDS:
        d = root / sub.format(k=k)
        meta = json.load(open(d / "meta.json")) if (d / "meta.json").is_file() else None
        for i, name in enumerate(names):
            row = {"kernel": k, "output": name, "build": build}
            op, bp = root / "oracle" / k / f"out{i}.npy", d / f"out{i}.npy"
            if meta is None or not bp.is_file() or not op.is_file():
                row["status"] = "missing"; rows.append(row); continue
            o, b = np.load(op, mmap_mode="r"), np.load(bp, mmap_mode="r")
            m = meta["outputs"][i]
            row.update(status="ok", harness_validated=meta["validated"], rtol=meta["rtol"], atol=meta["atol"],
                       shape="x".join(map(str, b.shape)), dtype=str(b.dtype),
                       oracle_identical_across_runs=m["oracle_identical_to_saved"],
                       sha256_build=m["sha256"], sha256_oracle=m["oracle_sha256"])
            if o.shape != b.shape or o.dtype != b.dtype:
                row["status"] = "shape_or_dtype_mismatch"; rows.append(row); continue
            r = compare(o, b)
            row.update(n_elements=r["n"], bitwise_identical="yes" if r["n_diff_bits"] == 0 else "no",
                       n_differing=r["n_diff_bits"], max_abs_diff=repr(r["max_abs"]), max_rel_diff=repr(r["max_rel"]),
                       max_ulp=r["max_ulp"], n_rel_undefined_oracle_zero=r["n_rel_undefined"],
                       n_nonfinite_mismatch=r["n_nonfinite_mismatch"])
            rows.append(row)
            print(f"{k:26s} {name:8s} {build:17s} identical={row['bitwise_identical']:3s} diff={r['n_diff_bits']:>10d} "
                  f"max_abs={r['max_abs']:.3e} max_rel={r['max_rel']:.3e} max_ulp={r['max_ulp']}", flush=True)
    fields = ["kernel", "output", "build", "status", "harness_validated", "rtol", "atol", "shape", "dtype", "n_elements",
              "bitwise_identical", "n_differing", "max_abs_diff", "max_rel_diff", "max_ulp",
              "n_rel_undefined_oracle_zero", "n_nonfinite_mismatch", "oracle_identical_across_runs",
              "sha256_build", "sha256_oracle"]
    pathlib.Path(a.out).parent.mkdir(parents=True, exist_ok=True)
    with open(a.out, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=fields); w.writeheader()
        for row in rows:
            w.writerow({f: row.get(f, "") for f in fields})


if __name__ == "__main__":
    sys.exit(main())
