#!/usr/bin/env python3
"""Concatenate the per-kernel array-task CSVs into one results.csv, in roster order."""
import csv, pathlib, sys
REPO = pathlib.Path(__file__).resolve().parent
FIELDS = ["kernel", "representation", "preset", "status", "time_ns_median", "time_ns_min", "time_ns_all", "compiler",
          "compiler_version", "flags", "threads", "n_warmup", "n_reps", "commit_hash",
          "timestamp", "notes"]
roster = [l.strip() for l in open(REPO / "roster40.txt") if l.strip()]
rows, missing = [], []
for k in roster:
    p = REPO / "results_parts" / f"{k}.csv"
    if not p.is_file():
        missing.append(k); continue
    rows.extend(list(csv.DictReader(open(p))))
with open(REPO / "results.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=FIELDS); w.writeheader(); w.writerows(rows)
print(f"merged {len(rows)} rows from {len(roster)-len(missing)}/{len(roster)} kernels -> results.csv")
if missing:
    print(f"MISSING (array task failed or still running): {missing}")
