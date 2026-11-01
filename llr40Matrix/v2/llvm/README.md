# LLR-40 with LLVM for C, C++ and Fortran

The language comparison of the thesis builds C, C++ and Fortran with GCC; under LLVM, only C (clang, `../clang/`)
and Numba were measured. A review of 2026-11-01 asked whether the parity of the three languages holds with LLVM's
front ends. This experiment builds the translated C, C++ and Fortran of every LLR-40 kernel with clang, clang++ and
flang 22.1.0 (the LLVM version of Numba 0.67), next to GCC 14.2's C as an anchor, and times them interleaved.

## Setup

Same protocol and machinery as `../clang/` (`sweep_llvm.py` is `sweep_clang.py` with the compiler pin extended to
C++ and Fortran): benchmark `26a4f0cf` (`HPCAgent-Bench-clang`, unpatched), preset M, `float64`, validated against
the NumPy oracle, 5 warm-up + 30 timed runs, three rounds in alternating order on one exclusive node per kernel,
one core on NUMA node 0, `perf stat` over the timed runs.

| label | what |
|---|---|
| `c` | translated C, GCC 14.2 (as in the matrix) |
| `clang` | the same source, clang 22.1.0 (`HPCAGENT_BENCH_BUILD_COMPILER_C=llvm`) |
| `clangxx` | translated C++, clang++ 22.1.0: the benchmark's `llvm` column. Its `cpp` column always builds with g++ (`cpp_runtime.FRAMEWORK_COMPILER`), whatever the pin says; a first test with the pin built g++ code. |
| `flang` | translated Fortran, flang 22.1.0 (`HPCAGENT_BENCH_BUILD_COMPILER_FORTRAN=llvm`) |

The LLVM 22.1.0 release for Linux/AArch64 is the one `../clang/` used (`llvmbin/` links clang, clang++ and flang).
Each row records the compilers in the timed library's `.comment` section (`lib_comment`); `analyze_llvm.py` checks
that every LLVM cell carries its compiler's mark. The flags are the benchmark's own for each block (recorded per
row); flang's baseline has no `-fno-signed-zeros`, `-fno-trapping-math` or `-fno-math-errno`.

## Files

`units.txt`, `run_llvm.sh`, `llvm.sbatch` (job 5229693), `sweep_llvm.py`, `parts/` (raw rows), `logs/`,
`analyze_llvm.py` -> `results.csv`, `cells.csv`, `ratios.csv`, `summary.md`.

```bash
sbatch --job-name=llvm --partition=normal --nodes=10 --time=01:30:00 --export=ALL,LLVM_STEP_MIN=55 llvm.sbatch
python3 analyze_llvm.py
```

## Results

`summary.md`. All 40 kernels, 3 rounds of every cell; `s115` is `incorrect` in every cell (its overflow, as with
GCC), everything else `ok`, and every LLVM library carries its compiler's mark. Ratios of medians, > 1: the
second-named faster:

| comparison | n | geomean | 95% CI | faster / within 3% / slower |
|---|---:|---:|---|---|
| clang++ over clang | 39 | 0.996 | 0.986-1.006 | 1 / 36 / 2 |
| flang over clang | 39 | 1.001 | 0.982-1.021 | 4 / 29 / 6 |
| clang over GCC (C) | 39 | 0.887 | 0.788-0.998 | 7 / 22 / 10 |

Built by LLVM, the three languages run at the same speed, as with GCC. C++ differs from C by more than 3% only on
the bandwidth-sensitive `s2275` and `s235` (slower) and on `fuse_stencil_through_transient` (3.3% faster). On `s3111`, flang's Fortran is as fast as clang's C: clang does not
vectorize the conditional sum either (0.40x of GCC, as in `../clang/`). Thesis: Section 6.2.2.
