# Follow-up: the open gaps of the extracted-kernel matrix and of LLR-40

Four results were left unexplained in the thesis (evaluation chapter, 2026-10-07):

1. translated C is 1.44x slower than Numba on `triangle_count`; the floor-division helper was the suspect;
2. Fortran is 0.90-0.92x as fast as C on the three WarpX kernels;
3. the hand-written Numba `warpx_esirkepov_deposition` is 0.61x as fast as C;
4. Fortran is 4-6% slower than C on four LLR-40 kernels (`scatter_accum_dup`, `tsvc_2_s252`, `tsvc_2_s4112`,
   `tsvc_2_vag`), in v1 and v2, untouched by `-fno-protect-parens` (LLR-40 follow-up 2b).

The experiments here answer all four, and found two problems in the extracted matrix itself (items 5 and 6).

Same benchmark commit (`26a4f0cf`, worktrees `HPCAgent-Bench-ext` and `-ext2`), harness path, flags, preset M,
5 warm-up + 30 timed reps, validation against the NumPy oracle, one core of one exclusive node, NUMA node 0
(`sweep_gap.py`, derived from `../sweep_extracted.py`). Every cell is validated (all `ok`). The cells of a kernel
run three times in alternating order on one node (rounds 1-3: as listed, reversed, as listed), with
`perf stat` (cycles, instructions, branch misses) gated to the timed reps. A fourth series per cell
(`round=record`) runs under `perf record -e cycles:u -c 100000`, gated the same way: `profiles/<k>/<cell>.report.txt`
(by DSO and symbol) and `.annotate.txt` (hottest kernel symbols). These are profiles, not timings.

Jobs (debug partition): 4998018 (9 units), 4998101 (`tri2`), 4998121 (`esirkN2`), 4998217 and 4998228
(`probe.sbatch`), 4998244 (`cometN`, with OpenBLAS from the uenv as in `../DEVIATIONS.md` item 9).
`summary_table.md` (from `analyze_gap.py`) has every cell: min of k over all rounds, the min of each round
with its position, and the perf counters per timed rep.

## Results

### 1. Triangle counting: the floor-division helper is the cause

| cell (unit `tri2`) | min of k | over C | instructions / rep | IPC |
|---|---:|---:|---:|---:|
| `c` (as emitted) | 46.76 ms | 1.00 | 4.20e8 | 2.75 |
| `cshift`: every `int_floor(x, 2)` / `int_floor(x, 32)` written `x >> 1` / `x >> 5` | **30.21 ms** | **1.55** | 2.55e8 | 2.57 |
| `numba` | 32.37 ms | 1.44 | 2.92e8 | 2.76 |

The only change in `cshift` (`variants/triangle_count_cshift*.c`, `make_variants.py`) is the division by the
powers of two. GCC's `>>` on a signed int64 is an arithmetic shift, which rounds towards minus infinity like
Python's `//`, so the result is the same (validated). C then needs 39% fewer instructions and is 1.55x
faster, a little faster than Numba. Branch misses are the same in all three (2.5-2.7e6 per rep), so the
gap is not branch prediction. Disassembly of the timed `cshift` function: `opt_reports/cshift/`.

**The first run of this (unit `tri`, job 4998018) was invalid** and is kept in `parts/superseded/`: the variant
replaced only `triangle_count_fp64.c`, but the harness runs `triangle_count_fp32` (item 5), so `cshift` timed
the unchanged code and tied with `c`. `tri2` replaces both files (`src32=`); its profile shows the shifts in
the function that ran (`profiles/triangle_count/cshift.annotate.txt`).

### 2. WarpX, Fortran: fresh memory for the temporaries (Boris push, field gather) and parentheses (deposition)

**Superseded.** The first version of this section (and of the thesis paragraph, commit `acf7653`) blamed an
extra loop counter. Round 2 (below) tested that and found it does not cost time. What round 1 measured:

