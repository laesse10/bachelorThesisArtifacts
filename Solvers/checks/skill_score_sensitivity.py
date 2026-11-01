"""Does the skill comparison depend on uncredited answers scoring 1?

The benchmark scores a wrong, missing or sanitizer-rejected answer 1 and a correct but slower one
below 1 (Section 6.6). This repeats each setting's comparison with its model's no-skill setting in two
ways that remove that asymmetry: only kernels credited in both settings, and every score below 1
raised to 1. Wilcoxon signed-rank test on the log ratios. Reads the solver14-precond snapshot.
"""
import csv
import math
import pathlib

from scipy.stats import wilcoxon

ROOT = pathlib.Path(__file__).resolve().parents[2] / "References"
DATA = next(p for p in (ROOT / "solver14-report" / "speedup.csv", ROOT / "old runs" / "solver14-report" / "speedup.csv")
            if p.is_file())
score = {}
for r in csv.DictReader(open(DATA)):
    if r["bench_speedup"] not in ("", "nan"):
        score[(r["model"], r["variant"], r["kernel"])] = (float(r["bench_speedup"]), r["bench_solved"] == "True")


def gm(x):
    return math.exp(sum(map(math.log, x)) / len(x))


def test(ratios):
    logs = [math.log(x) for x in ratios if x != 1]
    return (sum(x > 1 for x in ratios), sum(x < 1 for x in ratios),
            wilcoxon(logs).pvalue if len(logs) > 1 else float("nan"))


for model in ("oss120b", "qwen38"):
    for var in ("preconditioner-v4-single", "preconditioner-v4-split"):
        ks = [k for (m, v, k) in score if m == model and v == var and (model, "none", k) in score]
        if len(ks) < 14:
            print(f"{model} {var}: {len(ks)} of 14 kernels at the snapshot, skipped")
            continue
        s, n = (lambda k: score[(model, var, k)]), (lambda k: score[(model, "none", k)])
        both = [s(k)[0] / n(k)[0] for k in ks if s(k)[1] and n(k)[1]]
        raised = [max(s(k)[0], 1) / max(n(k)[0], 1) for k in ks]
        print(f"{model} {var}: as scored {gm([s(k)[0] / n(k)[0] for k in ks]):.2f} | "
              f"credited in both n={len(both)} {gm(both):.2f} W/L %d/%d p=%.2f | " % test(both)[:3]
              + f"raised to 1 {gm(raised):.2f} W/L %d/%d p=%.2f" % test(raised))
