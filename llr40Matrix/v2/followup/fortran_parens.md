# Experiment 2b: do protected parentheses explain every Fortran-vs-C gap?

**Hypothesis.** The translator wraps every Fortran subexpression in parentheses (`sum_val =
(sum_val + a(i))`, `((c*y) + x)`). gfortran honours them by default (`-fprotect-parens`): it keeps
them as `PAREN_EXPR` and neither reassociates nor contracts across them. That should (1) stop
reductions from vectorising and (2) keep scalar loops from using FMA, and the two effects together
should explain all of the reproducible Fortran-vs-C gaps in the matrix.

**Verdict: the mechanism and all four timing predictions are confirmed. "Explains all gaps" is
falsified, and so is effect 1 for `quasi_affine_reduce_odd`.**

- **Effect 1, reductions.** With `-fno-protect-parens`, `s3111`, `s311`, `s319` and
  `segment_reduce_ragged` vectorise, as reassociated vector partial sums plus a horizontal `faddp`
  (no in-order `fadda`). `quasi_affine_reduce_odd` does not. Its refusal changes from "not
  vectorized: unsupported use in stmt" to "Loop costings may not be worthwhile": the parentheses
  caused the old refusal, but without them the cost model still declines.
- **Effect 2, FMA.** `scan_affine_decay`, `versioned_distance_update` and `s323` switch from
  `fmul`+`fadd` to `fmadd`.
- **The four predictions** (cycles per element from perf over the 30 timed reps, C on the same node):
  1. `s3111` goes past C: 72.9 ms against C's 123.8 ms, 1.19 against 2.01 cycles/element, 1.70x
     faster (Fortran default: 720.3 ms, 11.71).
  2. `scan_affine_decay` goes to C's value: 5.03 -> **4.04** cycles/element (C: 4.04).
  3. `versioned_distance_update` goes to C's value: 11.20 -> **10.16** (C measured here: 10.15;
     the "10.4" came from v2's minimum at an assumed clock).
  4. `s323` loses its advantage: Fortran default 4.53 cycles/element (2.7% fewer than C's 4.66),
     with the flag 4.95, now 6.3% MORE than C.
- **Not all gaps.** Four kernels whose Fortran is reproducibly 4-6% slower than C in v1 and in v2
  are untouched. For `scatter_accum_dup` (F/C 1.054 in v1, 1.057 in v2), `tsvc_2_s252` (1.056,
  1.040), `tsvc_2_s4112` (1.053, 1.044) and `tsvc_2_vag` (1.046, 1.038), the flag leaves the
  instruction sequence unchanged and the time within 0.3%. The largest reproducible gap in the
  other direction, `tsvc_2_s316` (F/C 0.29), is C's gcc-14 `fcsel` chain (`opt_findings.md`
  section 3), not Fortran's parentheses.
- **No cell stopped validating** (the harness tolerance is below). `tsvc_2_s115` is `incorrect` in
  both arms, as it was in v2: it is the FMA-contraction case.
- **Bit for bit:** default Fortran reproduces NumPy exactly on all 10 outputs of the eight kernels.
  C and C++ reproduce it on the seven reduction outputs but not on the three FMA kernels. With the
  flag, Fortran's FMA results become byte-identical to C's, and its reassociated sums differ from
  NumPy by 1.1e-13 to 2.4e-13 relative.

## Setup

- Variant: the harness compile line plus `-fno-protect-parens` and nothing else, in the gfortran
  baseline only (`variants/fno_protect_parens.patch`, worktree at `26a4f0cf`). C and C++ flags are
  unchanged. The exact argv is line 1 of every opt report.
