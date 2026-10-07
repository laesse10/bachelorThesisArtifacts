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

### 2. WarpX, Fortran: an extra loop counter in every vectorised loop

| kernel | Fortran / C time | gfortran 14 / C | Fortran / C instructions | IPC C, Fortran |
|---|---:|---:|---:|---:|
| `warpx_boris_push` | 0.896 | 0.893 | 1.126 | 2.41, 2.51 |
| `warpx_field_gather` | 0.915 | 0.924 | 1.083 | 2.23, 2.25 |
| `warpx_esirkepov_deposition` | 0.958 | 0.965 | 0.980 | 4.68, 4.47 |

- **Not the compiler version**: gfortran 14.2.0 (`bin14/gfortran -> /usr/bin/gfortran-14`, the same SUSE GCC
  14.2.0 as the C column) is as slow as gfortran 13.3.1 on all three.
- **Boris push and field gather: Fortran executes 13% and 8% more instructions at the same IPC.**
  `loop_shares.py` splits each profiled function at its backward branches. The loops correspond one to one,
  and each vectorised Fortran loop carries an iteration counter beside the address offset (`add x2, x2, #1;
  cmp x2, x11`), where C compares the offset with a precomputed end (`cmp x0, x1`): one instruction more per
  vector iteration, in loops of 7 to 14 instructions. Count of innermost vector loops with such a separate
  counter (from the annotated profiles): C 0 of 26 / 0 of 417 / 0 of 351; Fortran 25 of 26 (99.9% of the
  samples), 387 of 443 (35.5% of the samples) and 216 of 354 (14.7%) for Boris push, field gather, deposition.
- **Deposition: not explained.** The counter is there, but Fortran executes 2% fewer instructions overall
  and loses 4% through a lower IPC. That is within the noise of this 2.8 ms kernel (`../summary.md`).

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

## Files

| path | what |
|---|---|
| `sweep_gap.py` | the sweep (cells, rounds, perf gate, perf record, `src=`/`src32=`/`bin=`) |
| `run_gap.sh`, `gap.sbatch`, `units.txt` | one unit per exclusive node; the units |
| `make_variants.py`, `variants/` | the source variants, generated from `../emitted_sources` with asserted edits |
| `bin14/gfortran` | symlink to `/usr/bin/gfortran-14` (SUSE GCC 14.2.0) |
| `perfgate/sitecustomize.py` | the LLR-40 follow-up's perf gate, byte-identical |
| `analyze_gap.py` -> `summary_table.md` | the table of every cell |
| `loop_shares.py`, `nrt_share.py` | per-loop sample shares; samples in NRT_incref/decref |
| `nrt_probe.py`, `probe.sbatch` | names the JIT addresses of Numba's runtime functions |
| `parts/` | raw rows per unit (all timings, perf counters); `superseded/`: the invalid `tri` run |
| `profiles/` | perf reports and annotations; `nrt_probe/` |
| `opt_reports/` | opt reports + disassembly of the new builds: `gfortran14`, `gfortran13`, `gcc14` (same argv, for comparison), `cshift`, `nb*` (Numba asm), `fp32` (compile only, login node, same CPU) |
| `asm_so/` | the built libraries' kernel functions with load addresses (fp64 symbols only, so not for the integer kernels) |
| `emitted_sources/` | the emitted sources each unit started from |
| `logs/`, `srun_lines.txt` | Slurm and unit logs; every srun line |

Incidents: in job 4998101 the `esirkN2` unit did not start (`units.txt` line format) and was run as job 4998121.
In job 4998121 its address probe failed on relative paths, and in job 4998217 the probe of the original file
failed on Numba's on-disk cache (another module name). It was run again as job 4998228 with a private
`NUMBA_CACHE_DIR` (`probe.sbatch`). The opt-report step for `cshift` failed in job 4998101 (argument parsing
in `run_gap.sh`, fixed afterwards); its report was compiled on the login node with the harness argv.
