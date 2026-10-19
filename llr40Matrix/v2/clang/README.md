# LLR-40 with clang

The review of 2026-10-19 asked how LLVM compiles the C versions ("it would be interesting to run handwritten C in
clang to see LLVM too"). This experiment builds the translated C and the hand-written C of every LLR-40 kernel with
clang 22.1.0, the LLVM version Numba 0.67 is built on (`llvmlite.binding.llvm_version_info` = 22.1.0), and times
them against the same sources built by GCC 14.2 and against Numba, interleaved on one node.

## Setup

- Benchmark: HPCAgent-Bench `26a4f0cf` (the thesis commit), worktree `HPCAgent-Bench-clang`.
- clang: the official LLVM 22.1.0 release for Linux/AArch64 (`LLVM-22.1.0-Linux-ARM64.tar.xz`, unpacked under
  `/capstor/scratch/cscs/lhulsbergen/llvm/`); `llvmbin/clang` points to it. daint has no system clang.
- A clang cell sets `HPCAGENT_BENCH_BUILD_COMPILER_C=llvm`, the harness's arm pin `build.compiler.c`, so the
  harness builds the C source with its own `clang` toolchain block and flags (`flags.CPU_BASELINE_CLANG`:
  `-O3 -march=native -fopenmp=libomp`, the same floating-point flags as GCC, `-ffp-contract=fast`,
  `-Xarch_host -fveclib=libmvec`). `LD_LIBRARY_PATH` points at LLVM's `libomp`.
- Every row records which compilers left their mark in the timed library's `.comment` section (`lib_comment`).
  A clang cell shows `clang version 22.1.0` next to GCC's (from the crt objects); a GCC cell shows no clang.
- Protocol as in `../followup` and `extractedMatrix/followup`: preset M, `float64`, validated against the NumPy
  oracle before timing, 5 warm-up + 30 timed runs, three rounds in alternating order on one exclusive node,
  one core bound to NUMA node 0, `perf stat -e cycles,instructions,branch-misses,page-faults` over the timed
  runs only.

## Cells (`units.txt`)

| label | what |
|---|---|
| `c` | translated C, GCC 14.2 (the matrix's `c` column) |
| `clang` | the same source, clang 22.1.0 |
| `hand` | the hand-written `<k>_reference.c`, GCC 14.2 (the matrix's `c_reference` column) |
| `handclang` | the same source, clang 22.1.0 |
| `numba` | Numba 0.67 (LLVM 22.1) |
| `cpp`, `cpp14` | `tsvc_2_s316` only: translated C++ with g++ 13.3 and g++ 14.2, for the case study's counters |

Five kernels have no hand-written C (`compact_threshold_pack`, `scan_affine_decay`, `scatter_accum_dup`,
`segment_reduce_ragged`, `versioned_distance_update`).

## Files

| path | what |
|---|---|
| `sweep_clang.py` | `extractedMatrix/followup/sweep_gap.py` with `src=REF`, clang-cell records and `lib_comment` |
| `run_clang.sh`, `clang.sbatch` | units dealt round-robin over the job's nodes, one after the other per node |
| `parts/` | raw rows per unit, every timing (`time_ns_all`) and the perf counters |
| `analyze_clang.py` | `results.csv`, `cells.csv` (median and 95% interval per cell), `ratios.csv`, `summary.md` |
| `asm_so/` | disassembly of every built library's kernel functions (round 1) |
| `emitted_sources/` | the emitted sources the cells started from |

## Reproduce

```bash
S=/capstor/scratch/cscs/lhulsbergen
git -C $S/HPCAgent-Bench-v2 worktree add --detach $S/HPCAgent-Bench-clang 26a4f0cfc1540cead109dfc8d736f1c45263e390
curl -LO https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.0/LLVM-22.1.0-Linux-ARM64.tar.xz
tar xf LLVM-22.1.0-Linux-ARM64.tar.xz -C $S/llvm
sbatch --job-name=clang --partition=normal --nodes=10 --time=01:00:00 --export=ALL,CLANG_STEP_MIN=55 clang.sbatch
python3 analyze_clang.py
```
