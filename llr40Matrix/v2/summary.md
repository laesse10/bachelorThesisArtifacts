# LLR-40 measurement matrix, v2 -- summary

Benchmark commit `26a4f0cfc1540cead109dfc8d736f1c45263e390` (v1: `e2bceb68`). Agent submissions `2830ed3f` (unchanged).  
Machine: CSCS Alps `daint`, NVIDIA GH200, aarch64 (Neoverse-V2), one kernel per exclusive node.  
Toolchain as the harness actually ran it: **C (`c`, `c_reference`, agent-C) gcc-14 (SUSE Linux) 14.2.0**; C++ g++ (SUSE Linux) 13.3.1 20250313; Fortran GNU Fortran (SUSE Linux) 13.3.1 20250313. Python 3.12.13 (v1: 3.11.13; the v2 commit requires >=3.12, os.CLONE_NEWUSER in seal.py); numba 0.67.0; numpy 2.4.6.  
Preset M, float64, `OMP_NUM_THREADS=1`, 5 warmup + 30 timed reps, min-of-k. Protocol: [../protocol.md](../protocol.md), unchanged.  
Rows by Slurm job: `debug/4969572` 244, `debug/4970062` 18.

## Status, v2 against v1

| status | v1 | v2 |
|---|---:|---:|
| `ok` | 229 | 229 |
| `unsupported` | 6 | 6 |
| `incorrect` | 5 | 5 |
| **cells** | 240 | 240 |

Non-ok cells (rows, not omissions; error text in `notes`):

| kernel | representation | status | note |
|---|---|---|---|
| `compact_threshold_pack` | `c_reference` | unsupported | no hand-written _reference.c in the corpus |
| `scan_affine_decay` | `c_reference` | unsupported | no hand-written _reference.c in the corpus |
| `scatter_accum_dup` | `c_reference` | unsupported | no hand-written _reference.c in the corpus |
| `segment_reduce_ragged` | `c_reference` | unsupported | no hand-written _reference.c in the corpus |
| `tsvc_2_s115` | `c` | incorrect | output did not match the NumPy reference |
| `tsvc_2_s115` | `cpp` | incorrect | output did not match the NumPy reference |
| `tsvc_2_s115` | `fortran` | incorrect | output did not match the NumPy reference |
| `tsvc_2_s115` | `c_reference` | incorrect | source=tsvc_2_s115_reference.c sha256=123993dec309bef2; output did not match the NumPy ref |
| `tsvc_2_s115` | `agent` | incorrect | rank=5 arm=llr40v10-qwen38-c job=621383 seq=2 lang=c campaign_speedup=2.027 sha256=790a607 |
| `tsvc_2_s2233` | `agent` | unsupported | no agent submission in either campaign |
| `versioned_distance_update` | `c_reference` | unsupported | no hand-written _reference.c in the corpus |

## What changed against v1 (`diff_vs_v1.csv`)

Timed source across the two commits, all 240 cells: identical 118, preprocessed_identical 78, comments_only 33, code 5, n.a. (no _reference.c at either commit) 5, n.a. (no agent submission) 1. `preprocessed_identical` = the text differs but `gcc -E` of both is identical (the new prelude macro `NPB_HD` expands to nothing on the host).

**26 cells** moved by more than 5% in min-of-k or changed status (0 status changes).

By source change: identical 14, preprocessed_identical 7, comments_only 5.

