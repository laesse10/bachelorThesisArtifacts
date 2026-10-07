#!/usr/bin/env python3
"""Tables for the DaCe codegen experiment, from rows/*.json (dace_cells.py time) and vec/vec_reports.csv.

  results.csv     one row per timed cell: status, validated, fastest and median of the timed calls (ms),
                  every timing, speedup over the translated C cell of the same kernel
  vec_summary.csv loops GCC vectorises per (kernel, pipeline, codegen, compiler, flag set)
  tables.md       the tables the README quotes

Every series is the harness's Python-side timer (perf_counter around the call), the one series every
column reports at 5cfcb4f2: DaCe's own instrumentation returns no native time here.
"""
import csv
import json
import math
import pathlib
import statistics
from collections import defaultdict

HERE = pathlib.Path(__file__).resolve().parent
BONSALL = ("tsvc_2_s313", "tsvc_2_vdotr", "tsvc_2_s453", "tsvc_2_s314")
CELLS = ("c", "cpp", "canon_old", "canon_legacy", "canon_new", "none_old", "none_legacy", "none_new")


def cell_name(r):
    return r["pipeline"] if r["pipeline"] in ("c", "cpp") else f"{r['pipeline']}_{r['codegen']}"


def load_rows():
    rows = {}
    for f in sorted((HERE / "rows").glob("*.json")):
        r = json.loads(f.read_text())
        impls = r.get("impls") or {}
        impl = next(iter(impls.values()), {})
        series = impl.get("python") or []
        rows[(r["kernel"], cell_name(r))] = {
            "kernel": r["kernel"], "cell": cell_name(r), "status": r["status"],
            "validated": impl.get("validated", ""), "failure": impl.get("failure") or r.get("error", ""),
            "min_ms": min(series) if series else None, "median_ms": statistics.median(series) if series else None,
            "n": len(series), "series_ms": " ".join(f"{v:.4f}" for v in series),
            "node": r.get("node", ""), "slurm_job": r.get("slurm_job", ""),
            "implementation": r.get("implementation", ""), "emit_tree_reductions": r.get("emit_tree_reductions", ""),
            "explicit_copy": r.get("explicit_copy", ""),
        }
    return rows


def geomean(xs):
    xs = [x for x in xs if x and x > 0]
    return math.exp(sum(map(math.log, xs)) / len(xs)) if xs else None


def fmt(x, nd=2):
    return "—" if x is None else f"{x:.{nd}f}"


def main():
    rows = load_rows()
    kernels = sorted({k for k, _ in rows})
    out = []
    def good(r):
        return bool(r) and r["status"] == "ok" and r["validated"] is True and bool(r["min_ms"])

    for (k, c), r in sorted(rows.items()):
        row = dict(r)
        for base in ("c", "cpp"):
            b = rows.get((k, base))
            row[f"speedup_vs_{base}"] = (b["min_ms"] / r["min_ms"]) if good(r) and good(b) else None
        out.append(row)
    with open(HERE / "results.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(out[0]))
        w.writeheader()
        w.writerows(out)
    by = {(r["kernel"], r["cell"]): r for r in out}

    lines = ["# DaCe codegen experiment: tables", "",
             "Fastest of 30 timed calls in ms (preset M, float64, one core). In brackets, for the DaCe cells, the speedup over",
             "translated C++ (`cpp`) of the same kernel at the same commit: both are built by g++ 13, so the ratio is not",
             "confounded by the compiler. Translated C (`c`) is built by gcc 14. ✗: wrong result; –: not run or failed.", ""]
    for title, ks, cells in (("LLR-40, canonicalize pipeline", [k for k in kernels if k not in BONSALL], CELLS[:5]),
                             ("Bonsall's four kernels, both pipelines", [k for k in kernels if k in BONSALL], CELLS)):
        lines += [f"## {title}", "", "| kernel | " + " | ".join(cells) + " |", "|---" * (len(cells) + 1) + "|"]
        for k in ks:
            cols = []
            for c in cells:
                r = by.get((k, c))
                if r is None or r["status"] != "ok" or not r["min_ms"]:
                    cols.append("–")
                elif r["validated"] is not True:
                    cols.append(f"✗ {fmt(r['min_ms'], 1)}")
                elif c in ("c", "cpp"):
                    cols.append(fmt(r["min_ms"], 1))
                else:
                    cols.append(f"{fmt(r['min_ms'], 1)} ({fmt(r['speedup_vs_cpp'])}×)")
            lines.append(f"| {k} | " + " | ".join(cols) + " |")
        for base, label in (("cpp", "C++ (g++ 13)"), ("c", "C (gcc 14)")):
            gm = [geomean([by[(k, c)][f"speedup_vs_{base}"] for k in ks if (k, c) in by]) for c in cells[2:]]
            lines.append(f"| **geomean vs {label}** | | | " + " | ".join(f"**{fmt(g)}×**" for g in gm) + " |")
        lines.append("")
        for a, b in (("canon_old", "canon_legacy"), ("canon_legacy", "canon_new"), ("canon_old", "canon_new")):
            if b not in cells:
                continue
            ratios = [by[(k, a)]["min_ms"] / by[(k, b)]["min_ms"] for k in ks
                      if (k, a) in by and (k, b) in by and good(by[(k, a)]) and good(by[(k, b)])]
            if ratios:
                faster = sum(x > 1.03 for x in ratios); slower = sum(x < 1 / 1.03 for x in ratios)
                lines.append(f"- {b} over {a}: geomean {fmt(geomean(ratios))}×, faster on {faster}, slower on {slower}, "
                             f"within 3% on {len(ratios) - faster - slower} of {len(ratios)} kernels")
        lines.append("")

    vec_path = HERE / "vec" / "vec_reports.csv"
    if vec_path.exists():
        vec = list(csv.DictReader(open(vec_path)))
        with open(HERE / "vec_summary.csv", "w", newline="") as f:
            keep = ["kernel", "pipeline", "codegen", "cxx", "flagset", "compiled", "n_vectorized", "vectorized_loops",
                    "n_missed", "missed_loops", "error"]
            w = csv.DictWriter(f, fieldnames=keep)
            w.writeheader()
            w.writerows({k: r[k] for k in keep} for r in vec)
        idx = {(r["kernel"], r["pipeline"], r["codegen"], r["cxx"], r["flagset"]): r for r in vec}
        lines += ["## Loops GCC vectorises (compile only)", "",
                  "Loops of canon_cpu.cpp reported vectorised, old / legacy / new codegen.", ""]
        for pipe, ks in (("canon", [k for k in kernels if k not in BONSALL] + list(BONSALL)), ("none", list(BONSALL))):
            for cxx in ("g++-13", "g++-14"):
                lines += [f"### {pipe} pipeline, {cxx}", "", "| kernel | harness | bonsall | unsafe |", "|---|---|---|---|"]
                for k in ks:
                    cols = []
                    for fs in ("harness", "bonsall", "unsafe"):
                        vals = []
                        for cg in ("old", "legacy", "new"):
                            r = idx.get((k, pipe, cg, cxx, fs))
                            vals.append("–" if r is None else ("err" if r["compiled"] != "True" else r["n_vectorized"]))
                        cols.append(" / ".join(vals))
                    lines.append(f"| {k} | " + " | ".join(cols) + " |")
                lines.append("")
    (HERE / "tables.md").write_text("\n".join(lines) + "\n")
    print(f"{len(out)} timed cells; tables.md written")


if __name__ == "__main__":
    main()
