#!/bin/bash -l
#SBATCH --job-name=llr40v2-optrep-numba
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=00:30:00
#SBATCH --output=sweep_logs/optrep_numba.%j.out
#SBATCH --error=sweep_logs/optrep_numba.%j.err
# Task 2, numba rows only (the second OPTREP run hit a stale numba cache). Compile-only, compute
# node, same step geometry as the chain (1 task, 1 cpu).
set -euo pipefail
cd "${LLR40_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts/llr40Matrix/v2}"
PY=/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python
NODE=$(hostname)
CMD=(srun --nodes=1 --ntasks=1 --exclusive --cpu-bind=cores --hint=nomultithread
     env LLR40_BENCH=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-aux
     numactl --cpunodebind=0 --membind=0 "$PY" opt_reports.py --only numba)
echo "SRUN: ${CMD[*]}"
echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
"${CMD[@]}"
