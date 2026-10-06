# Extracted-kernel matrix: summary

Eight kernels Lars extracted (HPCAgent-Bench `26a4f0cf`, `benchmarks/scientific_computing/`), in five
columns, measured with the LLR-40 v2 protocol: preset M, validated against the NumPy oracle before
timing (integers bit-exact, float64 at rtol 1e-9 / atol 1e-11), 5 warm-up + 30 timed runs, min-of-k,
one exclusive node per kernel with one core (job 4990441). Same compilers as v2: C gcc 14.2.0,
C++ g++ 13.3.1, Fortran gfortran 13.3.1, Numba 0.67. Predictions were pushed before the first timed
run (`predictions.md`, commit c569108).

Files: `results.csv` (every timing, v2 layout plus provenance/adapter), `cells.csv`, `columns.csv`,
`vectorisation.csv`, `opt_reports/` (line 1 = exact argv; `.s.txt` = disassembly / numba
`inspect_asm()`), `emitted_sources/` (the timed translator output), `adapters/`, `DEVIATIONS.md`.

## Status

**30 ok, 4 build_error, 6 unsupported, 0 incorrect, 0 timeout.** x = speedup over translated C (`c`).

| kernel | `c` | `cpp` | `fortran` | `numba` | `native` |
|---|---|---|---|---|---|
| `comet_int4_gemm` | **build_error** | **build_error** | ok 0.358 ms (RSD 4.76%) | ok 0.155 ms (RSD 10.67%) | **build_error** |
| `quatrex_rgf` | ok 7.426 ms (x1.00, RSD 0.49%) | ok 7.450 ms (x1.00, RSD 0.66%) | ok 6.497 ms (x1.14, RSD 0.67%) | `numba_hand` ok 4.300 ms (x1.73, RSD 0.70%) | **unsupported** |
| `spgemm_hash` | ok 262.766 ms (x1.00, RSD 0.16%) | ok 263.605 ms (x1.00, RSD 0.18%) | ok 182.389 ms (x1.44, RSD 0.21%) | ok 153.984 ms (x1.71, RSD 0.42%) | **unsupported** |
| `warpx_boris_push` | ok 3.030 ms (x1.00, RSD 8.28%) | ok 3.017 ms (x1.00, RSD 5.63%) | ok 3.380 ms (x0.90, RSD 3.94%) | ok 2.043 ms (x1.48, RSD 3.38%) | ok 3.835 ms (x0.79, RSD 0.87%) |
| `warpx_esirkepov_deposition` | ok 2.681 ms (x1.00, RSD 3.03%) | ok 2.588 ms (x1.04, RSD 3.07%) | ok 2.902 ms (x0.92, RSD 1.11%) | `numba_hand` ok 4.387 ms (x0.61, RSD 18.13%) | **unsupported** |
| `warpx_field_gather` | ok 11.336 ms (x1.00, RSD 1.35%) | ok 11.639 ms (x0.97, RSD 1.20%) | ok 12.309 ms (x0.92, RSD 1.13%) | **build_error** | **unsupported** |
| `triangle_count` | ok 45.912 ms (x1.00, RSD 0.23%) | ok 47.568 ms (x0.96, RSD 0.27%) | ok 47.271 ms (x0.97, RSD 0.47%) | ok 31.905 ms (x1.44, RSD 0.73%) | **unsupported** |
| `nfa_frontier` | ok 9580.391 ms (x1.00, RSD 0.53%) | ok 9203.395 ms (x1.04, RSD 0.42%) | ok 9481.686 ms (x1.01, RSD 0.18%) | ok 10133.499 ms (x0.94, RSD 0.22%) | **unsupported** |

Provenance: `c`, `cpp`, `fortran` are translator output (autogen marker) for all 8 kernels. `numba`
is translator output for 6. **`numba_hand`** marks the hand-written Numba overrides of `quatrex_rgf`
and `warpx_esirkepov_deposition`. `native` is the hand-written upstream extraction, through a thin
adapter (`adapters/`): `comet_int4_gemm` from `tests/ports/comet_int4_gemm/comet_int4_gemm_ref.cpp`,
`warpx_boris_push` from its folder's `_reference.cpp`. The other six are `unsupported` with the
reason in `results.csv`: CUDA `spgemm_hash`, `triangle_count`; Python `quatrex_rgf`; VASim class
excerpt `nfa_frontier`; grid extents that the ABI does not pass for `warpx_esirkepov_deposition`,
`warpx_field_gather`.

