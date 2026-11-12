# solver14-precond results package

Snapshot 2026-10-19T21:47:21+02:00. HPCAgent-Bench v0.2 (tag 083c88e) campaign checkout /iopsstor/scratch/cscs/ybudanaz/hb-v02-campaign;
run roots /capstor/scratch/cscs/ybudanaz/x86_64/hpcagent-bench-runs/solver14-precond-20261018/<job>. Jobs: 686049 686050 686051 686052 686054 (complete), 686055 (NODE_FAIL, partial), 687301 (running at snapshot, partial).

Start with report.md (or report.html). speedup.csv has one row per model x variant x kernel (the reported episode), summary.csv one row per model x variant.
The layout and every definition are in report.md. All data was read without modifying the run roots: the DBs are copies taken at the snapshot, and the live 687301 files were read up to their size at that moment.
## Package layout (solver14-precond.zip)

```
solver14-precond/
  README.md  report.md  report.html  speedup.csv  summary.csv
  observations/          bench extract output: observations.db + .csv, sources_index.csv
  skills/solver-v4/      the shipped skill pages (single, split) + skills.yaml
  jobs/<job>/            results.db (merged, if any), judge/rank-*/*.db shards + upstream logs, services-<job>.out/.err,
                         prepare_job.sh, inference.json, agent-folders.jsonl, problems.jsonl, monitor/, shared prompt+skills, tasks/
  <model>/<variant>/<kernel>/job-<job>/
      OUTCOME.json       outcome, reason, final and best-score grade summaries
      final/             source of the final answer (submit or promoted grade) + final_grade.json
      best_scored/       best correct /score source, only if it differs from the final one
      graded/            every source the judge graded, in call order (NN_<kind>_<status>_<speedup>.c)
      judge_grades.csv   every judge request of the episode (score/submit/promoted/final), with detail
      judge_cells.csv    per-input cells of those grades (reference that won, times, ratio, p-value)
      episode/           claude.log (full stream-json transcript), prompt.txt, tokens.json, attempts.jsonl,
                         mcp.json, submission-spent.json, home/ (Claude Code session files)
      workspace/         the agent's /shared/agent-N text files; SKIPPED.tsv lists binaries left out
```

