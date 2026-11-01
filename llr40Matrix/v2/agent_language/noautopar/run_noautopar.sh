#!/bin/bash
# One unit: every valid agent submission of ONE kernel at T72, without gfortran's automatic
# parallelization, on ONE exclusive node in one whole-node srun step (as ../run_agents.sh).
# usage: run_noautopar.sh KERNEL NODE MINUTES DB_SHARD        (run from agent_language/noautopar/)
set -uo pipefail
K=$1; NODE=$2; MIN=$3; SHARD=$4
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT="${LLR40_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-agentlang-noautopar}"  # 26a4f0cf + n=1 patch
step() {
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpus-per-task=288 --cpu-bind=cores
             --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$K]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
echo "unit $K node=$NODE start=$(date -Is)"
PROBE='import os,re,sys;sys.path[:0]=[os.environ["LLR40_BENCH"],os.environ["LLR40_BENCH"]+"/hpcagent_bench/numpy_translators/src"];from hpcagent_bench import languages as L;f=L.baseline_flags("fortran");m=re.search(r"parallelize-loops=(\d+)",f);print("PROBE",os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)'
P72=$(step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" -c "$PROBE" | grep '^PROBE')
echo "T72 binding: $P72"
[ "$(echo "$P72" | awk '{print $3, $4}')" = "72 1" ] || { echo "GEOMETRY_FAILED [$K]: T72 '$P72'"; exit 3; }
step env LLR40_BENCH="$WT" "$PY" sweep_noautopar.py --kernel "$K" --out "parts/$K.csv" --scratch "scratch/$K" \
  --logs logs/cells --db-shard "$SHARD" && touch "parts/$K.done"
echo "unit $K node=$NODE end=$(date -Is)"