| kernel | Fortran / C time | gfortran 14 / C | Fortran / C instructions | IPC C, Fortran |
|---|---:|---:|---:|---:|
| `warpx_boris_push` | 0.896 | 0.893 | 1.126 | 2.41, 2.51 |
| `warpx_field_gather` | 0.915 | 0.924 | 1.083 | 2.23, 2.25 |
| `warpx_esirkepov_deposition` | 0.958 | 0.965 | 0.980 | 4.68, 4.47 |

- Not the compiler version: gfortran 14.2.0 (`bin14/gfortran`) is as slow as gfortran 13.3.1 on all three.
- gfortran keeps an iteration counter beside the address offset in its vectorised loops (`add x2, x2, #1;
  cmp x2, x11`; C: `cmp x0, x1` against a precomputed end). C gets the same counters when its arrays are
  reached through pointers to variable-length arrays, the way gfortran represents an array argument (checked
  in a test file: the counter appears with `double (*a)[n]` and in every Fortran spelling, including
  `do while` and `do concurrent`, and no GCC flag removes it). The `cvla` variants (round 2) execute up to
  10% more instructions than C and are not slower. **The counter is not the cause.**
- The causes (round 2, units `borisM`, `fgathM`, `esirkV`): see the table there.

### 3. Hand-written Numba deposition: reference counting

| cell (units `esirkN`, `esirkN2`) | min of k | over C (same node) | samples in NRT_incref/decref | IPC |
|---|---:|---:|---:|---:|
| `numba` (the hand-written file) | 4.32 ms | 0.65 | **33.0%** | 4.45 |
| `nbnort`: the per-particle functions `@njit(_nrt=False)`, nothing else | **2.91 ms** | **0.96** | 0.1% | 6.09 |
| `nbserial`: no private grids, no `parallel=True` | 5.09 ms | 0.53 | 52.2% | 3.56 |
| `nbnoviews`: `nbserial`, and seven 1-D buffers instead of seven row views | 7.27 ms | 0.37 | 63.4% | 2.73 |
| `nbfastmath`: `nbnoviews` with `fastmath={'nsz','contract'}` | 7.04 ms | 0.38 | 65.2% | 2.57 |

A third of the hand-written version's cycles go to Numba's atomic reference counting of the arrays and array
views that every particle's call passes around (`NRT_incref` / `NRT_decref`). Compiled without it, the same
code runs at 0.96x of C, with 8% fewer instructions but 33% fewer cycles: the atomic updates cost time, not
instructions. The three variants that moved the arrays around (`nbserial`, `nbnoviews`, `nbfastmath`) made
the reference counting worse, not better, which is why they are slower.

perf cannot name JIT code. `nrt_probe.py` (`probe.sbatch`) runs the file under `perf record` and prints the
addresses of Numba's runtime functions in the same process: `nrt_atomic_add` +0x000, `nrt_atomic_sub` +0x010,
`NRT_incref` +0x050, `NRT_decref` +0x060 from a 64 KiB-aligned base (`profiles/nrt_probe/*.addresses.txt`).
The hottest samples of every profile here are at +0x05c, +0x068, +0x070 and +0x078 of such a base, inside
`NRT_incref` and `NRT_decref`; `nrt_share.py` sums them. Static counts in Numba's assembly agree: 62 calls to
`NRT_incref` and 202 to `NRT_decref` in the hand-written file, 18 and 52 in `nbnort` (`opt_reports/nbnort/`).

### 4. LLR-40, the four small Fortran losses: the harness's index rebase (3) and noise (1)

| kernel | Fortran / C | instructions / rep, C and Fortran | samples in NumPy `INT_add` + `INT_subtract` | Fortran x (1 - that share) / C |
|---|---:|---:|---:|---:|
| `tsvc_2_s4112` | 1.051 | 2.11e8, 3.67e8 | 4.2% | 1.008 |
| `tsvc_2_vag` | 1.050 | 3.15e8, 5.23e8 | 4.1% | 1.008 |
| `scatter_accum_dup` | 1.058 | 4.70e8, 6.26e8 | 5.3% | 1.002 |
| `tsvc_2_s252` | 1.062 (min over rounds; gfortran 14: 1.003) | 9.007e8, 9.006e8 | 0 | -- |

