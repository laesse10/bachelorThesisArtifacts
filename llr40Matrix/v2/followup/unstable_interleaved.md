# Experiment 3: re-timing the five bandwidth-unstable kernels, interleaved

**Hypothesis.** For `tsvc_2_s1232`, `tsvc_2_s231`, `tsvc_2_s235`, `tsvc_2_s2275` and `tsvc_2_s275`,
the differences between the GCC columns are memory-bandwidth noise, not code. All four GCC columns
compile to the same vectorisation decisions, yet their v1 and v2 minima differ by 1.20-2.19x. In
34 of their 40 GCC series, at most one of 30 runs came within 15% of the kernel's fastest run.

**Verdict: confirmed for four kernels (`s1232`, `s231`, `s235`, `s275`), undecided for `s2275`.**

- When all six columns run interleaved on one node, the order of the four GCC columns changes from
  round to round. The rank agreement across rounds is not significantly better than chance: Kendall's W = 0.01,
  0.38, 0.12, 0.13 (permutation p = 1.00, 0.07, 0.57, 0.54). The gap between the best and worst
  GCC column minima (1.03-1.22x) is smaller than one column's own round-to-round spread (median
  1.33-1.46x). The column that is fastest in one round is slow in another.
- `s2275` meets the separation criterion set below: W = 0.47, p = 0.03, and a worst/best minimum
  gap of 1.435 against a median round spread of 1.306. Three things argue against calling it
  code: the four columns execute the same number of instructions per rep (0.7627-0.7632e9, within
  0.07%); the 1.435 comes from one fast round of `c_reference` (478.6 ms, its other rounds
  667-856 ms); and p = 0.03 does not survive a correction for testing five kernels (Bonferroni
  0.01). What remains is a weak tendency, `fortran` fastest in 4 of 6 rounds and `cpp` slowest
  in 4 of 6, which this experiment can neither confirm nor exclude.
- The pattern behind the hypothesis reappears under interleaving: in **104 of the 120** GCC series
  here, at most one of 30 runs came within 15% of the kernel's fastest GCC run (same definition
  as the "34 of 40": the reference is the fastest GCC-column run of that kernel, here over the six
  rounds).
- **The agent's advantage holds in every round, for every kernel:** 2.5-9.7x faster than the best
  other column of the same round (table below). Its series are tight (worst/best round
  1.006-1.010).

## Design

- One exclusive node per kernel (job 4982638: `s1232` nid005390, `s231` nid005443, `s235`
  nid005901, `s2275` nid005902, `s275` nid006494). R = 6 rounds. Each round runs every column's
  normal series once (5 warm-up + 30 timed, validated against the oracle first, as in the matrix),
  one series after the other. The column order is rotated by one each round (round r starts at
  column r of `c, c_reference, cpp, fortran, numba, agent`), so over six rounds every column runs
  once in every position. A round is one `sweep_followup.py --ordered --round r` step. Its order
  is in the `position` column and in the per-round lines below.
- All six columns are unchanged: the v2 emitted sources (byte-identical), the corpus
  `_reference.c`, and v2's chosen agent candidate pinned by sha256 (`v2_agent_cells.csv`). Preset
  M, `float64`, v2 environment and step geometry. All 180 series are `ok` with 30 timed reps, and
  every individual timing is in `unstable_interleaved.csv` (`time_ns_all`, one row per round x
  cell) and in `parts/unstable/`.
- Recorded alongside, cheaply: `perf stat -e cycles,instructions` over each series' 30 timed reps
  (`perfgate/`), and `/proc/vmstat` deltas around each series. The vmstat counters are node-wide
  and span the whole series process (build, oracle, validation, warm-ups, timed reps).

**Separation criterion.** The GCC columns "separate beyond the round-to-round spread" if both hold:
(a) their per-round ranking agrees across the six rounds more than chance, i.e. Kendall's W with a
permutation p < 0.05 (20,000 random rankings per round, seed fixed); and (b) the worst/best ratio
of their overall minima exceeds the median, over the four columns, of one column's worst/best ratio
of its six round minima. I fixed this criterion after seeing the first three to five rounds of each
kernel, so it was not pre-registered. It was not changed after the final rounds arrived.