| kernel | repr | v2/v1 | RSD v1 % | RSD v2 % | source | explanation |
|---|---|---:|---:|---:|---|---|
| `tsvc_2_s2275` | `c_reference` | 0.4565 | 4.66 | 17.34 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s2275` | `cpp` | 0.6755 | 5.08 | 16.68 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s235` | `fortran` | 0.7368 | 4.35 | 11.07 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s235` | `cpp` | 0.7501 | 14.73 | 13.58 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s1232` | `cpp` | 0.786 | 5.64 | 33.43 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s275` | `fortran` | 0.8032 | 7.97 | 14.0 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s275` | `numba` | 0.8147 | 11.53 | 8.83 | comments_only | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s2275` | `numba` | 0.8248 | 7.52 | 6.12 | comments_only | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s231` | `c_reference` | 0.8316 | 5.21 | 19.98 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s1232` | `fortran` | 0.8973 | 1.08 | 4.06 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s233` | `numba` | 0.9192 | 10.89 | 2.26 | comments_only | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s233` | `fortran` | 0.9202 | 3.37 | 3.95 | identical | source identical for the compiler; change is harness/environment/noise |
| `ext_break_capture` | `agent` | 0.921 | 1.07 | 0.61 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s231` | `c` | 0.9426 | 3.55 | 3.93 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s231` | `cpp` | 0.9446 | 2.81 | 6.5 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s1244` | `agent` | 1.0565 | 5.51 | 4.23 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s252` | `c_reference` | 1.0584 | 3.61 | 2.84 | identical | source identical for the compiler; change is harness/environment/noise |
| `wf_diff_skew` | `agent` | 1.0753 | 0.4 | 1.07 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s231` | `fortran` | 1.0846 | 4.93 | 7.28 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s235` | `numba` | 1.1596 | 7.81 | 16.41 | comments_only | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s235` | `c` | 1.1777 | 3.7 | 4.23 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s2275` | `c` | 1.2391 | 17.02 | 17.02 | preprocessed_identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s1232` | `c_reference` | 1.3258 | 6.67 | 16.49 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s1232` | `numba` | 1.343 | 12.44 | 7.09 | comments_only | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s2275` | `fortran` | 1.4135 | 10.21 | 15.18 | identical | source identical for the compiler; change is harness/environment/noise |
| `tsvc_2_s235` | `c_reference` | 1.6325 | 13.93 | 4.08 | identical | source identical for the compiler; change is harness/environment/noise |

Of the flagged cells whose source did not change, 19 have RSD > 5% in at least one of the two series (the diagnosed bandwidth artifact, protocol.md section 4).

## Agent column: every attempted candidate (`agent_attempts.csv`)

61 candidates attempted over 39 kernels: ok 38, build_error 18, incorrect 5.

Kernels whose agent cell is not the campaign's top pick, and why:

| kernel | v1 rank | v2 rank | rejected candidates (rank: status -- first error line) |
|---|---:|---:|---|
| `argmax_with_index` | 5 | 5 | 1: build_error -- argmax_with_index_fp64.c:16:10: fatal error: immintrin.h: No such file; 2: build_error -- argmax_with_index_fp64.c:16:10: fatal error: immintrin.h: No such file; 3: build_error -- argmax_with_index_fp64.c:16:10: fatal error: immintrin.h: No such file; 4: build_error -- argmax_with_index_fp64.c:25:10: fatal error: immintrin.h: No such file |
| `compact_threshold_pack` | 2 | 2 | 1: build_error -- compact_threshold_pack_fp64.c:3:10: fatal error: immintrin.h: No such  |
| `ext_break_capture` | 2 | 2 | 1: build_error -- ext_break_capture_fp64.c:2:10: fatal error: immintrin.h: No such file  |
| `fuse_diamond` | 3 | 3 | 1: build_error -- fuse_diamond_fp64.c:2:10: fatal error: immintrin.h: No such file or di; 2: build_error -- fuse_diamond_fp64.c:2:10: fatal error: immintrin.h: No such file or di |
| `scatter_accum_dup` | 2 | 2 | 1: build_error -- scatter_accum_dup_fp64.c:4:10: fatal error: xmmintrin.h: No such file  |
| `tsvc_2_s115` | 5 | none ok in 5 | 1: incorrect -- C (gcc) - default: NaN position mismatch: expected 20364 NaN, got 2036; 2: incorrect -- C (gcc) - default: NaN position mismatch: expected 20364 NaN, got 2036; 3: incorrect -- Fortran (gfortran) - default: NaN position mismatch: expected 20364 Na; 4: incorrect -- C (gcc) - default: NaN position mismatch: expected 20364 NaN, got 2036; 5: incorrect -- C (gcc) - default: NaN position mismatch: expected 20364 NaN, got 2036 |
| `tsvc_2_s1232` | 2 | 2 | 1: build_error -- tsvc_2_s1232_fp64.c:2:10: fatal error: immintrin.h: No such file or di |
| `tsvc_2_s233` | 2 | 2 | 1: build_error -- tsvc_2_s233_fp64.c:4:10: fatal error: immintrin.h: No such file or dir |
| `tsvc_2_s3110` | 3 | 3 | 1: build_error -- tsvc_2_s3110_fp64.c:23:10: fatal error: immintrin.h: No such file or d; 2: build_error -- tsvc_2_s3110_fp64.c:18:10: fatal error: immintrin.h: No such file or d |
| `tsvc_2_s3111` | 2 | 2 | 1: build_error -- tsvc_2_s3111_fp64.c:6:10: fatal error: immintrin.h: No such file or di |
| `tsvc_2_s318` | 2 | 2 | 1: build_error -- tsvc_2_s318_fp64.c:8:10: fatal error: immintrin.h: No such file or dir |
| `tsvc_2_s319` | 3 | 3 | 1: build_error -- tsvc_2_s319_fp64.c:13:10: fatal error: immintrin.h: No such file or di; 2: build_error -- tsvc_2_s319_fp64.c:11:10: fatal error: immintrin.h: No such file or di |
| `tsvc_2_vag` | 2 | 2 | 1: build_error -- tsvc_2_vag_fp64.c:2:10: fatal error: immintrin.h: No such file or dire |

