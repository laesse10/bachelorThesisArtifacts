#!/bin/bash -l
#SBATCH --job-name=numabind
#SBATCH --account=g34
#SBATCH --partition=debug
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --time=00:25:00
#SBATCH --output=numabind.%j.out
set -euo pipefail
cd /capstor/scratch/cscs/lhulsbergen/llr40-matrix/bench
export PYTHONPATH=$PWD:$PWD/hpcagent_bench/numpy_translators/src
export OMP_NUM_THREADS=1 OMP_PLACES=cores OMP_PROC_BIND=close NUMBA_NUM_THREADS=1
export HPCAGENT_BENCH_MEASUREMENT_WARMUP=5
P=/capstor/scratch/cscs/lhulsbergen/gaivenv/bin/python
echo "node=$(hostname)"
for mode in unpinned pinned; do
  if [ "$mode" = pinned ]; then PRE="numactl --cpunodebind=0 --membind=0"; else PRE=""; fi
  for rep in 1 2; do
    srun --exclusive --cpu-bind=cores --hint=nomultithread $PRE \
      $P -m hpcagent_bench.cli run --benchmark tsvc_2_s3110 --framework cpp \
      --precision fp64 --preset M --mode single_core --repeat 30 --no-validate \
      --output /capstor/scratch/cscs/lhulsbergen/llr40-matrix/numabind_${mode}_${rep}.jsonl >/dev/null 2>&1
    echo "$mode rep$rep done"
  done
done
