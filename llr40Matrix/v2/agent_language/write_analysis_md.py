#!/usr/bin/env python3
"""Tables for analysis.md, from stats.csv, validity.csv and spearman.csv (run analyze_agents.py first).
Prints Markdown; the prose around it is written by hand in analysis.md."""
import csv, pathlib

HERE = pathlib.Path(__file__).resolve().parent
stats = list(csv.DictReader(open(HERE / "stats.csv")))
val = list(csv.DictReader(open(HERE / "validity.csv")))
sp = list(csv.DictReader(open(HERE / "spearman.csv")))
NAMES = {"best": "best", "first": "FIRST", "median": "median", "bestfirstk": "best of first k"}
SET = [("default", "1", "default build, T1"), ("default", "72", "default build, T72"),
       ("default", "72/identical", "default build, T72, only T72 binary identical to T1"),
       ("gcc14", "1", "GCC 14.2 for both languages, T1")]

print("### Build and validation on aarch64\n")
print("| build | language | model | submissions | build | validate | x86 intrinsics | other build error | incorrect | timeout |")
print("|---|---|---|---:|---:|---:|---:|---:|---:|---:|")
for v in val:
    print(f"| {v['build']} | {v['language']} | {v['model']} | {v['submissions']} | {v['built']} ({float(v['built_share']):.0%}) | "
          f"{v['valid']} ({float(v['valid_share']):.0%}) | {v['build_error_x86_intrinsics']} | {v['build_error_other']} | "
          f"{v['incorrect']} | {v['timeout']} |")
for b, t, title in SET:
    print(f"\n### C against Fortran: {title}\n")
    print("Ratio t_Fortran / t_C per (model, kernel); > 1 means C is faster.\n")
    print("| statistic | model | pairs | geomean F/C | 95% interval | Wilcoxon p | C better | Fortran better | within 10% |")
    print("|---|---|---:|---:|---|---:|---:|---:|---:|")
    for s in stats:
        if s["build"] != b or s["threads"] != t or s["n_pairs"] in ("", "0"):
            continue
        p = s["wilcoxon_p"]
        mark = " **" if p and float(p) < 0.05 else ""
        print(f"| {NAMES[s['statistic']]} | {s['model']} | {s['n_pairs']} | {s['geomean_f_over_c']} | "
              f"{s['ci95_low']}-{s['ci95_high']} | {p}{mark} | {s['c_better']} | {s['fortran_better']} | {s['within_10pct']} |")
print("\n### Campaign speedup (MI300A) against GH200 speedup over translated C, per arm\n")
print("| arm | threads | n | Spearman rho | p |")
print("|---|---:|---:|---:|---:|")
for s in sp:
    print(f"| {s['arm']} | T{s['threads']} | {s['n']} | {s['spearman_rho']} | {s['p']} |")