Declared dtypes and graded configuration are in `results.csv` (`declared_dtypes`, `graded_config`).
`comet_int4_gemm` is int4 (stored as int8) with an int32 output. `quatrex_rgf` is complex128.
`spgemm_hash`, `triangle_count` and `nfa_frontier` are int64/uint8. The WarpX kernels are float64,
graded at momentum_push_type 0 (boris) and geom 3, n_rz_azimuthal_modes 1, the remaining branches 0
(esirkepov, field gather).

**Time per call outside 10 ms - 60 s, measured at M as instructed and reported as such:** all
`comet_int4_gemm` (0.16-0.36 ms), `quatrex_rgf` (4.3-7.5 ms), `warpx_boris_push` (2.0-3.8 ms) and
`warpx_esirkepov_deposition` (2.6-4.4 ms) cells are below 10 ms. `warpx_field_gather` is at 11-12 ms.
`nfa_frontier` takes 9.2-10.1 s per call, inside the band. RSD is at most 0.8% on the long kernels.
It reaches 3-8% on boris/esirkepov (2-4 ms calls), 10.7% on comet numba (0.16 ms) and 18.1% on
esirkepov `numba_hand`.

## Per column: speedup over translated C

Geometric mean over the kernels where both cells are ok, 95% interval from the t distribution on the
log ratios; faster / within 3% / slower. `comet_int4_gemm` has no `c` cell, so it is in no ratio.

| column | n | geomean speedup over `c` | 95% interval | faster / within 3% / slower |
|---|---:|---:|---|---|
| `cpp` | 7 | 1.002 | 0.976 - 1.028 | 2 / 4 / 1 |
| `fortran` | 7 | 1.030 | 0.881 - 1.204 | 2 / 2 / 3 |
| `numba` (translator output) | 4 | 1.362 | 0.908 - 2.042 | 3 / 0 / 1 |
| `numba_hand` | 2 | 1.027 | 0.001 - 756 (n = 2) | 1 / 0 / 1 |
| `numba` + `numba_hand` | 6 | 1.240 | 0.806 - 1.906 | 4 / 0 / 2 |
| `native` | 1 | 0.790 | -- (n = 1) | 0 / 0 / 1 |

None of the column means differs from 1 at 95%. With 7 kernels or fewer, the intervals are wide.

## Where the GCC columns' vectorisation decisions differ

Vectorised-loop counts from the reports (`vectorisation.csv`). They count loop versions, so the
multiversioned WarpX and quatrex lowerings give large numbers.

| kernel | `c` (gcc 14) | `cpp` (g++ 13) | `fortran` (gfortran 13) | native |
|---|---:|---:|---:|---:|
| `comet_int4_gemm` | build_error | build_error | 24 | 2 |
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
| `spgemm_hash` | fortran 1.44x faster than c and cpp | **The translator's Python-modulo helper in C.** `python_mod` for int64 is `(a % b + b) % b`, so the hash slot `python_mod(b_col * 107, ts)` costs two dependent 64-bit divisions (`opt_reports/spgemm_hash/c/spgemm_hash_fp64.c.s.txt`: `sdiv` at lines 171 and 174, again at 432 and 435) in both hash-insertion loops. gfortran's `MODULO` costs one `sdiv` per use (`.../fortran/spgemm_hash_fp64.f90.s.txt`, lines 210, 280, 292). Numba (LLVM) lowers the same modulo to one `sdiv` plus `csel` fixups, and is faster still (1.71x). |
| `quatrex_rgf` | fortran 1.14x faster than c and cpp | **C complex arithmetic (C99 Annex G).** The C and C++ disassembly calls libgcc's `__muldc3` (41 call sites) and `__divdc3` (4) for the NaN/Inf-recovery path of complex multiply and divide. The Fortran build has none, because gfortran uses Fortran's complex rules. Same vectorisation counts (cpp 167 = fortran 167). |
| `warpx_boris_push` | fortran 1.12x slower than c and cpp | **Fortran's protected parentheses block FMA** (LLR-40 follow-up 2b). The Fortran disassembly has no scalar `fmadd`/`fmsub`/`fnmsub` (C: 19/3/3, C++: 20/3/3), and 73 `fmul` + 27 `fadd` + 7 `fsub` instead of 45 + 7 + 1. Vectorisation is the same (26-27 loops). |
| `warpx_esirkepov_deposition` | fortran 1.12x slower than cpp (1.08x than c) | **Same, no FMA in Fortran:** `fmadd` 0 against 137 (C++) / 403 (C), `fmsub` 0 against 49 / 109. The c-vs-fortran gap is 8%, below the 10% line. |

No other pair of GCC columns differs by more than 10%: `warpx_field_gather` fortran/c 1.086 (also 0
`fmadd` against 348), `nfa_frontier` c/cpp 1.041, `triangle_count` cpp/c 1.036.

