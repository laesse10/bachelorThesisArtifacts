# LLR-40 with GCC 14 for every language

The benchmark's default build on daint compiles C with GCC 14.2 (the C block requests `-std=c23`, which
GCC 13 rejects) but C++ and Fortran with the system GCC 13.3, so the matrix's C-vs-C++ and C-vs-Fortran
comparisons also compare compiler versions. The review of 2026-10-19 asked why not GCC 14.2 for everything.
This experiment times C++ and Fortran built by GCC 14.2 next to the default build, for all 40 kernels.

## Setup

Same protocol and machinery as `../clang/` (`sweep_gcc14.py` is an unchanged copy of `sweep_clang.py`):
benchmark `26a4f0cf`, preset M, `float64`, validated against the NumPy oracle, 5 warm-up + 30 timed runs,
three rounds in alternating order on one exclusive node per kernel, one core on NUMA node 0, `perf stat`
over the timed runs. GCC 14.2 is reached the way the harness resolves its compilers, first on `PATH`:
`bin_gxx14/g++ -> /usr/bin/g++-14`, `bin14/gfortran -> /usr/bin/gfortran-14`. Every row records the
compilers in the timed library's `.comment` section (`lib_comment`); `analyze_gcc14.py` checks that each cell
carries the version it should.

| label | what |
|---|---|
| `c` | translated C, GCC 14.2 (as in the matrix) |
| `cpp`, `fortran` | translated C++ and Fortran, GCC 13.3 (as in the matrix) |
| `cpp14`, `fortran14` | the same sources, GCC 14.2 |

## Files

`units.txt`, `run_gcc14.sh`, `gcc14.sbatch` (job 5098546), `parts/` (raw rows, every timing),
`analyze_gcc14.py` -> `results.csv`, `cells.csv`, `ratios.csv`, `summary.md`; `asm_so/`, `emitted_sources/`.

```bash
sbatch --job-name=gcc14 --partition=normal --nodes=10 --time=01:00:00 --export=ALL,GCC14_STEP_MIN=55 gcc14.sbatch
python3 analyze_gcc14.py
```
