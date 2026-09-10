#!/bin/bash -l
#SBATCH --job-name=diag-bimodal
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=00:28:00
#SBATCH --output=diag.%j.out
set -euo pipefail
cd /capstor/scratch/cscs/lhulsbergen/llr40-matrix
export OMP_NUM_THREADS=1 OMP_PLACES=cores OMP_PROC_BIND=close
export PYTHONPATH=$PWD/bench:$PWD/bench/hpcagent_bench/numpy_translators/src
P=/capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python
echo "node=$(hostname)"
echo "THP=$(cat /sys/kernel/mm/transparent_hugepage/enabled) defrag=$(cat /sys/kernel/mm/transparent_hugepage/defrag)"
# Three consecutive trials: the artifact is intermittent, so raise the chance of catching one.
for trial in 1 2 3; do
  echo "===== TRIAL $trial ====="
  srun --exclusive --cpu-bind=cores --hint=nomultithread \
    perf stat -I 250 -x, -e cycles,instructions,dTLB-loads,dTLB-load-misses,L1-dcache-load-misses \
      -o perf_trial${trial}.csv \
    $P diagnose_bimodal.py
done
echo ALLDONE