## Summary
| kernel | rounds | Kendall's W, GCC ranks (permutation p) | GCC column minima, worst / best | one column's round-to-round spread, median (range) | fastest GCC column, round 1..6 | GCC columns separate? | agent / best other column, per round | agent fastest in every round? |
|---|---:|---|---:|---|---|---|---|---|
| `tsvc_2_s1232` | 6 | 0.01 (p = 1.00) | 1.138 | 1.408 (1.275-1.529) | `fortran`, `fortran`, `c_reference`, `c_reference`, `cpp`, `cpp` | no | 3.1-4.0x faster | yes |
| `tsvc_2_s231` | 6 | 0.38 (p = 0.07) | 1.216 | 1.327 (1.211-1.408) | `cpp`, `fortran`, `c`, `fortran`, `fortran`, `cpp` | no | 6.1-9.0x faster | yes |
| `tsvc_2_s235` | 6 | 0.12 (p = 0.57) | 1.032 | 1.458 (1.428-1.495) | `cpp`, `c`, `c`, `c_reference`, `c`, `fortran` | no | 6.9-9.7x faster | yes |
| `tsvc_2_s2275` | 6 | 0.47 (p = 0.03) | 1.435 | 1.306 (1.045-1.789) | `c_reference`, `fortran`, `fortran`, `c_reference`, `fortran`, `fortran` | **yes** | 6.6-9.4x faster | yes |
| `tsvc_2_s275` | 6 | 0.13 (p = 0.54) | 1.132 | 1.463 (1.356-1.611) | `cpp`, `fortran`, `fortran`, `cpp`, `c`, `fortran` | no | 2.5-3.6x faster | yes |

GCC series with at most one of 30 runs within 15% of the kernel's fastest GCC run (this experiment): 104 of 120

## What the counters show

