# LLR-40 measurement matrix

Six code representations of the 40 `llr-focus40` loop-level-reasoning kernels from
HPCAgent-Bench, built, validated against the NumPy oracle, and timed on CSCS Alps `daint`
(NVIDIA GH200, aarch64), one kernel per exclusive node.

**240 cells** (40 kernels x 6 representations) at preset M, plus 18 preset-S rows for three
kernels that are numerically degenerate at M. Every individual timing is retained.

| file | what |
|---|---|
| **`protocol.md`** | the measurement protocol, pinned BEFORE measuring, plus the bandwidth-artifact diagnosis |
| **`results.csv`** | the matrix -- one row per cell, all 30 raw timings per cell in `time_ns_all` |
| **`labels.csv`** | optimization class per kernel (31 declared from the manifest, 9 derived from source) |
| **`summary.md`** | what was measured, what failed and why, class distribution, outliers, threats to comparability |
| **`figures/`** | heatmap, class aggregate, coverage (`.png` + `.pdf`) |
| **`REPRODUCE.md`** | how to regenerate all of the above from source |

Derived tables, all generated (no hand-entered numbers): `class_aggregate.csv`, `coverage.csv`,
`outliers.csv`, `noise_floor.csv` (per-cell RSD + bimodal flag), `numerical_health.csv`.

`results_parts/` holds the 40 unmerged per-kernel CSVs and `sweep_logs/` the 80 per-task logs.
`diagnostics/` holds the evidence behind the four hypotheses `protocol.md` excludes by
measurement -- perf counters, the pinned/unpinned A/B, the THP probe, and both pilots.

## Headline results

- On **loop_interchange** and **loop_distribution** kernels the agent-optimised source beats every
  compiler-generated representation by **~6-12x at a single thread** -- GCC did not perform the
  interchange and the agent did. Where no restructuring is available (`indirect_addressing`) the
  agent is *slower* than plain C.
- **229 ok / 6 unsupported / 5 incorrect.** Every non-ok cell was predicted before measuring.
- The documented gfortran-vs-gcc reassociation confound **did not materialise**: median
  `fortran/c` over the 10 reduction kernels is exactly **1.00x**.
- Two kernels (`wf_triangular`, `wf_diff_skew`) are recorded `ok` but pass **vacuously** -- their
  output is only 50.9% / 9.5% finite. See `numerical_health.csv` and `summary.md`.

## Caveats

Timings are machine-specific (`-march=native`), and ~14% of reps carry an external
memory-bandwidth artifact that is diagnosed but whose root cause is unidentified; **all
aggregates therefore use min-of-k**. `REPRODUCE.md` section 5 lists the machine-independent
checks a reproduction can assert, and section 6 states the non-reproducible element plainly.
