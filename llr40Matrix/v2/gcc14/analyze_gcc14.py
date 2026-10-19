#!/usr/bin/env python3
"""Tables of the GCC 14 experiment (parts/*.csv from sweep_gcc14.py): C, C++ and Fortran all built by GCC 14.2,
next to the matrix's default build (C++ and Fortran by GCC 13.3).

Per (kernel, label): the median of all timed runs of the three rounds (90 runs), with the distribution-free 95%
interval of that median, and the ratios that matter here:

  cpp14_over_c          t(c) / t(cpp14)          C++ against C, both GCC 14.2
  fortran14_over_c      t(c) / t(fortran14)      Fortran against C, both GCC 14.2
  cpp_over_c            t(c) / t(cpp)            the matrix's comparison (C++ by GCC 13.3)
  fortran_over_c        t(c) / t(fortran)        the matrix's comparison (Fortran by GCC 13.3)
  cpp14_over_cpp        t(cpp) / t(cpp14)        GCC 14 against GCC 13, C++
  fortran14_over_fortran t(fortran) / t(fortran14) GCC 14 against GCC 13, Fortran

> 1 means the second-named build is slower. Writes results.csv (one row per cell and round, all timings),
cells.csv (one row per kernel and label), ratios.csv (one row per kernel) and summary.md.

usage: python3 analyze_clang.py
"""
import csv, glob, math, pathlib, statistics
from collections import defaultdict

HERE = pathlib.Path(__file__).resolve().parent
TIE = 0.03


def median_interval(t, confidence=0.95):
    from scipy.stats import binom
    t, n, j = sorted(t), len(t), 1
    while j + 1 <= n // 2 and binom.cdf(n - j - 1, n, 0.5) - binom.cdf(j, n, 0.5) >= confidence:
        j += 1
    return t[j - 1], t[n - j]


def geomean_ci(xs):
    from scipy.stats import t as T
    lx = [math.log(x) for x in xs]
    m = statistics.mean(lx)
    h = T.ppf(0.975, len(lx) - 1) * statistics.stdev(lx) / math.sqrt(len(lx))
    return math.exp(m), math.exp(m - h), math.exp(m + h)


rows = []
for f in sorted(glob.glob(str(HERE / "parts" / "*.csv"))):
    rows += list(csv.DictReader(open(f)))
if not rows:
    raise SystemExit("no rows in parts/")
with open(HERE / "results.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)

runs, status, libc, perf = defaultdict(list), defaultdict(set), defaultdict(set), defaultdict(lambda: defaultdict(int))
for r in rows:
    key = (r["kernel"], r["label"])
    status[key].add(r["status"])
    if r.get("lib_comment"):
        libc[key].add(r["lib_comment"])
    if r["status"] == "ok":
        runs[key] += [int(x) for x in r["time_ns_all"].split()]
        for c in ("perf_cycles", "perf_instructions", "perf_branch_misses"):
            if r.get(c) not in ("", None):
                perf[key][c] += int(float(r[c]))
        perf[key]["reps"] += int(r.get("perf_gated_reps", "0").split(";")[0] or 0)

cells = []
for key in sorted(status):
    t = runs.get(key, [])
    lo, hi = median_interval(t) if len(t) >= 6 else (None, None)
    cells.append({"kernel": key[0], "label": key[1], "status": "/".join(sorted(status[key])), "n_runs": len(t),
                  "median_ms": f"{statistics.median(t) / 1e6:.4f}" if t else "",
                  "ci95_lo_ms": f"{lo / 1e6:.4f}" if lo else "", "ci95_hi_ms": f"{hi / 1e6:.4f}" if hi else "",
                  "lib_comment": " | ".join(sorted(libc.get(key, []))),
                  **{c: perf[key][c] for c in ("perf_cycles", "perf_instructions", "perf_branch_misses", "reps")}})
with open(HERE / "cells.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(cells[0].keys())); w.writeheader(); w.writerows(cells)

med = {(c["kernel"], c["label"]): float(c["median_ms"]) for c in cells if c["median_ms"] and c["status"] == "ok"}
PAIRS = {"cpp14_over_c": ("c", "cpp14"), "fortran14_over_c": ("c", "fortran14"),
         "cpp_over_c": ("c", "cpp"), "fortran_over_c": ("c", "fortran"),
         "cpp14_over_cpp": ("cpp", "cpp14"), "fortran14_over_fortran": ("fortran", "fortran14")}
kernels = sorted({c["kernel"] for c in cells})
ratios = []
for k in kernels:
    row = {"kernel": k}
    for name, (base, other) in PAIRS.items():
        a, b = med.get((k, base)), med.get((k, other))
        row[name] = f"{a / b:.4f}" if a and b else ""
    ratios.append(row)
with open(HERE / "ratios.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["kernel", *PAIRS]); w.writeheader(); w.writerows(ratios)

out = ["# LLR-40 with GCC 14.2 for C, C++ and Fortran", "",
       "Median of the 90 timed runs per cell (three interleaved rounds of 30, one exclusive node per kernel).",
       "A ratio above 1: the named build is faster than its baseline.", ""]
out += ["| comparison | n | geomean | 95% CI | faster / within 3% / slower |", "|---|---:|---:|---|---|"]
for name in PAIRS:
    xs = [float(r[name]) for r in ratios if r[name]]
    if len(xs) >= 2:
        g, lo, hi = geomean_ci(xs)
        f = sum(x > 1 + TIE for x in xs); s = sum(x < 1 - TIE for x in xs)
        out.append(f"| {name} | {len(xs)} | {g:.3f} | {lo:.3f} - {hi:.3f} | {f} / {len(xs) - f - s} / {s} |")
within = {}
for tag, labels in (("GCC 14 for all", ("c", "cpp14", "fortran14")), ("default build", ("c", "cpp", "fortran"))):
    n = ok = 0
    for k in kernels:
        ts = [med.get((k, l)) for l in labels]
        if all(ts):
            n += 1; ok += max(ts) / min(ts) <= 1.05
    within[tag] = (ok, n)
out += ["", "C, C++ and Fortran within 5% of each other: " +
        "; ".join(f"{t}: {a} of {b} kernels" for t, (a, b) in within.items())]
out += ["", "| kernel | " + " | ".join(PAIRS) + " |", "|---|" + "---:|" * len(PAIRS)]
for r in ratios:
    out.append(f"| `{r['kernel']}` | " + " | ".join(r[p] for p in PAIRS) + " |")
bad = [c for c in cells if c["status"] != "ok"]
out += ["", "Cells not ok: " + (", ".join(f"`{c['kernel']}`/{c['label']} ({c['status']})" for c in bad) or "none")]
want = {"c": "14.2", "cpp14": "14.2", "fortran14": "14.2", "cpp": "13.3", "fortran": "13.3"}
wrong = [f"{c['kernel']}/{c['label']}" for c in cells if c["status"] == "ok" and want[c["label"]] not in c["lib_comment"]]
out += ["", "Cells whose library does not carry the expected GCC version: " + (", ".join(wrong) or "none")]
(HERE / "summary.md").write_text("\n".join(out) + "\n")
print("\n".join(out))
