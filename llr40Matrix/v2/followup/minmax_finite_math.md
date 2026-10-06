# Experiment 1: do conditional min/max reductions need `-ffinite-math-only`?

**Hypothesis.** GCC refuses every conditional min/max reduction in the matrix ("not vectorized:
unsupported use in stmt") because it only turns `if (a[i] < x) x = a[i]` into a MIN/MAX reduction
when NaNs can be ignored. The protocol leaves `-ffinite-math-only` out on purpose.

**Verdict: confirmed for the one plain min reduction (`tsvc_2_s316`), falsified for the three
reductions that also carry an index (`tsvc_2_s318`, `tsvc_2_s3110`, `argmax_with_index`).**

- With the flag, `s316` vectorises in all four columns and validates. All four columns run at
  68.4-69.0 ms. Against v2 that is 4.0x faster for `c`/`c_reference` (275.8 ms) and 1.16x faster
  for `cpp`/`fortran` (80.0 ms).
- The other three still do not vectorise, and for the same first reason as in v2. The flag makes
  them **3.4-3.7x slower**: `s318` 80.2 -> 294.4 ms, `s3110` 120.2 -> 414.5 ms, `argmax_with_index`
  71.6 -> 251.0 ms (`c`; the other columns agree within 0.1%). With NaNs ignored, GCC if-converts
  the update into a compare and select on the loop-carried chain, where v2 had a well-predicted branch.

So `-ffinite-math-only` is necessary (for GCC 13 and 14 here) to vectorise a plain conditional
min, but not sufficient for any reduction that also has to carry the position of the extremum.
For the protocol, leaving the flag out costs `s316` and protects the other three.

## Setup

- Variant: the harness compile line plus `-ffinite-math-only` and nothing else. It is one patch to
  `hpcagent_bench/flags.py` (`variants/finite_math_only.patch`) in a separate worktree at
  `26a4f0cf`. The flag goes into the GCC C/C++ and gfortran baselines right after
  `-ffp-contract=fast`. The resolved flags are in the `flags` column of `parts/minmax/*.csv`, and
  the exact compile argv is line 1 of every opt report.
- Everything else is as in the v2 matrix: same harness path (`sweep_followup.py` = v2's
  `sweep_variant.py` plus a `variant` column), same sources (the emitted lowerings, byte-identical
  to v2's; the corpus `_reference.c`), preset M, `float64`, 5 warm-up + 30 timed reps, min-of-k,
  `OMP_NUM_THREADS=1`, `NUMBA_NUM_THREADS=1`, `OMP_PROC_BIND=close`, and one kernel per exclusive
  node in v2's step geometry (`srun --nodes=1 --ntasks=1 -w <node> --exclusive --cpu-bind=cores
  --hint=nomultithread`, timed child on NUMA node 0). Each node first checked that the build child
  sees one core and `-ftree-parallelize-loops=1` (`PROBE ... 1 1` in `logs/`).
- Compilers, identical to v2's: C columns `/usr/bin/gcc-14` 14.2.0 (the harness's `-std=c23`
  floor), C++ and Fortran 13.3.1.
- Every cell was validated against the NumPy oracle before it was timed. All 16 are `ok`.
- Jobs: `tsvc_2_s316` in 4982623; the other three in 4982637 (see `DEVIATIONS.md`, item 3). Every
  srun line is in `srun_lines.txt`.
- Opt reports and disassembly: `opt_reports/minmax/<kernel>/<column>/`, built as
  `v2/opt_reports.py` builds them (`opt_reports_followup.py`). Raw rows with every timing:
  `parts/minmax/`. Table: `minmax_finite_math.csv`.

## Per cell

cyc/elem = min-of-k x 3.26 GHz / elements per call (2e8, 17409^2, 1.8e8). The clock is the
one measured with perf in experiment 4 on the same partition (3.257-3.263 GHz).

| kernel | column | compiler | vectorised with the flag? (report line) | oracle | min-of-k, flag (ms) | min-of-k, v2 (ms) | flag / v2 | cyc/elem flag -> v2 | first missed reason, flag |
|---|---|---|---|---|---:|---:|---:|---|---|
| `tsvc_2_s316` | `c` | gcc 14.2.0 | **yes** `opt_reports/minmax/tsvc_2_s316/c/tsvc_2_s316_fp64.c.optreport.txt:3`: loop vectorized using 16 byte vectors | ok | 68.96 | 275.84 | 0.25 | 1.12 -> 4.50 |  |
| `tsvc_2_s316` | `c_reference` | gcc 14.2.0 | **yes** `opt_reports/minmax/tsvc_2_s316/c_reference/tsvc_2_s316_fp64.c.optreport.txt:3`: loop vectorized using 16 byte vectors | ok | 68.37 | 275.99 | 0.25 | 1.11 -> 4.50 |  |
| `tsvc_2_s316` | `cpp` | g++ 13.3.1 | **yes** `opt_reports/minmax/tsvc_2_s316/cpp/tsvc_2_s316_fp64.cpp.optreport.txt:3`: loop vectorized using 16 byte vectors | ok | 68.93 | 79.98 | 0.86 | 1.12 -> 1.30 |  |
| `tsvc_2_s316` | `fortran` | gfortran 13.3.1 | **yes** `opt_reports/minmax/tsvc_2_s316/fortran/tsvc_2_s316_fp64.f90.optreport.txt:3`: loop vectorized using 16 byte vectors | ok | 68.78 | 80.22 | 0.86 | 1.12 -> 1.31 |  |
| `tsvc_2_s318` | `c` | gcc 14.2.0 | no | ok | 294.37 | 80.18 | 3.67 | 4.80 -> 1.31 | `opt_reports/minmax/tsvc_2_s318/c/tsvc_2_s318_fp64.c.optreport.txt:5`: not vectorized: unsupported use in stmt. |
| `tsvc_2_s318` | `c_reference` | gcc 14.2.0 | no | ok | 294.27 | 80.18 | 3.67 | 4.80 -> 1.31 | `opt_reports/minmax/tsvc_2_s318/c_reference/tsvc_2_s318_fp64.c.optreport.txt:5`: not vectorized: unsupported use in stmt. |
| `tsvc_2_s318` | `cpp` | g++ 13.3.1 | no | ok | 294.34 | 79.74 | 3.69 | 4.80 -> 1.30 | `opt_reports/minmax/tsvc_2_s318/cpp/tsvc_2_s318_fp64.cpp.optreport.txt:5`: not vectorized: unsupported use in stmt. |
| `tsvc_2_s318` | `fortran` | gfortran 13.3.1 | no | ok | 294.36 | 79.99 | 3.68 | 4.80 -> 1.30 | `opt_reports/minmax/tsvc_2_s318/fortran/tsvc_2_s318_fp64.f90.optreport.txt:5`: not vectorized: unsupported use in stmt. |
| `tsvc_2_s3110` | `c` | gcc 14.2.0 | no | ok | 414.54 | 120.16 | 3.45 | 4.46 -> 1.29 | `opt_reports/minmax/tsvc_2_s3110/c/tsvc_2_s3110_fp64.c.optreport.txt:8`: not vectorized: complicated access pattern. (inner loop: unsupported use in stmt.) |
| `tsvc_2_s3110` | `c_reference` | gcc 14.2.0 | no | ok | 414.52 | 120.76 | 3.43 | 4.46 -> 1.30 | `opt_reports/minmax/tsvc_2_s3110/c_reference/tsvc_2_s3110_fp64.c.optreport.txt:4`: not vectorized: complicated access pattern. (inner loop: unsupported use in stmt.) |
| `tsvc_2_s3110` | `cpp` | g++ 13.3.1 | no | ok | 414.70 | 120.39 | 3.44 | 4.46 -> 1.29 | `opt_reports/minmax/tsvc_2_s3110/cpp/tsvc_2_s3110_fp64.cpp.optreport.txt:8`: not vectorized: complicated access pattern. (inner loop: unsupported use in stmt.) |
| `tsvc_2_s3110` | `fortran` | gfortran 13.3.1 | no | ok | 414.65 | 121.47 | 3.41 | 4.46 -> 1.31 | `opt_reports/minmax/tsvc_2_s3110/fortran/tsvc_2_s3110_fp64.f90.optreport.txt:4`: not vectorized: complicated access pattern. (inner loop: unsupported use in stmt.) |
| `argmax_with_index` | `c` | gcc 14.2.0 | no | ok | 251.01 | 71.56 | 3.51 | 4.55 -> 1.30 | `opt_reports/minmax/argmax_with_index/c/argmax_with_index_fp64.c.optreport.txt:4`: not vectorized: unsupported use in stmt. |
| `argmax_with_index` | `c_reference` | gcc 14.2.0 | no | ok | 251.07 | 71.48 | 3.51 | 4.55 -> 1.29 | `opt_reports/minmax/argmax_with_index/c_reference/argmax_with_index_fp64.c.optreport.txt:4`: not vectorized: unsupported use in stmt. |
| `argmax_with_index` | `cpp` | g++ 13.3.1 | no | ok | 251.05 | 71.49 | 3.51 | 4.55 -> 1.29 | `opt_reports/minmax/argmax_with_index/cpp/argmax_with_index_fp64.cpp.optreport.txt:4`: not vectorized: unsupported use in stmt. |
| `argmax_with_index` | `fortran` | gfortran 13.3.1 | no | ok | 251.03 | 71.74 | 3.50 | 4.55 -> 1.30 | `opt_reports/minmax/argmax_with_index/fortran/argmax_with_index_fp64.f90.optreport.txt:4`: not vectorized: unsupported use in stmt. |

RSD of the 16 series is 0.11-1.68%, except `argmax_with_index`/`cpp` at 7.93%. That series has
26 of 30 reps within 0.4% of its minimum and two outliers (259 and 364 ms); its minimum agrees
with the other three columns to 0.02%.

## What changed in the code

**`s316`, plain running minimum.** v2's reports for the same compiler binaries and the same argv
without the flag say "not vectorized: unsupported use in stmt" at the same lines (e.g.
`v2/opt_reports/tsvc_2_s316/c/tsvc_2_s316_fp64.c.optreport.txt:4`, at commit 632952d). With the
flag, both GCC versions vectorise it:

- gcc 14 (`c`, `c_reference`): a 16-byte NEON body and an SVE variable-length version
  (`...c.optreport.txt:4`: "loop vectorized using variable length vectors"), reduced with
  `fminnmv`. Asm counts: `fminnmv=1 fminnm=8 ld1d=1 whilelo=2`, against v2's `fcsel=1 fcmpe=1`.
- g++/gfortran 13 (`cpp`, `fortran`): a 16-byte NEON body with `fminnm` (13 in the asm), against
  v2's `fcmpe` plus a `b.gt` branch.

It is the flag, not a version difference: both versions vectorise with it and neither does without
it. The flag also removes v2's C-versus-C++ gap on this kernel. That gap was gcc 14's `fcsel` chain
against gcc 13's branch (`opt_findings.md` section 3). Now the four columns agree within 0.9%. At
68.4-69.0 ms for a 1.6 GB array the vectorised loop streams at about 23 GB/s, i.e. it is memory
bound.

**`s318`, `s3110`, `argmax_with_index`: an extremum and its position.** Each loop carries an index
next to the value (`index_ = i` / `xindex, yindex` / `idx = i`, updated under the same compare).
With the flag, GCC still reports "not vectorized: unsupported use in stmt", the first missed reason
v2 already had. For `s3110` the first line is the outer loop's "complicated access pattern", and
its inner loop is again "unsupported use in stmt". Ignoring NaNs does not make a coupled value and
index reduction vectorisable for GCC 13/14.

What the flag does change is the scalar code. In v2 the update was a branch to an out-of-line
block that a running maximum over random data almost never takes, so it predicts well and stays off
the critical path (`v2/opt_reports/tsvc_2_s318/c/tsvc_2_s318_fp64.c.s.txt`: `fcmpe` + `b.mi`). With
the flag, GCC if-converts it into `fcmpe` + `fcsel` (value) + `csel` (index)
(`opt_reports/minmax/tsvc_2_s318/c/tsvc_2_s318_fp64.c.s.txt`, offsets 0x28-0x34). The compare
and the select now sit on the loop-carried dependence through `maxv`, so every element pays their
latency: 4.5-4.8 cycles per element instead of 1.3. All four columns and both GCC versions do this.
It is the same mechanism that made v2's gcc-14 `s316` `c` cell 3.4x slower than `cpp`.

## Caveats

- The oracle inputs contain no NaNs (uniform on a finite interval), so `ok` here means the
  vectorised and if-converted code is correct for these inputs. The oracle cannot show where NaN
  semantics differ. That difference is exactly why the protocol omits the flag.
- The comparison is against the v2 cells in `v2/results.csv` (632952d), measured in another job on
  other nodes, as the experiment asks. The changes measured here are 1.16-4.0x, and each kernel's
  four columns agree to within 0.9% (RSD at most 1.7% apart from the one series above), far beyond
  that noise.
