#!/usr/bin/env python3
"""Rank each roster kernel's llr40v10 agent submissions by campaign-recorded speedup.

Emits agent_picks.json: kernel -> ordered candidate list (best first). The sweep walks the
list and takes the first that BUILDS and VALIDATES here; campaign verdicts are never trusted
as correctness (they were produced on Beverin/MI300A/x86_64).
"""
import csv, json, os, pathlib, sys

# Overridable: clone of ThrudPrimrose/ICLR26Reproducibility (see REPRODUCE.md).
ART = pathlib.Path(os.environ.get(
    "LLR40_ICLR26",
    "/capstor/scratch/cscs/lhulsbergen/ICLR26Reproducibility")) / "paper_artifacts/experiments/llr40"
ARM_PREFIX = "llr40v10"          # the only arms with full 40-kernel coverage

roster = [l.strip() for l in open("roster40.txt") if l.strip()]

obs = [r for r in csv.DictReader(open(ART / "data/llr40_observations.csv"))
       if r["record"] == "submission" and r["arm"].startswith(ARM_PREFIX)]
src = [r for r in csv.DictReader(open(ART / "data/llr40_sources_index.csv"))
       if r["kind"] == "candidate" and r["arm"].startswith(ARM_PREFIX)]

# (benchmark, arm, job, seq) -> source row. seq in the index == attempt_index in observations.
by_key = {(r["benchmark"], r["arm"], r["job"], r["seq"]): r for r in src}

picks, missing = {}, []
for k in roster:
    cands = []
    for o in (r for r in obs if r["benchmark"] == k):
        s = by_key.get((k, o["arm"], o["job"], o["attempt_index"]))
        if s is None:
            continue
        p = ART / "artifacts" / s["rel_path"]
        if not p.is_file():
            continue
        cands.append({
            "path": str(p), "arm": o["arm"], "job": o["job"], "seq": o["attempt_index"],
            "language": o["delivered_language"] or o["language"],
            "campaign_speedup": float(o["speedup"] or 0.0),
            "sha256": s["sha256"], "n_bytes": int(s["n_bytes"]),
        })
    # best campaign speedup first; de-duplicate identical source bytes (same file resubmitted)
    cands.sort(key=lambda c: -c["campaign_speedup"])
    seen, uniq = set(), []
    for c in cands:
        if c["sha256"] in seen:
            continue
        seen.add(c["sha256"]); uniq.append(c)
    picks[k] = uniq
    if not uniq:
        missing.append(k)

json.dump(picks, open("agent_picks.json", "w"), indent=1)
tot = sum(len(v) for v in picks.values())
print(f"kernels: {len(roster)}  with >=1 candidate: {len(roster)-len(missing)}  unique candidates: {tot}")
print(f"NO agent candidate (-> unsupported row): {missing or 'none'}")
langs = {}
for v in picks.values():
    if v:
        langs[v[0]["language"]] = langs.get(v[0]["language"], 0) + 1
print("top-pick language distribution:", langs)
