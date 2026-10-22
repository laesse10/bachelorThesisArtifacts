# HPCAgent-Bench v0.2.1: llr40 and scicomp40 agent baselines

The no-packet control arm of the v0.2.1 campaign, for two models (OSS-120B, Qwen3.8-27B), two delivery
languages (C, HIP) and two kernel sets (llr40: 40 loop-level kernels; scicomp40: 38 scientific-computing kernels).
No CPF arms. Compiler columns (DaCe canon CPU/GPU, Pluto, Polly, PPCG, Numba fastmath) sit beside the agents
in the same plots.

- Harness: HPCAgent-Bench `e63f9f1ce` (release-v0.2.1), DaCe pin `baac24733`.
- Runs: 2026-10-20 22:00 to 2026-10-21 21:30 (UTC). One agent episode per kernel, budget per kernel as in v0.2.1.
- Snapshot: 2026-10-22. Every setup below has finished; every kernel has an episode.

## Results

Speedup is over the best of the GCC (`-O3 -march=native -fopenmp`) and Numba references of each kernel. The
geomean runs over the kernels with a graded, verified answer (`n`), with a 95% BCa bootstrap interval. Tokens are
billed tokens per kernel episode, over every episode.

| set | lang | model | solved / kernels | n in geomean | geomean speedup [95% CI] | tokens per kernel |
|---|---|---|---|---|---|---|
| llr40 | C | Qwen3.8-27B | 39 / 40 | 38 | **10.4x** [7.4, 14.3] | 1.65M |
| llr40 | C | OSS-120B | 28 / 40 | 28 | **6.1x** [4.1, 8.6] | 59K |
| llr40 | HIP | Qwen3.8-27B | 38 / 40 | 38 | **178x** [123, 246] | 1.79M |
| llr40 | HIP | OSS-120B | 36 / 40 | 34 | **34.5x** [14.4, 69.2] | 111K |
| scicomp40 | C | Qwen3.8-27B | 23 / 38 | 19 | **4.4x** [2.6, 9.8] | 1.51M |
| scicomp40 | C | OSS-120B | 20 / 38 | 18 | **2.7x** [1.7, 4.9] | 233K |
| scicomp40 | HIP | Qwen3.8-27B | 14 / 38 | 12 | **39.8x** [15.1, 96.5] | 995K |
| scicomp40 | HIP | OSS-120B | 13 / 38 | 11 | **7.5x** [2.6, 26.6] | 460K |

"Solved" counts kernels with at least one verified submission; `n` counts those the geomean can pair with the
reference timing (a verified answer whose grade timed out or had no reference timing is solved but not in `n`).

Compiler columns over the same denominator (all kernels each compiler handled):

| set | DaCe CPU canon | DaCe GPU canon | Pluto | Polly | PPCG (HIP) | Numba fastmath |
|---|---|---|---|---|---|---|
| llr40 | 6.7x (40) | 47.2x (40) | 1.8x (32) | 0.72x (40) | 10.8x (16) | 0.68x (40) |
| scicomp40 | 1.0x (34) | 4.1x (31) | 1.2x (7) | 0.41x (35) | 4.1x (6) | 3.7x (34) |

Reading the numbers:

- Qwen3.8 solves more and faster on both sets, at 16 to 28x the tokens of OSS-120B on llr40 and 2 to 6.5x on
  scicomp40.
- scicomp40 is much harder: about half the kernels solved in C and about a third in HIP, against nearly all of
  llr40.
- On llr40 the DaCe GPU canon column (47x) sits between the two HIP agents; on scicomp40 both HIP agents beat it
  on the kernels they solve, but solve fewer than a third.
- The agent geomeans cover only solved kernels, so the scicomp40 HIP rows describe 11 to 12 kernels; read them
  with the solved column.

## Caveats

- **PPCG (HIP) column is provisional.** These numbers predate the fix that makes a device-resident PPCG entry
  synchronize before it returns (HB `154312c82`, v0.3.0); without it the timed region could end before the
  kernels. Treat the PPCG column as an upper bound until it is regraded on the v0.3.0 image.
- Compiler columns come from the canon sweep database (`data/canon.db`), measured on the sweep's own preset;
  the agent rows come from the agent episodes. Both use the same best-of(GCC, Numba) denominator.
- Pluto declines kernels it cannot model (hence 32/40 and 7/38).

## Contents

```
README.md                         this report
plots/
  <set>-<lang>-baselines.{pdf,png}  per kernel: both models + compiler columns, tokens below, geomean column
  <set>-compilers.{pdf,png}         compiler columns alone, over Numba
  *-kernels.csv                     every mark in the plot (one row per kernel and column)
  *-summary.csv                     geomean, interval, n, wins/losses per row
  *-tokens-summary.csv              tokens per row
  COMMANDS.sh                       the exact commands that drew every plot
data/
  results/<setup>/results.db        the run's results database (episodes, grades, attempts), SOURCE.txt = job id + path
  observations/observations.db      all eight setups as one observation table (SQLite)
  observations/observations.csv     the same rows as CSV
  observations/sources/             every submitted source the judge graded (sources_index.csv maps rows to files)
  canon.db                          compiler sweep database (compiler columns)
  tag-llr40.txt, tag-scicomp40.txt  kernel lists of the two sets
```

Not included: the agent transcripts (about 75 GB across the eight runs). They stay on the cluster under
`hpcagent-bench-runs/{llr40v021-20261020,scicomp40-20261020,scicomp40-20261021}/<job>/`.

Setups and jobs:

| setup | job |
|---|---|
| llr40v021-oss120b-c | 688594 |
| llr40v021-oss120b-hip | 688595 |
| llr40v021-qwen38-c | 688592 |
| llr40v021-qwen38-hip | 688593 |
| scicomp40-oss120b-c | 688580 |
| scicomp40-oss120b-hip | 688581 |
| scicomp40-qwen38-c | 688578 |
| scicomp40-qwen38-hip | 688621 |

## Reproducing

With HPCAgent-Bench installed (`pip install -e .` in a checkout), from this directory:

```bash
sqlite3 data/observations/observations.db \
  "select setup, count(distinct kernel), count(distinct case when correct=1 then kernel end) from observations group by setup"
bash plots/COMMANDS.sh     # paths in it are relative to this directory
```
