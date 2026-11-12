#!/usr/bin/env python3
"""Scores of the solver14 rerun (2026-11-11) from the job databases in ../solver14-20261111/.

Writes scores.csv (one row per setup and kernel) and summary.csv (one row per setup, the columns of
the thesis's Table A.6). Score: the credited speedup of the episode's last final or promoted grade;
an episode without one counts as 1. Read: episodes with a tool call naming a page under /skills/.
"""
import csv, glob, json, math, pathlib, sqlite3, statistics
from scipy.stats import t, wilcoxon

HERE = pathlib.Path(__file__).resolve().parent
RUN = HERE.parent / "solver14-20261111"
JOBS = {"734916": ("qwen38", "none"), "734910": ("qwen38", "v2-single"), "734911": ("qwen38", "v2-split"),
        "734917": ("oss120b", "none"), "734918": ("oss120b", "v2-single"), "734919": ("oss120b", "v2-split")}

rows, scores = [], {}
for job, (model, skill) in JOBS.items():
    db_path = RUN / job / "results.db"
    if not db_path.exists():          # 734910: node failure, no merged database
        continue
    db = sqlite3.connect(f"file:{db_path}?mode=ro", uri=True)
    for eid, kernel, result, wall_ms in db.execute("select id, kernel, result, wall_ms from episodes"):
        g = db.execute("""select kind, correct, credited_speedup from grades where episode_id = ?
                          and kind in ('final', 'promoted') order by ts_ms desc limit 1""", (eid,)).fetchone()
        credited = bool(g and g[2] > 0)
        score = g[2] if credited else 1.0
        scores[(model, skill, kernel)] = score
        rows.append({"job": job, "model": model, "skill": skill, "kernel": kernel, "result": result,
                     "wall_h": f"{wall_ms / 3.6e6:.1f}", "final_kind": g[0] if g else "",
                     "credited": int(credited), "score": f"{score:.4g}"})

with open(HERE / "scores.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)


def reads(job):
    n = 0
    for log in glob.glob(str(RUN / job / "agents" / "node-*" / "problem-*" / "claude.log")):
        for line in open(log, errors="replace"):
            if '"tool_use"' in line and "/skills/solver-preconditioners" in line:
                n += 1
                break
    return n


summary = []
for job, (model, skill) in JOBS.items():
    ks = sorted(k for (m, s, k) in scores if m == model and s == skill)
    if not ks:
        summary.append({"model": model, "skill": skill, "job": job, "note": "node failure, no results"})
        continue
    row = {"model": model, "skill": skill, "job": job, "read": f"{reads(job)}/14" if skill != "none" else "",
           "solved": sum(r["credited"] for r in rows if r["job"] == job),
           "geomean": f"{math.exp(statistics.mean(math.log(scores[(model, skill, k)]) for k in ks)):.2f}"}
    if skill != "none":
        lr = [math.log(scores[(model, skill, k)] / scores[(model, "none", k)]) for k in ks]
        m, h = statistics.mean(lr), t.ppf(0.975, len(lr) - 1) * statistics.stdev(lr) / math.sqrt(len(lr))
        row.update({"ratio_vs_none": f"{math.exp(m):.2f}", "ci95_low": f"{math.exp(m - h):.2f}",
                    "ci95_high": f"{math.exp(m + h):.2f}", "wins": sum(x > 0 for x in lr),
                    "losses": sum(x < 0 for x in lr), "wilcoxon_p": f"{wilcoxon(lr).pvalue:.2f}"})
    summary.append(row)

fields = ["model", "skill", "job", "read", "solved", "geomean", "ratio_vs_none", "ci95_low", "ci95_high",
          "wins", "losses", "wilcoxon_p", "note"]
with open(HERE / "summary.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=fields); w.writeheader(); w.writerows(summary)
for r in summary:
    print(r)