## Prediction against outcome

| kernel | column | predicted | outcome | |
|---|---|---|---|---|
| `comet_int4_gemm` | `c` | ok | build_error | **falsified:** the translator lowers the int32 matmuls to `cblas_dgemm`; `cblas.h` is absent on bare-metal daint (DEVIATIONS 4) |
| | `cpp` | ok | build_error | **falsified**, same |
| | `fortran` | ok | ok | held |
| | `numba` | ok | ok | held |
| | `native` | ok | build_error | **falsified:** the adapter compiles, but the cpp build also compiles the translator's `fp32.cpp`, which needs `cblas.h` (DEVIATIONS 5) |
| `quatrex_rgf` | `c`/`cpp`/`fortran` | ok, risk of incorrect | ok | held; the translated complex inverse stays within rtol 1e-9 |
| | `numba_hand` | ok | ok | held; also the predicted "may beat translated C" (1.73x) |
| | `native` | unsupported | unsupported | held |
| `spgemm_hash` | 4 columns | ok | ok | held (status). Predicted numba slower than GCC: **falsified**, numba 1.71x faster |
| | `native` | unsupported | unsupported | held |
| `warpx_boris_push` | 5 columns | ok | ok | held; Fortran slightly slower as predicted (0.90x) |
| `warpx_esirkepov_deposition` | `c`/`cpp`/`fortran` | ok | ok | held |
| | `numba_hand` | ok, slower than GCC | ok, 0.61x | held |
| | `native` | unsupported | unsupported | held |
| `warpx_field_gather` | `c`/`cpp`/`fortran` | ok | ok | held |
| | `numba` | ok | build_error | **falsified:** numba typing fails at the first call ("cannot index array(float64, 2d, C) with 3 indices"), in a config branch it types but never runs |
| | `native` | unsupported | unsupported | held |
| `triangle_count` | 4 columns | ok | ok | held (status). Predicted numba slower: **falsified**, numba 1.44x faster |
| | `native` | unsupported | unsupported | held |
| `nfa_frontier` | 4 columns | ok | ok | held; numba slower as predicted (0.94x) |
| | `native` | unsupported | unsupported | held |

Statuses: 36 of 40 as predicted. Totals predicted 34 ok / 6 unsupported, measured 30 ok / 4 build_error
/ 6 unsupported. The per-call time estimates held in order of magnitude, except `nfa_frontier`, which
was predicted at 1-5 s and measured at 9-10 s.

## Which LLR-40 patterns hold on these kernels

- **Fortran parentheses blocking FMA: holds.** On all three float WarpX kernels the Fortran build has
  no scalar `fmadd`, and it is 0.90x / 0.92x / 0.92x of translated C. Parentheses blocking
  REDUCTION vectorisation does not show here: no kernel has a float reduction that decides its time.
- **GCC 14 (C) against GCC 13 (C++): no difference.** cpp/c geomean 1.002 (0.976-1.028). The largest
  single difference is 4% (`nfa_frontier`, `triangle_count`). The s316-type 3x effect, where GCC 14
  turns a well-predicted branch into a select on the loop-carried chain, does not appear.
- **Branch against select: no decisive case.** Select counts differ between the GCC columns on the
  search kernels (`triangle_count` csel 7 in C against 3 in C++), with time differences of 3-4% at
  most.
- **Numba against GCC: does NOT hold.** In LLR-40, Numba trailed GCC. Here the translator's Numba
  output is faster than translated C on 3 of 4 kernels: `spgemm_hash` 1.71x (single-division modulo,
  above), `warpx_boris_push` 1.48x, `triangle_count` 1.44x. For boris, LLVM vectorises far more:
  597 NEON + 48 SVE vector instructions against 64 in GCC's C. That is consistent with the speedup but
  not proven by the static counts. For `triangle_count` the reports do not show why
  (**unexplained**). Numba is slower only on `nfa_frontier` (0.94x). Hand-written Numba wins with
  LAPACK on `quatrex_rgf` (1.73x) and loses with its privatised scatter on
  `warpx_esirkepov_deposition` (0.61x).
- **New here, absent from LLR-40:**
  - C's complex arithmetic rules cost the C columns 14% on `quatrex_rgf`.
  - The translator's Python-modulo helper costs C and C++ a second division per modulo
    (`spgemm_hash`, 1.44x).
  - Translated C needs a BLAS library the bare-metal machine lacks (`comet_int4_gemm`).
  - A hand-written upstream kernel loses vectorisation to OpenMP outlining (native boris, 0.79x).
