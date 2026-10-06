#!/usr/bin/env python3
"""Part A analysis: per-cell table, per-column geometric means against translated C, vectorisation
counts per GCC column. Writes cells.csv, columns.csv, vectorisation.csv; summary.md quotes them.

speedup over translated C = min-of-k(c) / min-of-k(cell), per kernel, both cells `ok`.
Per column: the geometric mean of that speedup over the kernels where both are ok, with a 95% interval
from the t distribution on the log ratios (n-1 degrees of freedom; none for n < 2), and the counts
faster (speedup > 1.03) / within 3% / slower (< 1/1.03).
"""
import csv, math, pathlib, re, statistics

HERE = pathlib.Path(__file__).resolve().parent
rows = list(csv.DictReader(open(HERE / "results.csv")))
ORDER = ["comet_int4_gemm", "quatrex_rgf", "spgemm_hash", "warpx_boris_push", "warpx_esirkepov_deposition",
         "warpx_field_gather", "triangle_count", "nfa_frontier"]
COLS = ["c", "cpp", "fortran", "numba", "numba_hand", "native"]
cell = {(r["kernel"], r["representation"]): r for r in rows}


def ns(r):
    return [int(x) for x in r["time_ns_all"].split()] if r and r["time_ns_all"] else []


def tq975(df):
    from scipy.stats import t
    return float(t.ppf(0.975, df))


out = []
for k in ORDER:
    c = cell.get((k, "c"))
    tc = min(ns(c)) if c and c["status"] == "ok" else None
    for col in COLS:
        r = cell.get((k, col))
        if not r:
            continue
        t = ns(r)
        mn = min(t) if t else None
        out.append({"kernel": k, "column": col, "status": r["status"], "provenance": r["provenance"],
                    "adapter": r["adapter"], "n_reps": len(t), "min_ms": f"{mn / 1e6:.4f}" if mn else "",
                    "median_ms": f"{statistics.median(t) / 1e6:.4f}" if t else "",
                    "rsd_pct": f"{100 * statistics.pstdev(t) / statistics.mean(t):.2f}" if len(t) > 1 else "",
                    "speedup_over_c": f"{tc / mn:.3f}" if (tc and mn and r["status"] == "ok") else "",
                    "below_10ms_per_call": ("yes" if mn < 10e6 else "no") if mn else "",
                    "notes": r["notes"][:300]})
with open(HERE / "cells.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(out[0].keys())); w.writeheader(); w.writerows(out)

cols = []
for col in ["cpp", "fortran", "numba", "numba_hand", "native", "numba+numba_hand"]:
    members = col.split("+")
    sp = [float(o["speedup_over_c"]) for o in out if o["column"] in members and o["speedup_over_c"]]
    kernels = [o["kernel"] for o in out if o["column"] in members and o["speedup_over_c"]]
    n = len(sp)
    if n == 0:
        cols.append({"column": col, "n": 0}); continue
    lg = [math.log(x) for x in sp]
    gm = math.exp(statistics.mean(lg))
    if n >= 2:
        h = tq975(n - 1) * statistics.stdev(lg) / math.sqrt(n)
        lo, hi = math.exp(statistics.mean(lg) - h), math.exp(statistics.mean(lg) + h)
    else:
        lo = hi = None
    cols.append({"column": col, "n": n, "geomean_speedup_over_c": f"{gm:.3f}",
                 "ci95_low": f"{lo:.3f}" if lo else "", "ci95_high": f"{hi:.3f}" if hi else "",
                 "faster": sum(x > 1.03 for x in sp), "within_3pct": sum(1 / 1.03 <= x <= 1.03 for x in sp),
                 "slower": sum(x < 1 / 1.03 for x in sp), "kernels": " ".join(kernels)})
with open(HERE / "columns.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["column", "n", "geomean_speedup_over_c", "ci95_low", "ci95_high", "faster",
                                       "within_3pct", "slower", "kernels"])
    w.writeheader(); w.writerows(cols)

# vectorisation decisions per GCC column, from the opt reports
opt = {}
for f in sorted(HERE.glob("parts/optrep.*.csv")):
    for r in csv.DictReader(open(f)):
        opt[(r["kernel"], r["representation"])] = r
vec = []
for k in ORDER:
    rec = {"kernel": k}
    for col in ("c", "cpp", "fortran", "native"):
        o = opt.get((k, col))
        if not o:
            continue
        rec[f"{col}_status"] = o["status"]
        rec[f"{col}_vectorized"] = o["n_vec_optimized"]
        rec[f"{col}_missed"] = o["n_vec_missed"]
        if o.get("optreport") and (HERE / o["optreport"]).is_file():
            lines = (HERE / o["optreport"]).read_text().splitlines()
            first = next(((i, l) for i, l in enumerate(lines, 1) if "loop vectorized" in l), None)
            rec[f"{col}_first_vectorized"] = f"{o['optreport']}:{first[0]}: {first[1].split(': ', 1)[-1]}" if first else ""
    vec.append(rec)
keys = sorted({k for r in vec for k in r}, key=lambda x: (x != "kernel", x))
with open(HERE / "vectorisation.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=keys); w.writeheader(); w.writerows(vec)

for o in out:
    print(f"{o['kernel']:28s} {o['column']:11s} {o['status']:12s} {o['min_ms']:>12s} rsd={o['rsd_pct']:>6s} "
          f"x_c={o['speedup_over_c']:>6s}")
print()
for c in cols:
    print(c)
print()
for v in vec:
    print({k: v[k] for k in v if k.endswith("vectorized") or k == "kernel"})