## Optimization classes

14 classes over 40 kernels (31 declared, 9 derived). Largest: `reduction` 10/40. 9 classes have <=2 members (13 kernels).

**Discrepancy with protocol.md, left as written there:** protocol.md section 5 says reductions are 8 of 40, 13 classes, 7 with <=2 members. `labels.csv` gives reduction 10, 14 classes, 9 with <=2 members. Protocol's 8 reductions equals the 8 DECLARED (manifest) reduction labels; the derived `argmax_with_index`, `quasi_affine_reduce_odd` make 10. Its 13 classes / 7 small classes match neither the declared-only labels (12 / 8) nor the full set, and the repository history (one commit) does not show where they came from.

| class | kernels |
|---|---:|
| reduction | 10 |
| control_flow | 5 |
| loop_interchange | 5 |
| indirect_addressing | 4 |
| loop_fusion | 3 |
| dependence_distance | 2 |
| linear_dependence | 2 |
| scalar_expansion | 2 |
| wavefront | 2 |
| packing | 1 |
| node_splitting | 1 |
| interprocedural_dataflow | 1 |
| loop_distribution | 1 |
| recurrence | 1 |

The 9 derived labels were checked by hand against the source: see [derived_labels_check.md](derived_labels_check.md) (6 agree, 2 disputable, 1 disputed). Under the corpus's own label for its TSVC original (`s121`: induction variables), `ext_war_unit` would be the roster's only `induction_variable` member.

## Run-to-run spread

Over 229 ok cells: median RSD 1.17%, max 41.10%; 54 (23.6%) bimodal-suspect (RSD > 5%). All aggregates use min-of-k.

## Geometric-mean slowdown per class x representation

### v1 method (all ok cells, normalised to the row's fastest)

| class | n | `c` | `c_reference` | `cpp` | `fortran` | `numba` | `agent` |
|---|---:|---:|---:|---:|---:|---:|---:|
| control_flow | 5 | 1.92 | 1.92 | 1.92 | 1.84 | 1.50 | 1.00 |
| dependence_distance | 2 | 1.58 | 1.01 | 1.58 | 1.64 | 1.64 | 1.01 |
| indirect_addressing | 4 | 1.02 | 1.02 | 1.02 | 1.06 | 1.16 | 1.31 |
| interprocedural_dataflow | 1 | 1.23 | 1.00 | 1.21 | 1.22 | 1.23 | 1.00 |
| linear_dependence | 2 | 1.23 | 1.24 | 1.24 | 1.23 | 1.10 | 1.00 |
| loop_distribution | 1 | 8.90 | 5.68 | 8.08 | 9.11 | 8.60 | 1.00 |
| loop_fusion | 3 | 2.60 | 1.10 | 2.60 | 2.60 | 3.13 | 1.01 |
| loop_interchange | 5 | 5.35 | 5.45 | 5.19 | 5.24 | 5.42 | 1.00 |
| node_splitting | 1 | 1.02 | 1.00 | 1.01 | 1.01 | 1.01 | 1.09 |
| packing | 1 | 1.04 | -- | 1.05 | 1.05 | 1.00 | 1.35 |
| recurrence | 1 | 1.04 | 1.03 | 1.10 | 1.00 | 1.02 | 1.03 |
| reduction | 10 | 1.22 | 1.25 | 1.08 | 1.31 | 1.74 | 1.21 |
| scalar_expansion | 2 | 1.09 | 1.12 | 1.13 | 1.12 | 1.10 | 1.02 |
| wavefront | 2 | 1.00 | 1.01 | 1.00 | 1.00 | 1.99 | 1.31 |

Kept for comparability with v1. A kernel missing a column is normalised to a different 'fastest' column than its class, which is what produced the `tsvc_2_s2233` pseudo-outliers.

### Complete-case kernels, normalised to the row's fastest

