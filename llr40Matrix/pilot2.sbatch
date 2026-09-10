#!/bin/bash -l
#SBATCH --job-name=llr40-pilot2
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=01:30:00
#SBATCH --output=pilot2.%j.out
#SBATCH --error=pilot2.%j.err
set -euo pipefail
cd /capstor/scratch/cscs/lhulsbergen/llr40-matrix
echo "node=$(hostname) jobid=$SLURM_JOB_ID cores=$(nproc) $(date -Is)"
srun --exclusive --cpu-bind=cores --hint=nomultithread \
  /capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python sweep.py \
    --kernels tsvc_2_s3110 --preset M --warmup 5 --reps 30 \
    --out pilot2_results.csv --scratch /capstor/scratch/cscs/lhulsbergen/llr40-matrix/scratch
echo "DONE $(date -Is)"
