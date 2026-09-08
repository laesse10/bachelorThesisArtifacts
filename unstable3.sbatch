#!/bin/bash -l
#SBATCH --job-name=llr40-unstable3
#SBATCH --account=g34
#SBATCH --partition=normal
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=00:40:00
#SBATCH --output=unstable3.%j.out
#
# The three kernels whose NumPy reference OVERFLOWS to inf/nan at preset M
# (s115 overflows by n=128; preset M is n=20481), making their correctness verdicts
# meaningless there. Preset S (n=64) is the largest defined preset at which the
# reference stays finite, so these are recorded as clearly-marked EXTRA rows carrying
# preset=S, alongside -- never replacing -- their preset-M rows.
set -euo pipefail
cd /capstor/scratch/cscs/lhulsbergen/llr40-matrix
printf '%s\n' tsvc_2_s115 wf_triangular wf_diff_skew > roster_unstable3.txt
echo "node=$(hostname) $(date -Is)"
srun --exclusive --cpu-bind=cores --hint=nomultithread \
  /capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python sweep.py \
    --kernels roster_unstable3.txt --preset S --warmup 5 --reps 30 \
    --out results_unstable3_presetS.csv \
    --scratch /capstor/scratch/cscs/lhulsbergen/llr40-matrix/scratch/unstable3
echo "DONE $(date -Is)"