- **Same code, different stalls.** Within a column, instructions per series vary by at most 0.17%
  (CV). The cycles:u count per counter-second is constant on each node (spread at most 0.005 GHz
  across a kernel's 36 series), so the clock does not change between fast and slow series. Time
  varies through IPC: the GCC series run at IPC 0.25-0.47 and the slower series have the lower IPC.
  This is the diagnosed external-bandwidth artifact (protocol.md section 4), now seen with all
  columns sharing one node's history.
- **Two real code differences that do not show in time.** `s231` `fortran` executes 11% fewer
  instructions per rep than the other GCC columns (0.843e9 against 0.948e9), and `s275` `cpp` 10%
  fewer (0.680e9 against 0.755e9). Neither column separates in time: the loops are memory bound,
  so fewer instructions buy nothing measurable.
- **Node memory state (an observation, not a diagnosis).** In all five kernels, slower GCC series
  coincide with fewer transparent-huge-page allocations and more page faults during that series'
  process. Spearman(series median time, `thp_fault_alloc`) is -0.74, -0.51, -0.64, -0.51, -0.54,
  and Spearman(median time, `pgfault`) is +0.75, +0.48, +0.64, +0.46, +0.51, with zero compaction
  stalls and zero page migrations. Base pages are 64 KB here and a PMD huge page is 512 MB, so a
  series with about 30 fewer huge-page faults has roughly 15 GB more of its allocations on base
  pages. The counters are node-wide and cover the whole series process, so this is a correlation
  to follow up, not a cause. protocol.md section 4 had excluded THP fallback for `s3110` from dTLB
  miss rates.
- The rotation balances any run-order effect. Over all GCC series, a series' round minimum divided
  by the round's best GCC minimum has median 1.19 in position 1 and 1.01-1.13 in positions 2-6.

## Against v1 and v2

None of the large v1/v2 column gaps reappears as a stable column difference. Example: v2's
`s2275` `c_reference` minimum of 426.5 ms was 1.4-1.6x below the other GCC columns. Here
`c_reference` sits at the same level as `c` and `fortran` (median round minimum 676.7 ms against
675.5 and 680.1), and its single fast round (478.6 ms) falls in the same round as `c`'s fastest
(540.2 ms). Likewise `s231` `c_reference` (v2: 547.5 ms, the fastest GCC cell) is the slowest GCC
column here in three of six rounds and the fastest in none.

## Per kernel

### `tsvc_2_s1232`

Fastest GCC-column run in this experiment: 38.06 ms.

| column | overall min (ms) | round 1 | round 2 | round 3 | round 4 | round 5 | round 6 | worst / best round | runs within 15% of kernel's fastest GCC run | runs within 15% of own fastest | v2 min | v1 min | IPC range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `c` | 43.31 | 48.76 | 56.24 | 50.50 | 43.31 | 43.92 | 56.11 | 1.299 | 4.4% | 21.7% | 39.13 | 38.41 | 0.34-0.45 |
| `c_reference` | 38.41 | 56.35 | 49.39 | 38.41 | 42.84 | 58.74 | 55.80 | 1.529 | 16.1% | 16.7% | 56.19 | 42.38 | 0.33-0.43 |
| `cpp` | 42.42 | 52.92 | 54.09 | 53.11 | 52.82 | 42.42 | 48.50 | 1.275 | 7.8% | 11.1% | 50.07 | 63.69 | 0.39-0.42 |
| `fortran` | 38.06 | 38.06 | 42.75 | 57.74 | 46.86 | 46.27 | 53.35 | 1.517 | 11.7% | 11.7% | 55.64 | 62.01 | 0.34-0.47 |
| `numba` | 60.82 | 73.81 | 60.82 | 80.09 | 75.36 | 80.46 | 75.75 | 1.323 | 0.0% | 8.3% | 83.49 | 62.17 | 0.64-0.70 |
| `agent` | 12.21 | 12.27 | 12.22 | 12.21 | 12.28 | 12.21 | 12.22 | 1.006 | 100.0% | 99.4% | 13.08 | 12.97 | 0.84-0.84 |

- round 1 (run order c, c_reference, cpp, fortran, numba, agent): `agent` 12.27 < `fortran` 38.06 < `c` 48.76 < `cpp` 52.92 < `c_reference` 56.35 < `numba` 73.81
- round 2 (run order c_reference, cpp, fortran, numba, agent, c): `agent` 12.22 < `fortran` 42.75 < `c_reference` 49.39 < `cpp` 54.09 < `c` 56.24 < `numba` 60.82
- round 3 (run order cpp, fortran, numba, agent, c, c_reference): `agent` 12.21 < `c_reference` 38.41 < `c` 50.50 < `cpp` 53.11 < `fortran` 57.74 < `numba` 80.09
- round 4 (run order fortran, numba, agent, c, c_reference, cpp): `agent` 12.28 < `c_reference` 42.84 < `c` 43.31 < `fortran` 46.86 < `cpp` 52.82 < `numba` 75.36
- round 5 (run order numba, agent, c, c_reference, cpp, fortran): `agent` 12.21 < `cpp` 42.42 < `c` 43.92 < `fortran` 46.27 < `c_reference` 58.74 < `numba` 80.46
- round 6 (run order agent, c, c_reference, cpp, fortran, numba): `agent` 12.22 < `cpp` 48.50 < `fortran` 53.35 < `c_reference` 55.80 < `c` 56.11 < `numba` 75.75

perf: instructions per series vary by at most 0.17% (CV) within a column; cycles:u per counter-second 3.244-3.249 GHz over all 36 series. vmstat (GCC series, node-wide, whole series process): Spearman(median time, THP faults) = -0.74, Spearman(median time, page faults) = 0.75; compaction stalls 0, page migrations 0.

### `tsvc_2_s231`

Fastest GCC-column run in this experiment: 528.53 ms.

| column | overall min (ms) | round 1 | round 2 | round 3 | round 4 | round 5 | round 6 | worst / best round | runs within 15% of kernel's fastest GCC run | runs within 15% of own fastest | v2 min | v1 min | IPC range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `c` | 545.33 | 712.62 | 713.11 | 545.33 | 668.04 | 626.33 | 714.07 | 1.309 | 0.6% | 1.1% | 723.29 | 767.35 | 0.36-0.41 |
| `c_reference` | 642.72 | 675.22 | 710.09 | 675.97 | 713.44 | 642.72 | 778.09 | 1.211 | 0.0% | 50.0% | 547.47 | 658.33 | 0.38-0.42 |
| `cpp` | 530.67 | 660.55 | 693.44 | 747.39 | 660.97 | 533.88 | 530.67 | 1.408 | 1.1% | 1.1% | 694.00 | 734.68 | 0.35-0.41 |
| `fortran` | 528.53 | 710.40 | 657.16 | 653.24 | 530.38 | 528.53 | 653.44 | 1.344 | 8.3% | 8.3% | 706.69 | 651.54 | 0.32-0.42 |
| `numba` | 419.07 | 430.81 | 622.79 | 502.76 | 419.07 | 597.05 | 674.10 | 1.609 | 26.7% | 8.3% | 594.69 | 569.43 | 0.39-0.48 |
| `agent` | 68.87 | 69.48 | 68.91 | 69.09 | 69.19 | 69.20 | 68.87 | 1.009 | 100.0% | 100.0% | 69.88 | 69.23 | 1.64-1.66 |

- round 1 (run order c, c_reference, cpp, fortran, numba, agent): `agent` 69.48 < `numba` 430.81 < `cpp` 660.55 < `c_reference` 675.22 < `fortran` 710.40 < `c` 712.62
- round 2 (run order c_reference, cpp, fortran, numba, agent, c): `agent` 68.91 < `numba` 622.79 < `fortran` 657.16 < `cpp` 693.44 < `c_reference` 710.09 < `c` 713.11
- round 3 (run order cpp, fortran, numba, agent, c, c_reference): `agent` 69.09 < `numba` 502.76 < `c` 545.33 < `fortran` 653.24 < `c_reference` 675.97 < `cpp` 747.39
- round 4 (run order fortran, numba, agent, c, c_reference, cpp): `agent` 69.19 < `numba` 419.07 < `fortran` 530.38 < `cpp` 660.97 < `c` 668.04 < `c_reference` 713.44
- round 5 (run order numba, agent, c, c_reference, cpp, fortran): `agent` 69.20 < `fortran` 528.53 < `cpp` 533.88 < `numba` 597.05 < `c` 626.33 < `c_reference` 642.72
- round 6 (run order agent, c, c_reference, cpp, fortran, numba): `agent` 68.87 < `cpp` 530.67 < `fortran` 653.44 < `numba` 674.10 < `c` 714.07 < `c_reference` 778.09

perf: instructions per series vary by at most 0.07% (CV) within a column; cycles:u per counter-second 3.204-3.206 GHz over all 36 series. vmstat (GCC series, node-wide, whole series process): Spearman(median time, THP faults) = -0.51, Spearman(median time, page faults) = 0.48; compaction stalls 0, page migrations 0.

### `tsvc_2_s235`

Fastest GCC-column run in this experiment: 521.22 ms.

| column | overall min (ms) | round 1 | round 2 | round 3 | round 4 | round 5 | round 6 | worst / best round | runs within 15% of kernel's fastest GCC run | runs within 15% of own fastest | v2 min | v1 min | IPC range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `c` | 521.22 | 597.37 | 521.22 | 536.39 | 550.81 | 555.09 | 779.11 | 1.495 | 16.7% | 16.7% | 789.17 | 670.07 | 0.38-0.47 |
| `c_reference` | 535.35 | 706.42 | 708.15 | 538.88 | 535.35 | 682.30 | 764.37 | 1.428 | 16.1% | 16.1% | 799.18 | 489.55 | 0.38-0.47 |
| `cpp` | 537.79 | 537.79 | 538.26 | 652.40 | 780.06 | 706.62 | 707.25 | 1.450 | 16.1% | 16.7% | 544.96 | 726.51 | 0.38-0.45 |
| `fortran` | 530.47 | 775.83 | 532.11 | 709.45 | 777.09 | 671.27 | 530.47 | 1.465 | 8.3% | 8.3% | 539.44 | 732.19 | 0.38-0.45 |
| `numba` | 397.70 | 554.43 | 419.23 | 508.82 | 402.12 | 577.29 | 397.70 | 1.452 | 27.2% | 24.4% | 460.21 | 396.88 | 0.44-0.57 |
| `agent` | 56.94 | 57.26 | 57.16 | 56.96 | 57.02 | 56.94 | 57.28 | 1.006 | 100.0% | 99.4% | 57.96 | 57.03 | 2.60-2.63 |

- round 1 (run order c, c_reference, cpp, fortran, numba, agent): `agent` 57.26 < `cpp` 537.79 < `numba` 554.43 < `c` 597.37 < `c_reference` 706.42 < `fortran` 775.83
- round 2 (run order c_reference, cpp, fortran, numba, agent, c): `agent` 57.16 < `numba` 419.23 < `c` 521.22 < `fortran` 532.11 < `cpp` 538.26 < `c_reference` 708.15
- round 3 (run order cpp, fortran, numba, agent, c, c_reference): `agent` 56.96 < `numba` 508.82 < `c` 536.39 < `c_reference` 538.88 < `cpp` 652.40 < `fortran` 709.45
- round 4 (run order fortran, numba, agent, c, c_reference, cpp): `agent` 57.02 < `numba` 402.12 < `c_reference` 535.35 < `c` 550.81 < `fortran` 777.09 < `cpp` 780.06
- round 5 (run order numba, agent, c, c_reference, cpp, fortran): `agent` 56.94 < `c` 555.09 < `numba` 577.29 < `fortran` 671.27 < `c_reference` 682.30 < `cpp` 706.62
- round 6 (run order agent, c, c_reference, cpp, fortran, numba): `agent` 57.28 < `numba` 397.70 < `fortran` 530.47 < `cpp` 707.25 < `c_reference` 764.37 < `c` 779.11

perf: instructions per series vary by at most 0.06% (CV) within a column; cycles:u per counter-second 3.214-3.216 GHz over all 36 series. vmstat (GCC series, node-wide, whole series process): Spearman(median time, THP faults) = -0.64, Spearman(median time, page faults) = 0.64; compaction stalls 0, page migrations 0.

### `tsvc_2_s2275`

Fastest GCC-column run in this experiment: 478.65 ms.

| column | overall min (ms) | round 1 | round 2 | round 3 | round 4 | round 5 | round 6 | worst / best round | runs within 15% of kernel's fastest GCC run | runs within 15% of own fastest | v2 min | v1 min | IPC range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `c` | 540.15 | 685.57 | 668.07 | 669.41 | 540.15 | 681.69 | 688.19 | 1.274 | 0.6% | 0.6% | 668.73 | 539.68 | 0.29-0.30 |
| `c_reference` | 478.65 | 667.37 | 668.10 | 692.68 | 478.65 | 685.25 | 856.15 | 1.789 | 0.6% | 0.6% | 426.45 | 934.27 | 0.26-0.30 |
| `cpp` | 686.86 | 691.24 | 689.53 | 686.86 | 851.14 | 919.35 | 734.64 | 1.338 | 0.0% | 25.0% | 606.94 | 898.52 | 0.26-0.30 |
| `fortran` | 659.52 | 688.89 | 666.63 | 659.52 | 687.78 | 675.04 | 685.09 | 1.045 | 0.0% | 49.4% | 684.13 | 484.00 | 0.29-0.30 |
| `numba` | 474.19 | 474.19 | 599.45 | 666.87 | 597.71 | 660.27 | 670.24 | 1.413 | 8.3% | 8.3% | 645.97 | 783.22 | 0.30-0.34 |
| `agent` | 71.56 | 71.97 | 72.25 | 72.04 | 71.97 | 71.79 | 71.56 | 1.010 | 100.0% | 99.4% | 75.14 | 74.53 | 1.27-1.29 |

- round 1 (run order c, c_reference, cpp, fortran, numba, agent): `agent` 71.97 < `numba` 474.19 < `c_reference` 667.37 < `c` 685.57 < `fortran` 688.89 < `cpp` 691.24
- round 2 (run order c_reference, cpp, fortran, numba, agent, c): `agent` 72.25 < `numba` 599.45 < `fortran` 666.63 < `c` 668.07 < `c_reference` 668.10 < `cpp` 689.53
- round 3 (run order cpp, fortran, numba, agent, c, c_reference): `agent` 72.04 < `fortran` 659.52 < `numba` 666.87 < `c` 669.41 < `cpp` 686.86 < `c_reference` 692.68
- round 4 (run order fortran, numba, agent, c, c_reference, cpp): `agent` 71.97 < `c_reference` 478.65 < `c` 540.15 < `numba` 597.71 < `fortran` 687.78 < `cpp` 851.14
- round 5 (run order numba, agent, c, c_reference, cpp, fortran): `agent` 71.79 < `numba` 660.27 < `fortran` 675.04 < `c` 681.69 < `c_reference` 685.25 < `cpp` 919.35
- round 6 (run order agent, c, c_reference, cpp, fortran, numba): `agent` 71.56 < `numba` 670.24 < `fortran` 685.09 < `c` 688.19 < `cpp` 734.64 < `c_reference` 856.15

perf: instructions per series vary by at most 0.06% (CV) within a column; cycles:u per counter-second 3.253-3.255 GHz over all 36 series. vmstat (GCC series, node-wide, whole series process): Spearman(median time, THP faults) = -0.51, Spearman(median time, page faults) = 0.46; compaction stalls 0, page migrations 0.

### `tsvc_2_s275`

Fastest GCC-column run in this experiment: 494.38 ms.

| column | overall min (ms) | round 1 | round 2 | round 3 | round 4 | round 5 | round 6 | worst / best round | runs within 15% of kernel's fastest GCC run | runs within 15% of own fastest | v2 min | v1 min | IPC range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `c` | 527.34 | 697.20 | 714.50 | 527.34 | 701.96 | 557.53 | 715.26 | 1.356 | 1.1% | 1.1% | 703.96 | 695.54 | 0.28-0.32 |
| `c_reference` | 559.65 | 559.65 | 697.12 | 800.00 | 801.80 | 696.91 | 789.05 | 1.433 | 0.6% | 0.6% | 720.85 | 717.17 | 0.28-0.33 |
| `cpp` | 521.69 | 521.69 | 619.24 | 715.16 | 568.90 | 778.54 | 713.82 | 1.492 | 0.6% | 8.9% | 719.88 | 714.84 | 0.25-0.33 |
| `fortran` | 494.38 | 796.40 | 608.59 | 494.38 | 712.60 | 712.64 | 712.88 | 1.611 | 0.6% | 0.6% | 545.75 | 679.50 | 0.28-0.32 |
| `numba` | 400.03 | 542.62 | 583.39 | 541.77 | 583.94 | 583.29 | 400.03 | 1.460 | 17.2% | 0.6% | 476.22 | 584.55 | 0.41-0.43 |
| `agent` | 160.92 | 161.72 | 161.56 | 161.49 | 162.06 | 161.37 | 160.92 | 1.007 | 100.0% | 99.4% | 163.87 | 161.90 | 1.43-1.44 |

- round 1 (run order c, c_reference, cpp, fortran, numba, agent): `agent` 161.72 < `cpp` 521.69 < `numba` 542.62 < `c_reference` 559.65 < `c` 697.20 < `fortran` 796.40
- round 2 (run order c_reference, cpp, fortran, numba, agent, c): `agent` 161.56 < `numba` 583.39 < `fortran` 608.59 < `cpp` 619.24 < `c_reference` 697.12 < `c` 714.50
- round 3 (run order cpp, fortran, numba, agent, c, c_reference): `agent` 161.49 < `fortran` 494.38 < `c` 527.34 < `numba` 541.77 < `cpp` 715.16 < `c_reference` 800.00
- round 4 (run order fortran, numba, agent, c, c_reference, cpp): `agent` 162.06 < `cpp` 568.90 < `numba` 583.94 < `c` 701.96 < `fortran` 712.60 < `c_reference` 801.80
- round 5 (run order numba, agent, c, c_reference, cpp, fortran): `agent` 161.37 < `c` 557.53 < `numba` 583.29 < `c_reference` 696.91 < `fortran` 712.64 < `cpp` 778.54
- round 6 (run order agent, c, c_reference, cpp, fortran, numba): `agent` 160.92 < `numba` 400.03 < `fortran` 712.88 < `cpp` 713.82 < `c` 715.26 < `c_reference` 789.05

perf: instructions per series vary by at most 0.06% (CV) within a column; cycles:u per counter-second 3.242-3.243 GHz over all 36 series. vmstat (GCC series, node-wide, whole series process): Spearman(median time, THP faults) = -0.54, Spearman(median time, page faults) = 0.51; compaction stalls 0, page migrations 0.
