# LLR-40 measurement matrix -- summary

Machine: CSCS Alps `daint`, NVIDIA GH200, aarch64 (Neoverse-V2), exclusive nodes.  
Toolchain: GNU 13.3.1.  Preset M, float64, `OMP_NUM_THREADS=1`.  
Protocol: [protocol.md](protocol.md), pinned before measuring.

## What was measured

- **40 kernels x 6 representations = 240 cells** at preset M.
- Per cell: 5 warmup runs discarded, then 30+ timed runs, **every individual timing stored** in `time_ns_all`.
- NumPy is the correctness oracle for every cell and is **not** a timed column.
- Plus **18 extra rows at preset S** for the three kernels whose reference overflows at M (see below). They carry `preset=S` and are excluded from all aggregates.

| status | cells | share |
|---|---:|---:|
| `ok` | 229 | 95.4% |
| `unsupported` | 6 | 2.5% |
| `incorrect` | 5 | 2.1% |

## What did not work

11 of 240 cells did not produce a validated timing. These are recorded rows carrying their error text in `notes`, not omissions.

| representation | non-ok cells |
|---|---:|
| `c_reference` | 6 |
| `agent` | 2 |
| `c` | 1 |
| `cpp` | 1 |
| `fortran` | 1 |

Kernels with the most non-ok cells:

| kernel | non-ok | statuses |
|---|---:|---|
| `tsvc_2_s115` | 5 | incorrect |
| `compact_threshold_pack` | 1 | unsupported |
| `scan_affine_decay` | 1 | unsupported |
| `scatter_accum_dup` | 1 | unsupported |
| `segment_reduce_ragged` | 1 | unsupported |
| `tsvc_2_s2233` | 1 | unsupported |
| `versioned_distance_update` | 1 | unsupported |

## Optimization-class distribution

