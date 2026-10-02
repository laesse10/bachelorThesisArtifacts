#!/bin/bash -l
#SBATCH --job-name=llr40v2-sweep
#SBATCH --account=g34          # override: sbatch -A <acct>
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=03:00:00
#SBATCH --array=1-40%20
#SBATCH --output=sweep_logs/sweep.%A_%a.out
#SBATCH --error=sweep_logs/sweep.%A_%a.err
#
# v2 of ../sweep_array.sbatch: same time limit, array shape and srun line. Differences: runs from
# v2/, points at the v2 benchmark checkout + Python 3.12 venv, writes per-kernel agent-attempt
# parts, and records the exact srun line (log + srun_lines.txt).
# One ARRAY TASK = one kernel = its own EXCLUSIVE node. Kernels never share a node.
set -euo pipefail
cd "${LLR40_ROOT:-/capstor/scratch/cscs/lhulsbergen/bachelorThesisArtifacts/llr40Matrix/v2}"
export LLR40_BENCH="${LLR40_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-v2}"
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
KERNEL=$(sed -n "${SLURM_ARRAY_TASK_ID}p" roster40.txt)
mkdir -p agent_attempts_parts results_parts
echo "task=$SLURM_ARRAY_TASK_ID kernel=$KERNEL node=$(hostname) cores=$(nproc) $(date -Is)"
CMD=(srun --exclusive --cpu-bind=cores --hint=nomultithread
  "$PY" sweep.py
    --kernels "$KERNEL" --preset M --warmup 5 --reps 30
    --out "results_parts/${KERNEL}.csv"
    --scratch "scratch/${SLURM_ARRAY_TASK_ID}"
    --attempts-out "agent_attempts_parts/${KERNEL}.csv"
    --attempt-logs agent_attempt_logs)
echo "SRUN: ${CMD[*]}"
echo "$(date -Is) job=${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID} node=$(hostname) ${CMD[*]}" >> srun_lines.txt
"${CMD[@]}"
echo "DONE $KERNEL $(date -Is)"
