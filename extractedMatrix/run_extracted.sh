#!/bin/bash
# One extracted-kernel unit on ONE named node (one timed kernel per node, never two at once), with
# LLR-40 v2's step geometry: srun --nodes=1 --ntasks=1 -w NODE --exclusive --cpu-bind=cores
# --hint=nomultithread (one task, one core); sweep_extracted.py binds the timed child to NUMA node 0.
# The compile-only opt-report step runs AFTER the timed step on the same node.
# usage: run_extracted.sh KERNEL NODE MINUTES DB_SHARD       (run from extractedMatrix/)
set -uo pipefail
K=$1; NODE=$2; MIN=$3; SHARD=$4
PY="${LLR40_PYTHON:-/capstor/scratch/cscs/lhulsbergen/venv_llr40v2/bin/python}"
WT="${LLR40_BENCH:-/capstor/scratch/cscs/lhulsbergen/HPCAgent-Bench-ext}"   # 26a4f0cf, no patch
mkdir -p parts logs/cells
step() {
  local CMD=(srun --nodes=1 --ntasks=1 -w "$NODE" --exclusive --cpu-bind=cores --hint=nomultithread --time="$MIN" "$@")
  echo "SRUN[$K]: ${CMD[*]}"
  echo "$(date -Is) job=${SLURM_JOB_ID} partition=${SLURM_JOB_PARTITION} node=$NODE ${CMD[*]}" >> srun_lines.txt
  "${CMD[@]}"
}
echo "unit $K node=$NODE start=$(date -Is) g++=$(command -v g++) gfortran=$(command -v gfortran)"
# geometry guard, as v2's chain: the build child must see 1 core and -ftree-parallelize-loops=1
P=$(step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" -c \
  "import os,re,sys;sys.path[:0]=[os.environ['LLR40_BENCH'],os.environ['LLR40_BENCH']+'/hpcagent_bench/numpy_translators/src'];from hpcagent_bench import languages as L;f=L.baseline_flags('fortran');m=re.search(r'parallelize-loops=(\d+)',f);print('PROBE',os.uname().nodename,len(os.sched_getaffinity(0)),m.group(1) if m else 0)" | grep '^PROBE')
echo "$P"
[ "$(echo "$P" | awk '{print $3, $4}')" = "1 1" ] || { echo "GEOMETRY_FAILED [$K]: $P"; exit 3; }
step env LLR40_BENCH="$WT" "$PY" sweep_extracted.py --kernels "$K" --preset M --warmup 5 --reps 30 \
  --out "parts/$K.csv" --scratch "scratch/$K" --logs logs/cells --emitted-dir emitted_sources --db-shard "$SHARD"
# opt reports + disassembly (compiled cells) and numba inspect_asm, from the sources that were timed
E=emitted_sources/$K
CELLS=("$K:c:c:$E/${K}_fp64.c" "$K:cpp:cpp:$E/${K}_fp64.cpp" "$K:fortran:fortran:$E/${K}_fp64.f90")
NB=$(grep -l "hpcagent_bench-autogen" "$E/${K}_numba_np.py" >/dev/null && echo numba || echo numba_hand)
CELLS+=("$K:$NB:python:$E/${K}_numba_np.py")
case "$K" in
  comet_int4_gemm)  CELLS+=("$K:native:cpp:adapters/comet_int4_gemm_native.cpp") ;;
  warpx_boris_push) CELLS+=("$K:native:cpp:adapters/warpx_boris_push_native.cpp") ;;
esac
step env LLR40_BENCH="$WT" numactl --cpunodebind=0 --membind=0 "$PY" opt_reports_extracted.py \
  --out-dir opt_reports --index "parts/optrep.$K.csv" "${CELLS[@]}"
echo "unit $K node=$NODE end=$(date -Is)"
