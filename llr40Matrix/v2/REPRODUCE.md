# Reproducing the v2 matrix

The measurement protocol is `../protocol.md`, unchanged. `DEVIATIONS.md` lists every way this run
departs from it, from the task text, or from v1. Inputs:

| what | where | commit |
|---|---|---|
| benchmark + harness | `git@github.com:spcl/HPCAgent-Bench.git` | `26a4f0cfc1540cead109dfc8d736f1c45263e390` |
| agent submissions | `https://github.com/ThrudPrimrose/ICLR26Reproducibility.git` | `2830ed3f5285c747de53dda2b04229d6a3c0f243` |

Environment: CPython 3.12.13 (the commit requires >= 3.12) with `requirements-frozen.txt`, GCC as in
`env_record.txt`. Paths default to the CSCS layout; `LLR40_BENCH`, `LLR40_PYTHON` and `LLR40_ROOT`
override them.

```bash
# (a) roster, agent picks, labels -- all three are identical to v1's
#     (roster40.txt from the experiment_tags llr-focus40 grep in ../REPRODUCE.md)
LLR40_BENCH=$BENCH python build_agent_picks.py
LLR40_BENCH=$BENCH python build_labels.py

# variant worktrees: the pinned commit plus ONE patch each (no patch for -abwith)
git -C $BENCH worktree add --detach $BENCH-F       26a4f0cf && git -C $BENCH-F     apply variants/presetF.patch
git -C $BENCH worktree add --detach $BENCH-fpoff   26a4f0cf && git -C $BENCH-fpoff apply variants/fpcontract_off.patch
git -C $BENCH worktree add --detach $BENCH-nopar   26a4f0cf && git -C $BENCH-nopar apply variants/no_parallelize_loops.patch
git -C $BENCH worktree add --detach $BENCH-abwith  26a4f0cf
git -C $BENCH worktree add --detach $BENCH-aux     26a4f0cf     # opt-report argv + reference paths

# (b) task 3.1: largest finite size (NumPy only)          -> finite_sizes.csv
LLR40_BENCH=$BENCH-aux python finite_search.py

# (c) everything timed + the opt reports, as chained 30-min debug jobs. The job resubmits itself
#     until chain_claims/COMPLETE exists. Each job first checks the step geometry on every node.
sbatch chain_debug.sbatch            # sbatch --nodes=N for a smaller job
#     (normal-partition equivalents: sweep_array.sbatch, variants_array.sbatch, opt_reports.sbatch)

# (d) merge + analysis
python merge_results.py              # results.csv, agent_attempts.csv
python diff_vs_v1.py                 # diff_vs_v1.csv   (needs emitted_sources_v1/, regenerated at e2bceb68)
python ftree_ab.py                   # ftree_parallelize_ab.csv
python analyze.py                    # class aggregates (v1 method + complete-case), outliers, noise floor, figures
./opt_evidence.sh                    # opt_findings_evidence/ (compile-only experiments)
python write_opt_findings.py         # opt_findings.md
python build_summary.py              # summary.md
```

Checks that should hold anywhere: 240 preset-M rows with 229 ok / 6 unsupported / 5 incorrect;
18 preset-F rows, all ok; the 4 `-ffp-contract=off` rows for `tsvc_2_s115` all ok; every
`emitted_sources/` file byte-identical to a fresh autogen at `26a4f0cf`.