14 classes over 40 kernels; labels are 31 declared (from the manifest's `loop_level_reasoning.category`) and 9 derived from the kernel source.

| class | kernels | share |
|---|---:|---:|
| reduction | 10 | 25.0% |
| control_flow | 5 | 12.5% |
| loop_interchange | 5 | 12.5% |
| indirect_addressing | 4 | 10.0% |
| loop_fusion | 3 | 7.5% |
| dependence_distance | 2 | 5.0% |
| linear_dependence | 2 | 5.0% |
| scalar_expansion | 2 | 5.0% |
| wavefront | 2 | 5.0% |
| packing | 1 | 2.5% |
| node_splitting | 1 | 2.5% |
| interprocedural_dataflow | 1 | 2.5% |
| loop_distribution | 1 | 2.5% |
| recurrence | 1 | 2.5% |

**The corpus does not support strong cross-class comparison.** The largest class is `reduction` at 10/40 (25%) -- not dominant -- but **9 of 14 classes have <=2 members**, covering 13 kernels. A per-class geometric mean over one or two kernels is an anecdote, not an estimate. The task's expected class `induction_variable` has **no member at all** in this roster.

## Run-to-run spread and the bandwidth artifact

Over 229 `ok` cells: median RSD **1.07%**, max **33.44%**. **51 cells (22.3%) are flagged bimodal-suspect** (RSD > 5%).

An intermittent ~1.6x slowdown affects some series in contiguous blocks. It was diagnosed (see protocol.md): CPU frequency, core migration, huge-page fallback and NUMA balancing are all **excluded by direct measurement**; the counters show the core stalled on DRAM at constant clock. It is external memory-system interference, not a property of the code.

**Every aggregate therefore uses min-of-k, not the median or mean.** On the pilot kernel this is decisive: across the five compiled/agent cells the spread of the MINIMUM is 1.01%, while the spread of the MEAN is 25.45% -- the latter being entirely artifact.

Cells flagged bimodal-suspect:

| kernel | representation | RSD % | max/min |
|---|---|---:|---:|
| `compact_threshold_pack` | `numba` | 33.4 | 2.975 |
| `compact_threshold_pack` | `c` | 30.5 | 2.787 |
| `tsvc_2_s323` | `numba` | 23.5 | 2.413 |
| `tsvc_2_vpvts` | `agent` | 21.6 | 2.242 |
| `tsvc_2_s3110` | `cpp` | 21.4 | 1.611 |
| `tsvc_2_s252` | `agent` | 21.3 | 2.224 |
| `tsvc_2_s252` | `fortran` | 21.2 | 2.222 |
| `tsvc_2_s3110` | `c_reference` | 20.9 | 1.596 |
| `tsvc_2_s3110` | `fortran` | 20.6 | 1.585 |
| `tsvc_2_s3112` | `c` | 20.2 | 2.154 |
| `tsvc_2_s1232` | `c` | 19.4 | 1.509 |
| `tsvc_2_s2275` | `c` | 17.0 | 2.085 |
| `compact_threshold_pack` | `cpp` | 16.9 | 1.974 |
| `fuse_stencil_through_transient` | `fortran` | 16.2 | 1.928 |
| `tsvc_2_s3110` | `c` | 16.1 | 1.558 |
| `tsvc_2_s319` | `agent` | 16.0 | 1.913 |
| `fuse_stencil_through_transient` | `c` | 15.7 | 1.912 |
| `tsvc_2_s2233` | `numba` | 15.5 | 1.666 |
| `versioned_distance_update` | `agent` | 15.0 | 1.85 |
| `tsvc_2_s235` | `cpp` | 14.7 | 1.413 |

## Geometric-mean slowdown per class x representation

| class | `c` | `c_reference` | `cpp` | `fortran` | `numba` | `agent` |
|---|---|---|---|---|---|---|
| control_flow | 1.89 | 1.90 | 1.91 | 1.90 | 1.56 | 1.00 |
| dependence_distance | 1.57 | 1.02 | 1.57 | 1.64 | 1.64 | 1.01 |
| indirect_addressing | 1.01 | 1.02 | 1.02 | 1.06 | 1.15 | 1.31 |
| interprocedural_dataflow | 1.24 | 1.00 | 1.23 | 1.24 | 1.23 | 1.01 |
| linear_dependence | 1.24 | 1.24 | 1.25 | 1.24 | 1.10 | 1.00 |
| loop_distribution | 7.24 | 12.54 | 12.06 | 6.49 | 10.51 | 1.00 |
| loop_fusion | 2.63 | 1.11 | 2.59 | 2.59 | 3.11 | 1.00 |
| loop_interchange | 5.28 | 4.91 | 5.89 | 5.74 | 5.04 | 1.00 |
| node_splitting | 1.00 | 1.00 | 1.00 | 1.04 | 1.03 | 1.05 |
| packing | 1.05 | -- | 1.05 | 1.04 | 1.00 | 1.35 |
| recurrence | 1.03 | 1.03 | 1.10 | 1.00 | 1.02 | 1.03 |
| reduction | 1.22 | 1.25 | 1.08 | 1.31 | 1.74 | 1.20 |
| scalar_expansion | 1.08 | 1.08 | 1.11 | 1.11 | 1.11 | 1.04 |
| wavefront | 1.00 | 1.00 | 1.00 | 1.00 | 1.98 | 1.26 |

(1.00 = fastest representation for that kernel, averaged geometrically within the class.)

## Outliers

8 cells deviate from their class geomean by >=3x. These are the most interesting cases for follow-up.

| kernel | representation | class | cell | class geomean | deviation | direction |
|---|---|---|---:|---:|---:|---|
| `tsvc_2_s2233` | `cpp` | loop_interchange | 1.0 | 5.891 | 0.17x | faster than class |
| `tsvc_2_s2233` | `fortran` | loop_interchange | 1.009 | 5.745 | 0.18x | faster than class |
| `tsvc_2_s2233` | `c` | loop_interchange | 1.01 | 5.28 | 0.19x | faster than class |
| `tsvc_2_s2233` | `c_reference` | loop_interchange | 1.001 | 4.914 | 0.2x | faster than class |
| `tsvc_2_s2233` | `numba` | loop_interchange | 1.081 | 5.045 | 0.21x | faster than class |
| `tsvc_2_s3111` | `fortran` | reduction | 5.825 | 1.313 | 4.44x | slower than class |
| `tsvc_2_s316` | `c` | reduction | 3.99 | 1.22 | 3.27x | slower than class |
| `tsvc_2_s316` | `c_reference` | reduction | 3.99 | 1.253 | 3.18x | slower than class |

## Numerical degeneracy at preset M -- including two cells that pass vacuously

Preset sizes are chosen for performance, not numerical sanity. Three kernels accumulate geometrically and their output is mostly not a number at preset M. Measured by running the compiled kernel on the real preset-M inputs and counting finite outputs:

| kernel | n | output elems | finite | inf | NaN | verdict |
|---|---:|---:|---:|---:|---:|---|
| `tsvc_2_s115` | 20,481 | 20,481 | 0.57% | 2 | 20,362 | DEGENERATE: majority non-finite |
| `wf_triangular` | 17,409 | 303,073,281 | 50.91% | 8,863,111 | 139,921,436 | DEGENERATE: majority non-finite |
| `wf_diff_skew` | 12,289 | 151,019,521 | 9.52% | 4,982,354 | 131,657,331 | DEGENERATE: majority non-finite |
| `tsvc_2_s3110` | 17,409 | 4 | 100.0% | 0 | 0 | healthy |

**This splits two ways, and the mechanism is exact.**

- `tsvc_2_s115` computes `a[i] -= aa[j,i] * a[j]` -- a **multiply-add**. `-ffp-contract=fast` fuses it into an FMA, which rounds differently from NumPy's separate multiply and subtract. In a kernel whose values span 300+ decades that last-bit difference changes *where* overflow happens, so the arrays genuinely differ and all four GCC columns are recorded `incorrect`. **numba is the only column that passes** -- it never sets `fastmath`, so it reproduces NumPy bit-for-bit including the inf/NaN pattern.
- `wf_triangular` and `wf_diff_skew` are **pure additions** (`a[i,j] + a[i-1,j] + ...`). There is no multiply for `-ffp-contract=fast` to fuse, so every representation produces the *identical* overflowed array, and the oracle comparison succeeds. **These two are recorded `ok` while being 50.9% and 9.5% finite respectively.** Their timings measure real work, but their correctness verdict is vacuous -- two identical garbage arrays compared equal. Do not read those `ok`s as evidence the representations are correct.

All three are additionally measured at preset S (n=64), where the reference stays finite; those rows carry `preset=S` and all pass. At ~10 us they are call-overhead dominated, so they establish validity, not performance.

## Threats to comparability

1. **gfortran reassociates floating point, gcc-C does not.** Under the pinned flag set (`-fno-math-errno -fno-trapping-math -fno-signed-zeros`, no `-fassociative-math`), gfortran treats those flags as licence to reassociate while gcc-C and clang do not -- the benchmark authors measured this as a 3.1x C-vs-Fortran gap living entirely in the flag list. **Reduction is the largest class here**, so any C-vs-Fortran difference on a reduction kernel is suspect.
2. **gfortran additionally receives `-ftree-parallelize-loops`, gcc-C/C++ do not** -- only the gfortran block declares `doconcurrent_ref`, and it is appended outside any build-mode check.
3. **The `agent` column is not like-for-like.** Submissions were produced on different hardware (AMD MI300A, x86_64) against a different baseline, and are OpenMP-parallel code timed at ONE thread. On the pilot kernel an agent submission with a recorded campaign speedup of 14.83x ran at exactly baseline speed here. The column measures 'agent source, run serially'.
4. **`c` and `c_reference` differ in origin, not just text** -- translator-generated versus hand-written C. The corpus keeps them distinct deliberately.
5. **`-ffast-math` was NOT used**, contrary to the original task text: it changes computed values and would fail the NumPy oracle. Recorded as a knowing deviation.
6. **Three kernels are numerically unusable at preset M.** `tsvc_2_s115`, `wf_triangular` and `wf_diff_skew` grow geometrically; s115 overflows to inf/nan by n=128 while preset M is n=20481. Their preset-M correctness verdicts reflect that, not code quality -- and numba passes where the GCC columns fail purely because numba never sets `fastmath` and so reproduces NumPy bit-for-bit.