| class | n | `c` | `c_reference` | `cpp` | `fortran` | `numba` | `agent` |
|---|---:|---:|---:|---:|---:|---:|---:|
| control_flow | 5 | 1.92 | 1.92 | 1.92 | 1.84 | 1.50 | 1.00 |
| dependence_distance | 1 | 1.02 | 1.01 | 1.02 | 1.00 | 1.02 | 1.02 |
| indirect_addressing | 2 | 1.03 | 1.02 | 1.03 | 1.07 | 1.02 | 1.05 |
| interprocedural_dataflow | 1 | 1.23 | 1.00 | 1.21 | 1.22 | 1.23 | 1.00 |
| linear_dependence | 1 | 1.23 | 1.24 | 1.24 | 1.23 | 1.21 | 1.00 |
| loop_distribution | 1 | 8.90 | 5.68 | 8.08 | 9.11 | 8.60 | 1.00 |
| loop_fusion | 3 | 2.60 | 1.10 | 2.60 | 2.60 | 3.13 | 1.01 |
| loop_interchange | 4 | 8.10 | 8.33 | 7.77 | 7.92 | 8.12 | 1.00 |
| node_splitting | 1 | 1.02 | 1.00 | 1.01 | 1.01 | 1.01 | 1.09 |
| recurrence | 1 | 1.04 | 1.03 | 1.10 | 1.00 | 1.02 | 1.03 |
| reduction | 9 | 1.25 | 1.25 | 1.08 | 1.32 | 1.81 | 1.14 |
| scalar_expansion | 2 | 1.09 | 1.12 | 1.13 | 1.12 | 1.10 | 1.02 |
| wavefront | 2 | 1.00 | 1.01 | 1.00 | 1.00 | 1.99 | 1.31 |

Only kernels with all 6 columns ok (33 of 40).

### Complete-case kernels, normalised to `c`

| class | n | `c` | `c_reference` | `cpp` | `fortran` | `numba` | `agent` |
|---|---:|---:|---:|---:|---:|---:|---:|
| control_flow | 5 | 1.00 | 1.00 | 1.00 | 0.96 | 0.78 | 0.52 |
| dependence_distance | 1 | 1.00 | 1.00 | 1.00 | 0.98 | 1.00 | 1.00 |
| indirect_addressing | 2 | 1.00 | 0.99 | 1.00 | 1.04 | 0.99 | 1.02 |
| interprocedural_dataflow | 1 | 1.00 | 0.81 | 0.99 | 0.99 | 1.00 | 0.82 |
| linear_dependence | 1 | 1.00 | 1.01 | 1.00 | 1.00 | 0.99 | 0.81 |
| loop_distribution | 1 | 1.00 | 0.64 | 0.91 | 1.02 | 0.97 | 0.11 |
| loop_fusion | 3 | 1.00 | 0.42 | 1.00 | 1.00 | 1.21 | 0.39 |
| loop_interchange | 4 | 1.00 | 1.03 | 0.96 | 0.98 | 1.00 | 0.12 |
| node_splitting | 1 | 1.00 | 0.98 | 0.99 | 0.99 | 0.99 | 1.07 |
| recurrence | 1 | 1.00 | 1.00 | 1.06 | 0.97 | 0.99 | 1.00 |
| reduction | 9 | 1.00 | 1.00 | 0.87 | 1.06 | 1.45 | 0.91 |
| scalar_expansion | 2 | 1.00 | 1.02 | 1.03 | 1.02 | 1.01 | 0.93 |
| wavefront | 2 | 1.00 | 1.01 | 1.00 | 1.00 | 1.99 | 1.31 |

1.00 = the autogen C column; < 1 is faster than `c`.

Excluded from the complete-case aggregates:

| kernel | class | non-ok columns |
|---|---|---|
| `versioned_distance_update` | dependence_distance | c_reference:unsupported |
| `scatter_accum_dup` | indirect_addressing | c_reference:unsupported |
| `segment_reduce_ragged` | indirect_addressing | c_reference:unsupported |
| `tsvc_2_s115` | linear_dependence | c:incorrect c_reference:incorrect cpp:incorrect fortran:incorrect agent:incorrect |
| `tsvc_2_s2233` | loop_interchange | agent:unsupported |
| `compact_threshold_pack` | packing | c_reference:unsupported |
| `scan_affine_decay` | reduction | c_reference:unsupported |

### Outliers (>= 3x from the class geomean)

v1 method: 8; complete-case: 3; complete-case vs `c`: 5.

Complete-case, normalised to the row's fastest:

| kernel | representation | class | cell | class geomean | deviation |
|---|---|---|---:|---:|---:|
| `tsvc_2_s3111` | `fortran` | reduction | 5.833 | 1.322 | 4.41x |
| `tsvc_2_s316` | `c` | reduction | 3.98 | 1.248 | 3.19x |
| `tsvc_2_s316` | `c_reference` | reduction | 3.982 | 1.254 | 3.18x |