- **All 40 kernels, Fortran column**, full protocol: validate against the oracle, then 5 warm-up +
  30 timed reps, preset M, `float64`, v2 environment and step geometry. Each kernel ran as an A/B
  on one node: the unchanged Fortran cell and the flag cell, one after the other, with the order
  alternating along `fparens_roster.txt` (as v2's `-ftree-parallelize-loops` A/B did). So "moved by
  more than 5%" compares like with like. For the eight kernels, the C cell was timed in the
  unchanged arm too, so that the predictions are checked against C on the same node. perf counted
  cycles and instructions over the timed reps of every cell (user space; every cell 30 gated reps).
- Jobs: 4990019 (10 nodes, all 40 kernels); 4990145 (both arms of five kernels re-measured on one
  node after a harness database lock, `DEVIATIONS.md` item 14); 4990146 (opt reports, compile-only).
- Opt reports and disassembly for both arms of all 40 kernels: `opt_reports/fparens/default/` and
  `opt_reports/fparens/noparens/` (as `v2/opt_reports.py` builds them).
- Outputs, bit for bit: for the eight kernels, separate single-rep runs of four builds (`c`, `cpp`,
  `fortran` unchanged, `fortran` with the flag). A hook (`outdump/sitecustomize.py`) saved, on the
  harness's first validation call, the arrays the harness itself compares: the oracle's output and
  the build's output, from the harness's own preset-M inputs and seed. The oracle was saved once
  per kernel, and every later run's oracle was checked bitwise against it: identical in all 24
  runs (30 outputs). The arrays are 24 GB in all, so they are not in git. They are in
  `/capstor/store/cscs/userlab/g34/lhulsbergen/llr40_followup/fparens_outputs/` (`oracle/`,
  `default/`, `noparens/`), and every array's sha256 is in `fortran_parens_outputs.csv`.
- "NumPy" means the harness oracle. At `26a4f0cf` that is the NumPy reference compiled by numba
  (`njit_reference`: no fastmath, no parallel). On the harness's preset-S inputs it is bitwise
  identical to the same reference run interpreted, for all eight kernels (`oracle_njit_check.py`,
  `oracle_njit_check.txt`).

## The eight kernels

cyc/elem = perf cycles / (30 x elements per call); elements = 2e8, 2.2e8, 5.7e7, 9e7 (odd `i`),
6e7 (`row_ptr[NSEG]`), 95e6-1, 95e6-1, 57e6-1. All cells `ok`. cycles:u per counter-second: 3.19-3.29 GHz.

| kernel | C: min-of-k (ms), cyc/elem | Fortran default: min, cyc/elem | Fortran `-fno-protect-parens`: min, cyc/elem | flag / default | vectorised with the flag? (report line) | asm, default -> flag |
|---|---|---|---|---:|---|---|
| `tsvc_2_s3111` | 123.76, **2.012** | 720.31, **11.708** | 72.87, **1.188** | 0.101 | yes `opt_reports/fparens/noparens/tsvc_2_s3111/fortran/tsvc_2_s3111_fp64.f90.optreport.txt:3`: loop vectorized using 16 byte vectors | fadd_scalar=1 -> fadd_scalar=4 fadd_vector=10 faddp=3 |
| `tsvc_2_s311` | 138.54, **2.011** | 138.59, **2.010** | 76.43, **1.111** | 0.551 | yes `opt_reports/fparens/noparens/tsvc_2_s311/fortran/tsvc_2_s311_fp64.f90.optreport.txt:3`: loop vectorized using 16 byte vectors | fadd_scalar=1 -> fadd_scalar=3 fadd_vector=10 faddp=2 |
| `tsvc_2_s319` | 75.39, **4.223** | 75.16, **4.219** | 72.60, **4.072** | 0.966 | yes `opt_reports/fparens/noparens/tsvc_2_s319/fortran/tsvc_2_s319_fp64.f90.optreport.txt:3`: loop vectorized using 16 byte vectors | fadd_scalar=4 -> fadd_scalar=4 fadd_vector=13 faddp=2 |
| `quasi_affine_reduce_odd` | 63.49, **2.277** | 63.22, **2.274** | 63.03, **2.268** | 0.997 | no `opt_reports/fparens/noparens/quasi_affine_reduce_odd/fortran/quasi_affine_reduce_odd_fp64.f90.optreport.txt:4`: Loop costings may not be worthwhile. | fadd_scalar=1 -> fadd_scalar=1 |
| `segment_reduce_ragged` | 38.54, **2.092** | 39.41, **2.139** | 40.46, **2.198** | 1.027 | yes `opt_reports/fparens/noparens/segment_reduce_ragged/fortran/segment_reduce_ragged_fp64.f90.optreport.txt:5`: loop vectorized using 16 byte vectors | fmul_scalar=1 fadd_scalar=1 -> fmadd=2 fmla_vec=7 fadd_vector=3 faddp=2 |
| `scan_affine_decay` | 116.65, **4.043** | 145.16, **5.032** | 116.40, **4.038** | 0.802 | no `opt_reports/fparens/noparens/scan_affine_decay/fortran/scan_affine_decay_fp64.f90.optreport.txt:4`: not vectorized, possible dependence between data-refs (*y_12(D))[_2] and (*y_12(D))[i_l0_18] | fmul_scalar=1 fadd_scalar=1 -> fmadd=1 |
| `versioned_distance_update` | 298.82, **10.150** | 327.67, **11.197** | 298.69, **10.159** | 0.912 | yes `opt_reports/fparens/noparens/versioned_distance_update/fortran/versioned_distance_update_fp64.f90.optreport.txt:3`: loop vectorized using 16 byte vectors | fmla_vec=1 fmul_scalar=4 fadd_scalar=2 -> fmadd=2 fmla_vec=1 fmul_scalar=2 |
| `tsvc_2_s323` | 80.40, **4.659** | 77.75, **4.533** | 85.49, **4.954** | 1.099 | no `opt_reports/fparens/noparens/tsvc_2_s323/fortran/tsvc_2_s323_fp64.f90.optreport.txt:4`: not vectorized, possible dependence between data-refs (*b_14(D))[_1] and (*b_14(D))[i_l0_24] | fmul_scalar=2 fadd_scalar=2 -> fmadd=2 |

Notes:
- `s311` and `quasi_affine_reduce_odd` accumulate into an array element (`sum_out(1)`, `out(1)`),
  not a scalar. With the flag, `s311` still vectorises and runs at 1.11 cycles/element, 1.81x
  faster than C's in-order `fadda` (2.01). In v2 this kernel showed "the same refusal, without a
  time cost", and the flag turns it into a large gain over C.
- `s319` vectorises, but the gain is small (4.22 -> 4.07 cycles/element): the loop also stores two
  arrays, and the reduction was not the bottleneck.
- `segment_reduce_ragged` vectorises with `fmla` and becomes 2.7% *slower* (2.14 -> 2.20). Its
  inner trip count is 24 on average, so the vector loop's set-up and horizontal reduction cost more
  than they save.
- `scan_affine_decay` and `s323` are recurrences ("possible dependence between data-refs") with
  or without the flag. Only their FMA use changes.
- `versioned_distance_update`'s C cell and flag cell each have RSD 12.4%, from one slow rep each. The
  other cells have RSD 0.1-3.3%. The min-of-k and the perf cycles per element are unaffected.

## Bit for bit against NumPy

| kernel | output | elements | C | C++ | Fortran default | Fortran `-fno-protect-parens` |
|---|---|---:|---|---|---|---|
| `tsvc_2_s3111` | `b` | 2 | **identical** | **identical** | **identical** | 1 differ; max abs 1.21e-02, rel 2.43e-13, 1,592 ULP |
| `tsvc_2_s311` | `sum_out` | 220,000,000 | **identical** | **identical** | **identical** | 1 differ; max abs 7.68e-07, rel 1.96e-13, 1,649 ULP |
| `tsvc_2_s319` | `a` | 57,000,000 | **identical** | **identical** | **identical** | **identical** |
| `tsvc_2_s319` | `b` | 57,000,000 | **identical** | **identical** | **identical** | 1 differ; max abs 3.89e-06, rel 2.23e-13, 1,045 ULP |
| `quasi_affine_reduce_odd` | `out` | 1 | **identical** | **identical** | **identical** | **identical** |
| `segment_reduce_ragged` | `out` | 2,500,000 | **identical** | **identical** | **identical** | 1,022,002 differ; max abs 5.09e-11, rel 4.55e-15, 28 ULP |
| `scan_affine_decay` | `y` | 95,000,000 | 33,543,995 differ; max abs 3.55e-15, rel 6.84e-16, 5 ULP | 33,543,995 differ; max abs 3.55e-15, rel 6.84e-16, 5 ULP | **identical** | 33,543,995 differ; max abs 3.55e-15, rel 6.84e-16, 5 ULP |
| `versioned_distance_update` | `a` | 95,000,000 | 35,120,724 differ; max abs 1.78e-15, rel 6.73e-16, 4 ULP | 35,120,724 differ; max abs 1.78e-15, rel 6.73e-16, 4 ULP | **identical** | 35,120,724 differ; max abs 1.78e-15, rel 6.73e-16, 4 ULP |
| `tsvc_2_s323` | `a` | 57,000,000 | 56,896,242 differ; max abs 2.43e-05, rel 4.54e-07, 2,820,637,470 ULP | 56,896,242 differ; max abs 2.43e-05, rel 4.54e-07, 2,820,637,470 ULP | **identical** | 56,896,242 differ; max abs 2.43e-05, rel 4.54e-07, 2,820,637,470 ULP |
| `tsvc_2_s323` | `b` | 57,000,000 | 56,896,249 differ; max abs 2.43e-05, rel 1.21e-07, 723,593,091 ULP | 56,896,249 differ; max abs 2.43e-05, rel 1.21e-07, 723,593,091 ULP | **identical** | 56,896,249 differ; max abs 2.43e-05, rel 1.21e-07, 723,593,091 ULP |

- **Default Fortran is bitwise identical to NumPy for every output.** With the parentheses
  protected, gfortran evaluates each statement exactly as the NumPy loop does: same operations,
  same order, each rounded separately.
- **C and C++** (byte-identical to each other everywhere) match NumPy wherever no FMA is formed:
  the reductions stay in order (`fadda`, or scalar), so all seven reduction outputs are identical.
  On the three FMA kernels, between 35% and almost 100% of elements differ. `scan_affine_decay` and
  `versioned_distance_update` stay within 4-5 ULP. `s323` carries the difference along its
  recurrence: values reach 3.5e9, and the largest gaps are 2.4e-5 absolute and 4.5e-7 relative,
  where the latter is at small elements (2.8e9 ULP there).
- **Fortran with the flag** matches NumPy only where its arithmetic did not change (`s319`'s `a`,
  `quasi_affine_reduce_odd`). On the three FMA kernels its outputs are byte-identical to C's, the
  same sha256. The reassociated sums (`s3111`, `s311`, `s319`'s `b[0]`) differ by 1.1e-13 to
  2.4e-13 relative (1045-1649 ULP), and `segment_reduce_ragged`'s per-segment sums by at most
  28 ULP.
- Every one of these builds validates. The harness compares with `allclose` at rtol 1e-9 and an
  absolute floor raised to `eps * sqrt(n) * max|ref|` (`frameworks/utilities.py:compare_arrays`).
  For `s323` that floor is 5.9e-3, against a largest absolute difference of 2.4e-5.

## Other kernels: Fortran min-of-k moving by more than 5%

Same-node A/B, flag / default. Same instruction sequence = identical mnemonics in the kernel's
disassembly (registers and operand order may differ).

| kernel | flag / default | same instruction sequence? | evidence | reading |
|---|---:|---|---|---|
| `fuse_stencil_through_transient` | 0.931 | yes (2 instructions differ in register numbers only) | vectorised in both arms, same report line | not a code change. The unchanged arm was the slow one: 120.3 ms against v2's 114.4 |
| `tsvc_2_s1232` | 1.247 | yes (identical) | "control flow in loop" in both | noise. Bandwidth-unstable (experiment 3); default-arm RSD 17% |
| `tsvc_2_s231` | 1.229 | yes (identical) | "control flow in loop" in both | noise. Bandwidth-unstable; RSD 8% |
| `tsvc_2_s2275` | 1.160 | no: `fmadd` replaces `fmul`+`fadd` | not vectorised in either ("not suitable for strided load") | undecided. Code changed, but this is a bandwidth-unstable kernel (same code varies up to 1.8x between rounds in experiment 3; default-arm RSD 14%) |
| `tsvc_2_s235` | 1.381 | no: `fmadd` replaces `fmul`+`fadd` | not vectorised in either ("control flow in loop") | undecided, same reason (default-arm RSD 17%) |
| `tsvc_2_s275` | 1.113 | no: `fmadd` replaces `fmul`+`fadd` | not vectorised in either ("control flow in loop") | undecided, same reason (default-arm RSD 12%) |

Every other kernel moved by less than 5%. Among the eight, `s319` (0.966) and
`segment_reduce_ragged` (1.027) moved by less than 5%; the other six are discussed above. The full
per-kernel record is in `fortran_parens.csv`: status, timings, report line, asm counts, and
instruction-sequence comparison.

## Reproducible Fortran-vs-C gaps in v1 and v2, and what the flag does to them

A gap is "reproducible" here if Fortran/C min-of-k is at least 3% off parity, in the same
direction, in both v1 and v2. `tsvc_2_s1232` (1.61, 1.42) is excluded as bandwidth noise
(experiment 3).

| kernel | F/C v1 | F/C v2 | Fortran with the flag / v2 C | flag changes the code? | explained by the parentheses? |
|---|---:|---:|---:|---|---|
| `tsvc_2_s3111` | 5.824 | 5.760 | 0.576 | yes (vectorised partial sums) | **yes** |
| `scan_affine_decay` | 1.246 | 1.252 | 0.984 | yes (`fmadd`) | **yes** |
| `versioned_distance_update` | 1.110 | 1.092 | 0.986 | yes (`fmadd`) | **yes** |
| `tsvc_2_s323` | 0.971 | 0.965 | 1.030 | yes (`fmadd`) | **yes** (just under the 3% rule in v1; in the hypothesis) |
| `scatter_accum_dup` | 1.054 | 1.057 | 1.033 | no | **no** |
| `tsvc_2_s252` | 1.056 | 1.040 | 1.044 | no | **no** |
| `tsvc_2_s4112` | 1.053 | 1.044 | 1.024 | no | **no** |
| `tsvc_2_vag` | 1.046 | 1.038 | 1.015 | no | **no** |
| `tsvc_2_s316` | 0.289 | 0.291 | 0.291 | no | **no** (C's gcc-14 `fcsel` chain) |

The four unexplained Fortran-slower gaps are small (4-6%), and their cause was not investigated
here. The C columns are built by gcc 14.2.0 and Fortran by gfortran 13.3.1, so a code-generation
difference between the two front ends or versions is the obvious candidate.