- **`s4112`, `vag`, `scatter_accum_dup`: the harness, not the language.** Fortran is 1-based, so the harness
  shifts every index array before the Fortran call and back afterwards
  (`hpcagent_bench/benchmarks/cpp_runtime.py`, `wrap_kernel.call`: `arg += delta`, the call, `arg -= delta`).
  Both passes run inside the timed bracket, and only for Fortran. They are the NumPy `INT_add` /
  `INT_subtract` samples in the Fortran profiles (none in C), and account for the whole gap: without them,
  Fortran is within 0.8% of C. The kernels' loops are identical (`s4112`) or differ by one instruction
  (`vag`: the 1-based offset) or in addressing mode (`scatter_accum_dup`). The rebase applies to five of the
  48 kernels (`index_rebase`): these three, and `argmax_with_index` and `ext_break_capture`, whose rebased
  argument is a one-element output (negligible).
- **`s252`: no difference in the code.** C and Fortran execute the same instructions (identical sequence,
  `../../llr40Matrix/v2/opt_reports/tsvc_2_s252/`; 9.006e8 per rep in both). Across the rounds, C took
  81.0-85.8 ms, Fortran 86.0-86.9 ms and the gfortran 14 build of the same code 81.3-86.0 ms. The times
  overlap, so the gap is the run-to-run variation of this memory-bound kernel, not the language. The loop
  sits at a different address in the two libraries (`asm_so/tsvc_2_s252/`), but the gfortran 14 build, at
  the same address as gfortran 13's, reached C's time.

### 5. Integer-only kernels run the fp32 symbol (affects the extracted matrix's descriptions)

The harness's native wrapper picks `<k>_fp64` only if some argument is a float64 or complex128 array
(`cpp_runtime.wrap_kernel.call`: `fptype = "fp64" if is_double else "fp32"`), whatever `--precision` says.
`comet_int4_gemm`, `spgemm_hash`, `triangle_count` and `nfa_frontier` have integer arguments only, so the
matrix timed `<k>_fp32` in the c, cpp and fortran columns, while `../opt_reports/` and `../summary.md`
analysed the fp64 functions. Diff of the emitted fp32 against fp64 sources (names normalised):

- `triangle_count`, `nfa_frontier`: identical. Nothing changes.
- `spgemm_hash`: `_select_bin` takes a `float` instead of a `double`. The fp32 builds have the same nine
  integer divisions as the fp64 ones (`opt_reports/fp32/spgemm_hash/`, compile only), so the division
  analysis stands.
- `comet_int4_gemm`: float32 buffers and **`cblas_sgemm`**, not `cblas_dgemm` (still exact: the counts are
  far below 2^24). The Fortran fp32 loops vectorise the same way and still build each vector from single-lane
  loads (`ld1 {v.s}[n]`), now with a stride of 32 float32 values (`opt_reports/fp32/comet_int4_gemm/`).

No LLR-40 kernel is affected: every one has a float64 argument.

### 6. The hand-written CoMet cell timed the translated code

A consequence of item 5. The `native` adapter (`../adapters/comet_int4_gemm_native.cpp`) is placed over
`comet_int4_gemm_fp64.cpp` and exports `comet_int4_gemm_fp64`, but the harness calls `comet_int4_gemm_fp32`,
which in that library is still the translator's fp32 C++ code. The recorded `native` time (0.068 ms, "ties
with C") is therefore the translated code calling OpenBLAS. Unit `cometN` times the adapter properly: `native32`
places the same adapter over the fp64 file and a thin fp32 twin (`variants/comet_int4_gemm_native_fp32.cpp`,
same argument mapping) over the fp32 file. Unit `cometN` (one node, OpenBLAS from the uenv):

