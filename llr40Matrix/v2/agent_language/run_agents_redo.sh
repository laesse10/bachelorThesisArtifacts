#!/bin/bash
# One Part B unit: every agent submission of ONE kernel, on ONE exclusive node, in one whole-node
# srun step; sweep_agents.py binds each harness call with numactl: T1 to one core (cpu 0, memory on
# NUMA node 0, as v2's one-core steps) and T72 to one Grace socket (NUMA node 0, 72 cores).
# Runs are strictly sequential: never two timed runs on the node at once.
# usage: run_agents_redo.sh KERNEL NODE MINUTES DB_SHARD   (run from agent_language/)
# T72 REDO pass: re-times only the default/T72 cells the first runner refused (sweep_agents.py --redo-t72).
set -uo pipefail
K=$1; NODE=$2; MIN=$3; SHARD=$4
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT="${LLR40_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-agentlang}"   # 26a4f0cf, no patch
step() {
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpus-per-task=288 --cpu-bind=cores
             --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$K]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
echo "redo-t72 unit $K node=$NODE start=$(date -Is)"
PROBE='import os,re,sys;sys.path[:0]=[os.environ["LLR40_BENCH"],os.environ["LLR40_BENCH"]+"/hpcagent_bench/numpy_translators/src"];from hpcagent_bench import languages as L;f=L.baseline_flags("fortran");m=re.search(r"parallelize-loops=(\d+)",f);print("PROBE",os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)'
P1=$(step env LLR40_BENCH="$WT" numactl --physcpubind=0 --membind=0 "$PY" -c "$PROBE" | grep '^PROBE')
P72=$(step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" -c "$PROBE" | grep '^PROBE')
echo "T1 binding:  $P1"; echo "T72 binding: $P72"
[ "$(echo "$P1" | awk '{print $3, $4}')" = "1 1" ] && [ "$(echo "$P72" | awk '{print $3}')" = "72" ] \
  || { echo "GEOMETRY_FAILED [$K]: T1 '$P1' T72 '$P72'"; exit 3; }
step env LLR40_BENCH="$WT" "$PY" sweep_agents.py --kernel "$K" --out "parts/$K.csv" --scratch "scratch/$K" \
  --logs logs/cells --db-shard "$SHARD" --redo-t72 && touch "parts/$K.redo_t72.done"
echo "redo-t72 unit $K node=$NODE end=$(date -Is)"
