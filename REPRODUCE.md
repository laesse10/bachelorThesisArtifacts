# Reproducing the LLR-40 measurement matrix

Everything here regenerates `results.csv`, `labels.csv`, `figures/` and `summary.md` from
source. The measurement protocol itself is `protocol.md` and is not re-decided here.

**Read this first: what is and is not portable.** The matrix was measured on CSCS Alps `daint`
(NVIDIA GH200, aarch64 Neoverse-V2). The *pipeline* is portable; the *numbers* are not, and are
not expected to be. Timings depend on the machine, and `-march=native` bakes in the host ISA.
Reproducing on other hardware reproduces the method and the correctness verdicts, not the
nanoseconds.

---

## 1. External inputs (NOT vendored here)

Two repositories are required. Neither lives inside this directory.

| what | where | pinned commit |
|---|---|---|
| benchmark corpus + harness | `git@github.com:spcl/HPCAgent-Bench.git` | `e2bceb68db2b32dc2203a6c6d4bc169dff291a49` |
| agent submissions (the `agent` column) | `https://github.com/ThrudPrimrose/ICLR26Reproducibility.git` | `2830ed3f5285c747de53dda2b04229d6a3c0f243` |

```bash
git clone git@github.com:spcl/HPCAgent-Bench.git
git -C HPCAgent-Bench checkout e2bceb68db2b32dc2203a6c6d4bc169dff291a49
git clone https://github.com/ThrudPrimrose/ICLR26Reproducibility.git
git -C ICLR26Reproducibility checkout 2830ed3f5285c747de53dda2b04229d6a3c0f243
```

**`bench/` in this directory is a git WORKTREE, not a clone.** Its `.git` is a pointer file into
`/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench/.git`. It therefore breaks if this directory is
copied elsewhere or if that parent clone is deleted. Do not rely on it — point `LLR40_BENCH` at
your own checkout of the commit above.

## 2. Environment

Python 3.11.13, dependencies frozen in `requirements-frozen.txt` (key pins: numpy 2.4.6,
numba 0.67.0, llvmlite 0.49.0, matplotlib 3.11.1, PyYAML 6.0.3).

```bash
python3.11 -m venv venv && ./venv/bin/pip install -r requirements-frozen.txt
```

Compilers: **GCC 13.3.1** for all three compiled languages (`gcc`, `g++`, `gfortran`) — see
`env_record.txt`. clang/flang were unavailable on this system; `protocol.md` §2 records why GNU
throughout is the right substitution rather than a compromise.

## 3. Configuration

All site-specific paths are environment variables; the defaults reproduce the recorded runs.

```bash
export LLR40_ROOT=$PWD                      # this directory
export LLR40_BENCH=/path/to/HPCAgent-Bench  # checkout of the pinned commit
export LLR40_ICLR26=/path/to/ICLR26Reproducibility
export LLR40_PYTHON=$PWD/venv/bin/python
```

Slurm account is `-A g34` in the `.sbatch` files; override with `sbatch -A <account>`.

## 4. Run order

```bash
# (a) freeze the roster: the 40 manifests tagged llr-focus40  -> roster40.txt
cd "$LLR40_BENCH/hpcagent_bench/benchmarks/loop_level_reasoning"
for d in */; do k=${d%/}; grep -q 'llr-focus40' "$k/$k.yaml" 2>/dev/null && echo "$k"; done \
  | sort > "$LLR40_ROOT/roster40.txt"          # must be exactly 40 lines
cd "$LLR40_ROOT"

# (b) rank the agent submissions per kernel     -> agent_picks.json
$LLR40_PYTHON build_agent_picks.py             # 39/40 kernels; tsvc_2_s2233 has none

# (c) optimization-class labels                 -> labels.csv   (Phase 4)
$LLR40_PYTHON build_labels.py

# (d) THE SWEEP (Phase 3): 40 array tasks, ONE KERNEL PER EXCLUSIVE NODE
sbatch sweep_array.sbatch                      # ~1h40m wall at 20-way concurrency
$LLR40_PYTHON merge_results.py                 # results_parts/*.csv -> results.csv

# (e) the three numerically degenerate kernels, additionally at preset S
sbatch unstable3.sbatch                        # -> results_unstable3_presetS.csv
# then append those 18 rows to results.csv (they carry preset=S)

# (f) aggregate (Phase 5) + write the summary
$LLR40_PYTHON analyze.py --results results.csv --labels labels.csv
$LLR40_PYTHON build_summary.py --results results.csv
```

Exclusivity is not optional: one kernel per node, `--exclusive`. Running several kernels
concurrently on one node contends for memory bandwidth and the timings become unusable.

## 5. Verifying a reproduction

Checks that should hold on any machine (they are properties of the corpus, not of daint):

- `roster40.txt` has **exactly 40** kernels, matching the `focus40` column of
  `$LLR40_ICLR26/paper_artifacts/experiments/llr40/data/llr40_observations.csv`.
- `results.csv` has **240** `preset=M` rows (40 x 6) plus 18 `preset=S` rows.
- Status counts: **229 ok / 6 unsupported / 5 incorrect**. The 6 unsupported are the 5 kernels
  with no hand-written `_reference.c` plus `tsvc_2_s2233` (no agent submission in either
  campaign). The 5 incorrect are all `tsvc_2_s115`.
- `numerical_health.csv`: `wf_triangular` and `wf_diff_skew` are `ok` but only 50.9% / 9.5%
  finite — they pass *vacuously*. See `summary.md`.

Timings will differ. Two qualitative results should survive a change of machine, because they are
about code structure rather than clock speed: the agent column winning by ~6-12x on
`loop_interchange` / `loop_distribution`, and numba being the only column that passes on
`tsvc_2_s115`.

## 6. Known non-reproducible element

`results.csv` timings carry an intermittent ~1.6x memory-bandwidth artifact affecting ~14% of
reps (51 of 229 ok cells flagged, `noise_floor.csv:bimodal_suspect`). It is diagnosed in
`protocol.md` §4 — CPU frequency, core migration, huge pages and NUMA balancing are all excluded
by measurement; the cause is external to the measured process and was not identified. **All
aggregates use min-of-k for this reason.** A reproduction on the same machine should expect a
similar artifact rate; the per-rep values in `time_ns_all` are retained so the distribution can be
re-examined without re-running.