| cell | min of k | over C | instructions / rep | function that ran (profile) |
|---|---:|---:|---:|---|
| `c` | 0.085 ms | 1.00 | 8.48e5 | `comet_int4_gemm_fp32` + OpenBLAS `sgemm_small_kernel_b0_nn_NEOVERSEV1` |
| `cpp` | 0.086 ms | 0.98 | 8.59e5 | same, C++ build |
| `native` (as in the matrix: adapter over fp64 only) | 0.084 ms | 1.00 | 8.59e5 | `comet_int4_gemm_fp32` + `sgemm`: the translated code |
| `native32` (adapter over both) | **0.149 ms** | **0.565** | 1.80e6 | `tc_int4_gemm_impl` (the hand-written code) |

The hand-written code is 0.57x of C and executes 2.1x the instructions. C takes 0.085 ms here against 0.068 ms
in the matrix (another node, perf counters on); the ratio is taken on one node. (`warpx_boris_push`'s `native` cell is not affected: its arguments are
float64.)

## Round 2 (2026-10-07 to 2026-10-09): every explanation in the evaluation chapter, tested

Goal: every speed difference that the evaluation chapter explains, or that the extracted-kernel table shows
beyond noise, is backed by an experiment that changes only the suspected cause. Jobs (debug partition):
4998574 (batch A), 4998606 (B1), 5009929 (`cometL`, uenv OpenBLAS), 5009930 (B2a), 5009993 (B2b), 5010073 (C).
Same protocol as round 1; `perf stat` also counts page faults from round B2a on. Every cell validated (`ok`).

How a cell changes one thing (`sweep_gap.py` cell syntax, `units.txt`):
- `src=` / `src32=`: a source variant in `variants/` (each file's header states its one edit; diff it
  against `emitted_sources/<k>/`, or `../emitted_sources/<k>/` for the extracted kernels);
- `bin=`: a compiler wrapper directory first on PATH: `bin14` (gfortran 14.2.0), `bin_gxx14` (g++ 14.2.0),
  `bin_cx` (gcc 14 / g++ 13 with `-fcx-fortran-rules`), `bin_noparens` (gfortran 13 with `-fno-protect-parens`);
