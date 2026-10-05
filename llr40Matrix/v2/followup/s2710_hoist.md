# Experiment 2: does hoisting `x[0]` let GCC vectorise `s2710`?

**Hypothesis.** GCC fails to vectorise `tsvc_2_s2710` only because the translated C reads `x[0]`
(a scalar passed as a one-element array) inside the loop, in one branch only. In v2 GCC reports
`relevant stmt not supported: .MASK_LOAD (x_42(D), 64B, ...)` at the `if ((x[0] > 0.0))` line.
Numba (LLVM) vectorises the loop and is 2.4x faster.

**Verdict: confirmed.** With only `const double x0 = x[0];` before the loop and `x0` in place of
`x[0]` inside it, GCC vectorises the loop in all three sources: the translated C (variant A), and
the translated C++ and the hand-written C reference (variant B). All three validate against the
oracle. Min-of-k falls by 2.5-2.7x, from 216.7-220.6 ms to 82.9-87.6 ms. That is level with v2's
Numba cell (93.4 ms) and agent cell (84.5 ms), not 2.4x behind them.

## Setup

- Variant sources: `variants/src/tsvc_2_s2710_fp64.hoist.c`, `...hoist.cpp`,
  `tsvc_2_s2710_reference.hoist.c`, with diffs in `variants/*.hoist.*.patch` (made by
  `make_variants.py`). Each has exactly the one change, plus, for the two emitted files, the
  autogen marker line replaced by a comment so the harness keeps the file (`DEVIATIONS.md` item 5).
- Timed through the v2 harness path with the variant substituted for the column's source
  (`sweep_followup.py --override`). The bytes in place were re-checked after the run. Preset M,
  `float64`, 5 + 30 reps, v2 environment and step geometry, job 4982624, node nid006347, all three
  cells on that one node, one after the other. Compilers as in v2: C gcc 14.2.0, C++ g++ 13.3.1.
- Opt reports and disassembly: `opt_reports/s2710/tsvc_2_s2710/<column>/` (harness argv plus the
  three `-fopt-info` flags, as `v2/opt_reports.py`).

## Per cell

| variant | column | vectorised? (report line) | oracle | min-of-k (ms) | median (ms) | v2, same column (ms) | speedup vs v2 | / v2 `numba` (93.44 ms) |
|---|---|---|---|---:|---:|---:|---:|---:|
| A | `c` | **yes** `opt_reports/s2710/tsvc_2_s2710/c/tsvc_2_s2710_fp64.c.optreport.txt:5`: loop vectorized using variable length vectors | ok | 82.94 | 93.78 | 220.60 | 2.66x | 0.89 |
| B | `cpp` | **yes** `opt_reports/s2710/tsvc_2_s2710/cpp/tsvc_2_s2710_fp64.cpp.optreport.txt:5`: loop vectorized using variable length vectors | ok | 85.49 | 102.91 | 216.71 | 2.53x | 0.91 |
| B | `c_reference` | **yes** `opt_reports/s2710/tsvc_2_s2710/c_reference/tsvc_2_s2710_fp64.c.optreport.txt:5`: loop vectorized using variable length vectors | ok | 87.57 | 101.35 | 219.74 | 2.51x | 0.94 |

v2 cells (632952d, `v2/results.csv`): `c` 220.60, `cpp` 216.71, `c_reference` 219.74, `numba`
93.44, `agent` 84.45 ms.

## What GCC does differently

- **v2, unchanged sources.** GCC unswitches the loop only on `LEN_1D > 10`, then gives up on the
  `x[0]` read: `v2/opt_reports/tsvc_2_s2710/c/tsvc_2_s2710_fp64.c.optreport.txt:5`: `missed: not
  vectorized: relevant stmt not supported: _56 = .MASK_LOAD (x_42(D), 64B, _130);`. The same line
  appears in the C++ report (`...cpp.optreport.txt:5`) and in the **hand-written reference**
  (`v2/opt_reports/tsvc_2_s2710/c_reference/tsvc_2_s2710_fp64.c.optreport.txt:5`), which has the
  same read in the same place. `x[0]` is loaded only on the `a[i] <= b[i]` path, and GCC will not
  speculate a load the source may never execute. If-conversion therefore turns it into a per-lane
  masked load from one invariant address, and the vectoriser does not support that. The v2 loop
  is scalar and branchy (`ldr` 16, `str` 6, `b.gt` 5, `fcmpe` 4 in the kernel's disassembly).
- **Hoisted.** `x0 > 0.0` is now loop-invariant, and GCC unswitches on it first:
  `opt_reports/s2710/tsvc_2_s2710/c/tsvc_2_s2710_fp64.c.optreport.txt:3`: `unswitching loop 1 on
  'if' with condition: x0_37 > 0.0`. Line 4 unswitches on `LEN_1D > 10`, and lines 5-7 vectorise
  the resulting versions with SVE ("variable length vectors"). The disassembly is predicated SVE:
  `ld1d` 19, `st1d` 9, `whilelo` 8, `fcmgt` 3, `fmla` 11, with one scalar `ldr` left (the hoisted
  `x[0]`). The C++ and reference reports read the same (lines 3-5).

One semantic difference comes with the change: the variant reads `x[0]` once, unconditionally,
even if no iteration takes the branch that uses it. For the harness inputs `x` is a valid array of
`LEN_1D` elements, so this is safe here. It is also the reason GCC may not make the change itself.

## Spread

The vectorised loop reads five arrays and writes up to three, so it is bandwidth bound and shows the matrix's diagnosed
two-level bandwidth artifact (protocol.md section 4) inside each series:

| column | fast level (reps) | slow level (reps) | outliers |
|---|---|---|---|
| `c` | 82.9-83.6 ms (14) | 93.8-94.9 ms (15) | 187.2 |
| `cpp` | 85.5-88.3 ms (15) | 117.6-123.2 ms (14) | 537.3 |
| `c_reference` | 87.6-90.2 ms (15) | 112.5-113.2 ms (15) | none |

This is why the RSDs are 20%, 69% and 12% and the medians sit 13-20% above the minima. Even the
slow level is about 2x faster than v2's scalar loop. The comparison with Numba should be read
with that spread in mind. The hoisted C's minimum is 6-11% below v2's Numba minimum, but that Numba
cell was measured in another job, and the difference is smaller than the gap between the two
bandwidth levels. What the experiment settles is that the 2.4x gap is gone, not which of the two
is faster.