## The three numerically degenerate kernels

Largest size at which the NumPy reference output is 100% finite (binary search on `LEN_2D`, harness inputs, `finite_search.py`):

| kernel | preset M | largest finite | non-monotone points in the +-16 scan |
|---|---:|---:|---|
| `tsvc_2_s115` | 20481 | **118** | finite above: 121 |
| `wf_triangular` | 17409 | **517** | none |
| `wf_diff_skew` | 12289 | **1017** | none |

**Preset F** (all 6 columns at that size, full protocol; `preset=F`, outside every aggregate):

| kernel | `c` | `c_reference` | `cpp` | `fortran` | `numba` | `agent` |
|---|---|---|---|---|---|---|
| `tsvc_2_s115` | ok 11.7 us | ok 10.9 us | ok 11.4 us | ok 12.2 us | ok 5.8 us | ok 33.9 us |
| `wf_diff_skew` | ok 263.7 us | ok 262.8 us | ok 262.5 us | ok 262.9 us | ok 304.1 us | ok 464.4 us |
| `wf_triangular` | ok 88.4 us | ok 88.3 us | ok 88.4 us | ok 88.7 us | ok 314.1 us | ok 142.5 us |

**`tsvc_2_s115` with `-ffp-contract=off`** substituted for `-ffp-contract=fast` (preset M, `flags_variant` set, outside every aggregate):

| column | status (fast) | status (off) | min (fast) ms | min (off) ms |
|---|---|---|---:|---:|
| `c` | incorrect | ok | 83.06 | 84.49 |
| `cpp` | incorrect | ok | 83.30 | 85.22 |
| `fortran` | incorrect | ok | 84.34 | 85.21 |
| `c_reference` | incorrect | ok | 83.97 | 85.05 |

**FMA contraction alone explains the incorrect verdict**: with contraction off, all four GCC columns validate at preset M, and nothing else in the flag string differs.

## `-ftree-parallelize-loops` A/B (fortran column, reduction kernels)

10 kernels (`tsvc_2_s3110` is itself one of the reduction class, so the requested '10 reductions + s3110' is 10 kernels). Ratio = min-of-k without / with; > 1 means the flag helps.

In the protocol's geometry the flag resolves to `-ftree-parallelize-loops=1`: geomean ratio **0.9992** (range 0.9931-1.0074); parloops outlined no loop in either arm (GOMP_parallel call sites: 0 / 0).

Supplementary, whole-node step geometry (flag resolved to `-ftree-parallelize-loops=72`, `supplementary_wholenode/`): geomean **1.0021** (range 0.9977-1.0104); GOMP_parallel call sites in the with arm: 0.

| kernel | order | min with (ms) | min without (ms) | ratio | RSD with % | RSD without % |
|---|---|---:|---:|---:|---:|---:|
| `argmax_with_index` | with,without | 71.79 | 71.60 | 0.9975 | 0.44 | 0.68 |
| `quasi_affine_reduce_odd` | without,with | 63.87 | 63.67 | 0.9969 | 0.62 | 0.69 |
| `scan_affine_decay` | with,without | 147.00 | 147.56 | 1.0038 | 0.29 | 0.38 |
| `tsvc_2_s311` | without,with | 137.08 | 136.27 | 0.9941 | 1.73 | 0.43 |
| `tsvc_2_s3110` | with,without | 119.38 | 120.26 | 1.0074 | 11.03 | 22.63 |
| `tsvc_2_s3111` | without,with | 730.75 | 727.40 | 0.9954 | 2.65 | 0.38 |
| `tsvc_2_s3112` | with,without | 80.25 | 80.55 | 1.0036 | 0.31 | 0.5 |
| `tsvc_2_s316` | without,with | 80.30 | 80.48 | 1.0023 | 0.78 | 0.77 |
| `tsvc_2_s318` | with,without | 80.83 | 80.65 | 0.9977 | 0.45 | 0.77 |
| `tsvc_2_s319` | without,with | 76.06 | 75.53 | 0.9931 | 0.41 | 0.57 |

## Optimisation reports

240 cells indexed in `opt_reports_index.csv`: ok 234, unsupported 6. Findings: [opt_findings.md](opt_findings.md).

## Incidents and deviations

See [DEVIATIONS.md](DEVIATIONS.md): every way this run departs from the task text or from v1, and every discarded or supplementary run, with the reason.

