# LLR-40 v2 follow-up experiments

Five small experiments, each testing one hypothesis that came from reading GCC's optimisation
reports for the v2 matrix. They use the same benchmark commit (`26a4f0cf`), harness, compile
lines, environment, preset M, `float64` and step geometry as v2. Every variant is validated
against the NumPy oracle before it is timed. Variant rows never enter the matrix or its
aggregates, and they carry a `variant` column.

| # | hypothesis | write-up | table |
|---|---|---|---|
| 1 | conditional min/max reductions need `-ffinite-math-only` | [minmax_finite_math.md](minmax_finite_math.md) | `minmax_finite_math.csv` |
| 2a | hoisting `x[0]` lets GCC vectorise `s2710` | [s2710_hoist.md](s2710_hoist.md) | `s2710_hoist.csv` |
| 2b | gfortran's protected parentheses explain the Fortran-vs-C gaps | [fortran_parens.md](fortran_parens.md) | `fortran_parens.csv`, `fortran_parens_outputs.csv` |
| 3 | the five bandwidth-unstable kernels' GCC columns differ by noise, not code | [unstable_interleaved.md](unstable_interleaved.md) | `unstable_interleaved.csv` |
| 4 | register vs memory is about 6 cycles of store forwarding | [register_vs_memory.md](register_vs_memory.md) | `register_vs_memory.csv` |

Deviations and incidents: [DEVIATIONS.md](DEVIATIONS.md). Every srun line: `srun_lines.txt`.

## Layout

| path | what |
|---|---|
| `sweep_followup.py` | v2's `sweep_variant.py` plus variant/override/round/perf options (header lists them) |
| `perfgate/sitecustomize.py` | perf counters enabled over the timed reps only (inert unless `--perf`) |
| `outdump/sitecustomize.py` | saves the arrays the harness validates, oracle and build (inert unless `--dump-outputs`) |
| `fortran_parens_outputs.py`, `oracle_njit_check.py` | 2b bit-for-bit comparison; check that the njit oracle equals interpreted NumPy |
| `opt_reports_followup.py` | v2's opt-report + disassembly builder, for follow-up cells |
| `run_unit.sh`, `followup.sbatch` | one unit per kernel per exclusive node; how each experiment was run |
| `make_variants.py`, `variants/` | the flag patch, the source variants (`variants/src/`) and their patches |
| `analyze_followup.py` | builds the tables of all five experiments; reads v2 at `632952d` with `git show` |
| `parts/` | raw rows per unit: every timing (`time_ns_all`), perf, vmstat, opt-report index |
| `opt_reports/` | `-fopt-info` reports (line 1 = exact argv) and `objdump -d` of the kernel |
| `emitted_sources/` | the emitted lowerings each unit timed (byte-identical to v2's) |
| `logs/` | Slurm and per-unit logs; `discarded/`: runs not used, with the reason |
| `v2_agent_cells.csv` | v2's chosen agent candidate per kernel (sha256), from `632952d` |
| `fparens_roster.txt` | 2b's 40 kernels, the eight analysed ones first (kernel i runs on node i mod N) |

The 2b output arrays (24 GB) are not in git: `/capstor/store/cscs/userlab/g34/lhulsbergen/llr40_followup/fparens_outputs/`,
with every array's sha256 in `fortran_parens_outputs.csv`.

## Reproduce

```bash
BENCH=/path/to/HPCAgent-Bench            # spcl/HPCAgent-Bench
git -C $BENCH worktree add --detach $BENCH-v2-fu        26a4f0cfc1540cead109dfc8d736f1c45263e390
git -C $BENCH worktree add --detach $BENCH-v2-fu-finite 26a4f0cfc1540cead109dfc8d736f1c45263e390
git -C $BENCH-v2-fu-finite apply variants/finite_math_only.patch
git -C $BENCH worktree add --detach $BENCH-v2-fu-noparens 26a4f0cfc1540cead109dfc8d736f1c45263e390
git -C $BENCH-v2-fu-noparens apply variants/fno_protect_parens.patch
python3 make_variants.py                 # needs v2's emitted sources (git show 632952d:llr40Matrix/v2/emitted_sources/...)
# paths in run_unit.sh / followup.sbatch default to the CSCS layout
sbatch --job-name=fu-minmax   --nodes=4 --time=01:00:00 --export=ALL,FU_EXP=MINMAX,FU_KERNELS=tsvc_2_s316:tsvc_2_s318:tsvc_2_s3110:argmax_with_index,FU_STEP_MIN=50 followup.sbatch
sbatch --job-name=fu-s2710    --nodes=1 --time=00:45:00 --export=ALL,FU_EXP=S2710,FU_KERNELS=tsvc_2_s2710,FU_STEP_MIN=40 followup.sbatch
sbatch --job-name=fu-unstable --nodes=5 --time=03:00:00 --export=ALL,FU_EXP=UNSTABLE,FU_KERNELS=tsvc_2_s1232:tsvc_2_s231:tsvc_2_s235:tsvc_2_s2275:tsvc_2_s275,FU_STEP_MIN=40 followup.sbatch
sbatch --job-name=fu-regmem   --nodes=2 --time=01:30:00 --export=ALL,FU_EXP=REGMEM,FU_KERNELS=versioned_distance_update:wf_triangular,FU_STEP_MIN=40 followup.sbatch
sbatch --job-name=fu-fparens  --nodes=10 --time=02:00:00 --export=ALL,FU_EXP=FPARENS,FU_KERNELS=$(paste -sd: fparens_roster.txt),FU_STEP_MIN=40 followup.sbatch
python3 analyze_followup.py minmax       # and s2710, unstable, unstable_md, regmem, fparens
```
