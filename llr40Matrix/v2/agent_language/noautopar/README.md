# The agents' C and Fortran at 72 threads, without automatic parallelization

The default/T72 runs of `../sweep_agents.py` let the harness size gfortran's `-ftree-parallelize-loops` from the
affinity, so 86 of the 196 valid Fortran libraries were built with automatic parallelization at 72 threads,
while the C build has no such step (`../DEVIATIONS.md` 4). A review of 2026-11-01 asked for the comparison
without it. This sweep re-times every valid submission of both languages at T72 with
`-ftree-parallelize-loops=1`, as in the T1 build, and everything else as in the original T72 runs.

## Setup

- Harness: `/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-agentlang-noautopar`, a copy of the original
  checkout at `26a4f0cf` whose only change is `hpcagent_bench/languages.py` line 946,
  `format(n=grading_ncores())` -> `format(n=1)`. `run_noautopar.sh` checks, in the T72 binding, that the harness
  resolves `-ftree-parallelize-loops=1` and sees 72 cores before it starts a kernel; `sweep_noautopar.py` checks
  the Fortran flag string again and records it in every row.
- Binding and environment: as the original T72 (`../sweep_agents.wrap`): `numactl --cpunodebind=0 --membind=0`,
  `--mode multi_core`, `OMP_NUM_THREADS=72`, `OMP_PLACES=cores`, `OMP_PROC_BIND=close`; 5 warm-up and 30 timed
  runs, validated first.
- Candidates: every submission whose default/T1 row in `../parts/` is `ok`, in the original order, and the
  translated C of each kernel. Rows have the original fields, with `build` = `noautopar`.
- Python: `venv_llr40v2`, rebuilt on 2026-11-01 with the recorded versions (numpy 2.4.6, numba 0.67.0,
  llvmlite 0.49.0) after the scratch purge had deleted files of the original.

## Files

`sweep_noautopar.py`, `run_noautopar.sh`, `noautopar.sbatch` (job 5229692; 5229835 re-ran
`quasi_affine_reduce_odd`, whose first step failed to launch), `parts/` (raw rows), `logs/`.
`../analyze_agents.py` reads `parts/` as the setting `noautopar/72`.

```bash
sbatch --job-name=al-noautopar --nodes=8 --time=03:00:00 --export=ALL,AL_KERNELS=<k1:k2:...>,AL_STEP_MIN=120 noautopar.sbatch
cd .. && python3 analyze_agents.py && TIME_STAT=median python3 analyze_agents.py
```

## Results

All 38 kernels, 453 candidate rows plus 38 translated-C rows, every one `ok`. Against the original T72
(medians): C 0.99x, the 103 Fortran libraries identical to their T1 build 1.00x, the 86 that gfortran had
autoparallelised 1.94x slower (up to 24x). C against Fortran, pooled (`../median/stats.csv`, setting
`noautopar/72`): best 1.37x [1.09, 1.73], first 1.34x [0.96, 1.86], median 1.68x [1.29, 2.19] (p = 0.001, the
only one of the 20 pooled tests below Bonferroni's 0.0025), best of first k 1.42x [1.10, 1.83]. Kimi-K2.7 and
Qwen3.8-27B carry the difference. Thesis: Section 6.5 and Figure 6.28's right panel.
