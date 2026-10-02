#!/usr/bin/env python3
"""Work list for the chained debug-partition run (chain_debug.sbatch).

A UNIT is one kernel's worth of timed work on one exclusive node:
  MAIN  <k>  the 6 cells of the main sweep             (results_parts/<k>.csv, 6 rows)
  F     <k>  preset F, 6 cells                          (variant_parts/presetF.<k>.csv, 6 rows)
  FPOFF <k>  -ffp-contract=off, c/cpp/fortran/c_ref     (variant_parts/fpoff.<k>.csv, 4 rows)
  AB    <k>  fortran with / without parallelize-loops   (variant_parts/ab{with,without}.<k>.csv, 1 row each)
  OPTREP -   task-2 opt reports, compile-only, once every MAIN unit is done (opt_reports_index.csv)
Order: MAIN longest-first by v1 wall time, then the variants, then OPTREP.

  chain_status.py --todo      units not yet complete, one per line ("KIND<TAB>kernel")
  chain_status.py --progress  one integer: rows recorded so far (the stall guard compares it)
"""
import csv, pathlib, re, sys
from datetime import datetime

REPO = pathlib.Path(__file__).resolve().parent
roster = [l.strip() for l in open(REPO / "roster40.txt") if l.strip()]


def v1_minutes():
    out = {}
    for f in (REPO.parent / "sweep_logs").glob("sweep.*.out"):
        t = f.read_text()
        s = re.search(r"kernel=(\S+) .* (20\S+)$", t.splitlines()[0]) if t else None
        e = re.search(r"DONE \S+ (20\S+)", t)
        if s and e:
            out[s.group(1)] = (datetime.fromisoformat(e.group(1)) - datetime.fromisoformat(s.group(2))).seconds
    return out


def nrows(p):
    return sum(1 for _ in csv.DictReader(open(p))) if p.is_file() else 0


def units():
    dur = v1_minutes()
    u = [("MAIN", k) for k in sorted(roster, key=lambda k: -dur.get(k, 0))]
    jobs = [l.split("\t") for l in (REPO / "variants/jobs.tsv").read_text().splitlines() if l.strip()]
    u += [(kind, k) for kind, k in jobs]
    u.append(("OPTREP", "-"))
    return u


def done(kind, k):
    vp = REPO / "variant_parts"
    if kind == "MAIN":
        return nrows(REPO / "results_parts" / f"{k}.csv") >= 6
    if kind == "F":
        return nrows(vp / f"presetF.{k}.csv") >= 6
    if kind == "FPOFF":
        return nrows(vp / f"fpoff.{k}.csv") >= 4
    if kind == "AB":
        return nrows(vp / f"abwith.{k}.csv") >= 1 and nrows(vp / f"abwithout.{k}.csv") >= 1
    if kind == "OPTREP":
        return (REPO / "opt_reports_index.csv").is_file()
    raise ValueError(kind)


def todo():
    main_left = any(not done("MAIN", k) for k in roster)
    return [(kind, k) for kind, k in units()
            if not done(kind, k) and not (kind == "OPTREP" and main_left)]


def progress():
    n = sum(nrows(p) for p in (REPO / "results_parts").glob("*.csv"))
    n += sum(nrows(p) for p in (REPO / "variant_parts").glob("*.csv"))
    n += nrows(REPO / "opt_reports_index.csv")
    return n


if __name__ == "__main__":
    if "--progress" in sys.argv:
        print(progress())
    else:
        for kind, k in todo():
            print(f"{kind}\t{k}")
