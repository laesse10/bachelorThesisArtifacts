#!/bin/bash
# One DaCe follow-up unit on ONE named node: its cells in 3 rounds (as listed, reversed, as listed), each cell a
# separate dace_gap.py process in v2's step geometry (one task, one core, NUMA node 0, one OpenMP thread).
# usage: run_dace_gap.sh UNIT NODE MINUTES          (run from llr40Matrix/v2/dace/followup/)
set -uo pipefail
U=$1; NODE=$2; MIN=$3
LINE=$(grep -E "^$U +\|" units.txt) || { echo "no unit $U"; exit 2; }
IFS='|' read -r _ K CELLS <<< "$LINE"; K=$(echo $K)
PY="${DACE_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_dace5cf/bin/python}"
export DACE_BENCH="${DACE_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-dace}"
OUT=$PWD/out; mkdir -p "$OUT" logs
read -ra CS <<< "$CELLS"
echo "unit $U kernel=$K node=$NODE start=$(date -Is) bench=$(git -C "$DACE_BENCH" rev-parse --short HEAD)"
for R in 1 2 3; do
  if [ $((R % 2)) -eq 1 ]; then ORDER=("${CS[@]}"); else ORDER=(); for ((i=${#CS[@]}-1; i>=0; i--)); do ORDER+=("${CS[$i]}"); done; fi
  for C in "${ORDER[@]}"; do
    L=${C%%=*}; REST=${C#*=}; P=${REST%%:*}; X=()
    case "$REST" in *:cpp=*) X=(--override-cpp "${REST##*:cpp=}");; *:src=*) X=(--override-src "${REST##*:src=}");; esac
    CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpu-bind=cores --hint=nomultithread --time="$MIN"
         env OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores numactl --cpunodebind=0 --membind=0
         "$PY" dace_gap.py "$K" "$P" --label "$L" --round "$R" "${X[@]}" --out "$OUT")
    echo "$(date -Is) job=${SLURM_JOB_ID} node=$NODE ${CMD[*]}" >> srun_lines.txt
    "${CMD[@]}" > "logs/cell.$K.$L.$R.${SLURM_JOB_ID}.log" 2>&1
    grep -h '^TIME ' "logs/cell.$K.$L.$R.${SLURM_JOB_ID}.log" | cut -c1-400 || echo "TIME_FAILED $K $L $R"
  done
done
echo "unit $U end=$(date -Is)"
