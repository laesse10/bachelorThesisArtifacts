#!/bin/bash -l
#SBATCH --job-name=llr40v2-optrep
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=03:00:00
#SBATCH --output=sweep_logs/optreports.%j.out
#SBATCH --error=sweep_logs/optreports.%j.err
# Task 2: compile-only, on a compute node (-march=native), same binding as the sweep's build child.
set -euo pipefail
cd "${LLR40_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts/llr40Matrix/v2}"
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
CMD=(srun --exclusive --cpu-bind=cores --hint=nomultithread
  env LLR40_BENCH=/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2-aux
  numactl --cpunodebind=0 --membind=0 "$PY" opt_reports.py)
echo "SRUN: ${CMD[*]}"
echo "$(date -Is) job=${SLURM_JOB_ID} node=$(hostname) ${CMD[*]}" >> srun_lines.txt
"${CMD[@]}"
