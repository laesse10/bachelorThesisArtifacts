#!/usr/bin/env python3
"""DaCe against translated C++, GCC 13 (../rows, job 4992604) and GCC 14.2 (rows/, job 5121174), same method.

Per cell, the median of its 30 timed calls; per kernel, C++ time over DaCe time (canon pipeline, new code
generator, the DaCe column) on the 40 LLR-40 kernels; geometric mean with a 95% log-t interval. Also the
code-generator table (old, legacy, new) and the kernels whose times moved by more than 10% between the compilers.
Writes summary.md.
"""
import json, math, pathlib, statistics
from scipy.stats import t

HERE = pathlib.Path(__file__).resolve().parent


def load(d):
    out = {}
    for f in pathlib.Path(d).glob("*.json"):
        r = json.loads(f.read_text())
        cell = r["pipeline"] if r["pipeline"] in ("c", "cpp") else f"{r['pipeline']}_{r['codegen']}"
        impl = next(iter((r.get("impls") or {}).values()), {})
        s = impl.get("python") or []
        out[(r["kernel"], cell)] = statistics.median(s) if (r["status"] == "ok" and impl.get("validated") and s) else None
    return out


def gm(v):
    lg = [math.log(x) for x in v]; n = len(lg); m = statistics.mean(lg)
    h = t.ppf(.975, n - 1) * statistics.stdev(lg) / math.sqrt(n)
    return math.exp(m), math.exp(m - h), math.exp(m + h)


g13, g14 = load(HERE.parent / "rows"), load(HERE / "rows")
roster = [l.strip() for l in open(HERE.parent.parent / "roster40.txt") if l.strip()]
lines = ["# DaCe against translated C++ with GCC 13 and with GCC 14.2", "",
         "| build | DaCe over C++ (geomean, 95% CI) | kernels differing by > 3% |", "|---|---|---:|"]
for name, d in (("GCC 13.3 (job 4992604)", g13), ("GCC 14.2 (job 5121174)", g14)):
    r = [d[(k, "cpp")] / d[(k, "canon_new")] for k in roster]
    g = gm(r)
    lines.append(f"| {name} | {g[0]:.2f} [{g[1]:.2f}, {g[2]:.2f}] | {sum(abs(math.log(x)) > math.log(1.03) for x in r)} of 40 |")
lines += ["", "Kernels whose C++ or DaCe time moved by more than 10% (ms, median):", "",
          "| kernel | C++ 13 | C++ 14 | DaCe 13 | DaCe 14 | ratio 13 | ratio 14 |", "|---|---:|---:|---:|---:|---:|---:|"]
for k in roster:
    c13, c14, d13, d14 = g13[(k, "cpp")], g14[(k, "cpp")], g13[(k, "canon_new")], g14[(k, "canon_new")]
    if max(abs(math.log(c14 / c13)), abs(math.log(d14 / d13))) > math.log(1.10):
        lines.append(f"| `{k}` | {c13:.1f} | {c14:.1f} | {d13:.1f} | {d14:.1f} | {c13 / d13:.2f} | {c14 / d14:.2f} |")
lines += ["", "Code-generator configurations (ms, median; GCC 13 / GCC 14.2):", "",
          "| kernel | pipeline | C++ | old | legacy | new |", "|---|---|---:|---:|---:|---:|"]
for k, pipes in (("tsvc_2_s3111", ["canon"]), ("tsvc_2_s319", ["canon"]), ("tsvc_2_s313", ["canon", "none"]),
                 ("tsvc_2_vdotr", ["canon", "none"])):
    for pp in pipes:
        cells = ("cpp", f"{pp}_old", f"{pp}_legacy", f"{pp}_new")
        lines.append(f"| `{k}` | {pp} | " + " | ".join(f"{g13[(k, c)]:.1f} / {g14[(k, c)]:.1f}" for c in cells) + " |")
stamps = sorted({line.strip() for f in (HERE / "comment").glob("*.txt") for line in f.read_text().splitlines()
                 if "GCC:" in line})
lines += ["", f"Compiler stamps of the 232 libraries built in job 5121174: {stamps}."]
(HERE / "summary.md").write_text("\n".join(lines) + "\n")
print("\n".join(lines))
