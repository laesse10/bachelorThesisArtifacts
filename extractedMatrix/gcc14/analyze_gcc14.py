#!/usr/bin/env python3
"""The extracted kernels with GCC 14.2 for every language (job 5121173): ratios over translated C.

Reads parts/*.csv (sweep_gcc14.py, three interleaved rounds per kernel on one node) and ../results.csv (the
matrix, job 4990441 and DEVIATIONS 9). Per cell, the median over all timed runs of the three rounds (90 runs).
Checks that every library carries the compiler its label asks for (``lib_comment``): c, cpp14, fortran14 GCC
14.2; cpp, fortran GCC 13.3. Writes cells.csv and summary.md.
"""
import csv
import math
import pathlib
import statistics

HERE = pathlib.Path(__file__).resolve().parent
KERNELS = ("comet_int4_gemm", "quatrex_rgf", "spgemm_hash", "warpx_boris_push", "warpx_esirkepov_deposition",
           "warpx_field_gather", "triangle_count", "nfa_frontier")
LABELS = ("c", "cpp", "cpp14", "fortran", "fortran14")
WANT = {"c": "14.2", "cpp": "13.3", "cpp14": "14.2", "fortran": "13.3", "fortran14": "14.2"}


def geomean_ci(values):
    from scipy.stats import t
    lg = [math.log(v) for v in values]
    n, m = len(lg), statistics.mean(lg)
    h = t.ppf(0.975, n - 1) * statistics.stdev(lg) / math.sqrt(n)
    return math.exp(m), math.exp(m - h), math.exp(m + h)


def main():
    runs, stamps, status = {}, {}, {}
    for k in KERNELS:
        for r in csv.DictReader(open(HERE / "parts" / f"{k}.csv")):
            key = (k, r["label"])
            status.setdefault(key, set()).add(r["status"])
            if r["status"] == "ok" and r["time_ns_all"]:
                runs.setdefault(key, []).extend(float(x) for x in r["time_ns_all"].split())
            stamps.setdefault(key, set()).add(r["lib_comment"])
    bad_stamp = [(k, l, s) for (k, l), s in stamps.items() if not any(WANT[l] in x for x in s)]
    matrix = {(r["kernel"], r["representation"]): r for r in csv.DictReader(open(HERE.parent / "results.csv"))}

    def mat_ratio(k, col):
        c, x = matrix.get((k, "c")), matrix.get((k, col))
        if not (c and x and c["time_ns_all"] and x["time_ns_all"]):
            return math.nan
        med = lambda row: statistics.median(float(v) for v in row["time_ns_all"].split())
        return med(c) / med(x)

    rows = []
    for k in KERNELS:
        c = statistics.median(runs[(k, "c")])
        row = {"kernel": k, "c_ms": c / 1e6}
        for lab in LABELS[1:]:
            row[lab] = c / statistics.median(runs[(k, lab)])
        row["matrix_cpp"] = mat_ratio(k, "cpp")
        row["matrix_fortran"] = mat_ratio(k, "fortran")
        rows.append(row)
    with open(HERE / "cells.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows({key: (f"{v:.4f}" if isinstance(v, float) else v) for key, v in r.items()} for r in rows)

    lines = ["# Extracted kernels with GCC 14.2 for every language (job 5121173)", "",
             "Speedup over translated C (GCC 14.2), median of the 90 timed runs of three interleaved rounds per cell, "
             "one node per kernel. `cpp`/`fortran`: GCC 13.3 (the benchmark's default build); `cpp14`/`fortran14`: the "
             "same sources built by GCC 14.2. `matrix`: the same ratio in the matrix (job 4990441, medians).", "",
             "| kernel | C (ms) | C++ 13 | C++ 14 | Fortran 13 | Fortran 14 | matrix C++ | matrix Fortran |",
             "|---|---:|---:|---:|---:|---:|---:|---:|"]
    for r in rows:
        lines.append(f"| `{r['kernel']}` | {r['c_ms']:.3f} | {r['cpp']:.2f} | {r['cpp14']:.2f} | {r['fortran']:.2f} | "
                     f"{r['fortran14']:.2f} | {r['matrix_cpp']:.2f} | {r['matrix_fortran']:.2f} |")
    lines += ["", "| column | geomean over C | 95% CI | within 3% of C |", "|---|---:|---|---:|"]
    for lab in LABELS[1:]:
        v = [r[lab] for r in rows]
        g, lo, hi = geomean_ci(v)
        lines.append(f"| {lab} | {g:.3f} | {lo:.3f} - {hi:.3f} | {sum(1 / 1.03 <= x <= 1.03 for x in v)} of {len(v)} |")
    lines += ["", f"Cell statuses: {sorted({s for v in status.values() for s in v})}. Compiler stamps that do not match "
              f"their label: {bad_stamp or 'none'}."]
    (HERE / "summary.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
