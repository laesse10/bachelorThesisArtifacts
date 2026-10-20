#!/usr/bin/env python3
"""numba021 cells beside the matrix: the v0.2.1-translated Numba cells (job 5105398) and the C cell
re-timed in the same job, against results.csv (job 4990441 and DEVIATIONS 9).

Checks first that the timed sources are the intended bytes: the Numba module is the v0.2.1 file from
sources/, and the re-timed C is byte-identical to the matrix's emitted C. Speedups are min over min,
as analyze_extracted.py. Writes cells.csv and columns.csv here.
"""
import csv, hashlib, math, pathlib, statistics

HERE = pathlib.Path(__file__).resolve().parent
MATRIX = HERE.parent
sha = lambda p: hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()


def ns(r):
    return [int(x) for x in r["time_ns_all"].split()] if r and r["time_ns_all"] else []


def tq975(df):
    from scipy.stats import t
    return float(t.ppf(0.975, df))


for k in ("warpx_field_gather", "warpx_esirkepov_deposition"):
    timed = HERE / "emitted_sources" / k / f"{k}_numba_np.py"
    assert sha(timed) == sha(HERE / "sources" / f"{k}_numba.v021.py"), f"{k}: timed numba is not the v0.2.1 file"
fg_c = "warpx_field_gather/warpx_field_gather_fp64.c"
assert sha(HERE / "emitted_sources" / fg_c) == sha(MATRIX / "emitted_sources" / fg_c), "re-timed C differs"

matrix = {(r["kernel"], r["representation"]): r for r in csv.DictReader(open(MATRIX / "results.csv"))}
new = {}
for k in ("warpx_field_gather", "warpx_esirkepov_deposition"):
    for r in csv.DictReader(open(HERE / "parts" / f"{k}.csv")):
        new[(k, r["representation"])] = r

out = []
for (k, col), r in new.items():
    t = ns(r)
    mn = min(t) if t else None
    c_mat = min(ns(matrix[(k, "c")]))
    c_job = min(ns(new[(k, "c")])) if (k, "c") in new else None
    out.append({"kernel": k, "column": col, "status": r["status"], "slurm_job": r["slurm_job"], "node": r["node"],
                "n_reps": len(t), "min_ms": f"{mn / 1e6:.4f}" if mn else "",
                "median_ms": f"{statistics.median(t) / 1e6:.4f}" if t else "",
                "rsd_pct": f"{100 * statistics.pstdev(t) / statistics.mean(t):.2f}" if len(t) > 1 else "",
                "matrix_min_ms": (f"{min(ns(matrix[(k, col)])) / 1e6:.4f}"
                                  if (k, col) in matrix and ns(matrix[(k, col)]) else ""),
                "speedup_over_matrix_c": f"{c_mat / mn:.3f}" if mn and r["status"] == "ok" else "",
                "speedup_over_job_c": f"{c_job / mn:.3f}" if mn and c_job and r["status"] == "ok" else "",
                "source_sha256": r["source_sha256"], "notes": r["notes"][:300]})
with open(HERE / "cells.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(out[0].keys())); w.writeheader(); w.writerows(out)

# the matrix's numba column (translator output, 5 kernels) with warpx_field_gather added
cells = list(csv.DictReader(open(MATRIX / "cells.csv")))
base = {o["kernel"]: float(o["speedup_over_c"]) for o in cells if o["column"] == "numba" and o["speedup_over_c"]}
fg = next(o for o in out if o["kernel"] == "warpx_field_gather" and o["column"] == "numba")
hand = {o["kernel"]: float(o["speedup_over_c"]) for o in cells if o["column"] == "numba_hand" and o["speedup_over_c"]}
cols = []
for name, sp in [("numba (matrix, 26a4f0cf translator)", dict(base)),
                 ("numba + field_gather v0.2.1 (vs matrix c)", {**base, "warpx_field_gather": float(fg["speedup_over_matrix_c"])}),
                 ("numba + field_gather v0.2.1 (vs job c)", {**base, "warpx_field_gather": float(fg["speedup_over_job_c"])}),
                 ("numba + field_gather v0.2.1 - comet (vs matrix c)",
                  {k: v for k, v in {**base, "warpx_field_gather": float(fg["speedup_over_matrix_c"])}.items()
                   if k != "comet_int4_gemm"}),
                 ("numba + numba_hand + field_gather v0.2.1 (vs matrix c)",
                  {**base, **hand, "warpx_field_gather": float(fg["speedup_over_matrix_c"])})]:
    lg = [math.log(x) for x in sp.values()]
    n = len(lg); gm = math.exp(statistics.mean(lg)); h = tq975(n - 1) * statistics.stdev(lg) / math.sqrt(n)
    v = list(sp.values())
    cols.append({"column": name, "n": n, "geomean_speedup_over_c": f"{gm:.3f}",
                 "ci95_low": f"{math.exp(statistics.mean(lg) - h):.3f}", "ci95_high": f"{math.exp(statistics.mean(lg) + h):.3f}",
                 "faster": sum(x > 1.03 for x in v), "within_3pct": sum(1 / 1.03 <= x <= 1.03 for x in v),
                 "slower": sum(x < 1 / 1.03 for x in v), "kernels": " ".join(sp)})
with open(HERE / "columns.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(cols[0].keys())); w.writeheader(); w.writerows(cols)
for o in out:
    print({k: o[k] for k in ("kernel", "column", "status", "n_reps", "min_ms", "median_ms", "rsd_pct", "matrix_min_ms",
                             "speedup_over_matrix_c", "speedup_over_job_c")})
for c in cols:
    print(c)