- `env=`: an environment variable for the harness run (`NUMBA_LOOP_VECTORIZE=0`, `NUMBA_SLP_VECTORIZE=0`;
  glibc's `MALLOC_MMAP_THRESHOLD_=33554432` and `MALLOC_TRIM_THRESHOLD_=68719476736`).

| unit | kernel | cell: min of k over all rounds (ms), speedup over the unit's `c` |
|---|---|---|
| `s316c` | `tsvc_2_s316` | `c` 274 (1.00); `cpp` 79.1 (3.46); `cpp14` 274 (1.00); `fortran` 79.2 (3.46); `fortran14` 274 (1.00); `numba` 122 (2.24) |
| `qtx` | `quatrex_rgf` | `c` 7.16 (1.00); `ccx` 6.06 (1.18); `cpp` 7.18 (1.00); `cppcx` 6 (1.19); `fortran` 6.28 (1.14) |
| `borisnb` | `warpx_boris_push` | `c` 3.03 (1.00); `numba` 2.08 (1.45); `nbnovec` 2.59 (1.17) |
| `tri3` | `triangle_count` | `c` 46.8 (1.00); `cpp` 48.3 (0.97); `cpp14` 46.6 (1.00) |
| `nfa` | `nfa_frontier` | `c` 9.74e+03 (1.00); `cpp` 9.37e+03 (1.04); `cpp14` 9.76e+03 (1.00); `numba` 1.04e+04 (0.94) |
| `s318r` | `tsvc_2_s318` | `c` 79 (1.00); `csel` 284 (0.28); `numba` 284 (0.28) |
| `s3110r` | `tsvc_2_s3110` | `c` 118 (1.00); `csel` 417 (0.28); `numba` 314 (0.38) |
| `argmr` | `argmax_with_index` | `c` 71.6 (1.00); `csel` 248 (0.29); `numba` 180 (0.40) |
| `s3111r` | `tsvc_2_s3111` | `c` 123 (1.00); `novec` 712 (0.17); `novecsel` 462 (0.27); `numba` 308 (0.40) |
| `argh` | `argmax_with_index` | `c` 71.4 (1.00); `csel` 250 (0.29); `cselhalf` 206 (0.35); `numba` 181 (0.40) |
| `s3110h` | `tsvc_2_s3110` | `c` 119 (1.00); `csel` 417 (0.29); `cselhalf` 302 (0.40); `numba` 314 (0.38) |
| `borisV` | `warpx_boris_push` | `c` 2.99 (1.00); `cvla` 3 (0.99); `fortran` 3.33 (0.90); `numba` 2.06 (1.45); `nbnofuse` 3.53 (0.84); `nbnofusenovec` 3.62 (0.82) |
| `spg` | `spgemm_hash` | `c` 256 (1.00); `cmod1` 186 (1.38); `cmask` 130 (1.97); `fortran` 178 (1.44); `numba` 151 (1.70) |
| `cometL` | `comet_int4_gemm` | `c` 0.0833 (1.00); `cloops` 0.593 (0.14); `cloopsT` 0.548 (0.15); `fortran` 0.378 (0.22); `numba` 0.167 (0.50); `native32` 0.151 (0.55); `native32noomp` 0.129 (0.65) |
| `borisH` | `warpx_boris_push` | `c` 3.03 (1.00); `fortran` 3.38 (0.90); `native` 3.84 (0.79); `nativenoomp` 3.19 (0.95); `fortnp` 3.39 (0.90) |
| `qtxN` | `quatrex_rgf` | `c` 7.26 (1.00); `numba` 4.17 (1.74); `nbnoblas` 7.43 (0.98) |
| `agscat` | `scatter_accum_dup` | `c` 343 (1.00); `agent` 931 (0.37); `agent1t` 924 (0.37); `agent1tplain` 471 (0.73); `catomic` 517 (0.66); `catomicomp` 506 (0.68) |
| `agscan` | `scan_affine_decay` | `c` 117 (1.00); `agent` 232 (0.50); `agent3p` 347 (0.34) |
| `agscat2` | `scatter_accum_dup` | `c` 333 (1.00); `agent1tplain` 454 (0.73); `agent1tnostage` 333 (1.00) |
| `esirkV` | `warpx_esirkepov_deposition` | `c` 2.69 (1.00); `cvla` 2.66 (1.01); `fortran` 2.96 (0.91); `fortnp` 2.85 (0.95) |
| `fgathV` | `warpx_field_gather` | `c` 11.5 (1.00); `cvla` 11.6 (1.00); `fortran` 12.7 (0.91); `fortnp` 12.8 (0.90) |
| `borisM` | `warpx_boris_push` | `c` 3.04 (1.00); `fortran` 3.41 (0.89); `cmal` 2.9 (1.05); `fmal` 2.9 (1.05) |
| `fgathM` | `warpx_field_gather` | `c` 11.4 (1.00); `fortran` 12.1 (0.95); `cmal` 7.61 (1.50); `fmal` 7.83 (1.46) |
| `fuseD` | `fuse_diamond` | `c` 168 (1.00); `cstatic` 145 (1.16); `cfused` 29.3 (5.74); `hand` 29.4 (5.73); `agent` 29.4 (5.71) |
| `fuseS` | `fuse_stencil_through_transient` | `c` 112 (1.00); `cstatic` 96.3 (1.16); `cfused` 46.9 (2.38); `hand` 47 (2.37); `agent` 47 (2.37) |
| `i231` | `tsvc_2_s231` | `c` 656 (1.00); `cinter` 68.2 (9.61); `agent` 68.3 (9.61) |
| `i2233` | `tsvc_2_s2233` | `c` 421 (1.00); `cinter` 66 (6.38) |
| `i235` | `tsvc_2_s235` | `c` 535 (1.00); `cinter` 68.2 (7.84); `agent` 56.8 (9.42) |
| `i2275` | `tsvc_2_s2275` | `c` 684 (1.00); `cinter` 73.2 (9.35); `agent` 73.8 (9.27) |
| `i1232` | `tsvc_2_s1232` | `c` 38.5 (1.00); `cinter` 11.9 (3.24); `agent` 11.9 (3.23) |
| `nfaU` | `nfa_frontier` | `c` 9.69e+03 (1.00); `numba` 1.03e+04 (0.94); `nbuidx` 8e+03 (1.21) |

What each unit shows (the thesis states these, evaluation chapter):

| difference | experiment | outcome |
|---|---|---|
| s316: C 3.4x slower than C++/Fortran | C++ and Fortran built by GCC 14 (`s316c`) | as slow as C: the compiler version |
| QuaTrEx: C 14% slower than Fortran | `-fcx-fortran-rules` (`qtx`) | C 1.18x, C++ 1.19x: C's complex rules |
| QuaTrEx: hand Numba 1.73x | 37 products and 2 inverses as loops (`qtxN`) | 0.98x of C: BLAS/LAPACK |
| triangle / NFA: C++ 0.97x / 1.04x | C++ built by g++ 14 (`tri3`, `nfa`) | 1.00x both: the compiler version |
| SpGEMM: Fortran 1.44x, Numba 1.70x | one division per remainder; bit masks (`spg`) | C 1.38x; C 1.97x: the divisions |
| CoMet: Fortran 0.22x, Numba 0.50x, hand 0.55x | C with the loop nest instead of BLAS (`cometL`) | C 0.14x (unvectorised); contiguous operand only 0.15x: the library call, not the stride |
| CoMet hand-written | no OpenMP pragma | 0.55x -> 0.65x: the region costs 15%, the rest is OpenBLAS |
| s318 Numba 0.28x | C with selects (`s318r`) | 284 ms = Numba 284 ms |
| argmax / s3110 Numba 0.40x / 0.38x | C with a select on every second element (`argh`, `s3110h`) | 0.35x / 0.40x (all selects: 0.29x): Numba selects on half the elements |
| s3111 Numba 0.40x | C unvectorised (`s3111r`) | 0.17x (branch mispredicted half the time): C's lead is SVE's ordered vector reduction |
| Boris push Numba 1.45x | Numba without vectorisation; without parfor fusion (`borisnb`, `borisV`) | 1.17x; 0.84x: fusion, then vectorisation |
| NFA Numba 0.94x | all indices `np.uint64` (`nfaU`, no wraparound guards) | 1.21x: the guards |
| Boris push / field gather Fortran 0.89x / 0.91x | glibc keeps freed memory (`borisM`, `fgathM`) | no page faults; Fortran = C (2.90 ms) / within 3%; C itself 1.05x / 1.50x faster |
| same | C with gfortran's counters (`cvla`); `-fno-protect-parens` | not slower; no change: neither is the cause |
| deposition Fortran 0.91x | `-fno-protect-parens` (`esirkV`) | 2.85 ms vs 2.97; C 2.69-2.84 ms: the parentheses |
| hand Boris push 0.79x | no OpenMP pragma (`borisH`) | 0.95x (tie), 43% fewer instructions |
| agent scan 0.50x | pass 1 twice (`agscan`) | 0.34x: each pass costs one C run |
| agent scatter 0.37x | 1 thread; plain add; no staging copies (`agscat`, `agscat2`) | 0.37x; 0.73x; 1.00x: atomics and copies, not the thread override |
| DaCe scatter 0.68x | C with `#pragma omp atomic` per element (`agscat`) | 0.66x (0.68x inside an OpenMP loop) |
| agents 3-10x (interchange kernels) | translated C interchanged/distributed only (`i231`..`i1232`) | equal to the agent on s231, s2275, s1232; s2233 6.4x; s235 agent 1.2x faster still |
| hand C fuse 5.6x / 2.3x | translated C fused; temporaries kept instead (`fuseD`, `fuseS`) | equal to hand and agent; 1.16x only |

Not explained: DaCe's `compact_threshold_pack` (0.44x) and `s323` (0.50x), which the thesis only names.

Incidents and notes (round 2):
- `nfa` (job 4998574): both timed rounds complete; the profile pass hit the step's time limit after `c` and `cpp`.
  `nfaU` re-measured C and Numba (2 rounds of 10 reps for this 10 s kernel, as `nfa`).
- `s3111_novecsel`: GCC turns every C spelling of the select back into a branch, so the select is one line of
  inline assembly (`fcmp` + `fcsel`), the instruction pattern of Numba's loop.
- A Numba-side test of the select (LLVM `-two-entry-phi-node-folding-threshold=0`,
  `-aarch64-enable-early-ifcvt=false`) was tried on the login node: the select is already in Numba's IR, and
  neither option removes it. So the select was tested from the C side.
- `units.txt` held `borisH` twice for a while (merged before B2a started); `borisH` gained `fortran` and
  `fortnp` cells before the job started.
- `opt_reports/fp32/`, the `cvla` codegen checks and the Fortran loop-form tests were compiled on the login node
  (same Neoverse V2 CPU, harness argv), not timed.

## Files

| path | what |
|---|---|
| `sweep_gap.py` | the sweep (cells, rounds, perf gate, perf record, `src=`/`src32=`/`bin=`) |
| `run_gap.sh`, `gap.sbatch`, `units.txt` | one unit per exclusive node; the units |
| `make_variants.py`, `variants/` | the source variants, generated from `../emitted_sources` with asserted edits |
| `bin14/`, `bin_gxx14/`, `bin_cx/`, `bin_noparens/` | compiler wrappers (round 2) |
| `perfgate/sitecustomize.py` | the LLR-40 follow-up's perf gate, byte-identical |
| `analyze_gap.py` -> `summary_table.md` | the table of every cell |
| `loop_shares.py`, `nrt_share.py` | per-loop sample shares; samples in NRT_incref/decref |
| `nrt_probe.py`, `probe.sbatch` | names the JIT addresses of Numba's runtime functions |
| `parts/` | raw rows per unit (all timings, perf counters); `superseded/`: the invalid `tri` run |
| `profiles/` | perf reports and annotations, one per (kernel, cell label): a later unit with the same label overwrites the earlier one (same build, newer run); `nrt_probe/` |
| `opt_reports/` | opt reports + disassembly of the new builds: `gfortran14`, `gfortran13`, `gcc14` (same argv, for comparison), `cshift`, `nb*` (Numba asm), `fp32` (compile only, login node, same CPU) |
| `asm_so/` | the built libraries' kernel functions with load addresses (fp64 symbols only, so not for the integer kernels) |
| `emitted_sources/` | the emitted sources each unit started from |
| `scratch_src/` | sources the round-2 variants were made from: LLR-40 emitted C (same as `emitted_sources/`), the fp32 files of CoMet and SpGEMM, the hand-written originals of the Boris push and CoMet |
| `logs/`, `srun_lines.txt` | Slurm and unit logs; every srun line |

Incidents: in job 4998101 the `esirkN2` unit did not start (`units.txt` line format) and was run as job 4998121.
In job 4998121 its address probe failed on relative paths, and in job 4998217 the probe of the original file
failed on Numba's on-disk cache (another module name). It was run again as job 4998228 with a private
`NUMBA_CACHE_DIR` (`probe.sbatch`). The opt-report step for `cshift` failed in job 4998101 (argument parsing
in `run_gap.sh`, fixed afterwards); its report was compiled on the login node with the harness argv.
