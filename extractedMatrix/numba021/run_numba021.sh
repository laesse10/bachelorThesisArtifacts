#!/bin/bash
# One numba021 unit on ONE named node: run_extracted.sh with the 26a4f0cf tree HPCAgent-Bench-ext3, whose
# kernel .cache holds the v0.2.1-emitted Numba module (tools/seed_cache.py), and only the columns REPRS.
# Same step geometry, geometry probe, preset M, 5 warm-up + 30 timed, sweep_extracted.py and
# opt_reports_extracted.py, unchanged. Output goes to numba021/, never to the matrix's parts/ or logs/.
# usage: run_numba021.sh KERNEL NODE MINUTES DB_SHARD REPRS        (run from extractedMatrix/)
set -uo pipefail
K=$1; NODE=$2; MIN=$3; SHARD=$4; REPRS=$5
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT="${LLR40_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-ext3}"   # 26a4f0cf + seeded numba .cache
O=numba021
mkdir -p $O/parts $O/logs/cells
step() {
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpu-bind=cores --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$K]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> $O/srun_lines.txt
  "${CMD[@]}"
}
echo "unit $K node=$NODE reprs=$REPRS start=$(date -Is) g++=$(command -v g++) gfortran=$(command -v gfortran)"
P=$(step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" -c \
  "import os,re,sys;sys.path[:0]=[os.environ['LLR40_BENCH'],os.environ['LLR40_BENCH']+'/hpcagent_bench/numpy_translators/src'];from hpcagent_bench import languages as L;f=L.baseline_flags('fortran');m=re.search(r'parallelize-loops=(\d+)',f);print('PROBE',os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)" | grep '^PROBE')
echo "$P"
[ "$(echo "$P" | awk '{print $3, $4}')" = "1 1" ] || { echo "GEOMETRY_FAILED [$K]: $P"; exit 3; }
step env LLR40_BENCH="$WT" "$PY" sweep_extracted.py --kernels "$K" --reprs "$REPRS" --preset M --warmup 5 --reps 30 \
  --out "$O/parts/$K.csv" --scratch "scratch/numba021.$K" --logs $O/logs/cells --emitted-dir $O/emitted_sources \
  --db-shard "$SHARD"
E=$O/emitted_sources/$K
CELLS=("$K:numba:python:$E/${K}_numba_np.py")
[[ ",$REPRS," == *",c,"* ]] && CELLS+=("$K:c:c:$E/${K}_fp64.c")
step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" opt_reports_extracted.py \
  --out-dir $O/opt_reports --index "$O/parts/optrep.$K.csv" "${CELLS[@]}"
echo "unit $K node=$NODE end=$(date -Is)"
