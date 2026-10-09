# Extracted-kernel matrix: summary

Eight kernels Lars extracted (HPCAgent-Bench `26a4f0cf`, `benchmarks/scientific_computing/`), in five
columns, measured with the LLR-40 v2 protocol: preset M, validated against the NumPy oracle before
timing (integers bit-exact, float64 at rtol 1e-9 / atol 1e-11), 5 warm-up + 30 timed runs, min-of-k,
one exclusive node per kernel with one core (job 4990441; the three `comet_int4_gemm` cells that
needed BLAS in job 4996043, DEVIATIONS 9). Same compilers as v2: C gcc 14.2.0, C++ g++ 13.3.1,
Fortran gfortran 13.3.1, Numba 0.67. Predictions were pushed before the first timed run
(`predictions.md`, commit c569108).

Files: `results.csv` (every timing, v2 layout plus provenance/adapter), `cells.csv`, `columns.csv`,
`vectorisation.csv`, `opt_reports/` (line 1 = exact argv; `.s.txt` = disassembly / numba
`inspect_asm()`), `emitted_sources/` (the timed translator output), `adapters/`, `DEVIATIONS.md`,
`precision_probe.py` + `precision_probe.csv` (how far each float cell is from NumPy).

> **Corrections (2026-10-07, `followup/README.md`, `DEVIATIONS.md` item 12).** The integer-only kernels ran
> their fp32 functions: CoMet's C/C++ call `cblas_sgemm` on float32 copies, not `cblas_dgemm`. The `native`
> CoMet cell timed the translated code; the hand-written code is 0.565x of C. The `warpx_boris_push` and
> `warpx_field_gather` Fortran losses come from fresh memory for the per-call temporaries (page faults; the
> deposition's from the protected parentheses), not from gfortran's extra loop counter (round 2); the
> `triangle_count` gap is the floor-division helper (shifts: C 1.55x faster); the hand-written Numba deposition
> loses a third of its time to NRT reference counting. The text below is as first written.

## Status

**33 ok, 1 build_error, 6 unsupported, 0 incorrect, 0 timeout.** x = speedup over translated C (`c`).

| kernel | `c` | `cpp` | `fortran` | `numba` | `native` |
|---|---|---|---|---|---|
| `comet_int4_gemm` | ok 0.068 ms (x1.00, RSD 3.76%) | ok 0.069 ms (x0.99, RSD 3.07%) | ok 0.358 ms (x0.19, RSD 4.76%) | ok 0.155 ms (x0.44, RSD 10.67%) | ok 0.068 ms (x0.99, RSD 1.86%) |
| `quatrex_rgf` | ok 7.426 ms (x1.00, RSD 0.49%) | ok 7.450 ms (x1.00, RSD 0.66%) | ok 6.497 ms (x1.14, RSD 0.67%) | `numba_hand` ok 4.300 ms (x1.73, RSD 0.70%) | **unsupported** |
| `spgemm_hash` | ok 262.766 ms (x1.00, RSD 0.16%) | ok 263.605 ms (x1.00, RSD 0.18%) | ok 182.389 ms (x1.44, RSD 0.21%) | ok 153.984 ms (x1.71, RSD 0.42%) | **unsupported** |
| `warpx_boris_push` | ok 3.030 ms (x1.00, RSD 8.28%) | ok 3.017 ms (x1.00, RSD 5.63%) | ok 3.380 ms (x0.90, RSD 3.94%) | ok 2.043 ms (x1.48, RSD 3.38%) | ok 3.835 ms (x0.79, RSD 0.87%) |
| `warpx_esirkepov_deposition` | ok 2.681 ms (x1.00, RSD 3.03%) | ok 2.588 ms (x1.04, RSD 3.07%) | ok 2.902 ms (x0.92, RSD 1.11%) | `numba_hand` ok 4.387 ms (x0.61, RSD 18.13%) | **unsupported** |
| `warpx_field_gather` | ok 11.336 ms (x1.00, RSD 1.35%) | ok 11.639 ms (x0.97, RSD 1.20%) | ok 12.309 ms (x0.92, RSD 1.13%) | **build_error** | **unsupported** |
| `triangle_count` | ok 45.912 ms (x1.00, RSD 0.23%) | ok 47.568 ms (x0.96, RSD 0.27%) | ok 47.271 ms (x0.97, RSD 0.47%) | ok 31.905 ms (x1.44, RSD 0.73%) | **unsupported** |
| `nfa_frontier` | ok 9580.391 ms (x1.00, RSD 0.53%) | ok 9203.395 ms (x1.04, RSD 0.42%) | ok 9481.686 ms (x1.01, RSD 0.18%) | ok 10133.499 ms (x0.94, RSD 0.22%) | **unsupported** |

Provenance: `c`, `cpp`, `fortran` are translator output (autogen marker) for all 8 kernels. `numba`
is translator output for 6. **`numba_hand`** marks the hand-written Numba overrides of `quatrex_rgf`
and `warpx_esirkepov_deposition` (the translator's Numba for Esirkepov fails in parfors array analysis,
"Dimension mismatch for sx_new"; for QuaTrEx it is correct but keeps the energy loop serial, per the
overrides' docstrings). `native` is the hand-written upstream extraction, through a thin adapter
(`adapters/`): `comet_int4_gemm` from `tests/ports/comet_int4_gemm/comet_int4_gemm_ref.cpp` (the
kernel folder's `comet_int4_gemm_reference.cpp` is translator output), `warpx_boris_push` from its
folder's `_reference.cpp`. The other six are `unsupported` with the reason in `results.csv`: CUDA
`spgemm_hash`, `triangle_count`; Python `quatrex_rgf`; VASim class excerpt `nfa_frontier`; grid
extents that the ABI does not pass for `warpx_esirkepov_deposition`, `warpx_field_gather`.

The one build error, `warpx_field_gather` `numba`, is a translator bug on the graded path
(DEVIATIONS 10): the 3-D tap lowers a 4-D broadcast with axes dropped, and Numba rejects it at typing.

Declared dtypes and graded configuration are in `results.csv` (`declared_dtypes`, `graded_config`).
`comet_int4_gemm` is int4 (stored as int8) with an int32 output. `quatrex_rgf` is complex128.
`spgemm_hash`, `triangle_count` and `nfa_frontier` are int64/uint8. The WarpX kernels are float64,
graded at momentum_push_type 0 (boris) and geom 3, n_rz_azimuthal_modes 1, the remaining branches 0
(esirkepov, field gather).

**Time per call outside 10 ms - 60 s, measured at M as instructed and reported as such:** all
`comet_int4_gemm` (0.07-0.36 ms), `quatrex_rgf` (4.3-7.5 ms), `warpx_boris_push` (2.0-3.8 ms) and
`warpx_esirkepov_deposition` (2.6-4.4 ms) cells are below 10 ms. `warpx_field_gather` is at 11-12 ms.
`nfa_frontier` takes 9.2-10.1 s per call, inside the band. RSD is at most 0.8% on `spgemm_hash`,
`triangle_count`, `nfa_frontier` and `quatrex_rgf`, and 1.1-1.4% on `warpx_field_gather`. It reaches
0.9-8.3% on boris/esirkepov (2-4 ms calls), 1.9-10.7% on comet (0.07-0.36 ms) and 18.1% on esirkepov
`numba_hand`. Differences of that size in those cells are ties.

## Per column: speedup over translated C

Geometric mean over the kernels where both cells are ok, 95% interval from the t distribution on the
log ratios; faster / within 3% / slower. "- comet" repeats the mean without `comet_int4_gemm`, the one
kernel whose translated C/C++ calls a BLAS library (`cblas_dgemm`) where Fortran and Numba get loops.

| column | n | geomean speedup over `c` | 95% interval | faster / within 3% / slower |
|---|---:|---:|---|---|
| `cpp` | 8 | 1.000 | 0.978 - 1.023 | 2 / 5 / 1 |
| `fortran` | 8 | 0.834 | 0.498 - 1.398 | 2 / 2 / 4 |
| `numba` (translator output) | 5 | 1.085 | 0.546 - 2.157 | 3 / 0 / 2 |
| `numba_hand` | 2 | 1.027 | 0.001 - 756 (n = 2) | 1 / 0 / 1 |
| `numba` + `numba_hand` | 7 | 1.068 | 0.647 - 1.765 | 4 / 0 / 3 |
| `native` | 2 | 0.886 | 0.207 - 3.787 | 0 / 1 / 1 |
| `cpp` - comet | 7 | 1.002 | 0.976 - 1.028 | 2 / 4 / 1 |
| `fortran` - comet | 7 | 1.030 | 0.881 - 1.204 | 2 / 2 / 3 |
| `numba` - comet | 4 | 1.362 | 0.908 - 2.042 | 3 / 0 / 1 |

None of the column means differs from 1 at 95%. With 8 kernels or fewer, the intervals are wide.

## Where the GCC columns' vectorisation decisions differ

Vectorised-loop counts from the reports (`vectorisation.csv`). They count loop versions, so the
multiversioned WarpX and quatrex lowerings give large numbers.

| kernel | `c` (gcc 14) | `cpp` (g++ 13) | `fortran` (gfortran 13) | native |
|---|---:|---:|---:|---:|
| `comet_int4_gemm` | 14 | 16 | 24 | 2 |
| `quatrex_rgf` | 159 | 167 | 167 | -- |
| `spgemm_hash` | 4 | 4 | 4 | -- |
| `warpx_boris_push` | 26 | 27 | 27 | **0** |
| `warpx_esirkepov_deposition` | 382 | 462 | 416 | -- |
| `warpx_field_gather` | 447 | 564 | 519 | -- |
| `triangle_count` | 0 | 0 | 0 | -- |
| `nfa_frontier` | 0 | 0 | 0 | -- |

- `triangle_count` and `nfa_frontier` vectorise nothing in any column (data-dependent searches and
  frontier updates), and `spgemm_hash` the same 4 loops in all three. The count differences on the
  large multiversioned kernels (esirkepov 382/462/416, field gather 447/564/519) come with times within
  8% of each other (cpp vs c 0.97-1.04). They are not where the gaps come from (next section).
- **native `warpx_boris_push` vectorises nothing**, against 26 loops in translated C:
  `opt_reports/warpx_boris_push/native/warpx_boris_push_native.cpp.optreport.txt`: "number of
  versioning for alias run-time tests exceeds 10 (--param vect-max-version-for-alias-checks)" at the
  particle loop (reference line 114). The original runs that loop under `omp parallel for`. GCC
  outlines it into `warpx_boris_push_original._omp_fn.0`, which loses the parameters'
  `__restrict__`, so the vectoriser would need more than 10 alias checks. That makes it 0.79x of
  translated C at one thread.

## Every gap above 10% between c, cpp and fortran, and its cause

| kernel | gap | cause (evidence) |
|---|---|---|
| `comet_int4_gemm` | fortran 5.3x slower than c and cpp (0.19x) | **Library call against loop nest.** The C/C++ translator lowers each of the four int32 matmuls to `cblas_dgemm` on double copies (`emitted_sources/comet_int4_gemm/comet_int4_gemm_fp64.c`, lines 218-254); the Fortran translator has no BLAS lowering and writes triple loops. gfortran vectorises their inner sum (opt report, f90 line 58: two partial-sum vectors with `fmla`), but the operand is the translator's transposed copy `x_cb1(j, l)`, which in column-major order is read with a stride of `num_vector` doubles: the loop builds every vector from scalar loads (`ldr d` + `mov v.d[1]`, `fortran/comet_int4_gemm_fp64.f90.s.txt` lines 475-488). The hand-written port (one integer pass over the int8 codes, vectorised at `comet_int4_gemm_ref.cpp:121`) is as fast as the BLAS version (0.99x). Numba's int32 triple loops (59 `mla`) take 2.3x. |
| `spgemm_hash` | fortran 1.44x faster than c and cpp; numba 1.71x | **Integer divisions from the translator's Python-semantics helpers.** `python_mod` for int64 is `(a % b + b) % b` and `int_floor` a truncating division plus fixup. Per compare-and-swap of the bitonic sort (which runs over the whole hash table, 32-4096 slots, for every row), translated C does 3 divisions, two of them dependent (`c/...s.txt` lines 463-470: `udiv`, `sdiv`, `udiv`), and 2 dependent ones in the final merge loop (lines 494, 499). gfortran's `MODULO` costs one division: 2 independent ones in the stage loop (`fortran/...s.txt` lines 7e4/7e8) and 1 in the merge loop (84c). LLVM proves `stride` a power of two and turns `idx % stride` into an `and` (numba `.s.txt` `.LBB0_7`: one `sdiv`), and in the merge loop keeps the remainder as a wrapping counter (no division). The hash slot `python_mod(b_col*107, ts)` costs 2 dependent `sdiv` in C (lines 171/174, 432/435) and one in Fortran and Numba. Order of the division counts = order of the times (263 > 182 > 154 ms). |
| `quatrex_rgf` | fortran 1.14x faster than c and cpp | **C complex arithmetic (C99 Annex G).** The C and C++ disassembly calls libgcc's `__muldc3` (41 call sites) and `__divdc3` (4) for the NaN/Inf-recovery path of complex multiply and divide. The Fortran build has none, because gfortran uses Fortran's complex rules. Same vectorisation counts (cpp 167 = fortran 167). |
| `warpx_boris_push` | fortran 1.12x slower than c and cpp | **Not identified.** The first version of this summary blamed Fortran's protected parentheses for blocking FMA. That holds only for Fortran's SCALAR code (0 `fmadd`/`fmsub`/`fnmsub` against 25 in C). The vectorised loops, which process the particles, use FMA in Fortran as much as in C (vector `fmla`+`fmls`: 19+6 Fortran, 17+6 C). Vectorisation is the same (26-27 loops). |
| `warpx_esirkepov_deposition` | fortran 1.12x slower than cpp (1.08x than c) | **Not identified**, same caveat: no scalar FMA in Fortran, but vector and SVE FMA in all three (Fortran 51+39 NEON, 36 SVE). c-vs-fortran is 8% and cpp-vs-c 3.6%, within the 3% RSD of these 2.6 ms cells for the latter. |

No other pair of GCC columns differs by more than 10%: `warpx_field_gather` fortran/c 1.086 (cause not
identified either; Fortran has vector FMA there too), `nfa_frontier` c/cpp 1.041, `triangle_count` cpp/c
1.036.

## Numba against translated C

- `spgemm_hash` 1.71x: fewer divisions (above).
- `triangle_count` 1.44x: the visible difference is the floor divisions of the binary search.
  Python `// 2` and `// 32` become single arithmetic shifts in LLVM (`numba .s.txt` `.LBB0_8`:
  `asr`, `mul`, `adds ... asr #5`). C, C++ and Fortran go through the translator's `int_floor`, which
  GCC lowers as truncating division plus sign fixups: `add`, `csel`, `asr`, `tst`, `sub`, `csel` before
  each probe's load (`c .s.txt` lines 44-54; Fortran lines 42-51 the same). All three GCC columns run at
  46-48 ms. Consistent with the gap, not proven by static code.
- `warpx_boris_push` 1.48x: LLVM vectorises far more (597 NEON + 48 SVE vector instructions against 64
  in GCC's C). Consistent, not proven.
- `comet_int4_gemm` 0.44x: int32 triple loops against a BLAS call.
- `nfa_frontier` 0.94x: not investigated (6%).
- Hand-written: `quatrex_rgf` 1.73x (BLAS `@` and LAPACK `inv`, against the translator's own complex
  inverse in loops, with `__muldc3`/`__divdc3`); `warpx_esirkepov_deposition` 0.61x (privatised
  scatter with an extra pass over the grids; LLVM emits 67 NEON vector instructions against 1330 in C;
  RSD 18%). The cause of the latter is not isolated.

## Precision (precision_probe.csv, DEVIATIONS 11)

All integer cells are bit-exact (the harness compares integers exactly). Float cells, element by
element against NumPy (one of the harness's two identical comparisons per output):

| kernel | `c` / `cpp` | `fortran` | Numba |
|---|---|---|---|
| `warpx_boris_push` | 24.7% differ, max rel 5.7e-12 | 52.8%, 6.1e-11 | 0 (generated) |
| `warpx_esirkepov_deposition` | 24.1%, 8.9e-6 | 25.2%, 8.9e-6 | 0 (hand-written) |
| `warpx_field_gather` | 20.8%, 7.6e-13 | 27.7%, 1.7e-12 | build error |
| `quatrex_rgf` | 98.5%, 3.4e-12 | 98.5%, 3.9e-12 | 0 (hand-written) |

Numba reproduces NumPy exactly. Fortran is NOT exact on any WarpX kernel, unlike LLR-40, consistent
with its vectorised loops using FMA. The Esirkepov maximum (8.9e-6 in all three GCC columns) is far
above rtol 1e-9, so it can only pass through the absolute part of the tolerance: those elements are
small compared with the largest current. QuaTrEx's GCC columns use the translator's own inverse, not
LAPACK, so their deviation is not FMA alone. All cells pass.

## Prediction against outcome

| kernel | column | predicted | outcome | |
|---|---|---|---|---|
| `comet_int4_gemm` | `c` | ok | ok (after DEVIATIONS 9) | status held; the reason did not: the translator lowers the int32 matmuls to `cblas_dgemm`, not to loops, so on bare-metal daint the cell needed a uenv OpenBLAS |
| | `cpp` | ok | ok (after DEVIATIONS 9) | same |
| | `fortran` | ok | ok | held |
| | `numba` | ok | ok | held |
| | `native` | ok | ok (after DEVIATIONS 9) | held; needed BLAS only because the harness also compiles the translator's `fp32.cpp` (DEVIATIONS 5) |
| `quatrex_rgf` | `c`/`cpp`/`fortran` | ok, risk of incorrect | ok | held; the translated complex inverse stays within rtol 1e-9 |
| | `numba_hand` | ok | ok | held; also the predicted "may beat translated C" (1.73x) |
| | `native` | unsupported | unsupported | held |
| `spgemm_hash` | 4 columns | ok | ok | held (status). Predicted numba slower than GCC: **falsified**, numba 1.71x faster |
| | `native` | unsupported | unsupported | held |
| `warpx_boris_push` | 5 columns | ok | ok | held; Fortran slightly slower as predicted (0.90x), but not for the predicted reason |
| `warpx_esirkepov_deposition` | `c`/`cpp`/`fortran` | ok | ok | held |
| | `numba_hand` | ok, slower than GCC | ok, 0.61x | held |
| | `native` | unsupported | unsupported | held |
| `warpx_field_gather` | `c`/`cpp`/`fortran` | ok | ok | held |
| | `numba` | ok | build_error | **falsified:** the translator mis-lowers a 4-D broadcast in the graded 3-D path; Numba typing fails at the first call (DEVIATIONS 10) |
| | `native` | unsupported | unsupported | held |
| `triangle_count` | 4 columns | ok | ok | held (status). Predicted numba slower: **falsified**, numba 1.44x faster |
| | `native` | unsupported | unsupported | held |
| `nfa_frontier` | 4 columns | ok | ok | held; numba slower as predicted (0.94x) |
| | `native` | unsupported | unsupported | held |

Statuses: 39 of 40 as predicted (36 before the CoMet re-measurement). Totals predicted 34 ok / 6
unsupported, measured 33 ok / 1 build_error / 6 unsupported. Also falsified: Fortran slower on
`quatrex_rgf` (predicted among the FMA kernels; it is 1.14x faster). The per-call time estimates held in
order of magnitude, except `nfa_frontier`, predicted at 1-5 s and measured at 9-10 s.

## Which LLR-40 patterns hold on these kernels

- **C, C++ and Fortran at the same speed: holds for C++, not for Fortran.** cpp/c geomean 1.000
  (0.978-1.023), every kernel within 4.1%. Fortran ranges from 0.19x (CoMet) to 1.44x (SpGEMM),
  geomean 0.834 (0.498-1.398); 1.030 without CoMet.
- **Large gaps from single code-generation decisions: holds,** but the decisions are mostly the
  translator's: a BLAS call only in C/C++ (CoMet), Python-semantics division helpers (SpGEMM, triangle
  count), C's complex rules (QuaTrEx). The compiler-version effect (GCC 14 vs 13, the s316 select) does
  not appear: the largest cpp-vs-c difference is 4%.
- **Fortran parentheses blocking FMA: not the explanation here.** Scalar Fortran code has no FMA, but
  the vectorised loops do. Fortran is 8-10% slower on the three WarpX kernels for a reason we did not
  find, and it is not bit-exact against NumPy on any of them.
- **Numba the most uneven column: holds,** in a different direction. In LLR-40 Numba trailed GCC
  (0.88x). Here translated Numba is 1.44-1.71x faster on three kernels and 0.44x / 0.94x on two; it
  fails to type on one (field gather) and needed hand-written overrides on two.
- **New here, absent from LLR-40:** C's complex arithmetic rules (QuaTrEx, 14%); the translator's
  integer division helpers (SpGEMM, triangle count); a BLAS dependency the bare-metal machine lacks
  (CoMet); a hand-written upstream kernel losing vectorisation to OpenMP outlining (native boris, 0.79x).
