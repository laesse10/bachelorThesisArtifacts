#!/bin/bash -l
#SBATCH --job-name=thp-probe
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=00:20:00
#SBATCH --output=thp_probe.%j.out
set -euo pipefail
cd /capstor/scratch/cscs/lhulsbergen/llr40-matrix
echo "node=$(hostname)"
echo "THP: $(cat /sys/kernel/mm/transparent_hugepage/enabled) | defrag: $(cat /sys/kernel/mm/transparent_hugepage/defrag)"
export OMP_NUM_THREADS=1
srun --exclusive --cpu-bind=cores --hint=nomultithread \
  /capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python thp_probe.py
