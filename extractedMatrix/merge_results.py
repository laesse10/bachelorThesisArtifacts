#!/usr/bin/env python3
"""Merge parts/<kernel>.csv into results.csv (v2 column layout + provenance/adapter/...).

One reclassification, applied from the cell's saved driver log and kept visible in `status_runner`:
a cell the runner recorded as `incorrect` with NO timings, whose log shows a numba
TypingError/LoweringError/UnsupportedError, never produced output. Numba compiles at the first call,
so this is a build failure, and its status becomes `build_error`. sweep_extracted.py classifies it
this way since warpx_field_gather/numba (job 4990441), the only cell affected. See DEVIATIONS.md.
"""
import csv, pathlib, re

HERE = pathlib.Path(__file__).resolve().parent
ORDER = ["comet_int4_gemm", "quatrex_rgf", "spgemm_hash", "warpx_boris_push", "warpx_esirkepov_deposition",
         "warpx_field_gather", "triangle_count", "nfa_frontier"]
NUMBA_ERR = re.compile(r"numba\.core\.errors\.(TypingError|LoweringError|UnsupportedError)[^\n]*(\n[^\n]*){0,3}")

rows = []
for k in ORDER:
    for r in csv.DictReader(open(HERE / "parts" / f"{k}.csv")):
        r["status_runner"] = r["status"]
        if r["status"] == "incorrect" and r["n_reps"] in ("", "0"):
            log = HERE / "logs" / "cells" / f"{k}.{r['representation']}.log"
            m = NUMBA_ERR.search(log.read_text()) if log.is_file() else None
            if m:
                r["status"] = "build_error"
                r["notes"] = ("numba compile failure at the first call (reclassified from the runner's "
                              f"'incorrect', no output was produced): {' '.join(m.group(0).split())[:400]}")
        rows.append(r)
fields = list(rows[0].keys())
fields.insert(fields.index("status") + 1, fields.pop(fields.index("status_runner")))
with open(HERE / "results.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=fields); w.writeheader(); w.writerows(rows)
print(f"wrote results.csv: {len(rows)} rows")
for r in rows:
    print(f"{r['kernel']:28s} {r['representation']:11s} {r['status']:12s} {r['provenance']:8s} "
          f"{(int(r['time_ns_min']) / 1e6 if r['time_ns_min'] else 0):10.3f} ms  {r['notes'][:70]}")
