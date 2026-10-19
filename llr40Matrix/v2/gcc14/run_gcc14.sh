#!/bin/bash
# Run units of units.txt one after the other on ONE named node, with the LLR-40 step geometry:
# srun --nodes=1 --ntasks=1 -w NODE --exclusive --cpu-bind=cores --hint=nomultithread; sweep_gcc14.py binds every
# timed child to NUMA node 0.   usage: run_clang.sh NODE MINUTES DB_SHARD UNIT [UNIT ...]   (from llr40Matrix/v2/clang/)
set -uo pipefail
NODE=$1; MIN=$2; SHARD=$3; shift 3
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
BENCH="${LLR40_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-clang}"
mkdir -p parts logs
step() {
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpu-bind=cores --hint=nomultithread --time="$MIN" "$@")
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
P=$(step env LLR40_BENCH="$BENCH" numactl --cpunodebind=0 --membind=0 "$PY" -c \
  "import os,re,sys;sys.path[:0]=[os.environ['LLR40_BENCH'],os.environ['LLR40_BENCH']+'/hpcagent_bench/numpy_translators/src'];from hpcagent_bench import languages as L;f=L.baseline_flags('fortran');m=re.search(r'parallelize-loops=(\d+)',f);print('PROBE',os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)" | grep '^PROBE')
echo "$P"
[ "$(echo "$P" | awk '{print $3, $4}')" = "1 1" ] || { echo "GEOMETRY_FAILED: $P"; exit 3; }
for U in "$@"; do
  LINE=$(grep -E "^$U +\|" units.txt) || { echo "no unit $U"; continue; }
  IFS='|' read -r _ K CELLS <<< "$LINE"; K=$(echo $K)
  echo "unit $U kernel=$K node=$NODE start=$(date -Is)"
  step env LLR40_BENCH="$BENCH" "$PY" sweep_gcc14.py --kernel "$K" --cells $CELLS --rounds 3 --perf \
    --out "parts/$U.csv" --scratch "scratch/$U" --db-shard "$SHARD"
  echo "unit $U end=$(date -Is)"
done
