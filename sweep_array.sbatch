#!/bin/bash -l
#SBATCH --job-name=llr40-sweep
#SBATCH --account=g34          # override: sbatch -A <acct>
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=03:00:00
#SBATCH --array=1-40%20
#SBATCH --output=sweep_logs/sweep.%A_%a.out
#SBATCH --error=sweep_logs/sweep.%A_%a.err
#
# One ARRAY TASK = one kernel = its own EXCLUSIVE node. Kernels never share a node, so every
# timed run still owns the machine -- the protocol's exclusivity holds exactly as in the pilot.
# The alternative (many kernels concurrently on one node) would contend for memory bandwidth
# and is not usable for timing.
set -euo pipefail
cd "${LLR40_ROOT:-/capstor/scratch/cscs/lhulsbergen/llr40-matrix}"
KERNEL=$(sed -n "${SLURM_ARRAY_TASK_ID}p" roster40.txt)
echo "task=$SLURM_ARRAY_TASK_ID kernel=$KERNEL node=$(hostname) cores=$(nproc) $(date -Is)"
srun --exclusive --cpu-bind=cores --hint=nomultithread \
  "${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python}" sweep.py \
    --kernels "$KERNEL" --preset M --warmup 5 --reps 30 \
    --out "results_parts/${KERNEL}.csv" \
    --scratch "scratch/${SLURM_ARRAY_TASK_ID}"
echo "DONE $KERNEL $(date -Is)"
