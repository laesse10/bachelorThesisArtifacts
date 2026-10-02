#!/usr/bin/env python3
"""Merge the v2 array-task parts, in roster order.

results.csv        = the 240 preset-M cells (results_parts/) + the variant rows that belong in the
                     matrix file: preset=F (task 3.2) and flags_variant=-ffp-contract=off (task 3.3).
                     Both are tagged and never enter an aggregate (analyze.py filters them).
agent_attempts.csv = every attempted agent candidate of the main sweep (agent_attempts_parts/).
The task-4 A/B rows stay in variant_parts/ and are reduced by ftree_ab.py.
"""
import csv, pathlib

REPO = pathlib.Path(__file__).resolve().parent
FIELDS = ["kernel", "representation", "preset", "flags_variant", "status", "time_ns_median", "time_ns_min",
          "time_ns_all", "compiler", "compiler_version", "flags", "threads", "n_warmup", "n_reps",
          "commit_hash", "timestamp", "notes", "slurm_job"]
roster = [l.strip() for l in open(REPO / "roster40.txt") if l.strip()]


def parts(d, pattern):
    out = []
    for k in roster:
        for p in sorted((REPO / d).glob(pattern.format(k=k))):
            out.append((k, p))
    return out


rows, missing = [], []
for k in roster:
    p = REPO / "results_parts" / f"{k}.csv"
    if not p.is_file():
        missing.append(k); continue
    rows.extend(list(csv.DictReader(open(p))))
n_main = len(rows)
for tag in ("presetF", "fpoff"):
    for k, p in parts("variant_parts", tag + ".{k}.csv"):
        rows.extend(list(csv.DictReader(open(p))))
with open(REPO / "results.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=FIELDS, extrasaction="ignore"); w.writeheader()
    for r in rows:
        w.writerow({f: r.get(f, "") for f in FIELDS})
print(f"merged {n_main} main rows from {len(roster)-len(missing)}/{len(roster)} kernels "
      f"+ {len(rows)-n_main} variant rows (preset=F, -ffp-contract=off) -> results.csv")
if missing:
    print(f"MISSING (array task failed or still running): {missing}")

att = []
for k in roster:
    p = REPO / "agent_attempts_parts" / f"{k}.csv"
    if p.is_file():
        att.extend(list(csv.DictReader(open(p))))
# first_error_line is re-derived from the saved driver log with sweep.py's own extractor, so a
# pattern-order fix there applies to attempts already measured (the logs are the evidence).
import importlib.util, os, sys
sys.argv = sys.argv[:1]
os.environ.setdefault("LLR40_BENCH", "/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2")
_sp = importlib.util.spec_from_file_location("_sweep", REPO / "sweep.py")
_sw = importlib.util.module_from_spec(_sp); _sp.loader.exec_module(_sw)
for a in att:
    lp = REPO / "agent_attempt_logs" / a.get("log", "")
    if a.get("log") and lp.is_file():
        a["first_error_line"] = _sw.first_error_line(a["status"], a["first_error_line"], lp.read_text())
# a resumed run can re-log a candidate; keep the last record per (kernel, rank)
att = list({(a["kernel"], str(a["rank"])): a for a in att}.values())
if att:
    with open(REPO / "agent_attempts.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(att[0])); w.writeheader(); w.writerows(att)
print(f"merged {len(att)} agent attempts -> agent_attempts.csv")
